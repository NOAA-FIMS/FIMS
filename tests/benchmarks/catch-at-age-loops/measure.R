# Run in a fresh process with R_LIBS selecting an isolated FIMS installation.
args <- commandArgs(TRUE)
library(FIMS)
data("data_big", package = "FIMS")
frame <- FIMSFrame(data_big)
parameters <- setup_default_parameters(data = frame)
parameters$estimation_type[parameters$label == "log_M"] <- "fixed_effects"
input <- initialize_fims(parameters, data = frame)
make <- function() TMB::MakeADFun(data = list(), parameters = input$parameters,
  map = input$map, random = NULL, DLL = "FIMS", silent = TRUE)
measure <- function(f, n) {
  median(replicate(5, {
    start <- proc.time()
    for (i in seq_len(n)) f(i)
    delta <- proc.time() - start
    sum(delta[c("user.self", "sys.self")]) / n
  }))
}
obj <- make()
p <- obj$par
points <- list(p, p + 0.01, p - 0.01, p)
checks <- lapply(points, function(x) list(value = obj$fn(x),
  gradient = obj$gr(x), report = obj$report(x)))
recording <- measure(function(i) {
  temporary <- make()
  TMB::FreeADFun(temporary)
}, 5)
objective <- measure(function(i) obj$fn(p + (i %% 2) * 1e-7), 200)
gradient <- measure(function(i) obj$gr(p + (i %% 2) * 1e-7), 200)
report <- measure(function(i) obj$report(p + (i %% 2) * 1e-7), 100)
fit_time <- system.time(fit <- nlminb(p, obj$fn, obj$gr,
  control = list(iter.max = 10000, eval.max = 20000)))[["elapsed"]]
saveRDS(list(checks = checks, fit = fit,
  timing = c(recording = recording, objective = objective,
             gradient = gradient, report = report, fit_elapsed = fit_time),
  session = sessionInfo()), args[1])
print(c(recording = recording, objective = objective, gradient = gradient,
        report = report, fit_elapsed = fit_time, convergence = fit$convergence))
clear()
