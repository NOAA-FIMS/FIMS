# Reuse repeated distribution specifications

After bulk parameter transfer, distribution parsing accounted for about 64% of
sampled module initialization time. Observation rows often repeat identical
distribution strings. The parser now reuses their parsed representation and
link label within a single call, avoiding repeated parsing and validation.

Reuse applies to character specifications with literal numeric normal or
lognormal scales, and multinomial specifications. Nonliteral scales still
evaluate for every observation and clear prior cached results because evaluation
can have side effects. Missing rows retain their neutral ancillary values.
Calls and expression inputs retain their existing path. All-unique strings skip
the cache. Nothing is retained between parser calls.

## Results

The baseline is `d509f5a4`, including bulk transfer and the merged data updates.
Both isolated installations use the same data. Fresh processes alternate
before/after/after/before at each size. The measured runs were performed without
the test suite running alongside them.

| Years | Module initialization before | After | Speedup |
| ---: | ---: | ---: | ---: |
| 30 | 0.288 s | 0.138 s | 2.09x |
| 120 | 0.747 s | 0.231 s | 3.23x |

Times are medians of two processes per variant. All eight runs agree on objective
values, full gradients, and ordinary reports within `1e-10`. The 120-year
workload repeats observations from `data_big` to examine scaling; it is not a
scientific assessment. Frame construction and parameter setup are unchanged.

The cache trades a small allocation increase for speed. Separate R allocation
profiles measured 9.51 to 9.75 MiB at 30 years and 30.50 to 31.37 MiB at 120 years.
These are cumulative R allocations and exclude native allocations. Whole-process
peak RSS medians were 330.6 to 345.7 MiB and 423.5 to 441.0 MiB, respectively;
there is no demonstrated peak-memory reduction.

All 452 targeted assertions passed, with no failures, errors, warnings, or skips.
Coverage includes parser ordering and missing values, per-observation evaluation
and rejection of invalid scale expressions, module initialization, bulk transfer,
VariableVector, parameter setup, evaluation reuse, report layout, and selective
uncertainty. This was not a rerun of the complete testthat suite.

## Reproduce

Build the baseline and candidate into separate R libraries, then use the shared
[initialization benchmark](../initialization/README.md):

```sh
python3 tests/benchmarks/initialization/run.py \
  /path/to/before-library /path/to/after-library /tmp/new-parser-run
Rscript tests/benchmarks/initialization/summarize.R \
  /tmp/new-parser-run tests/benchmarks/distribution-parser/results
R_LIBS=/path/to/after-library Rscript tests/benchmarks/initialization/profile.R \
  120 /tmp/parser-profile true
```

Raw runs and isolated builds are in `/private/tmp/fims-parser-cache`.
See [timings](results/summary.csv), [numerical checks](results/validation.csv),
[test counts](results/test-summary.csv), and [profiles](results/profiles).
