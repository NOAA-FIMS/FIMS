args <- commandArgs(TRUE)
input <- args[1]
output <- args[2]
dir.create(output, recursive = TRUE, showWarnings = FALSE)
files <- list.files(input, "^[1-6]-(before|after)[.]rds$", full.names = TRUE)
stopifnot(length(files) == 6L)
runs <- lapply(files, readRDS)
reference <- runs[[1]]
validation <- lapply(seq_along(runs), function(i) {
  run <- runs[[i]]
  checks <- c(values_gradients_reports = isTRUE(all.equal(
    run$checks, reference$checks, tolerance = 1e-10)),
    fitted_parameters = isTRUE(all.equal(run$fit$par, reference$fit$par,
                                        tolerance = 1e-8)),
    fitted_objective = isTRUE(all.equal(run$fit$objective,
                                       reference$fit$objective, tolerance = 1e-10)),
    optimizer_status = identical(run$fit$convergence, reference$fit$convergence) &&
      identical(run$fit$message, reference$fit$message))
  stopifnot(all(checks))
  data.frame(run = basename(files[i]), check = names(checks), passed = checks)
})
write.csv(do.call(rbind, validation), file.path(output, "validation.csv"),
          row.names = FALSE)
timings <- do.call(rbind, lapply(seq_along(runs), function(i) {
  resource <- readLines(sub("[.]rds$", ".resources", files[i]))
  rss <- as.numeric(sub(" maximum resident set size.*", "",
                        trimws(grep("maximum resident set size", resource,
                                    value = TRUE)))) / 1024^2
  data.frame(run = basename(files[i]),
             variant = if (grepl("before", files[i])) "before" else "after",
             as.list(runs[[i]]$timing), peak_rss_mib = rss)
}))
write.csv(timings, file.path(output, "runs.csv"), row.names = FALSE)
metrics <- names(timings)[-(1:2)]
summary <- do.call(rbind, lapply(metrics, function(metric) {
  before <- median(timings[timings$variant == "before", metric])
  after <- median(timings[timings$variant == "after", metric])
  data.frame(metric, before, after, percent_change = 100 * (after / before - 1))
}))
write.csv(summary, file.path(output, "summary.csv"), row.names = FALSE)
writeLines(capture.output(runs[[1]]$session), file.path(output, "session.txt"))
print(summary)
