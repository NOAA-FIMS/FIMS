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

options <- projection_script_arguments("fims-demo-projections-tmb-laplace.rds")
on.exit(FIMS::clear(), add = TRUE)
setup <- build_fims_demo_projection(options$projection_years)

start <- proc.time()[["elapsed"]]
obj <- TMB::MakeADFun(
  data = list(),
  parameters = setup$initialized$parameters,
  random = "re",
  DLL = "FIMS",
  silent = TRUE
)
objective <- obj$fn(obj$par)
gradient <- as.numeric(obj$gr(obj$par))
elapsed <- proc.time()[["elapsed"]] - start
random_mode <- obj$env$last.par[obj$env$random]

result <- list(
  backend = "tmb",
  objective_scope = "single Laplace-profiled random-effects evaluation",
  projection_years = options$projection_years,
  initial_objective = unname(objective),
  objective = unname(objective),
  gradient = unname(gradient),
  gradient_norm = sqrt(sum(gradient^2)),
  converged = all(is.finite(c(objective, gradient))),
  message = "TMB marginal objective and gradient evaluated",
  iterations = NA_integer_,
  evaluations = 1L,
  elapsed_seconds = unname(elapsed),
  fixed = unname(obj$par),
  random = unname(random_mode)
)

saveRDS(result, options$output)
cat(sprintf(
  paste0(
    "TMB Laplace projection evaluation\n",
    "  years: %d\n  objective: %.12g\n  gradient norm: %.6g\n",
    "  elapsed: %.3f s\n  output: %s\n"
  ),
  result$projection_years,
  result$objective,
  result$gradient_norm,
  result$elapsed_seconds,
  normalizePath(options$output, mustWork = FALSE)
))
