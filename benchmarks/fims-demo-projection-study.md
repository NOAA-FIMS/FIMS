# FIMS projection Laplace comparison

Date: 2026-08-05

## Scope

- Model setup follows `vignettes/fims-demo-projections.Rmd`.
- Each backend and projection horizon runs in a fresh R process.
- Both backends perform one matched Laplace-profiled objective evaluation from
  the same initialized fixed and random parameters. This is not the older TMB
  vignette workflow with optimizer restarts and `sdreport`.
- Peak resident memory is measured by macOS `/usr/bin/time -l`.
- FIMS was compiled with C++17 `-O3`; the emitted compiler commands were
  checked to ensure R's platform `-O2` did not override it.
- Quadra is based on `NOAA-FIMS/Quadra` commit `0d1f9c6`, with the current dense
  Hessian topology freeze, cached destination routing, and value-only mixed
  derivative skip integrated into the FIMS adapter.

## Results

Both backends completed all horizons from 1 through 30 years. The reported TMB
crash at year 12 was not reproduced in isolated processes.

| Projection years | Quadra wall (s) | TMB wall (s) | Quadra RSS (MiB) | TMB RSS (MiB) | Absolute objective difference |
|---:|---:|---:|---:|---:|---:|
| 1 | 1.974 | 1.919 | 288.7 | 390.2 | < 1.1e-11 |
| 5 | 1.878 | 1.968 | 282.8 | 372.1 | < 1.1e-11 |
| 10 | 2.011 | 2.106 | 284.1 | 371.3 | < 1.1e-11 |
| 12 | 1.989 | 2.095 | 291.1 | 398.2 | < 1.1e-11 |
| 15 | 2.084 | 2.108 | 294.2 | 391.9 | < 1.1e-11 |
| 20 | 2.088 | 2.181 | 304.0 | 394.0 | < 1.1e-11 |
| 25 | 2.165 | 2.270 | 305.9 | 392.0 | < 1.1e-11 |
| 30 | 2.237 | 2.316 | 312.4 | 381.9 | < 1.1e-11 |

Across all 30 horizons, Quadra averaged 2.059 seconds and 296.5 MiB peak RSS;
TMB averaged 2.137 seconds and 385.5 MiB. Quadra therefore used about 23.1%
less peak resident memory and was about 3.7% faster on mean wall time. The
median per-horizon wall-time ratio was 0.958.

The maximum absolute objective difference across all horizons was
`1.0004e-11`, providing a strong matched-value check. Quadra selected dense
LDLT for this model's fully dense random-effect Hessian.

## Derivative metric interpretation

The raw CSV's `gradient_norm` field is backend-specific in this study:

- Quadra reports the joint random-effect gradient norm at the discovered mode.
- TMB reports the marginal fixed-effect gradient norm returned by `MakeADFun`.

Those values answer different questions and must not be compared as a
convergence metric. Quadra's validated random-mode gradient norm was
approximately `1.46e-10` at every horizon. The previous restricted-workspace
gradient mismatch is no longer present.

## Reproduction

```sh
Rscript benchmarks/run-fims-demo-projection-study.R \
  30 1 1e-5 benchmarks/fims-demo-projection-study-laplace-o3-1-30.csv \
  quadra,tmb
```

The consolidated measurements are in
`benchmarks/fims-demo-projection-study-laplace-o3-1-30.csv`; individual logs
and RDS results are retained in the two corresponding artifact directories for
years 1--15 and 16--30.
