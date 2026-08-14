# standalone xptr creators ----

expect_externalptr <- function(x) {
  expect_equal(typeof(x), "externalptr")
}

test_that("standalone XPtr creators return concrete and base pointers", {
  fims_namespace <- asNamespace("FIMS")
  create_age_comp <- get("create_age_comp_", fims_namespace)
  age_comp_to_fims_xptr <- get("age_comp_to_fims_xptr_", fims_namespace)
  create_length_comp <- get("create_length_comp_", fims_namespace)
  length_comp_to_fims_xptr <- get("length_comp_to_fims_xptr_", fims_namespace)
  create_catch <- get("create_catch_", fims_namespace)
  catch_to_fims_xptr <- get("catch_to_fims_xptr_", fims_namespace)
  create_index <- get("create_index_", fims_namespace)
  index_to_fims_xptr <- get("index_to_fims_xptr_", fims_namespace)
  create_fleet <- get("create_fleet_", fims_namespace)
  fleet_to_fims_xptr <- get("fleet_to_fims_xptr_", fims_namespace)
  create_ewaa_growth <- get("create_ewaa_growth_", fims_namespace)
  ewaa_growth_to_fims_xptr <- get("ewaa_growth_to_fims_xptr_", fims_namespace)
  create_logistic_maturity <- get("create_logistic_maturity_", fims_namespace)
  logistic_maturity_to_fims_xptr <- get(
    "logistic_maturity_to_fims_xptr_",
    fims_namespace
  )
  create_logistic_selectivity <- get(
    "create_logistic_selectivity_",
    fims_namespace
  )
  logistic_selectivity_to_fims_xptr <- get(
    "logistic_selectivity_to_fims_xptr_",
    fims_namespace
  )
  create_double_logistic_selectivity <- get(
    "create_double_logistic_selectivity_",
    fims_namespace
  )
  double_logistic_selectivity_to_fims_xptr <- get(
    "double_logistic_selectivity_to_fims_xptr_",
    fims_namespace
  )
  create_beverton_holt_recruitment <- get(
    "create_beverton_holt_recruitment_",
    fims_namespace
  )
  beverton_holt_recruitment_to_fims_xptr <- get(
    "beverton_holt_recruitment_to_fims_xptr_",
    fims_namespace
  )
  create_log_devs_recruitment_process <- get(
    "create_log_devs_recruitment_process_",
    fims_namespace
  )
  log_devs_recruitment_process_to_fims_xptr <- get(
    "log_devs_recruitment_process_to_fims_xptr_",
    fims_namespace
  )
  create_log_r_recruitment_process <- get(
    "create_log_r_recruitment_process_",
    fims_namespace
  )
  log_r_recruitment_process_to_fims_xptr <- get(
    "log_r_recruitment_process_to_fims_xptr_",
    fims_namespace
  )
  create_dnorm_distribution <- get("create_dnorm_distribution_", fims_namespace)
  dnorm_distribution_to_fims_xptr <- get(
    "dnorm_distribution_to_fims_xptr_",
    fims_namespace
  )
  create_dlnorm_distribution <- get(
    "create_dlnorm_distribution_",
    fims_namespace
  )
  dlnorm_distribution_to_fims_xptr <- get(
    "dlnorm_distribution_to_fims_xptr_",
    fims_namespace
  )
  create_dmultinom_distribution <- get(
    "create_dmultinom_distribution_",
    fims_namespace
  )
  dmultinom_distribution_to_fims_xptr <- get(
    "dmultinom_distribution_to_fims_xptr_",
    fims_namespace
  )
  create_catch_at_age <- get("create_catch_at_age_", fims_namespace)
  catch_at_age_to_fims_xptr <- get(
    "catch_at_age_to_fims_xptr_",
    fims_namespace
  )

  age_comp <- create_age_comp(3, 2)
  expect_externalptr(age_comp)
  expect_externalptr(age_comp_to_fims_xptr(age_comp))

  length_comp <- create_length_comp(3, 2)
  expect_externalptr(length_comp)
  expect_externalptr(length_comp_to_fims_xptr(length_comp))

  catch <- create_catch(3)
  expect_externalptr(catch)
  expect_externalptr(catch_to_fims_xptr(catch))

  index <- create_index(3)
  expect_externalptr(index)
  expect_externalptr(index_to_fims_xptr(index))

  fleet <- create_fleet()
  expect_externalptr(fleet)
  expect_externalptr(fleet_to_fims_xptr(fleet))

  growth <- create_ewaa_growth()
  expect_externalptr(growth)
  expect_externalptr(ewaa_growth_to_fims_xptr(growth))

  maturity <- create_logistic_maturity()
  expect_externalptr(maturity)
  expect_externalptr(logistic_maturity_to_fims_xptr(maturity))

  logistic_selectivity <- create_logistic_selectivity()
  expect_externalptr(logistic_selectivity)
  expect_externalptr(logistic_selectivity_to_fims_xptr(logistic_selectivity))

  double_logistic_selectivity <- create_double_logistic_selectivity()
  expect_externalptr(double_logistic_selectivity)
  expect_externalptr(
    double_logistic_selectivity_to_fims_xptr(double_logistic_selectivity)
  )

  recruitment <- create_beverton_holt_recruitment()
  expect_externalptr(recruitment)
  expect_externalptr(beverton_holt_recruitment_to_fims_xptr(recruitment))

  log_devs <- create_log_devs_recruitment_process()
  expect_externalptr(log_devs)
  expect_externalptr(log_devs_recruitment_process_to_fims_xptr(log_devs))

  log_r <- create_log_r_recruitment_process()
  expect_externalptr(log_r)
  expect_externalptr(log_r_recruitment_process_to_fims_xptr(log_r))

  dnorm_distribution <- create_dnorm_distribution()
  expect_externalptr(dnorm_distribution)
  expect_externalptr(dnorm_distribution_to_fims_xptr(dnorm_distribution))

  dlnorm_distribution <- create_dlnorm_distribution()
  expect_externalptr(dlnorm_distribution)
  expect_externalptr(dlnorm_distribution_to_fims_xptr(dlnorm_distribution))

  dmultinom_distribution <- create_dmultinom_distribution()
  expect_externalptr(dmultinom_distribution)
  expect_externalptr(
    dmultinom_distribution_to_fims_xptr(dmultinom_distribution)
  )

  catch_at_age <- create_catch_at_age()
  expect_externalptr(catch_at_age)
  expect_externalptr(catch_at_age_to_fims_xptr(catch_at_age))

  clear()
})

test_that("standalone XPtr setters configure interface inputs", {
  fims_namespace <- asNamespace("FIMS")
  fims_function <- function(name) {
    get(name, fims_namespace)
  }
  assumed_known <- "assumed_known"

  age_comp <- fims_function("create_age_comp_")(2, 3)
  expect_no_error(
    fims_function("set_age_comp_data_")(age_comp, seq_len(6))
  )
  expect_no_error(
    fims_function("set_age_comp_uncertainty_")(age_comp, rep(0.1, 6))
  )
  expect_equal(fims_function("get_age_comp_id_")(age_comp), 1)

  length_comp <- fims_function("create_length_comp_")(2, 3)
  expect_no_error(
    fims_function("set_length_comp_data_")(length_comp, seq_len(6))
  )
  expect_no_error(
    fims_function("set_length_comp_uncertainty_")(length_comp, rep(0.1, 6))
  )

  catch <- fims_function("create_catch_")(2)
  expect_no_error(fims_function("set_catch_data_")(catch, c(1, 2)))
  expect_no_error(
    fims_function("set_catch_uncertainty_")(catch, c(0.1, 0.2))
  )

  index <- fims_function("create_index_")(2)
  expect_no_error(fims_function("set_index_data_")(index, c(1, 2)))
  expect_no_error(
    fims_function("set_index_uncertainty_")(index, c(0.1, 0.2))
  )

  fleet <- fims_function("create_fleet_")()
  expect_no_error(fims_function("set_fleet_dimensions_")(fleet, 2, 3, 4))
  expect_no_error(fims_function("set_fleet_name_")(fleet, "survey"))
  expect_no_error(
    fims_function("set_fleet_units_")(fleet, "weight", "numbers")
  )
  expect_no_error(
    fims_function("set_fleet_data_ids_")(fleet, 1, 2, 3, 4)
  )
  expect_no_error(fims_function("set_fleet_selectivity_id_")(fleet, 1))
  expect_no_error(
    fims_function("set_fleet_log_q_")(fleet, -1, assumed_known)
  )
  expect_no_error(
    fims_function("set_fleet_log_Fmort_")(
      fleet,
      c(-2, -2),
      assumed_known
    )
  )
  expect_no_error(
    fims_function("set_fleet_age_to_length_conversion_")(
      fleet,
      c(0.25, 0.75),
      assumed_known
    )
  )

  recruitment <- fims_function("create_beverton_holt_recruitment_")()
  expect_no_error(
    fims_function("set_beverton_holt_n_years_")(recruitment, 2)
  )
  expect_no_error(
    fims_function("set_beverton_holt_process_id_")(recruitment, 1)
  )
  for (setter in c(
    "set_beverton_holt_logit_steep_",
    "set_beverton_holt_log_rzero_",
    "set_beverton_holt_log_devs_",
    "set_beverton_holt_log_r_"
  )) {
    expect_no_error(
      fims_function(setter)(recruitment, c(0, 0), assumed_known)
    )
  }

  logistic_selectivity <- fims_function("create_logistic_selectivity_")()
  expect_no_error(
    fims_function("set_logistic_selectivity_inflection_point_")(
      logistic_selectivity,
      2,
      assumed_known
    )
  )
  expect_no_error(
    fims_function("set_logistic_selectivity_slope_")(
      logistic_selectivity,
      1,
      assumed_known
    )
  )

  double_logistic <- fims_function("create_double_logistic_selectivity_")()
  for (setter in c(
    "set_double_logistic_selectivity_inflection_point_asc_",
    "set_double_logistic_selectivity_slope_asc_",
    "set_double_logistic_selectivity_inflection_point_desc_",
    "set_double_logistic_selectivity_slope_desc_"
  )) {
    expect_no_error(
      fims_function(setter)(double_logistic, 1, assumed_known)
    )
  }

  growth <- fims_function("create_ewaa_growth_")()
  expect_no_error(
    fims_function("set_ewaa_growth_data_")(
      growth,
      c(1, 2),
      c(0.5, 1),
      2
    )
  )

  maturity <- fims_function("create_logistic_maturity_")()
  expect_no_error(
    fims_function("set_logistic_maturity_inflection_point_")(
      maturity,
      2,
      assumed_known
    )
  )
  expect_no_error(
    fims_function("set_logistic_maturity_slope_")(
      maturity,
      1,
      assumed_known
    )
  )

  dnorm <- fims_function("create_dnorm_distribution_")()
  for (setter in c(
    "set_dnorm_observed_values_",
    "set_dnorm_expected_values_",
    "set_dnorm_expected_mean_",
    "set_dnorm_log_sd_"
  )) {
    expect_no_error(fims_function(setter)(dnorm, c(0, 0), assumed_known))
  }
  expect_no_error(fims_function("set_dnorm_observed_data_id_")(dnorm, 1))
  expect_no_error(
    fims_function("set_dnorm_distribution_links_")(dnorm, "data", 1L)
  )
  expect_no_error(fims_function("set_dnorm_distribution_mean_")(dnorm, 0))

  dlnorm <- fims_function("create_dlnorm_distribution_")()
  for (setter in c(
    "set_dlnorm_observed_values_",
    "set_dlnorm_expected_values_",
    "set_dlnorm_log_sd_"
  )) {
    expect_no_error(fims_function(setter)(dlnorm, c(0, 0), assumed_known))
  }
  expect_no_error(fims_function("set_dlnorm_observed_data_id_")(dlnorm, 1))
  expect_no_error(
    fims_function("set_dlnorm_distribution_links_")(dlnorm, "data", 1L)
  )

  dmultinom <- fims_function("create_dmultinom_distribution_")()
  for (setter in c(
    "set_dmultinom_observed_values_",
    "set_dmultinom_expected_values_"
  )) {
    expect_no_error(
      fims_function(setter)(dmultinom, c(0.5, 0.5), assumed_known)
    )
  }
  expect_no_error(fims_function("set_dmultinom_dims_")(dmultinom, c(1, 2)))
  expect_no_error(
    fims_function("set_dmultinom_observed_data_id_")(dmultinom, 1)
  )
  expect_no_error(
    fims_function("set_dmultinom_distribution_links_")(
      dmultinom,
      "data",
      1L
    )
  )
  expect_no_error(fims_function("set_dmultinom_note_")(dmultinom, "age"))

  population <- fims_function("create_population_")()
  catch_at_age <- fims_function("create_catch_at_age_")()
  expect_no_error(
    fims_function("add_population_to_catch_at_age_")(
      catch_at_age,
      population
    )
  )
  expect_equal(
    fims_function("get_catch_at_age_reporting_")(catch_at_age),
    FALSE
  )

  expect_error(
    fims_function("set_fleet_log_q_")(
      fleet,
      c(-1, -1),
      c("assumed_known", "fixed_effects", "random_effects")
    ),
    "must be 1 \\(broadcast\\) or equal"
  )

  clear()
})
