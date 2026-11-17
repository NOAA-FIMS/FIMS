# Repository root: empty output directory, optional recovery replicates and moment draws.
suppressPackageStartupMessages(library(FIMS))
files <- file.path("tests/integration/process-recovery", c("benchmark.R", "sensitivity.R", "information.R",
  "calibration.R", "robustness.R", "mode-handoff.R", "dm-validation.R", "dm-profiles.R", "public-dm.R"))
for (file in files) source(file)
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) >= 1L, length(args) <= 3L)
output <- args[[1L]]
replicates <- if (length(args) >= 2L) as.integer(args[[2L]]) else 5L
moment_draws <- if (length(args) >= 3L) as.integer(args[[3L]]) else 1000L
if (dir.exists(output) && length(list.files(output, all.files = TRUE, no.. = TRUE))) stop("Use an empty output directory")
dir.create(output, recursive = TRUE, showWarnings = FALSE)
metadata <- list(source_md5 = tools::md5sum(c(files, "tests/integration/process-recovery/run-public-dm.R")),
  production_md5 = tools::md5sum(c("R/simulate_fims.R", "R/composition_likelihood.R", "R/initialize_modules.R",
    "inst/include/distributions/functors/dirichlet_multinomial_lpmf.hpp")),
  git_head = system2("git", c("rev-parse", "HEAD"), stdout = TRUE), session = capture.output(sessionInfo()))
moments <- public_dm_moments(moment_draws)
saveRDS(moments, file.path(output, "moments.rds"))
for (name in names(moments)) write.csv(moments[[name]], file.path(output, paste0(name, ".csv")), row.names = FALSE)
result <- run_public_dm_recovery(replicates, checkpoint = file.path(output, "checkpoint.rds"))
result$configuration <- c(result$configuration, list(moment_draws = moment_draws,
  moment_seed = 8801L, moment_years = 3L), metadata)
result$moments <- moments
saveRDS(result, file.path(output, "result.rds"))
for (name in names(result$tables)) write.csv(result$tables[[name]], file.path(output, paste0(name, ".csv")), row.names = FALSE)
for (name in names(result$summaries)) write.csv(result$summaries[[name]], file.path(output, paste0(name, ".csv")), row.names = FALSE)
writeLines(sub("[[:blank:]]+$", "", capture.output(dput(result$configuration))), file.path(output, "configuration.R"))
message("Results written to ", normalizePath(output))
