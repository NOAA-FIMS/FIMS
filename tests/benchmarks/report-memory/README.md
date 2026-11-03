# Reporting memory experiment

The catch-at-age reporter formerly retained nested TMB copies of 33 derived
quantities, then retained flattened copies of all 33 before pushing them to the
uncertainty report. The changed reporter holds non-owning pointers to FIMS's
existing vectors and processes one quantity at a time:

1. On the double path, copy that quantity into its original nested R-report
   representation, then release the temporary nested C++ vectors.
2. Flatten directly from the original FIMS vectors, push to TMB's report stack,
   then release the temporary flat vector.

The pointer collections exist only during `Report()`. Sources stay alive and
are not resized during this method. Map insertion does not invalidate their
addresses. Quantity names are string literals with static lifetime, as required
by TMB's report stack. Ordinary report shapes and uncertainty report order are
unchanged. TMB itself is unchanged and still retains its own report storage.
Growth-only and age-to-length diagnostic reports keep their existing handling.

## Reproduction

Build the previous branch and changed branch into separate R libraries, then:

```sh
python3 tests/benchmarks/report-memory/run.py \
  /path/to/before-library /path/to/after-library /tmp/new-report-memory-run
Rscript tests/benchmarks/report-memory/summarize.R \
  /tmp/new-report-memory-run tests/benchmarks/report-memory/results
```

The driver runs each combination in fresh processes, in before/after/after/before
order, with BLAS/OpenMP limited to one thread. It measures process peak RSS with
`wait4`; this includes R loading, data preparation and model initialization.
Reporting and uncertainty phases also include object recording. Measurements
are not isolated C++ allocation counts or incremental phase-specific RSS.

Workloads use `data_big` at 30 years and repeat its observations to 300 and 1,200
years, with 12 ages. The repeated data are synthetic scaling workloads, not
scientific assessments. Only the first fixed-effect parameter is active; all
others, including recruitment deviations, are held constant to make large
uncertainty-report comparisons affordable. This benchmark isolates reporting
behavior, not random-effects fitting or sparse Hessian memory.

Each ordinary-report process retains reports at the initial parameter, a
perturbation and the initial parameter again. Uncertainty checks compare the
full `summary(sdreport, "report")`, including names, order, estimates and standard
errors. The validation script also compares objective values and gradients at
`1e-10` tolerance. The uncertainty calculation uses the existing default of no
full report covariance. No reports or standard errors are disabled by the change.

Uncertainty benchmarks are limited to 30 years: the exploratory 300-year
reference calculation took about eight minutes. Its completed result is retained
in the raw log but excluded from the paired summary. Larger workloads measure
recording and ordinary reporting only. The driver can resume completed cases
from `runs.json`; use a fresh output directory when changing either build.

The focused regression in `test-integration-report-layout.R` independently
checks all 33 uncertainty-report names/order, reported estimates, and standard
errors against finite differences of ordinary reports. Broader integration
coverage exercises growth and random-effects models.

For the 30-year example, the original two retained sets of copied `double`
values occupied 209,952 bytes; the largest new per-quantity buffer is 11,040
bytes. This is payload accounting, not an allocator/peak-RSS measurement. It
excludes vector metadata, R output storage, TMB's report stack and its internal
push temporaries, and the unchanged diagnostic reports. AD element sizes differ.

## Results

All 28 paired-workload cases passed numerical comparisons at `1e-10`, including
ordinary-report shapes/values and the 30-year uncertainty output. The new
finite-difference regression also passed. The reference is the preceding loop
optimization build, not main.

Median whole-process peak RSS (two runs per variant per workload):

| Years | Operation | Before (MiB) | After (MiB) |
| ---: | --- | ---: | ---: |
| 30 | Recording | 339.5 | 340.0 |
| 30 | Reporting | 348.0 | 350.3 |
| 30 | Uncertainty reporting | 549.7 | 522.4 |
| 300 | Recording | 635.3 | 642.6 |
| 300 | Reporting | 629.2 | 637.1 |
| 1,200 | Recording | 1,182.0 | 1,193.7 |
| 1,200 | Reporting | 1,260.4 | 1,211.2 |

The largest reporting workload had a roughly 49 MiB (3.9%) lower median peak.
Recording and smaller reporting workloads showed no consistent improvement.
Two runs per variant and whole-process peaks are insufficient to establish a
general percentage saving. Allocation timing, R garbage collection and retained
model/output storage also influence these measurements. The source-level
reduction in overlapping temporary payload is definite: at 1,200 years the
former two copied sets contain 8,371,872 bytes of double values, whereas the
largest new per-quantity buffer contains 441,600 bytes. These counts exclude
TMB's own copies and all other allocations.

See [individual runs](results/runs.csv), [summary](results/summary.csv),
[equivalence checks](results/validation.csv), and
[payload accounting](results/double-payload.csv). Timings are diagnostic only;
a short standalone regression overlapped the first large recording process.
Raw R objects and logs are in `/private/tmp/fims-report-memory`.

## Test-order issue found during validation

A combined filtered test run completed the model/growth checks, then crashed
in `DensityComponentBase<double>::get_n_x()` during output reshaping. The last
growth test intentionally rejects invalid reference lengths and leaves partial
FIMS initialization state behind. A reshaping test subsequently evaluates a
saved model against that state. Repeating this failed-initialization/reshape
sequence on the **unchanged reference build** also segfaulted. Running reshaping
and tidy tests in fresh processes passed on both builds. This is an existing
state-isolation issue, not evidence of a reporting-buffer regression; it remains
unfixed by this scoped change. Final validation runs the model/growth group and
the reshape/tidy group in separate processes.

Final focused validation: **563 passing expectations across 15 files, seven
existing skips, zero failures/errors and zero warnings**, with model/growth and
reshape/tidy groups run in separate processes as described above. This is
focused validation, not a new full-suite run. Results are recorded in
[test-summary.csv](results/test-summary.csv). `git diff --check` and benchmark
script syntax checks also passed.
