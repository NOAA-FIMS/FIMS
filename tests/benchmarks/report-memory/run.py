"""Usage: run.py BEFORE_LIBRARY AFTER_LIBRARY OUTPUT_DIRECTORY."""
import json
import os
from pathlib import Path
import subprocess
import sys
import time

before, after, output = sys.argv[1:]
root = Path(output).resolve()
root.mkdir(parents=True, exist_ok=True)
script = Path(__file__).with_name('measure.R').resolve()
records = json.loads((root / 'runs.json').read_text()) if (root / 'runs.json').exists() else []
completed = {record['label'] for record in records if record['exit_code'] == 0}
for years in (30, 300, 1200):
    for phase in (('recording', 'report', 'uncertainty') if years == 30 else ('recording', 'report')):
        for run, variant in enumerate(('before', 'after', 'after', 'before'), 1):
            label = f'{years}-{phase}-{run}-{variant}'
            if label in completed:
                continue
            env = dict(os.environ, R_LIBS=str(Path(before if variant == 'before' else after).resolve()),
                       OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1', VECLIB_MAXIMUM_THREADS='1')
            start = time.monotonic()
            with (root / f'{label}.log').open('w') as log:
                child = subprocess.Popen(['Rscript', str(script), str(years), phase,
                                          str(root / f'{label}.rds')], env=env,
                                         stdout=log, stderr=subprocess.STDOUT)
                _, status, usage = os.wait4(child.pid, 0)
                child.returncode = os.waitstatus_to_exitcode(status)
            # macOS wait4 reports ru_maxrss in bytes; Linux reports KiB.
            rss_mib = usage.ru_maxrss / (1024**2 if sys.platform == 'darwin' else 1024)
            records.append(dict(label=label, years=years, phase=phase, variant=variant,
                                wall_seconds=time.monotonic()-start, peak_rss_mib=rss_mib,
                                exit_code=child.returncode))
            (root / 'runs.json').write_text(json.dumps(records, indent=2))
            print(records[-1], flush=True)
            if child.returncode:
                raise RuntimeError(f'Failed: {label}; inspect its log')
