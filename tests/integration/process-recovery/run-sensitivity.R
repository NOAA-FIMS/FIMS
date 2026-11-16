# Run from repository root. Optional: output directory, replicate count.
suppressPackageStartupMessages(library(FIMS))
source("tests/integration/process-recovery/benchmark.R")
source("tests/integration/process-recovery/sensitivity.R")
args <- commandArgs(trailingOnly = TRUE)
output <- if (length(args)) args[[1]] else "/tmp/fims-process-sensitivity"
replicates <- if (length(args) >= 2L) as.integer(args[[2]]) else 3L
if (dir.exists(output) && length(list.files(output, all.files = TRUE, no.. = TRUE)))
  stop("Use an empty output directory to preserve earlier results.")
dir.create(output, recursive = TRUE, showWarnings = FALSE)
configuration <- list(source_md5 = tools::md5sum(c(
  "tests/integration/process-recovery/benchmark.R", "tests/integration/process-recovery/sensitivity.R")),
  git_head = system2("git", c("rev-parse", "HEAD"), stdout = TRUE))
result <- run_sensitivity_benchmark(replicates = replicates,
  checkpoint = file.path(output, "checkpoint.rds"))
result$configuration <- c(result$configuration, configuration)
saveRDS(result, file.path(output, "result.rds"))
for (name in names(result$tables)) write.csv(result$tables[[name]], file.path(output,
  paste0(name, ".csv")), row.names = FALSE)
dput(result$configuration, file = file.path(output, "configuration.R"))
message("Results written to ", normalizePath(output))
