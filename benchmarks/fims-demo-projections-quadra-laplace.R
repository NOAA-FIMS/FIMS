#!/usr/bin/env Rscript

if (!requireNamespace("FIMS", quietly = TRUE)) {
  stop("Install FIMS before running this script.")
}
library(FIMS)

common_candidates <- c(
  file.path("benchmarks", "fims-demo-projections-common.R"),
  "fims-demo-projections-common.R"
)
common <- common_candidates[file.exists(common_candidates)]
if (!length(common)) stop("Cannot find fims-demo-projections-common.R.")
source(common[[1]])

options <- projection_script_arguments("fims-demo-projections-quadra-laplace.rds")
on.exit(FIMS::clear(), add = TRUE)
setup <- build_fims_demo_projection(options$projection_years)
parameters <- setup$initialized$parameters

start <- proc.time()[["elapsed"]]
fit <- FIMS::EvaluateQuadraLaplaceModel(parameters$p, parameters$re)
elapsed <- proc.time()[["elapsed"]] - start
validation <- FIMS::EvaluateQuadraModel(parameters$p, fit$random)
validated_random_gradient_norm <- sqrt(sum(validation$random_gradient^2))

result <- list(
  backend = "quadra",
  objective_scope = "single Laplace-profiled random-effects evaluation",
  projection_years = options$projection_years,
  initial_objective = unname(fit$objective),
  objective = unname(fit$objective),
  joint_objective = unname(fit$joint_objective),
  gradient = unname(validation$random_gradient),
  gradient_norm = unname(validated_random_gradient_norm),
  workspace_gradient_norm = unname(fit$random_gradient_norm),
  converged = isTRUE(fit$converged) &&
    validated_random_gradient_norm <= options$gradient_tolerance,
  message = if (validated_random_gradient_norm <= options$gradient_tolerance) {
    fit$message
  } else {
    "Laplace workspace mode failed validation against the full joint gradient"
  },
  iterations = fit$random_newton_iterations,
  evaluations = 1L,
  elapsed_seconds = unname(elapsed),
  fixed = unname(parameters$p),
  random = unname(fit$random),
  factorization = fit$factorization,
  tape_rebuilt = fit$tape_rebuilt,
  tape_rebuild_count = fit$tape_rebuild_count,
  logdet_ok = fit$logdet_ok,
  random_step_norm = fit$random_step_norm
)

saveRDS(result, options$output)
cat(sprintf(
  paste0(
    "Quadra Laplace projection evaluation\n",
    "  years: %d\n  objective: %.12g\n  gradient norm: %.6g\n",
    "  factorization: %s\n  converged: %s\n  elapsed: %.3f s\n",
    "  output: %s\n"
  ),
  result$projection_years,
  result$objective,
  result$gradient_norm,
  result$factorization$backend,
  result$converged,
  result$elapsed_seconds,
  normalizePath(options$output, mustWork = FALSE)
))
