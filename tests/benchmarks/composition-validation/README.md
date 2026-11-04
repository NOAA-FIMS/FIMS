# Build composition diagnostics only on failure

Composition validation used to compute distinct values for every column and
construct group descriptions even when the group was valid. Those operations
now run only in the error branch. Unit and proportion checks, tolerance, and
missing-group behavior remain unchanged.

Baseline: `efb6c32e`. Isolated installations use identical data. Eight fresh
processes run before/after/after/before at each size, after regression tests finish.

| Years | Frame time before | After | R allocations before | After |
| ---: | ---: | ---: | ---: | ---: |
| 30 | 0.219 s | 0.176 s | 18.80 MiB | 18.32 MiB |
| 120 | 0.253 s | 0.208 s | 36.15 MiB | 34.35 MiB |

Median frame time falls 20% and 18%. Cumulative R allocation volume falls 3% and
5%; allocation counts fall from 14,087 to 12,074 and from 21,549 to 13,596.
Allocation profiles are separate from timed runs and exclude native allocation.
Whole-process peak RSS does not improve: medians are 343.8 to 348.8 MiB and
405.8 to 438.0 MiB. The sample has only two processes per variant and size.

Full objectives, gradients, and ordinary reports match within `1e-10` across
all eight runs. The 120-year workload repeats `data_big` observations for scaling.
All 604 targeted assertions pass with no failures, errors, warnings, or skips;
this is not a full suite run. Seven direct baseline comparisons also confirm
identical return values, printed diagnostics, and errors for valid proportions,
invalid sums, mixed units, counts, missing groups, missing units, and empty groups.

Reproduce timings with the shared [initialization benchmark](../initialization/README.md).
Run `check-diagnostics.R OUTPUT_RDS` under each isolated library and compare the
resulting lists with `identical()` to check diagnostic compatibility.

Raw runs and builds are in `/private/tmp/fims-validation`. See
[timings](results/summary.csv), [numerical checks](results/validation.csv),
[diagnostics](results/diagnostics.csv), and [profiles](results/profiles).
