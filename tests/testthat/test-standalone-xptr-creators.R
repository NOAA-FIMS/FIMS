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
