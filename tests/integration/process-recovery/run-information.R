# From repository root; optional output directory, replicates, years.
suppressPackageStartupMessages(library(FIMS))
files <- file.path("tests/integration/process-recovery", c("benchmark.R", "sensitivity.R", "information.R"))
for (file in files) source(file)
args <- commandArgs(trailingOnly = TRUE)
output <- if (length(args)) args[[1]] else "/tmp/fims-observation-information"
replicates <- if (length(args) >= 2L) as.integer(args[[2]]) else 3L
years <- if (length(args) >= 3L) as.integer(args[[3]]) else 18L
if (dir.exists(output) && length(list.files(output, all.files = TRUE, no.. = TRUE)))
  stop("Use an empty output directory to preserve earlier results.")
dir.create(output, recursive = TRUE, showWarnings = FALSE)
metadata <- list(source_md5 = tools::md5sum(files),
  git_head = system2("git", c("rev-parse", "HEAD"), stdout = TRUE))
result <- run_information_benchmark(years, replicates, checkpoint = file.path(output, "checkpoint.rds"))
result$configuration <- c(result$configuration, metadata)
saveRDS(result, file.path(output, "result.rds"))
for (name in names(result$tables)) write.csv(result$tables[[name]],
  file.path(output, paste0(name, ".csv")), row.names = FALSE)
dput(result$configuration, file = file.path(output, "configuration.R"))
message("Results written to ", normalizePath(output))
