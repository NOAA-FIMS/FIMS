# Usage: profile.R NEW_OUTPUT_DIRECTORY
args <- commandArgs(TRUE)
output <- args[1]
dir.create(output, recursive = TRUE, showWarnings = FALSE)
library(FIMS)
options(rlib_message_verbosity = "quiet")
data("data_big", package = "FIMS")
frame <- FIMSFrame(data_big)
parameters <- setup_default_parameters(data = frame)
input <- initialize_fims(parameters, data = frame)
stopifnot(length(input$parameters$re) > 0L)
Rprof(file.path(output, "fit.Rprof"), interval = 0.01)
start <- proc.time()[["elapsed"]]
fit <- tryCatch(fit_fims(input), finally = Rprof(NULL))
fit_elapsed <- proc.time()[["elapsed"]] - start
cpu <- summaryRprof(file.path(output, "fit.Rprof"))
write.csv(cpu$by.total, file.path(output, "fit-cpu.csv"))
write.csv(cpu$by.self, file.path(output, "fit-self.csv"))
start <- proc.time()[["elapsed"]]
estimates <- get_estimates(fit)
report <- get_report(fit)
extract_elapsed <- proc.time()[["elapsed"]] - start
opt <- get_opt(fit)
obj <- get_obj(fit)
sd <- get_sdreport(fit)
timing <- get_run_time(fit)
metrics <- data.frame(
  metric = c("fixed_parameters", "random_parameters", "fit_elapsed_seconds",
    names(timing), "extract_elapsed_seconds", "objective", "optimizer_code",
    "max_abs_gradient", "positive_definite_hessian", "estimate_rows"),
  value = c(length(obj$par), length(obj$env$random), fit_elapsed,
    as.numeric(timing), extract_elapsed, opt$objective, opt$convergence,
    max(abs(obj$gr(opt$par))), isTRUE(sd$pdHess), nrow(estimates))
)
write.csv(metrics, file.path(output, "metrics.csv"), row.names = FALSE)
saveRDS(list(opt = opt, gradient = obj$gr(opt$par), report = report,
             estimates = estimates), file.path(output, "numerics.rds"))
writeLines(trimws(capture.output(sessionInfo()), which = "right"),
           file.path(output, "session.txt"))
print(metrics)
print(head(cpu$by.total, 15))
clear()
