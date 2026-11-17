# Repository root; baseline RDS from run-dm-validation.R, empty output directory.
suppressPackageStartupMessages(library(FIMS))
files <- file.path("tests/integration/process-recovery", c("benchmark.R", "sensitivity.R", "information.R",
  "calibration.R", "robustness.R", "mode-handoff.R", "dm-validation.R", "dm-profiles.R"))
for (file in files) source(file)
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) == 2L)
baseline_path <- args[[1L]]; output <- args[[2L]]
baseline <- readRDS(baseline_path)
for (field in c("source_md5", "production_md5")) {
  hashes <- baseline$configuration[[field]]
  stopifnot(length(hashes) > 0L, identical(unname(hashes), unname(tools::md5sum(names(hashes)))))
}
if (dir.exists(output) && length(list.files(output, all.files = TRUE, no.. = TRUE))) stop("Use an empty output directory")
dir.create(output, recursive = TRUE, showWarnings = FALSE)
metadata <- list(baseline_md5 = tools::md5sum(baseline_path),
  baseline_git = baseline$configuration$git_head, source_md5 = tools::md5sum(c(files,
    "tests/integration/process-recovery/run-dm-profiles.R")),
  production_md5 = baseline$configuration$production_md5,
  git_head = system2("git", c("rev-parse", "HEAD"), stdout = TRUE), session = capture.output(sessionInfo()))
result <- run_dm_profiles(baseline, checkpoint = file.path(output, "checkpoint.rds"))
result$configuration <- c(result$configuration, metadata)
saveRDS(result, file.path(output, "result.rds"))
for (name in names(result$tables)) write.csv(result$tables[[name]], file.path(output, paste0(name, ".csv")), row.names = FALSE)
writeLines(sub("[[:blank:]]+$", "", capture.output(dput(result$configuration))), file.path(output, "configuration.R"))
message("Results written to ", normalizePath(output))
