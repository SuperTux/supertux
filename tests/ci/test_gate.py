import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('gate', Path(__file__).parents[2] / 'tools/ci/check_gate.py')
gate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gate)


def fixture(web=False, lint=False):
    selection = {k: 'false' for k in ('tests','lint','web','debug','linux','windows','macos','android','freebsd')}
    selection.update(web=str(web).lower(), tests=str(web).lower(), lint=str(lint).lower())
    jobs = {j:{'result':'skipped'} for j in ('focused_tests','lint_cppcheck','wasm_release','wasm_debug','native_linux','native_windows','native_macos','native_android','native_freebsd')}
    jobs['changes']={'result':'success','outputs':selection}
    if web:
        jobs['wasm_release']['result']='success'
        jobs['focused_tests' if not lint else 'lint_cppcheck']['result']='success'
    return jobs


class GateTests(unittest.TestCase):
    def test_intentional_skips_and_combined_lint_tests(self):
        for jobs in (fixture(),fixture(True),fixture(True,True)):
            gate.validate(jobs)

    def test_needed_failure_cancellation_and_missing_execution_fail(self):
        for result in ('failure','cancelled','skipped'):
            jobs=fixture(True)
            jobs['wasm_release']['result']=result
            with self.assertRaises(ValueError): gate.validate(jobs)

    def test_selector_failure_and_unexpected_job_failures_fail(self):
        jobs=fixture()
        jobs['changes']['result']='cancelled'
        with self.assertRaises(ValueError): gate.validate(jobs)
        for result in ('failure','cancelled'):
            jobs=fixture()
            jobs['wasm_debug']['result']=result
            with self.assertRaises(ValueError): gate.validate(jobs)
