testthat::skip_if(Sys.getenv("RUN_SLOW_TESTS") != "true",
                  "Skipping: RUN_SLOW_TESTS is not set to true.")

test_that("selective uncertainty preserves fitting and ordinary outputs", {
  on.exit(FIMS::clear(), add = TRUE)
  data("data_big", package = "FIMS")
  frame <- FIMS::FIMSFrame(data_big)
  parameters <- FIMS::setup_default_parameters(data = frame)
  parameters$estimation_type <- "constant"
  parameters$estimation_type[parameters$label == "log_rzero"] <- "fixed_effects"
  parameters$estimation_type[parameters$label == "log_devs"] <- "random_effects"
  input <- FIMS::initialize_fims(parameters, data = frame)
  all_fit <- FIMS::fit_fims(input, number_of_loops = 0)
  selection <- c("index_expected", "spawning_biomass", "spawning_biomass")
  selected <- FIMS::fit_fims(input, number_of_loops = 0,
                            uncertainty_reports = selection)
  none <- FIMS::fit_fims(input, number_of_loops = 0,
                        uncertainty_reports = character(0))
  all_sd <- FIMS::get_sdreport(all_fit)
  selected_sd <- FIMS::get_sdreport(selected)
  none_sd <- FIMS::get_sdreport(none)
  keep <- names(all_sd$value) %in% selection
  expect_true(all_sd$pdHess)
  expect_equal(selected_sd$value, all_sd$value[keep], tolerance = 1e-10)
  expect_equal(selected_sd$sd, all_sd$sd[keep], tolerance = 1e-8)
  expect_length(none_sd$value, 0)
  selected_cov <- TMB::sdreport(FIMS::get_obj(selected), getReportCovariance = TRUE)
  expect_equal(dim(selected_cov$cov), rep(length(selected_sd$value), 2))
  expect_equal(as.numeric(diag(selected_cov$cov)), as.numeric(selected_sd$sd)^2,
               tolerance = 1e-8)
  expect_equal(selected_sd$cov.fixed, all_sd$cov.fixed, tolerance = 1e-10)
  expect_equal(none_sd$cov.fixed, all_sd$cov.fixed, tolerance = 1e-10)
  expect_equal(summary(selected_sd, "random"), summary(all_sd, "random"),
               tolerance = 1e-8)
  expect_equal(summary(none_sd, "random"), summary(all_sd, "random"),
               tolerance = 1e-8)
  for (fit in list(selected, none)) {
    expect_equal(FIMS::get_opt(fit), FIMS::get_opt(all_fit), tolerance = 1e-10)
    expect_equal(FIMS::get_report(fit), FIMS::get_report(all_fit), tolerance = 1e-10)
    obj <- FIMS::get_obj(fit)
    reference <- FIMS::get_obj(all_fit)
    expect_equal(obj$fn(obj$par), reference$fn(obj$par), tolerance = 1e-10)
    expect_equal(obj$gr(obj$par), reference$gr(obj$par), tolerance = 1e-10)
  }
  estimates <- FIMS::get_estimates(selected)
  reference_estimates <- FIMS::get_estimates(all_fit)
  expect_equal(estimates$estimated, reference_estimates$estimated, tolerance = 1e-10)
  unselected <- estimates$estimation_type == "derived_quantity" &
    !estimates$label %in% selection
  expect_true(any(unselected))
  expect_true(all(is.na(estimates$uncertainty[unselected])))
  none_estimates <- FIMS::get_estimates(none)
  expect_equal(none_estimates$estimated, reference_estimates$estimated, tolerance = 1e-10)
  expect_true(all(is.na(none_estimates$uncertainty[
    none_estimates$estimation_type == "derived_quantity"])))
  # A second object's selection must not change an earlier object's AD reports.
  again <- TMB::sdreport(FIMS::get_obj(all_fit), getReportCovariance = FALSE)
  expect_equal(again$value, all_sd$value, tolerance = 1e-10)

  compact <- FIMS::compact_fims(selected, reports = "spawning_biomass")
  expect_equal(compact$estimates, estimates)
  expect_named(compact$report, "spawning_biomass")
  expect_equal(compact$report$spawning_biomass,
               FIMS::get_report(selected)$spawning_biomass)
  expect_equal(compact$opt, FIMS::get_opt(selected))
  expect_equal(compact$uncertainty_reports, unique(selection))
  expect_equal(as.numeric(FIMS::get_obj(selected)$fn(FIMS::get_opt(selected)$par)),
               FIMS::get_opt(selected)$objective, tolerance = 1e-10)
  has_live_reference <- function(x) {
    if (is.environment(x) || is.function(x) || typeof(x) == "externalptr") return(TRUE)
    if (is.list(x)) return(any(vapply(unclass(x), has_live_reference, logical(1))))
    FALSE
  }
  expect_false(has_live_reference(compact))
  expect_length(FIMS::compact_fims(selected, reports = character(0))$report, 0)
  expect_error(FIMS::compact_fims(selected, reports = "typo"), "Unknown report")
  expect_error(FIMS::compact_fims(selected, reports = NA_character_), "reports must")
  expect_error(FIMS::compact_fims(list()), "FIMSFit")
  for (bad in list(1, NA_character_, "", matrix("biomass"))) {
    expect_error(FIMS::fit_fims(input, uncertainty_reports = bad), "uncertainty_reports must")
  }
  expect_error(FIMS::fit_fims(input, uncertainty_reports = "biomas"),
               "Unknown uncertainty report.*biomas")
  FIMS::clear()
  expect_equal(unserialize(serialize(compact, NULL)), compact)
})
