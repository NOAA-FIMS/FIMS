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

options <- projection_script_arguments("fims-demo-projections-quadra.rds")
on.exit(FIMS::clear(), add = TRUE)
setup <- build_fims_demo_projection(options$projection_years)

parameters <- list(p = FIMS::get_fixed(), re = FIMS::get_random())

start <- proc.time()[["elapsed"]]
fit <- FIMS::fit_fims_quadra_joint(
  parameters$p,
  parameters$re,
  options$max_iterations,
  options$gradient_tolerance
)
elapsed <- proc.time()[["elapsed"]] - start

result <- list(
  backend = "quadra",
  objective_scope = "joint fixed-plus-random objective",
  projection_years = options$projection_years,
  initial_objective = unname(fit$initial_objective),
  objective = unname(fit$objective),
  gradient = unname(fit$gradient),
  gradient_norm = unname(fit$gradient_norm),
  converged = isTRUE(fit$converged),
  message = fit$message,
  iterations = fit$iterations,
  evaluations = fit$evaluations,
  elapsed_seconds = unname(elapsed),
  fixed = unname(fit$par),
  random = unname(fit$random),
  compact_tape_vertices = fit$compact_tape_vertices,
  compact_tape_bytes = fit$compact_tape_bytes
)

saveRDS(result, options$output)
cat(sprintf(
  paste0(
    "Quadra projection fit\n",
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
