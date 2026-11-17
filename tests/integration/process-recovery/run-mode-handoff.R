# From repository root. One controlled difficult dataset; not a coverage study.
suppressPackageStartupMessages(library(FIMS))
files <- file.path("tests/integration/process-recovery", c("benchmark.R", "sensitivity.R", "information.R", "calibration.R", "robustness.R", "mode-handoff.R"))
for (file in files) source(file)
args <- commandArgs(trailingOnly = TRUE)
output <- if (length(args)) args[[1L]] else "/tmp/fims-mode-handoff"
if (dir.exists(output) && length(list.files(output, all.files = TRUE, no.. = TRUE))) stop("Use an empty output directory")
dir.create(output, recursive = TRUE, showWarnings = FALSE)
metadata <- list(years = 18L, process_seed = 4301L, scenario = "strong composition overdispersion",
  concentration = 200, start_sd = .2, initial_rule = "proper normal",
  source_md5 = tools::md5sum(files),
  driver_md5 = tools::md5sum("tests/integration/process-recovery/run-mode-handoff.R"),
  production_md5 = tools::md5sum(c("inst/include/distributions/functors/dirichlet_multinomial_lpmf.hpp",
    "inst/include/interface/rcpp/rcpp_objects/rcpp_distribution.hpp", "R/composition_likelihood.R", "R/initialize_modules.R")), git_head = system2("git", c("rev-parse", "HEAD"), stdout = TRUE),
  session = capture.output(sessionInfo()))
fixture <- recovery_fixture(18)
paired <- robustness_operating_models(fixture, 4301)
operating <- paired$models$strong
raw <- get_data(operating$data)
spec <- list()
for (fleet in unique(raw$fleet[raw$type %in% c("age_comp", "length_comp")])) {
  types <- unique(raw$type[raw$fleet %in% fleet & raw$type %in% c("age_comp", "length_comp")])
  spec[[fleet]] <- stats::setNames(lapply(types, function(type)
    list(type = "dirichlet_multinomial", concentration = 200, estimation_type = "constant")), types)
}
results <- list()
for (case in c("multinomial", "matched_dm")) {
  message("case: ", case)
  composition <- if (case == "matched_dm") spec else NULL
  fitted <- mode_fit_snapshot(fixture, operating, .2, composition)
  reports <- lapply(c(FALSE, TRUE), function(warm) mode_report_snapshot(fixture, operating, fitted$snapshot, warm))
  results[[case]] <- list(fitted = fitted, reports = reports)
}
saveRDS(list(metadata = metadata, data = raw, truth = operating$truth, results = results), file.path(output, "result.rds"))
diagnostics <- dplyr::bind_rows(lapply(names(results), function(case) dplyr::bind_rows(lapply(results[[case]]$reports, function(x) cbind(case = case, x$diagnostics)))))
metrics <- dplyr::bind_rows(lapply(names(results), function(case) dplyr::bind_rows(lapply(results[[case]]$reports, function(x)
  if (nrow(x$state_metrics)) cbind(case = case, warm = x$diagnostics$warm, x$state_metrics) else NULL))))
points <- dplyr::bind_rows(lapply(names(results), function(case) cbind(case = case, results[[case]]$fitted$point)))
write.csv(diagnostics, file.path(output, "diagnostics.csv"), row.names = FALSE)
write.csv(metrics, file.path(output, "state-metrics.csv"), row.names = FALSE)
write.csv(points, file.path(output, "points.csv"), row.names = FALSE)
dput(metadata, file = file.path(output, "configuration.R"))
message("Results written to ", normalizePath(output))
