# Selective uncertainty and compact results

The default remains unchanged:

```r
fit <- fit_fims(input) # uncertainty for every existing derived report
```

To reduce derived-uncertainty work while retaining all ordinary outputs:

```r
important <- c("spawning_biomass", "biomass", "expected_recruitment",
               "catch_expected", "index_expected")
fit <- fit_fims(input, uncertainty_reports = important)
```

`NULL` selects all uncertainty reports; `character(0)` selects parameter
uncertainty only. Names apply to all populations/fleets. Duplicate requests are
ignored, report order is unchanged, and unknown names are errors. Unselected
derived quantities remain available in ordinary reports and estimates, with
`NA` uncertainty. Fixed- and random-effect standard errors are unaffected.
The selection is stored in each TMB object's data, not shared mutable model
state; later `TMB::sdreport(get_obj(fit))` calls retain the selection. TMB itself
is unchanged. Internally FIMS encodes names as a list of scalar strings because
TMB's data interface does not accept general character vectors.

For collecting many completed fits:

```r
result <- compact_fims(fit, reports = important)
saveRDS(result, "result.rds")
# Once the live fit/input are no longer needed:
rm(fit, input)
FIMS::clear()
gc()
```

A compact result is a plain list containing the complete estimates table
(including calculated standard errors), requested ordinary reports, optimizer
results, diagnostics, selection metadata, and FIMS version. It contains no live
TMB objective, input modules, full covariance matrices, or duplicate JSON.
It is not a `FIMSFit` and cannot refit or recompute uncertainties.
`compact_fims()` leaves the original fit intact; memory is not freed while other
references retain it. `clear()` affects FIMS's current global model, so call it
only after finishing work with that model. Compact results remain usable after
clearing and after serialization.

## Benchmark

```sh
python3 tests/benchmarks/selective-uncertainty/run.py \
  /path/to/installed-library /tmp/new-selective-run
Rscript tests/benchmarks/selective-uncertainty/summarize.R \
  /tmp/new-selective-run tests/benchmarks/selective-uncertainty/results
```

Fresh processes alternate all/selected/selected/all, with one BLAS/OpenMP
thread. Workloads repeat `data_big` to 30 and 120 years, holding all but one fixed
effect constant for affordable uncertainty comparisons. These are synthetic
reporting workloads, not full random-effects fitting benchmarks. RSS is the
whole-process peak from `wait4`, including loading, data preparation, recording,
ordinary reporting and uncertainty calculations. Timings isolate `sdreport`.
The validation checks objective, gradient, complete ordinary reports, selected
estimates/standard errors, and parameter covariance against the full calculation.

The integration regression separately checks real random-effects fits, default,
subset and empty selections, independent objects, malformed/unknown names,
compact snapshot contents and serialization after clearing FIMS.

## Selective-report results

Two fresh processes per variant and size, in alternating order:

| Years | Report values: all / selected | Peak RSS: all / selected (MiB) | sdreport: all / selected (s) |
| ---: | ---: | ---: | ---: |
| 30 | 13,122 / 213 | 522.9 / 350.2 | 4.377 / 0.032 |
| 120 | 52,362 / 843 | 607.4 / 505.4 | 66.397 / 0.161 |

Median whole-process peak reductions were about **33%** and **17%**, respectively.
These measurements apply to the stated one-active-parameter reporting workloads;
they do not establish the same savings for arbitrary random-effects fits. All
ordinary reports remain complete. Selected standard errors, objective values,
gradients and parameter covariance match the full calculation.

See [individual runs](results/runs.csv), [summary](results/summary.csv), and
[validation](results/validation.csv). Raw results are in
`/private/tmp/fims-selective/runs`.

## Retained-fit experiment

`run-retention.py LIBRARY NEW_OUTPUT_DIRECTORY` runs full/compact/compact/full
in fresh processes, retaining 20 completed 30-year fits per process. Each fit
has one estimated fixed effect and 29 recruitment random effects; other
parameters are constant. Both variants calculate uncertainties for the same
five reports. Garbage collection runs after each fit in both variants. Full
fits retain their TMB objects; compact results retain the complete estimates
table and the five ordinary reports. The driver measures whole-process peak
RSS, not exact bytes held by a particular R object. `object.size()` would omit
native tape storage and is not used as a substitute for process memory.

Retaining 20 fitted results gave these medians across two fresh processes per
variant:

| Retained form | Peak RSS (MiB) | Whole experiment (seconds) |
| --- | ---: | ---: |
| Full FIMSFit objects | 2,583.7 | 20.11 |
| Compact snapshots | 1,139.7 | 25.11 |

This is about **56% less peak memory** for the compact workflow. Constructing
complete estimates tables for every snapshot added about five seconds across
20 fits. Optimizer results, complete estimates (including standard errors), and
retained reports matched the full-fit reference at `1e-10`. This workload uses
one fixed effect and 29 random effects, so the percentage should not be assumed
for every assessment. The original full objects must become unreachable for
the memory benefit; retaining both forms defeats the purpose.

See [retention runs](results/retention-runs.csv),
[retention summary](results/retention-summary.csv), and
[retention validation](results/retention-validation.csv).

Final focused validation passed **117 expectations with no failures, errors,
warnings or skips**. Coverage includes convergence checks, default reporting,
repeated evaluations, uncertainty ordering and finite differences, selective
and parameter-only uncertainty, selected full covariance, independent objects,
input errors, and compact serialization after clearing FIMS. This is focused
validation, not a fresh full-suite run. Results are in
[test-summary.csv](results/test-summary.csv).
