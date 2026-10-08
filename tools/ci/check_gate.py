#!/usr/bin/env python3
"""Stable aggregate check: only intentionally unnecessary jobs may skip."""
import json
import os


def validate(jobs):
    if jobs['changes']['result'] != 'success':
        raise ValueError('Change selection did not succeed')
    selection = jobs['changes']['outputs']
    expected = {'focused_tests': 'tests', 'lint_cppcheck': 'lint', 'wasm_release': 'web', 'wasm_debug': 'debug'}
    expected.update({'native_' + p: p for p in ('linux', 'windows', 'macos', 'android', 'freebsd')})
    for job, key in expected.items():
        result = jobs[job]['result']
        needed = selection[key] == 'true'
        if job == 'focused_tests':
            needed = needed and selection['lint'] != 'true'
        acceptable = ('success',) if needed else ('success', 'skipped')
        if result not in acceptable:
            raise ValueError(f'{job}: needed={needed}, result={result}')


if __name__ == '__main__':
    validate(json.loads(os.environ['RESULTS']))
    print('All needed checks passed; unnecessary work intentionally skipped.')
