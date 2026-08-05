#!/usr/bin/env Rscript

if (!requireNamespace("FIMS", quietly = TRUE) ||
    !requireNamespace("TMB", quietly = TRUE)) {
  stop("Install FIMS and TMB before running this script.")
}
library(FIMS)

common_candidates <- c(
  file.path("benchmarks", "fims-demo-projections-common.R"),
  "fims-demo-projections-common.R"
)
common <- common_candidates[file.exists(common_candidates)]
if (!length(common)) stop("Cannot find fims-demo-projections-common.R.")
source(common[[1]])

options <- projection_script_arguments("fims-demo-projections-tmb.rds")
on.exit(FIMS::clear(), add = TRUE)
setup <- build_fims_demo_projection(options$projection_years)

# No `random` argument is supplied: p and re are optimized together so this
# matches fit_fims_quadra_joint rather than TMB's Laplace-profiled objective.
obj <- TMB::MakeADFun(
  data = list(),
  parameters = setup$initialized$parameters,
  DLL = "FIMS",
  silent = TRUE
)
initial_objective <- obj$fn(obj$par)

start <- proc.time()[["elapsed"]]
fit <- stats::nlminb(
  start = obj$par,
  objective = obj$fn,
  gradient = obj$gr,
  control = list(
    iter.max = options$max_iterations,
    eval.max = max(1000L, 5L * options$max_iterations)
  )
)
elapsed <- proc.time()[["elapsed"]] - start
gradient <- as.numeric(obj$gr(fit$par))
gradient_norm <- sqrt(sum(gradient^2))
n_fixed <- length(setup$initialized$parameters$p)
fixed <- fit$par[seq_len(n_fixed)]
random <- fit$par[n_fixed + seq_along(setup$initialized$parameters$re)]

result <- list(
  backend = "tmb",
  objective_scope = "joint fixed-plus-random objective",
  projection_years = options$projection_years,
  initial_objective = unname(initial_objective),
  objective = unname(fit$objective),
  gradient = unname(gradient),
  gradient_norm = unname(gradient_norm),
  converged = identical(fit$convergence, 0L) &&
    is.finite(gradient_norm) && gradient_norm <= options$gradient_tolerance,
  optimizer_convergence = fit$convergence,
  message = fit$message,
  iterations = unname(fit$iterations),
  evaluations = unname(fit$evaluations),
  elapsed_seconds = unname(elapsed),
  fixed = unname(fixed),
  random = unname(random)
)

saveRDS(result, options$output)
cat(sprintf(
  paste0(
    "TMB projection fit\n",
    "  years: %d\n  objective: %.12g\n  gradient norm: %.6g\n",
    "  converged: %s\n  elapsed: %.3f s\n  output: %s\n"
  ),
  result$projection_years,
  result$objective,
  result$gradient_norm,
  result$converged,
  result$elapsed_seconds,
  normalizePath(options$output, mustWork = FALSE)
))
