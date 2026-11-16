# From the repository root:
# Rscript tests/integration/process-recovery/run.R /tmp/fims-recovery 3 6 4301
suppressPackageStartupMessages(library(FIMS))
source("tests/integration/process-recovery/benchmark.R")
args <- commandArgs(trailingOnly = TRUE)
output <- if (length(args)) args[[1L]] else "/tmp/fims-process-recovery"
replicates <- if (length(args) >= 2L) as.integer(args[[2L]]) else 3L
years <- if (length(args) >= 3L) as.integer(args[[3L]]) else 6L
seed <- if (length(args) >= 4L) as.integer(args[[4L]]) else 4301L
if (dir.exists(output) && length(list.files(output, all.files = TRUE, no.. = TRUE)))
  stop("Use an empty output directory to preserve earlier benchmark results.")
dir.create(output, recursive = TRUE, showWarnings = FALSE)
result <- run_recovery_benchmark(replicates, years, seed)
result$configuration$source_md5 <- tools::md5sum("tests/integration/process-recovery/benchmark.R")
result$configuration$git_head <- system2("git", c("rev-parse", "HEAD"), stdout = TRUE)
saveRDS(result, file.path(output, "result.rds"))
for (name in names(result$tables)) write.csv(result$tables[[name]],
  file.path(output, paste0(name, ".csv")), row.names = FALSE)
summary <- recovery_summaries(result)
for (name in names(summary)) write.csv(summary[[name]], file.path(output,
  paste0(name, ".csv")), row.names = FALSE)
dput(result$configuration, file = file.path(output, "configuration.R"))
message("Results written to ", normalizePath(output))
