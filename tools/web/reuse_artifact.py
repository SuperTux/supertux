#!/usr/bin/env python3
"""Explicit successful same-repository/source run only, never an unverified latest."""
import argparse
import json
import subprocess
from pathlib import Path
from verify_artifact import verify

parser = argparse.ArgumentParser()
parser.add_argument('--run-id', type=int, required=True)
parser.add_argument('--repository', required=True)
parser.add_argument('--source-commit', required=True)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--deployment', action='store_true')
args = parser.parse_args()
run = json.loads(subprocess.check_output(['gh','api',f'repos/{args.repository}/actions/runs/{args.run_id}']))
assert run['status'] == 'completed' and run['conclusion'] == 'success', 'Validation run did not succeed'
assert run['path'] in ('.github/workflows/validation.yml', '.github/workflows/wasm.yml'), 'Wrong validation workflow'
assert run['head_sha'] == args.source_commit, 'Run source SHA mismatch'
assert run['head_repository']['full_name'] == args.repository, 'Foreign source repository'
if args.deployment:
    assert run['event'] != 'pull_request', 'PR artifacts cannot enter privileged deployment'
subprocess.run(['gh','run','download',str(args.run_id),'--repo',args.repository,'--name','wasm32-emscripten-Release-html','--dir',str(args.output)], check=True)
verify(args.output, args.source_commit)
print('Reusing exact verified Release artifact from run', args.run_id)
