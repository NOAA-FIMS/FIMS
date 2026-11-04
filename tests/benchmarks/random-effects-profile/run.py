"""Usage: run.py R_LIBRARY NEW_OUTPUT_DIRECTORY (macOS/Linux)."""
import json
import os
from pathlib import Path
import subprocess
import sys
import time

library, output = sys.argv[1:]
root = Path(output).resolve()
root.mkdir(parents=True, exist_ok=False)
env = dict(os.environ, R_LIBS=str(Path(library).resolve()),
           OMP_NUM_THREADS='1', VECLIB_MAXIMUM_THREADS='1', OPENBLAS_NUM_THREADS='1')
start = time.monotonic()
with (root / 'run.log').open('w') as log:
    child = subprocess.Popen(
        ['Rscript', str(Path(__file__).with_name('profile.R').resolve()), str(root)],
        env=env, stdout=log, stderr=subprocess.STDOUT)
    _, status, usage = os.wait4(child.pid, 0)
    child.returncode = os.waitstatus_to_exitcode(status)
result = dict(wall_seconds=time.monotonic() - start,
              peak_rss_mib=usage.ru_maxrss / (1024**2 if sys.platform == 'darwin' else 1024),
              exit_code=child.returncode)
(root / 'process.json').write_text(json.dumps(result, indent=2) + '\n')
print(result)
if child.returncode:
    raise SystemExit(child.returncode)
