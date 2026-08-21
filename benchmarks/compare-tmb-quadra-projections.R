#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = TRUE)
iterations <- if (length(args) >= 1L) as.integer(args[[1]]) else 50L
projection_years <- if (length(args) >= 2L) {
  as.integer(strsplit(args[[2]], ",", fixed = TRUE)[[1]])
} else {
  c(0L, 5L, 10L, 25L)
}
replays_per_sample <- if (length(args) >= 3L) as.integer(args[[3]]) else 100L

if (is.na(iterations) || iterations < 1L ||
    !length(projection_years) || anyNA(projection_years) ||
    any(projection_years < 0L) || is.na(replays_per_sample) ||
    replays_per_sample < 1L) {
  stop(
    "Usage: Rscript benchmarks/compare-tmb-quadra-projections.R ",
    "[samples] [comma-separated projection years] [replays per sample]"
  )
}

if (!requireNamespace("FIMS", quietly = TRUE)) {
  stop("Install FIMS before running this benchmark.")
}
library(FIMS)

projection_test <- file.path("tests", "testthat", "test-projections-looped.R")
if (!file.exists(projection_test)) {
  stop("Run this script from the FIMS repository root.")
}

Sys.setenv(FIMS_PROJECTION_HELPER_ONLY = "true")
on.exit(Sys.unsetenv("FIMS_PROJECTION_HELPER_ONLY"), add = TRUE)
source(projection_test)

summarize_times <- function(x) {
  c(
    median_ms = median(x) * 1000,
    p05_ms = unname(quantile(x, 0.05)) * 1000,
    p95_ms = unname(quantile(x, 0.95)) * 1000
  )
}

run_projection_benchmark <- function(years) {
  projected_f <- rep(om_output[["f"]][om_input$nyr], years)
  result <- suppressWarnings(run_FIMS_projection_scenario(
    om_input = om_input,
    om_output = om_output,
    em_input = em_input,
    n_projection_years = years,
    projected_landings = rep(-999, years),
    projected_F = projected_f,
    estimate_projected_F = rep("constant", years),
    projected_index = rep(-999, years),
    ssb_ratio_target = NULL,
    backend_benchmark_iterations = iterations,
    backend_benchmark_replays = replays_per_sample
  ))$backend_benchmark

  tmb <- summarize_times(result$tmb_joint_times)
  quadra <- summarize_times(result$quadra_joint_times)
  memory <- result$quadra_graph_memory

  data.frame(
    projection_years = years,
    fixed_effects = result$fixed_effects,
    random_effects = result$random_effects,
    objective_difference = result$objective_difference,
    tmb_joint_median_ms = unname(tmb[["median_ms"]]),
    quadra_joint_median_ms = unname(quadra[["median_ms"]]),
    quadra_speed_ratio = unname(
      tmb[["median_ms"]] / quadra[["median_ms"]]
    ),
    tmb_joint_p05_ms = unname(tmb[["p05_ms"]]),
    tmb_joint_p95_ms = unname(tmb[["p95_ms"]]),
    quadra_joint_p05_ms = unname(quadra[["p05_ms"]]),
    quadra_joint_p95_ms = unname(quadra[["p95_ms"]]),
    quadra_vertices = memory$vertex_count,
    quadra_reserved_mib = memory$total_tracked_reserved_bytes / 1024^2,
    stringsAsFactors = FALSE
  )
}

cat(sprintf(
  paste0(
    "Matched warmed joint-objective replay (%d samples x %d replays ",
    "per horizon).\n"
  ),
  iterations,
  replays_per_sample
))
results <- do.call(rbind, lapply(projection_years, run_projection_benchmark))
print(results, row.names = FALSE, digits = 6)

if (any(abs(results$objective_difference) > 1e-8)) {
  stop("TMB and Quadra joint objectives differ by more than 1e-8.")
}

cat(
  "\nScope: model construction, gradients, optimization, Laplace profiling, ",
  "and reporting are excluded from both replay timings.\n",
  sep = ""
)
