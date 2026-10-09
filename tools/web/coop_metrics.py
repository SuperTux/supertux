"""Co-op benchmark summaries. All durations are milliseconds, bytes are UTF-8."""
import math


def distribution(values):
    values = sorted(values)
    if any(not math.isfinite(v) or v < 0 for v in values):
        raise ValueError('Measurements must be finite and nonnegative')
    if not values:
        return {'count': 0, 'median': None, 'p95': None, 'max': None}
    middle = len(values) // 2
    median = values[middle] if len(values) % 2 else (values[middle - 1] + values[middle]) / 2
    return dict(count=len(values), median=round(median, 2),
                p95=round(values[math.ceil(.95 * len(values)) - 1], 2), max=round(values[-1], 2))


def summarize(trials):
    """Keep startup, response and windowed traffic distinct; never invent zeros."""
    result = {}
    for key in ('input_to_first_draw_ms', 'input_to_first_frame_ms', 'arrival_interval_ms',
                'draw_interval_ms', 'receive_to_draw_ms', 'draw_callback_ms', 'host_raf_interval_ms',
                'host_native_input_update_interval_ms'):
        result[key] = distribution([v for trial in trials for v in trial['observations'][key]])
    for key in ('host_cold_ready_ms', 'host_warm_ready_ms', 'guest_cold_ready_ms',
                'guest_warm_ready_ms', 'selection_to_ready_ms'):
        result[key] = distribution([trial['loading'][key] for trial in trials])
    elapsed = sum(trial['observations']['window_ms'] for trial in trials) / 1000
    if not elapsed > 0:
        raise ValueError('Traffic needs a positive measured window')
    frames = sum(trial['observations']['view_count'] for trial in trials)
    result['presentation_updates_per_second'] = round(frames / elapsed, 2)
    draws = sum(trial['observations'].get('presentation_draws', 0) for trial in trials)
    result['presentation_draws_per_second'] = round(draws / elapsed, 2)
    result['interpolated_draw_fraction'] = round(sum(trial['observations'].get('interpolated_draws', 0) for trial in trials) / draws, 3) if draws else None
    result['view_json_bytes_per_second'] = round(sum(trial['observations']['view_bytes'] for trial in trials) / elapsed)
    result['largest_view_json_bytes'] = max(trial['observations']['largest_view_bytes'] for trial in trials)
    result['max_host_buffered_bytes'] = max(trial['observations']['max_host_buffered_bytes'] for trial in trials)
    result['max_guest_buffered_bytes'] = max(trial['observations']['max_guest_buffered_bytes'] for trial in trials)
    result['input_samples'] = result['input_to_first_draw_ms']['count']
    return result
