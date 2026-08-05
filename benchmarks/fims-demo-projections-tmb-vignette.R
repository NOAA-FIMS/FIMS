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

options <- projection_script_arguments("fims-demo-projections-tmb-vignette.rds")
on.exit(FIMS::clear(), add = TRUE)
setup <- build_fims_demo_projection(options$projection_years)

start <- proc.time()[["elapsed"]]
fit <- FIMS::fit_fims(
  setup$initialized,
  get_sd = TRUE,
  number_of_loops = 3L,
  control = list(
    eval.max = max(1000L, 5L * options$max_iterations),
    iter.max = options$max_iterations,
    trace = 0
  )
)
elapsed <- proc.time()[["elapsed"]] - start
opt <- FIMS::get_opt(fit)
gradient <- FIMS::get_gradient(fit)
gradient_norm <- sqrt(sum(gradient^2))

result <- list(
  backend = "tmb",
  objective_scope = "vignette fit_fims Laplace objective plus sdreport",
  projection_years = options$projection_years,
  initial_objective = NA_real_,
  objective = unname(opt$objective),
  gradient = unname(gradient),
  gradient_norm = unname(gradient_norm),
  converged = identical(opt$convergence, 0L) &&
    is.finite(gradient_norm) && gradient_norm <= options$gradient_tolerance,
  optimizer_convergence = opt$convergence,
  message = opt$message,
  iterations = unname(opt$iterations),
  evaluations = unname(opt$evaluations),
  elapsed_seconds = unname(elapsed),
  fixed = unname(opt$par)
)

saveRDS(result, options$output)
cat(sprintf(
  paste0(
    "TMB vignette projection fit\n",
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
