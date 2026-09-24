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
