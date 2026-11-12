# Bulk module initialization

Local candidate measured 2026-11-12 against main `a739d031bb62d04f9366231182682b9ee389c1e2`.

Two changes use existing FIMS bulk setters: parameter vector initialization, and process-distribution scale/zero initialization. Variable identities, ordering, estimation types, process links and existing input validation are preserved. Parameter inputs with missing or unrecognized estimation types retain the scalar path. Fleet layout reuse and parser changes are excluded.

Main does not yet include parser PR #1956. These are standalone measurements against main, not incremental measurements after that PR. Rebase and repeat relevant benchmarks after preceding PRs land; individual percentage improvements must not be added together.

## Measurements

Fresh R processes, one BLAS/OpenMP thread, identical dependencies and compiled backend. Two runs per variant in before/after/after/before order. Times and process RSS are medians. The 120-year fixture repeats the 30-year observation pattern. Initialization excludes frame/parameter setup and AD construction; RSS includes startup and subsequent numerical checks.

| Years | Before initialization | After initialization | Runtime reduction | Before process peak | After process peak |
|---|---:|---:|---:|---:|---:|
| 30 | 2.1455 s | 1.7985 s | 16.2% | 335.6 MiB | 318.3 MiB |
| 120 | 8.4855 s | 7.3715 s | 13.1% | 439.2 MiB | 425.9 MiB |

The 30-year full workflow includes frame construction, parameter setup, initialization, default three-loop optimization, full uncertainty and extraction. Pipeline median: 21.3350 -> 20.4955 seconds (3.9% lower). Whole-process peak RSS: 889.3 -> 876.7 MiB (1.4% lower). Two repetitions do not establish a reliable general full-fit or memory improvement; fit-stage variation contributes to this observed difference. Peak RSS is not retained-object memory or allocation volume.

## Validation

- **464 assertions passed**, no failures/errors/warnings/skips: bulk parameter/process regressions, distribution formulas, module initialization, Rcpp distributions, and existing VariableVector tests. Includes mixed types, interleaved rows, surviving identities on resize/reassignment, missing-type compatibility, scalar/vector scales, both process families and both recruitment parameterizations.
- All eight initialization runs agree with their same-size baseline on input parameter vectors, free parameters, objective, gradient and ordinary reports at tolerance 1e-10.
- All four full fits converged with max absolute gradient below 0.01, positive-definite Hessians and finite report standard errors. All **36 comparisons passed** for parameters, TMB configuration, objective, optimum parameters, reports, estimates, covariance and uncertainty values/SDs. Tolerances are recorded in full-fit-validation.csv.
- No C++ or TMB settings changed. Candidate installation used `R CMD INSTALL --no-libs --no-test-load`, followed by copying the unchanged main backend's `libs` directory. Matching backend hashes and source/data hashes are in results/manifest.txt.
- This branch is local. Full-suite CI has not been run for this candidate. Main's stockplotr vignette compatibility fix is in PR #1956; it is not duplicated here.

Results, session details and assertion counts are under results. Raw profiles/model objects remain in scratch directories outside version control.

## Reproduce

Build isolated baseline/candidate libraries using identical dependencies and compiler flags. From this directory:

```
python3 run.py BEFORE_LIBRARY AFTER_LIBRARY NEW_INITIALIZATION_OUTPUT
Rscript summarize.R NEW_INITIALIZATION_OUTPUT RESULTS_DIRECTORY
```

Save the baseline package's `data_big` object to a separate RDS file, then:

```
python3 run-full-fit.py BEFORE_LIBRARY AFTER_LIBRARY DATA_RDS NEW_FULL_FIT_OUTPUT
Rscript summarize-full-fit.R NEW_FULL_FIT_OUTPUT RESULTS_DIRECTORY
```

The harnesses collect timing in fresh child processes and use wait4 for process peak RSS. Full-fit input expansion is outside the pipeline timer. Do not run competing benchmarks concurrently. profile.R can optionally collect allocation/CPU profiles separately; profiling was disabled for these timings.
