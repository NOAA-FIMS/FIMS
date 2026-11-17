# From repository root; output directory, replicates (default 5), years (default 18).
suppressPackageStartupMessages(library(FIMS))
files <- file.path("tests/integration/process-recovery", c("benchmark.R", "sensitivity.R", "information.R", "calibration.R", "robustness.R", "mode-handoff.R", "dm-validation.R"))
for (file in files) source(file)
args <- commandArgs(trailingOnly = TRUE)
output <- if (length(args)) args[[1L]] else "/tmp/fims-dm-validation"
replicates <- if (length(args) >= 2L) as.integer(args[[2L]]) else 5L
years <- if (length(args) >= 3L) as.integer(args[[3L]]) else 18L
if (dir.exists(output) && length(list.files(output, all.files = TRUE, no.. = TRUE))) stop("Use an empty output directory")
dir.create(output, recursive = TRUE, showWarnings = FALSE)
metadata <- list(source_md5 = tools::md5sum(c(files, "tests/integration/process-recovery/run-dm-validation.R")),
  production_md5 = tools::md5sum(c("inst/include/distributions/functors/dirichlet_multinomial_lpmf.hpp",
    "inst/include/interface/rcpp/rcpp_objects/rcpp_distribution.hpp", "R/composition_likelihood.R", "R/initialize_modules.R")),
  git_head = system2("git", c("rev-parse", "HEAD"), stdout = TRUE), session = capture.output(sessionInfo()))
result <- run_dm_validation(replicates, years, checkpoint = file.path(output, "checkpoint.rds"))
result$configuration <- c(result$configuration, metadata)
saveRDS(result, file.path(output, "result.rds"))
for (name in names(result$tables)) write.csv(result$tables[[name]], file.path(output, paste0(name, ".csv")), row.names = FALSE)
for (name in names(result$summaries)) write.csv(result$summaries[[name]], file.path(output, paste0(name, ".csv")), row.names = FALSE)
dput(result$configuration, file = file.path(output, "configuration.R"))
message("Results written to ", normalizePath(output))
