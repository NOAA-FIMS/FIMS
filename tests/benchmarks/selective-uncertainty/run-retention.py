"""Usage: run-retention.py LIBRARY NEW_OUTPUT_DIRECTORY."""
import json
import os
from pathlib import Path
import subprocess
import sys
import time

library, output = sys.argv[1:]
root = Path(output).resolve()
root.mkdir(parents=True, exist_ok=False)
script = Path(__file__).with_name('retention.R').resolve()
env = dict(os.environ, R_LIBS=str(Path(library).resolve()),
           OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1', VECLIB_MAXIMUM_THREADS='1')
records = []
for run, mode in enumerate(('full', 'compact', 'compact', 'full'), 1):
    label = f'{run}-{mode}'
    start = time.monotonic()
    with (root / f'{label}.log').open('w') as log:
        process = subprocess.Popen(['Rscript', str(script), mode, '20',
                                    str(root / f'{label}.rds')], env=env,
                                   stdout=log, stderr=subprocess.STDOUT)
        _, status, usage = os.wait4(process.pid, 0)
        process.returncode = os.waitstatus_to_exitcode(status)
    records.append(dict(label=label, mode=mode, fits=20,
                        wall_seconds=time.monotonic()-start,
                        peak_rss_mib=usage.ru_maxrss/(1024**2 if sys.platform == 'darwin' else 1024),
                        exit_code=process.returncode))
    (root / 'runs.json').write_text(json.dumps(records, indent=2))
    print(records[-1], flush=True)
    if process.returncode:
        raise RuntimeError(f'Failed: {label}')
