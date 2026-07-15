#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = TRUE)
iterations <- if (length(args)) as.integer(args[[1]]) else 100L
if (is.na(iterations) || iterations < 1L) {
  stop("Usage: Rscript benchmarks/compare-tmb-quadra-without-wrappers.R [iterations]")
}

if (!requireNamespace("FIMS", quietly = TRUE)) {
  stop("Install FIMS before running this benchmark.")
}
library(FIMS)

helper <- file.path("tests", "testthat", "helper-integration-tests-setup-function.R")
fixture <- file.path("tests", "testthat", "fixtures", "integration_test_data.RData")
if (!file.exists(helper) || !file.exists(fixture)) {
  stop("Run this script from the FIMS repository root.")
}

source(helper)
load(fixture)

cat("Constructing the without-wrappers catch-at-age fixture...\n")
result <- setup_and_run_FIMS_without_wrappers(
  iter_id = 1,
  om_input_list = om_input_list,
  om_output_list = om_output_list,
  em_input_list = em_input_list,
  estimation_mode = FALSE,
  random_effects = c(recruitment = "log_devs"),
  compare_backends = TRUE
)

comparison <- result$backend_comparison
cat(sprintf(
  "Joint objective: TMB %.12f, Quadra %.12f, difference %.3g\n\n",
  comparison$tmb_joint_objective,
  comparison$quadra_joint_objective,
  comparison$difference
))

# Warm both inference-facing paths before collecting samples.
invisible(result$obj$fn(result$obj$par))
invisible(BenchmarkQuadraModel(5L))

tmb_times <- numeric(iterations)
for (i in seq_len(iterations)) {
  timing_parameters <- result$obj$par
  timing_parameters[[1]] <- timing_parameters[[1]] +
    if (i %% 2L) 1e-8 else -1e-8
  start <- proc.time()[["elapsed"]]
  invisible(result$obj$fn(timing_parameters))
  tmb_times[[i]] <- proc.time()[["elapsed"]] - start
}
quadra_times <- BenchmarkQuadraModel(iterations)

summarize_times <- function(x) {
  c(
    mean_ms = mean(x) * 1000,
    median_ms = median(x) * 1000,
    p05_ms = unname(quantile(x, 0.05)) * 1000,
    p95_ms = unname(quantile(x, 0.95)) * 1000,
    min_ms = min(x) * 1000,
    max_ms = max(x) * 1000
  )
}

summary_table <- rbind(
  TMB_Laplace = summarize_times(tmb_times),
  Quadra_joint_replay = summarize_times(quadra_times)
)
print(round(summary_table, 4))

speedup <- median(tmb_times) / median(quadra_times)
cat(sprintf("\nMedian Quadra speedup: %.2fx\n", speedup))
cat(
  "Timing scope: warmed TMB Laplace/profiled fn() versus warmed Quadra ",
  "joint-objective replay. Model construction and Quadra reverse pass are excluded.\n",
  sep = ""
)
