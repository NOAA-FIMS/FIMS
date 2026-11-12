"""Usage: run-full-fit.py BEFORE_LIBRARY AFTER_LIBRARY DATA_RDS NEW_OUTPUT_DIRECTORY."""
import csv
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time

before, after, data, output = sys.argv[1:]
root = Path(output).resolve()
root.mkdir(parents=True, exist_ok=False)
shutil.copyfile(data, root / 'canonical-data.rds')
script = Path(__file__).with_name('full-fit.R').resolve()
rows = []
for index, variant in enumerate(('before', 'after', 'after', 'before'), 1):
    label = f'{index:02d}-{variant}'
    folder = root / label
    folder.mkdir()
    env = dict(os.environ, R_LIBS=str(Path(before if variant == 'before' else after).resolve()),
               OMP_NUM_THREADS='1', VECLIB_MAXIMUM_THREADS='1', OPENBLAS_NUM_THREADS='1')
    start = time.monotonic()
    with (folder / 'run.log').open('w') as log:
        child = subprocess.Popen(['Rscript', str(script), str(root), '30', 'fixed', str(folder)],
                                 env=env, stdout=log, stderr=subprocess.STDOUT)
        _, status, usage = os.wait4(child.pid, 0)
        child.returncode = os.waitstatus_to_exitcode(status)
    rows.append(dict(label=label, variant=variant, wall_seconds=time.monotonic()-start,
                     peak_rss_mib=usage.ru_maxrss/(1024**2 if sys.platform == 'darwin' else 1024),
                     exit_code=child.returncode))
    with (root / 'runs.csv').open('w') as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    print(rows[-1], flush=True)
    if child.returncode:
        raise SystemExit(child.returncode)
