# Recruitment recovery checkpoint — 2026-09-24

The recovery vignette and installed example now include independent observation
likelihood checks, guarded timing comparisons, retained Hessian failure reasons,
finite uncertainty checks, optimizer messages, RNG-state restoration, and saved
run metadata. Tests require successful fixed-seed noisy fits and validate outcome
accounting, prediction probabilities, reproducibility, and uncertainty failures.

Validation completed:
- Installed the working package into /tmp/fims-recovery-library.
- Ran tests/testthat/test-recruitment-recovery.R: 39 passes, no failures or warnings.
- Rendered vignettes/fims-recruitment-recovery.Rmd with 30 replicates: all 60
  noisy fits usable; both exact-data timing comparison fits usable.
- Maximum exact-data relative parameter error: 2.383092e-6.
- Maximum independent likelihood-difference discrepancy: 1.421085e-14.
- Inspected rendered tables and the four-panel recovery plot.
- git diff --check passed.

Temporary validation outputs (may be removed by OS cleanup):
/tmp/fims-recovery-validation/{tests.log,install.log,render.log,recovery.rds,
fims-recruitment-recovery.html,fims-recruitment-recovery.md}.

To reproduce, install this checkout, then run:

```r
library(FIMS)
testthat::test_file("tests/testthat/test-recruitment-recovery.R", stop_on_failure = TRUE)
render_env <- new.env()
rmarkdown::render("vignettes/fims-recruitment-recovery.Rmd", envir = render_env)
saveRDS(render_env$result, "recovery.rds")
```

Pandoc was supplied by RStudio at
/Applications/RStudio.app/Contents/Resources/app/quarto/bin/tools/aarch64
via RSTUDIO_PANDOC. This was targeted validation, not a full package check.
The independent likelihood uses FIMS predictions; population dynamics still
require separate analytic tests. Thirty replicates illustrate the workflow and
are not a precise coverage study. Existing timing-design.html and FIMS Notes
Timing.docx were left unchanged and excluded from the checkpoint.

## Broader validation follow-up

Related recruitment and observation-timing tests passed: 270 checks, no failures,
warnings, or skips. Run them with the installed package namespace available:

```r
testthat::test_dir("tests/testthat", filter = "recruitment|observation-timing",
                   package = "FIMS", load_package = "installed",
                   stop_on_failure = TRUE)
```

A plain test_dir() invocation without the package namespace cannot resolve
internal timing helpers; that was a test invocation issue, corrected above.
Added build exclusions for this progress note, timing-design.html, and
FIMS Notes Timing.docx so local work documents do not enter the source package.

The complete vignette build passed. A clean source archive retaining those built
vignettes is 17 MB. Generated fit/data/parameter/deterministic RDS fixtures are
now excluded from builds, matching the existing Git exclusions; committed
integration inputs and initial_value_scale.RDS remain included. Tests regenerate
the excluded files. Local cached fit files had inflated the initial archive by
several gigabytes.

R CMD check --no-manual --no-build-vignettes completed against that archive in
/tmp/fims-recovery-validation/clean-build/FIMS.Rcheck: zero errors, two warnings,
and two notes. The test suite regenerated its fixtures and reported 2,166 passes,
zero failures, three test warnings, and 13 skips. The skips cover CRAN-only
exclusions, opt-in slow tests, and explicitly disabled random-effects/Newton
tests. All 15 vignette code checks passed. Build/check logs are in
/tmp/fims-recovery-validation. The check predates the documentation fix below;
that fix was validated separately rather than rerunning the whole suite.
The check found missing documentation for AgeSpecificSelectivity; added its
alias and C++ documentation link to R/Rcpp_exports.R and man/Cpp_classes.Rd.
Reinstallation, tools::checkRd(), and tools::undoc() verify the documentation fix.
Existing non-portable compiler flags remain a check warning; installed size and
GNU make produce notes. Repository-index network access was unavailable, but
installed dependencies allowed the check to proceed.

## Recovery edge-case hardening

Added input validation before FIMS::clear(): positive finite starting values in
explicit parameter order, three ordered model years, positive catch/index and
log-scale SDs, nonnegative 3-by-5 composition matrices with positive row totals,
and whole survey Dates within their corresponding years. Run settings reject
invalid replicate counts and seeds before changing the caller's RNG state.

Summaries now separate failed (exceptions) and unusable (returned but rejected)
fits, with attempted = usable + failed + unusable. Injected-failure tests verify
all-failed runs return NA summaries, exceptions are retained, Hessian failures
preserve estimates and optimizer diagnostics, and outcomes are neither dropped
nor duplicated. Invalid-input tests mock registry clearing to establish that
rejection occurs before changing FIMS state.

Validation after these changes:
- Recovery tests: 94 passes, zero failures/warnings/skips.
- Related recruitment/timing tests: 325 passes, zero failures/warnings/skips.
- Updated 30-replicate vignette rendered: all 60 noisy fits usable.
- Reviewed rendered summary table and input-contract text; git diff --check clean.
Logs: /tmp/fims-recovery-validation/edge-{install,tests,related-tests,render}.log.
The earlier full package check applies before this edge-case change; the latest
changes were validated with the focused tests and vignette above.

## Independent dynamics validation and output fix

Added tests/testthat/test-recruitment-analytic.R: a three-age, three-year model
with fixed recruitment budgets 1000/1200/1400, M = 0.2, fishing rates
0.3/0.4/0.2, constant selectivity 0.5, q = 0.4, and weights 1/2/3. Expected
values use only these inputs, calendar constants, exponential survival, and
Baranov catch. They do not use FIMS-reported rates or states as an oracle.
Checks cover January/July entry dates and adjacent days, April exclusion of
late recruits, 2028 leap-year fractions, annual aging and the plus group,
recruitment allocation, abundance/catch/death conservation, and 100% January
or 100% July allocation while retaining the phase evaluator.

The exact-zero survey mortality fixture exposed invalid '-inf' JSON in derived
log catch, causing get_estimates() to fail. Updated derived-quantity JSON output
to use its existing -999 sentinel for all non-finite values, including both
interior and final vector elements. Raw reports retain -Inf. The regression
checks valid JSON, successful extraction, and unchanged finite survey indices.
NEWS and the recovery vignette describe these changes.

Clean rebuild passed using Apple's compiler with PATH limited to system/R
binaries. The first rebuild selected local Homebrew LLVM and failed in its
standard-library headers; the Apple compiler used in earlier validation worked.
Analytic/output tests: 92 passes, no failures/warnings/skips.
Updated recovery vignette rendered successfully.

The 500-replicate study (seed 20260924) completed with all 1,000 fitted models
usable, no exceptions, and no rejected fits. Conditional Wald interval results:

| Schedule | Parameter | Relative bias | Relative RMSE | Coverage | Coverage MCSE |
|---|---|---:|---:|---:|---:|
| January/July | F_2027 | 0.021575 | 0.204768 | 0.974 | 0.007117 |
| January/July | F_2028 | 0.011237 | 0.206607 | 0.950 | 0.009747 |
| January/July | F_2029 | 0.037085 | 0.232470 | 0.944 | 0.010282 |
| January/July | q | 0.013555 | 0.149292 | 0.950 | 0.009747 |
| January only | F_2027 | -0.143587 | 0.216647 | 0.880 | 0.014533 |
| January only | F_2028 | -0.166950 | 0.231339 | 0.844 | 0.016227 |
| January only | F_2029 | -0.133910 | 0.226740 | 0.880 | 0.014533 |
| January only | q | -0.132435 | 0.183246 | 0.828 | 0.016877 |

These results describe the fixed biological settings in this experiment, not
uncertainty in the recruitment schedule itself. No test imposes a target bias
or coverage. Raw results and session metadata are saved in
/tmp/fims-recovery-validation/recovery-500.rds; the log is recovery-500.log.
Reproduce with run_recruitment_recovery(500L, seed = 20260924L).

Broader post-rebuild validation completed: 542 passes, zero failures/warnings,
and two existing CRAN-guarded snapshot skips across recruitment, observation,
reshape_json_estimates, tidy, and get_estimates tests. This includes the 92 new
analytic/output checks. Logs are analytic-install.log, analytic-tests.log,
analytic-related-tests.log, and analytic-render.log under the validation folder.
This pass did not rerun the entire package check. The pre-existing formatting
edit to inst/examples/recruitment-phase-recovery.R was retained and excluded
from this checkpoint.
