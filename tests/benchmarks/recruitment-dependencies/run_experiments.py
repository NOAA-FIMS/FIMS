"""Run every diagnostic in a fresh R process, sequentially for timing fairness.
Usage: python3 run_experiments.py DIAGNOSTIC_LIBRARY OUTPUT
"""
import itertools
import os
from pathlib import Path
import shutil
import subprocess
import sys

library, output = map(lambda p: Path(p).resolve(), sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
source_scripts = Path(__file__).resolve().parent
# Freeze R code for the whole run; editing working-tree diagnostics must not
# change an in-flight experiment or race with R reading a script.
scripts = output / 'scripts'
scripts.mkdir(exist_ok=False)
for source in source_scripts.glob('*.R'):
    shutil.copy2(source, scripts / source.name)
env = os.environ.copy()
env.update(R_LIBS=str(library), OMP_NUM_THREADS='1',
           OPENBLAS_NUM_THREADS='1', VECLIB_MAXIMUM_THREADS='1')

def run(script, args, name):
    print('START', name, flush=True)
    with open(output / f'{name}.log', 'w') as log:
        subprocess.run(['Rscript', str(scripts / script), *map(str, args),
                        str(output / name)], env=env, check=True,
                       stdout=log, stderr=subprocess.STDOUT)
    print('DONE', name, flush=True)

for years in [30, 60, 120, 240]:
    for centered, no_plus, no_feedback in itertools.product([0, 1], repeat=3):
        name = f'ablation-{years}-c{centered}-p{1-no_plus}-f{1-no_feedback}'
        run('ablation.R', [years, centered, no_plus, no_feedback], name)
    run('lifted.R', [years], f'lifted-{years}')
