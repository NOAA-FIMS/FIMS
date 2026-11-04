# Bulk process transfer and shared fleet layouts

`initialize_process_distribution()` now transfers logged scales and initial zero
vectors through the existing bulk setters. Both normal and lognormal families
use the same transfer path; scalar estimation-type broadcasting and missing
types mapped to constant are preserved.

`initialize_fims()` resolves fleet length-bin layouts once per conversion mode
and shares them with fleet and length-composition setup. Previously each of
those initializers resolved layouts for every fleet again. Fixed conversion
retains the global fallback, while the growth-derived path uses fleet-specific
bins. Direct calls to the internal initializers can still resolve their own
layouts. No layouts persist beyond model initialization.

## Results

Baseline: `56a2df52`, including repeated-specification parsing and merged data
updates. The two isolated installations use identical data. Timings use fresh
processes in before/after/after/before order at each size, with tests finished
before measurement.

| Years | Initialization before | After | Reduction |
| ---: | ---: | ---: | ---: |
| 30 | 0.1515 s | 0.1170 s | 23% |
| 120 | 0.2430 s | 0.1480 s | 39% |

These are medians of two runs per variant for module initialization. The complete
objective, gradient, and ordinary reports match within `1e-10` across all eight
runs. The 120-year workload repeats `data_big` observations for scaling, rather
than representing a scientific assessment.

Separate R allocation profiles show initialization allocating 9.75 to 7.66 MiB
at 30 years and 31.37 to 24.36 MiB at 120 years, approximately 22% less. Counts
fall from 4,010 to 3,443 and 4,694 to 3,767. These are cumulative R allocations,
not peak memory, and exclude native allocations. Whole-process peak RSS is
mixed: median 349.2 to 339.7 MiB and 428.5 to 436.8 MiB, respectively.

Validation passed 486 assertions in the initialization/distribution regression
group and 51 in the slow growth/fleet-bin integration group, with no failures,
errors, or warnings. One existing Newton-polishing test skips because Newton
steps are not implemented in `fit_fims()`. New tests cover both process families,
both recruitment parameterizations, vector scales, mixed and missing estimation
types, scalar type broadcasting, and initialized zeros. Existing growth tests
exercise fleet-specific bin geometry, fallback behavior, convergence, and
reports. This was not a full testthat suite run.

## Reproduce

Use the shared [initialization benchmark](../initialization/README.md):

```sh
python3 tests/benchmarks/initialization/run.py \
  /path/to/before-library /path/to/after-library /tmp/new-process-layout-run
Rscript tests/benchmarks/initialization/summarize.R \
  /tmp/new-process-layout-run tests/benchmarks/process-layout/results
R_LIBS=/path/to/after-library Rscript tests/benchmarks/initialization/profile.R \
  120 /tmp/process-layout-profile true
```

Raw runs and isolated builds are in `/private/tmp/fims-process-layout`.
See [timings](results/summary.csv), [numerical checks](results/validation.csv),
[test counts](results/test-summary.csv), and [profiles](results/profiles).
