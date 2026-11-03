# Usage: measure.R YEARS PHASE OUTPUT; select an installed FIMS using R_LIBS.
args <- commandArgs(TRUE)
years <- as.integer(args[1])
phase <- args[2]
library(FIMS)
options(rlib_message_verbosity = "quiet")
make_input <- function(years) {
  data("data_big", package = "FIMS", envir = environment())
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
  frame <- FIMSFrame(do.call(rbind, c(list(fixed), observations, weights)))
  parameters <- setup_default_parameters(data = frame)
  input <- initialize_fims(parameters, data = frame)
  # A single fixed effect keeps delta-method comparisons affordable at scale.
  # All other parameters, including recruitment deviations, are held constant.
  input$map <- lapply(input$parameters, function(x) factor(rep(NA, length(x))))
  input$map$p <- factor(c(1L, rep(NA_integer_, length(input$parameters$p) - 1L)))
  input
}
input <- make_input(years)
make <- function(adreport = FALSE) TMB::MakeADFun(data = list(),
  parameters = input$parameters, map = input$map, random = NULL,
  DLL = "FIMS", silent = TRUE, ADreport = adreport)
start <- proc.time()
obj <- make()
recording <- proc.time() - start
p <- obj$par
stopifnot(length(p) == 1L)
output <- list(years = years, phase = phase, recording = recording,
               parameter = p, value = obj$fn(p), gradient = obj$gr(p))
start <- proc.time()
if (phase == "report") {
  output$reports <- lapply(c(0, 0.01, 0), function(delta) obj$report(p + delta))
  stopifnot(identical(output$reports[[1]], output$reports[[3]]))
} else if (phase == "uncertainty") {
  sd <- TMB::sdreport(obj, getReportCovariance = FALSE)
  output$uncertainty <- summary(sd, "report")
  output$pdHess <- sd$pdHess
  stopifnot(sd$pdHess, all(is.finite(output$uncertainty)))
} else if (phase != "recording") stop("Unknown phase")
output$phase_time <- proc.time() - start
output$session <- sessionInfo()
saveRDS(output, args[3])
print(c(years = years, recording_seconds = recording[["elapsed"]],
        phase_seconds = output$phase_time[["elapsed"]]))
clear()
