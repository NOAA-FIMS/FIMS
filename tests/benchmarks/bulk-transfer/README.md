# Bulk parameter transfer during initialization

`set_param_vector()` previously crossed the Rcpp interface repeatedly for every
parameter element, obtaining wrappers and assigning each value and estimation
type separately. Profiling after the distribution-lookup optimization showed
this path accounted for approximately 64% of initialization CPU samples at
120 years.

The existing `VariableVector` interface already provides `set_values()` and
`set_estimation_types()`. The initializer now resolves the target vector once,
resizes it once, and passes the values and per-element estimation types through
those methods. No new C++ interface or TMB changes are needed. The bulk methods
update existing Variables, preserving identities for surviving elements. Vectors
with missing estimation types retain scalar type assignment to preserve the
existing initialization warning path; their values still transfer in bulk.

## Results

Baseline is commit `16782ebf`, including the earlier distribution-lookup fix.
Fresh processes alternate before/after/after/before for each size, using the
existing [initialization benchmark](../initialization/README.md).

| Years | Module initialization before | After | Additional speedup |
| ---: | ---: | ---: | ---: |
| 30 | 0.5955 s | 0.2720 s | 2.19x |
| 120 | 2.0345 s | 0.7285 s | 2.79x |

These are medians of two processes per variant. Objective values, complete
gradients and ordinary reports agree within `1e-10` across all eight processes.
Frame construction and default parameter setup are essentially unchanged.
The 120-year workload repeats `data_big` observations for scaling and is not a
scientific assessment.

Separate allocation profiles show module initialization allocating 10.15 to
9.51 MiB at 30 years and 32.93 to 30.50 MiB at 120 years. Allocation counts fall
from 6,319 to 3,954 and from 13,749 to 4,634, respectively. These are cumulative
R allocations, not peak memory, and exclude native C++ allocations.
Whole-process peak RSS is mixed (median 351.8 to 339.8 MiB at 30 years; 415.8 to
428.1 MiB at 120 years). The demonstrated benefit is initialization speed and
reduced interface/allocation churn, not a universal peak-memory saving.

The targeted transfer regression checks mixed estimation types, input row
order, reuse of existing Variable and vector IDs, shrinking, missing estimation
types, and missing-field errors. The targeted initialization, VariableVector,
parameter setup, distribution parsing, evaluation reuse, report layout, and
selective uncertainty tests pass all 446 assertions with no failures or warnings.
This is a focused regression run, not a rerun of the full testthat suite.

## Reproduce

```sh
python3 tests/benchmarks/initialization/run.py \
  /path/to/before-library /path/to/after-library /tmp/new-transfer-run
Rscript tests/benchmarks/initialization/summarize.R \
  /tmp/new-transfer-run tests/benchmarks/bulk-transfer/results
R_LIBS=/path/to/after-library Rscript tests/benchmarks/initialization/profile.R \
  120 /tmp/transfer-profile true
```

Raw results and isolated builds are in `/private/tmp/fims-bulk-transfer`.
See [timings](results/summary.csv), [runs](results/runs.csv),
[numerical validation](results/validation.csv), and
[allocation/CPU profiles](results/profiles).
