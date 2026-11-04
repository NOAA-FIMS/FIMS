# Narrow missing-data joins and append missing rows

`create_missing_data()` now selects only observation keys and relevant types
before expansion and matching. `FIMSFrame()` appends generated missing rows
instead of performing a full join: those rows have already been excluded from
the input by their observation keys. Final sorting is unchanged.

## Results

Baseline: `d9cef13e`. Both isolated installations use identical data. Eight fresh
processes alternate before/after/after/before at each size, after regression tests
finish. Times below are medians of two processes per variant.

| Years | Frame time before | After | R allocations before | After |
| ---: | ---: | ---: | ---: | ---: |
| 30 | 0.2215 s | 0.2130 s | 20.94 MiB | 18.80 MiB |
| 120 | 0.3020 s | 0.2880 s | 43.82 MiB | 36.15 MiB |

Cumulative R allocation volume falls by 10% and 18%, respectively. Allocation
profiles are separate from the timed runs and exclude native allocations.
The observed 4–5% timing improvement is modest and based on a small sample.
Whole-process peak RSS remains mixed: medians of 331.5 to 335.5 MiB at 30 years
and 439.3 to 436.0 MiB at 120 years. No general peak-memory reduction is claimed.
Parameter and module allocation volumes are unchanged.

Complete objectives, gradients, and ordinary reports agree within `1e-10` in all
eight benchmark runs. The 120-year data repeat `data_big` observations for scaling
and do not represent a scientific assessment.

Separate exact comparisons cover ten valid frames and four invalid inputs.
The valid cases include missing index/composition years, duplicate observations,
missing units, extra metadata, and absent age or length columns. All frame slots,
data types, attributes, row order, and captured warnings match exactly. Invalid
inputs retain the same error messages. See [frame checks](results/frame-validation.csv).

Regression tests passed 595 assertions in the frame/initialization group and
51 in the slow growth group, with no failures, errors, or warnings. One existing
Newton-polishing test skips because Newton steps are not implemented. This is
not a full testthat suite run.

## Reproduce

```sh
python3 tests/benchmarks/initialization/run.py \
  /path/to/before-library /path/to/after-library /tmp/new-frame-run
Rscript tests/benchmarks/initialization/summarize.R \
  /tmp/new-frame-run tests/benchmarks/frame-construction/results
R_LIBS=/path/to/after-library Rscript tests/benchmarks/initialization/profile.R \
  120 /tmp/frame-profile true
R_LIBS=/path/to/before-library Rscript tests/benchmarks/frame-construction/check-frames.R \
  /tmp/before-frames.rds
R_LIBS=/path/to/after-library Rscript tests/benchmarks/frame-construction/check-frames.R \
  /tmp/after-frames.rds
R_LIBS=/path/to/after-library Rscript tests/benchmarks/frame-construction/compare-frames.R \
  /tmp/before-frames.rds /tmp/after-frames.rds /tmp/frame-validation.csv
```

Raw runs and builds are in `/private/tmp/fims-frame-joins`.
See [timings](results/summary.csv), [numerical checks](results/validation.csv),
[test counts](results/test-summary.csv), and [profiles](results/profiles).
