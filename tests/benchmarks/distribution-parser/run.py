"""Usage: run.py BEFORE_LIBRARY AFTER_LIBRARY NEW_OUTPUT_DIRECTORY."""
import json
import os
from pathlib import Path
import subprocess
import sys
import time
before, after, output = sys.argv[1:]
root = Path(output).resolve()
root.mkdir(parents=True, exist_ok=False)
script = Path(__file__).with_name('profile.R').resolve()
records = []
for years in (30, 120):
    for run, variant in enumerate(('before', 'after', 'after', 'before'), 1):
        label=f'{years}-{run}-{variant}'
        env=dict(os.environ,R_LIBS=str(Path(before if variant=='before' else after).resolve()),
                 OMP_NUM_THREADS='1',VECLIB_MAXIMUM_THREADS='1',OPENBLAS_NUM_THREADS='1')
        start=time.monotonic()
        with (root/f'{label}.log').open('w') as log:
            child=subprocess.Popen(['Rscript',str(script),str(years),str(root/label),'false'],
                                   env=env,stdout=log,stderr=subprocess.STDOUT)
            _,status,usage=os.wait4(child.pid,0)
            child.returncode=os.waitstatus_to_exitcode(status)
        records.append(dict(label=label,years=years,variant=variant,
                            wall_seconds=time.monotonic()-start,
                            peak_rss_mib=usage.ru_maxrss/(1024**2 if sys.platform=='darwin' else 1024),
                            exit_code=child.returncode))
        (root/'runs.json').write_text(json.dumps(records,indent=2))
        print(records[-1],flush=True)
        if child.returncode: raise RuntimeError(f'Failed: {label}')
