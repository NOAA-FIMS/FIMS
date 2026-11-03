# Catch-at-age loop efficiency

This change stays inside FIMS's existing evaluator and TMB fitting workflow.
It preserves the statistical model and all report names.

- Resolve seven derived-quantity vectors once per fleet in length-composition
  evaluation, instead of constructing string keys and searching the map for
  every age/length contribution.
- Reuse the age-to-length row buffer across ages and years. The fixed-matrix
  implementation fills this buffer in place; growth-derived implementations
  still retain their own row construction behavior.
- Evaluate population weight at age once per year/age for catch weights, then
  share that value across fleets. No values are cached between evaluations.

`measure.R` compares isolated installed libraries in fresh R processes using
`data_big`, including estimated natural mortality. It saves objective values,
all reported values, and joint gradients at the starting point and two perturbed
points, returning to the starting point to check state reuse. It also saves a
complete joint `nlminb` fit. This joint benchmark does not integrate random
effects; the full testthat suite separately covers the normal Laplace workflow.

Example:

```sh
R_LIBS=/path/to/reference-library Rscript \
  tests/benchmarks/catch-at-age-loops/measure.R /tmp/before.rds
R_LIBS=/path/to/changed-library Rscript \
  tests/benchmarks/catch-at-age-loops/measure.R /tmp/after.rds
```

Recording, objective, gradient and report timings are medians of five batches.
Objective/gradient inputs alternate to avoid unchanged-input reuse. Recording
includes freeing each temporary TMB object. Fit elapsed time covers one full
optimization per process. Use alternating process order and single-thread
BLAS/OpenMP settings for comparisons. Peak RSS covers the entire process, not
only evaluator storage, so small differences are not evidence of memory savings.

## Recorded comparison

Three fresh processes per version, in before/after/after/before/before/after
order, on the same Apple M3 Pro and compiler configuration. The reference is
this branch immediately before the loop changes (including the earlier
preparation and recruitment reuse changes), not `main`.

| Metric | Before | After | Change |
| --- | ---: | ---: | ---: |
| Recording and releasing a joint TMB object | 29.0 ms | 21.2 ms | -26.9% |
| Joint objective evaluation | 0.130 ms | 0.130 ms | unchanged |
| Joint gradient evaluation | 0.250 ms | 0.250 ms | unchanged |
| Full report evaluation | 3.30 ms | 1.39 ms | -57.9% |

All saved objective values, gradients and complete reports agree at tolerance
`1e-10` across all six processes, including perturbed inputs and return to the
starting point. The optimization attempts also match parameters and stopping
status, but **both versions stop with nlminb false convergence (8)** in this
joint workload with estimated mortality. Their timings are retained in the raw
results for transparency and are not evidence about converged fit performance.

Median process peak RSS was 429.7 MiB before and 419.3 MiB after, with overlapping
ranges (415.6–462.4 vs. 402.3–453.4 MiB). This does not establish a memory saving.
The main demonstrated improvement is in C++ evaluation/reporting and recording;
replaying the existing AD graph is effectively unchanged.

See [individual runs](results/runs.csv), [summary](results/summary.csv), and
[numerical comparisons](results/validation.csv). Raw R objects and build/test
logs for this run are in `/private/tmp/fims-loop-efficiency`.

## Full-suite validation

All 71 test files ran with `RUN_SLOW_TESTS=true`, using the same prepared fixture
snapshot as the earlier main/branch comparison, plus the evaluation-reuse
regression. The initial sandboxed run produced 1,907 passing expectations,
seven skips, three socket-creation errors and one warning. The three affected
files (`fims_likelihood`, `fims_retrospective`, and
`slow-parallel-caa-mle-wrappers`) were rerun with local worker sockets permitted.
All passed. Replacing those files' initial results with their rerun results gives
**1,937 passes, seven skips, zero failures/errors and zero warnings**; see
[test-summary.csv](results/test-summary.csv).

The initial run took about 888 seconds and the affected-file rerun about 425
seconds. These split runs are validation, not a comparable full-suite timing
against main. The earlier repeated main/branch benchmark remains in
`/private/tmp/fims-suite-benchmark/REPORT.md`; it predates these loop changes.
