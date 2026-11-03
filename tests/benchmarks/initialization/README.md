# Initialization profiling and distribution lookup

Profiling the installed FIMS build after commit `922453a5` found that parsing
observation distribution specifications accounted for approximately 81% of
30-year module initialization CPU samples and 84% at 120 years. Most of that
work came from a `dplyr::filter()`/`pull()` lookup of the distribution's central
parameter, repeated for every observation. Building data masks and temporary
tibbles dominated an otherwise small lookup.

The change uses base-R indexing into the existing distribution registry.
`which()` retains the matching row order and drops missing comparisons, as
`filter()` did. Parsing, expression evaluation, argument validation, missing
specifications and error handling remain on their existing paths. No parsing
results are cached, so ancillary expressions still execute once per input.

## Measurements

Unprofiled fresh processes ran before/after/after/before for each workload.
The larger workload repeats the 30-year `data_big` observations to 120 years;
it is a synthetic scaling example, not additional assessment data.

| Years | Module initialization before | After | Speedup |
| ---: | ---: | ---: | ---: |
| 30 | 2.230 s | 0.569 s | 3.92x |
| 120 | 8.902 s | 2.052 s | 4.34x |

These are medians of two processes per variant. Frame construction and default
parameter setup are timed separately and are essentially unaffected. Objective
values, complete gradients and ordinary reports agree at tolerance `1e-10`
across all eight runs after initialization.

Separate R allocation profiles measured:

| Years | Allocated during module initialization, before | After | Reduction |
| ---: | ---: | ---: | ---: |
| 30 | 21.85 MiB | 10.15 MiB | 54% |
| 120 | 79.75 MiB | 32.93 MiB | 59% |

These are **cumulative R allocations**, not retained bytes or peak resident
memory. They exclude native C++ allocations. Profiled CPU timings have sampling
and allocation-tracing overhead; use the unprofiled measurements above for
runtime comparisons. Whole-process peak RSS was mixed: median 339.3 to 344.1 MiB
at 30 years, and 454.8 to 425.1 MiB at 120 years. This does not establish a
consistent percentage reduction in peak memory.

After the lookup change, `set_param_vector()` accounts for approximately 64% of
120-year initialization CPU samples. It transfers values and estimation types
through Rcpp one element at a time, making a bulk-transfer interface the next
candidate. That optimization is not implemented here.

## Reproduce

```sh
R_LIBS=/path/to/library Rscript tests/benchmarks/initialization/profile.R \
  30 /tmp/profile-output true
python3 tests/benchmarks/initialization/run.py \
  /path/to/before-library /path/to/after-library /tmp/new-initialization-run
Rscript tests/benchmarks/initialization/summarize.R \
  /tmp/new-initialization-run tests/benchmarks/initialization/results
```

The driver limits BLAS/OpenMP to one thread. Raw profiles and installed builds
for the recorded study are in `/private/tmp/fims-initialization-profile`.
Selected CPU/allocation profiles are stored in [results/profiles](results/profiles).
See [timings](results/summary.csv), [individual runs](results/runs.csv), and
[numerical checks](results/validation.csv).

Focused validation passed **146 expectations, with zero failures, errors,
warnings or skips**, covering parser inputs/errors, module initialization,
default parameter setup, repeated evaluations, report layout and selective
uncertainty/compact results. This was not a new full-suite run. Counts are in
[test-summary.csv](results/test-summary.csv).
