# Usage: measure.R YEARS all|selected OUTPUT
args <- commandArgs(TRUE)
years <- as.integer(args[1])
variant <- args[2]
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
  input
}

input <- make_input(years)
input$map <- lapply(input$parameters, function(x) factor(rep(NA, length(x))))
input$map$p <- factor(c(1L, rep(NA_integer_, length(input$parameters$p) - 1L)))
selected <- c("spawning_biomass", "biomass", "expected_recruitment",
              "catch_expected", "index_expected")
data <- if (variant == "all") list() else list(uncertainty_reports = as.list(selected))
obj <- TMB::MakeADFun(data = data, parameters = input$parameters, map = input$map,
                      random = NULL, DLL = "FIMS", silent = TRUE)
value <- obj$fn(obj$par)
gradient <- obj$gr(obj$par)
report <- obj$report(obj$par)
start <- proc.time()
sd <- TMB::sdreport(obj, getReportCovariance = FALSE)
time <- proc.time() - start
stopifnot(sd$pdHess, all(is.finite(sd$sd)))
saveRDS(list(years = years, variant = variant, value = value, gradient = gradient,
             report = report, uncertainty = summary(sd, "report"),
             parameter_covariance = sd$cov.fixed, time = time,
             session = sessionInfo()), args[3])
print(c(years = years, reported_values = length(sd$value),
        uncertainty_seconds = time[["elapsed"]]))
clear()
