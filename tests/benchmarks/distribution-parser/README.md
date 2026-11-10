# Distribution parser reuse benchmark

This PR replaces a per-observation dplyr registry lookup with direct indexing and reuses repeated character specifications with literal numeric scales within one parser call. Scale expressions still execute and validate per observation; reuse does not persist across calls.

Measured 2026-11-10 against main `a739d031bb62d04f9366231182682b9ee389c1e2`. Both installed variants use the identical compiled main backend (SHA256 recorded in results/manifest.json); only this PR's R changes differ. The candidate was installed with `R CMD INSTALL --no-libs --no-test-load`, then the unchanged main library's `libs` directory was copied into the candidate library. Both packages load successfully. No TMB settings changed.

Two fresh processes per variant at each size, in before/after/after/before order, with one BLAS/OpenMP thread. The 120-year input repeats the 30-year observation pattern. Initialization timing excludes frame/parameter setup and AD construction. Peak RSS covers the entire process including the numerical checks. Two repetitions do not establish confidence intervals.

| Years | Before initialization | After initialization | Speedup | Before peak RSS (median) | After peak RSS (median) |
|---|---:|---:|---:|---:|---:|
| 30 | 2.2305 s | 0.4440 s | 5.02x | 332.4 MiB | 345.6 MiB |
| 120 | 8.5480 s | 1.4640 s | 5.84x | 439.4 MiB | 411.2 MiB |

Memory results are mixed: 4.0% higher at 30 years and 6.4% lower at 120 years. These are process peaks, not retained cache sizes or allocation volumes. No general memory reduction or full-fit runtime gain is claimed.

All eight runs' objectives, gradients and ordinary reports agree with their same-size baseline at tolerance 1e-10. Focused tests passed 157 assertions with no failures, errors, warnings or skips: parser, distribution formulas, module initialization, and Rcpp distributions. New tests cover mixed families/order/missing values, empty input, scale-expression side effects and errors, and independent results within/across calls. Test results are in results/tests.csv. Full-suite CI remains a separate pre-merge check.

## Reproduce

Build isolated before/after FIMS libraries with identical dependencies/compiler flags. Run from this directory:

```
python3 run.py BEFORE_LIBRARY AFTER_LIBRARY NEW_SCRATCH_DIRECTORY
Rscript summarize.R NEW_SCRATCH_DIRECTORY RESULTS_DIRECTORY
```

`profile.R` also accepts `YEARS OUTPUT_DIRECTORY true` for a separate CPU/allocation profiling run. Such profiling is not enabled in the timing results above. Do not run competing benchmarks concurrently. Compact metrics, source/backend hashes, numerical validation and session information are included under results; raw model objects remain outside version control.
