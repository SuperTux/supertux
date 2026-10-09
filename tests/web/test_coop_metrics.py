import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).parents[2] / 'tools/web'))
from coop_metrics import distribution, summarize


class MetricsTests(unittest.TestCase):
    def trial(self, window=1000, frames=10, byte_count=100):
        arrays = ('input_to_first_draw_ms', 'input_to_first_frame_ms', 'arrival_interval_ms',
                  'draw_interval_ms', 'receive_to_draw_ms', 'draw_callback_ms', 'host_raf_interval_ms',
                  'host_native_input_update_interval_ms')
        return dict(loading=dict.fromkeys(('host_cold_ready_ms', 'host_warm_ready_ms',
                    'guest_cold_ready_ms', 'guest_warm_ready_ms', 'selection_to_ready_ms'), 20),
                    observations={**dict.fromkeys(arrays, [10, 30]), 'window_ms': window,
                    'view_count': frames, 'view_bytes': byte_count, 'largest_view_bytes': 50,
                    'max_host_buffered_bytes': 0, 'max_guest_buffered_bytes': 0})

    def test_nearest_rank_p95_and_even_median(self):
        self.assertEqual(distribution(range(1, 21)), dict(count=20, median=10.5, p95=19, max=20))
        self.assertIsNone(distribution([])['median'])
        for invalid in ([float('nan')], [float('inf')], [-1]):
            with self.assertRaises(ValueError): distribution(invalid)

    def test_rates_use_total_measured_time_not_average_trial_rates(self):
        trials = [self.trial(), self.trial(window=9000, frames=30, byte_count=900)]
        before = copy.deepcopy(trials)
        result = summarize(trials)
        self.assertEqual(result['presentation_updates_per_second'], 4)
        self.assertEqual(result['view_json_bytes_per_second'], 100)
        self.assertEqual(result['input_samples'], 4)
        self.assertEqual(result['host_cold_ready_ms']['count'], 2)
        self.assertEqual(trials, before)
        with self.assertRaises(ValueError): summarize([self.trial(window=0)])
