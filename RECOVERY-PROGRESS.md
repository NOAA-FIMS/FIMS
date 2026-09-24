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
