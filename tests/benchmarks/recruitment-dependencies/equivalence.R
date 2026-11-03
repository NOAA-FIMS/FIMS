script <- sub("^--file=", "", grep("^--file=", commandArgs(), value = TRUE)[1])
source(file.path(dirname(script), "common.R"))
output <- commandArgs(TRUE)[1]
set_diagnostic()
context <- make_input(30)
deviations <- make_objective(context)
n <- length(deviations$par)
points <- list(rep(0, n), 0.2 * sin(seq_len(n)))
samples <- lapply(points, function(u) {
  report <- deviations$report(u)
  naa <- matrix(report$numbers_at_age[[1]], ncol = context$ages, byrow = TRUE)
  list(u = u, r = log(naa[2:context$years, 1]),
       value = deviations$fn(u), report = report)
})
set_diagnostic(centered = TRUE)
context$input$parameters$re <- samples[[1]]$r
centered <- make_objective(context)
checks <- lapply(samples, function(sample) {
  cr <- centered$report(sample$r)
  # Include every reported numeric quantity, including density contributions.
  c(value = relative_error(centered$fn(sample$r), sample$value),
    reports = relative_error(unlist(cr), unlist(sample$report)))
})
stopifnot(max(unlist(checks)) < 1e-9)

opt <- nlminb(centered$par, centered$fn, centered$gr,
              control = list(iter.max = 1000, eval.max = 2000, rel.tol = 1e-12))
r <- opt$par
for (iteration in seq_len(8)) {
  g <- centered$gr(r)
  if (max(abs(g)) < 1e-7) break
  candidate <- r - as.numeric(solve(centered$env$spHess(r), g))
  if (centered$fn(candidate) > centered$fn(r) + 1e-8) break
  r <- candidate
}
report <- centered$report(r)
sb <- report$spawning_biomass[[1]][seq_len(n)]
phi0 <- report$unfished_spawning_biomass[[1]][1] / context$rzero
h <- 0.75 # fixed default steepness in this experiment
expected <- (0.8 * context$rzero * h * sb) /
  (0.2 * context$rzero * phi0 * (1 - h) + sb * (h - 0.2))
u <- r - log(expected)
center_value <- centered$fn(r)
center_hessian <- centered$env$spHess(r)
set_diagnostic()
dev_value <- deviations$fn(u)
dev_hessian <- deviations$env$spHess(u)
logdet_error <- abs(as.numeric(determinant(center_hessian, logarithm = TRUE)$modulus) -
                     as.numeric(determinant(dev_hessian, logarithm = TRUE)$modulus))
mode_errors <- c(value = abs(center_value - dev_value),
                 logdet = logdet_error,
                 gradient_center = max(abs(centered$gr(r))),
                 gradient_deviations = max(abs(deviations$gr(u))))
print(checks)
print(mode_errors)
stopifnot(all(mode_errors < 1e-5))
saveRDS(list(point_checks = checks, mode_checks = mode_errors), output)
clear()
