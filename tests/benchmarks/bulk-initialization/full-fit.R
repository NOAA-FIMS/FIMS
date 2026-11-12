# Usage: workflow.R SCRATCH_ROOT YEARS fixed|growth OUTPUT_DIRECTORY
args <- commandArgs(TRUE)
root <- args[1]
years <- as.integer(args[2])
kind <- args[3]
output <- args[4]
library(FIMS)
options(rlib_message_verbosity = "quiet")
make_data <- function(years) {
  data_big <- readRDS(file.path(root, "canonical-data.rds"))
  fixed <- data_big[is.na(data_big$timing), ]
  observed <- data_big[!is.na(data_big$timing) & data_big$type != "weight_at_age", ]
  weight <- data_big[data_big$type == "weight_at_age", ]
  observations <- lapply(seq_len(years), function(y) {
    x <- observed[observed$timing == (y - 1L) %% 30L + 1L, ]
    x$timing <- y
    x
  })
  weights <- lapply(seq_len(years + 1L), function(y) {
    source_year <- if (y <= 31L) y else (y - 1L) %% 30L + 1L
    x <- weight[weight$timing == source_year, ]
    x$timing <- y
    x
  })
  do.call(rbind, c(list(fixed), observations, weights))
}

data <- make_data(years)
start <- proc.time()[["elapsed"]]
frame <- FIMSFrame(data)
t1 <- proc.time()[["elapsed"]]
parameters <- setup_default_parameters(data = frame)
if (kind == "growth") {
  parameters <- parameters |>
    dplyr::filter(module_name != "Growth") |>
    dplyr::bind_rows(setup_default_Growth(data = frame, module_type = "VonBertalanffySchnute"))
}
t2 <- proc.time()[["elapsed"]]
input <- initialize_fims(parameters, data = frame)
t3 <- proc.time()[["elapsed"]]
fit <- fit_fims(input,
  get_sd = kind == "fixed",
  number_of_loops = if (kind == "growth") 4 else 3
)
t4 <- proc.time()[["elapsed"]]
opt <- get_opt(fit)
report <- get_report(fit)
estimates <- get_estimates(fit)
sd <- get_sdreport(fit)
gradient <- get_gradient(fit)
t5 <- proc.time()[["elapsed"]]
stopifnot(opt$convergence == 0, max(abs(gradient)) < 0.01)
if (kind == "fixed") stopifnot(isTRUE(sd$pdHess), all(is.finite(sd$sd)))
config <- TMB::config(DLL = "FIMS")
stopifnot(!config$tmbad.sparse_hessian_compress)
write.csv(
  data.frame(years, kind,
    frame_seconds = t1 - start, parameters_seconds = t2 - t1,
    initialization_seconds = t3 - t2, fit_seconds = t4 - t3, extraction_seconds = t5 - t4,
    pipeline_seconds = t5 - start, optimization_seconds = as.numeric(get_run_time(fit)[["time_optimization"]], units = "secs"),
    uncertainty_seconds = as.numeric(get_run_time(fit)[["time_sdreport"]], units = "secs"),
    objective = opt$objective, max_gradient = max(abs(gradient)), optimizer_code = opt$convergence
  ),
  file.path(output, "metrics.csv"),
  row.names = FALSE
)
saveRDS(
  list(
    parameters = parameters, opt = opt, gradient = gradient, report = report, estimates = estimates,
    covariance = sd$cov.fixed, uncertainty_values = sd$value, uncertainty_sd = sd$sd, config = config
  ),
  file.path(output, "numerics.rds")
)
writeLines(capture.output(sessionInfo()), file.path(output, "session.txt"))
clear()
