# Usage: summarize-full-fit.R SCRATCH_ROOT OUTPUT_DIRECTORY
args <- commandArgs(TRUE)
root <- args[1]
out <- args[2]
dir.create(out, recursive = TRUE, showWarnings = FALSE)
runs <- read.csv(file.path(root, "runs.csv"))
stopifnot(nrow(runs) == 4L, all(runs$exit_code == 0))
reference <- readRDS(file.path(root, runs$label[1], "numerics.rds"))
checks <- list()
metrics <- list()
for (label in runs$label) {
  values <- readRDS(file.path(root, label, "numerics.rds"))
  quantities <- list(
    parameters = list(values$parameters, reference$parameters, 1e-10),
    configuration = list(values$config, reference$config, 0),
    objective = list(values$opt$objective, reference$opt$objective, 1e-8),
    optimum_parameters = list(values$opt$par, reference$opt$par, 1e-5),
    report = list(values$report, reference$report, 1e-5),
    estimates = list(values$estimates$estimated, reference$estimates$estimated, 1e-5),
    covariance = list(values$covariance, reference$covariance, 1e-5),
    uncertainty_values = list(values$uncertainty_values, reference$uncertainty_values, 1e-5),
    uncertainty_sd = list(values$uncertainty_sd, reference$uncertainty_sd, 1e-5)
  )
  for (name in names(quantities)) {
    q <- quantities[[name]]
    comparison <- all.equal(q[[1]], q[[2]], tolerance = q[[3]])
    checks[[length(checks) + 1L]] <- data.frame(
      label,
      quantity = name, tolerance = q[[3]], passed = isTRUE(comparison),
      detail = paste(comparison, collapse = "; ")
    )
  }
  metrics[[label]] <- cbind(label, read.csv(file.path(root, label, "metrics.csv")))
}
checks <- do.call(rbind, checks)
rows <- merge(runs, do.call(rbind, metrics), by = "label")
summary <- aggregate(
  cbind(
    wall_seconds, peak_rss_mib, initialization_seconds, fit_seconds, pipeline_seconds,
    optimization_seconds, uncertainty_seconds
  ) ~ variant,
  rows, median
)
write.csv(checks, file.path(out, "full-fit-validation.csv"), row.names = FALSE)
write.csv(rows, file.path(out, "full-fit-runs.csv"), row.names = FALSE)
write.csv(summary, file.path(out, "full-fit-summary.csv"), row.names = FALSE)
print(summary)
print(table(checks$passed))
stopifnot(all(checks$passed))
