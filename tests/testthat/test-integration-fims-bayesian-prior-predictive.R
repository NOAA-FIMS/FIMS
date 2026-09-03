load(testthat::test_path("fixtures", "integration_test_data.RData"))

# Set the iteration ID to 1 for accessing specific input/output list
iter_id <- 1

# Extract model input and output data for the specified iteration
om_input <- om_input_list[[iter_id]]
om_output <- om_output_list[[iter_id]]
em_input <- em_input_list[[iter_id]]

test_that("posterior equals prior with no data", {
  # This test sets up a model without data likelihood components. All parameters
  # without priors are fixed. Only selectivity parameters are estimated and given
  # priors, which are shared between the fishery and survey fleets. We run
  # Bayesian MCMC and expect the posterior means for the selectivity parameters
  # to match the prior means and the posterior variances to match the prior variances.

  # Set up fleet and survey without data distributions

  # Extract fishing fleet catch data (observed) and initialize index module
  catch <- em_input[["L.obs"]][["fleet1"]]
  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_index <- create_data("index", om_input[["nyr"]])
  set_data(fishing_fleet_index, catch)

  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the create function initializes the object with length nyr*nages
  # Here we fill in the values for the object with the observed age comps for fleet one
  # we multiply these proportions by the sample size for likelihood weighting
  fishing_fleet_age_comp <- create_data(
    "age_comp", om_input[["nyr"]], om_input[["nages"]]
  )
  set_data(
    fishing_fleet_age_comp,
    c(t(em_input[["L.age.obs"]][["fleet1"]])) * em_input[["n.L"]][["fleet1"]]
  )

  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- create_data(
    "length_comp", om_input[["nyr"]], om_input[["nlengths"]]
  )
  set_data(
    fishing_fleet_length_comp,
    c(t(em_input[["L.length.obs"]][["fleet1"]])) *
      em_input[["n.L.lengthcomp"]][["fleet1"]]
  )

  # Fleet
  # Create the fishing fleet
  # turn on estimation of inflection_point and slope
  fishing_fleet_selectivity <- create_selectivity("Logistic")
  set_variable_vector(
    fishing_fleet_selectivity, "inflection_point",
    om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]], "fixed_effects"
  )
  set_variable_vector(
    fishing_fleet_selectivity, "slope",
    om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]], "fixed_effects"
  )

  # Initialize the fishing fleet module and set its dimensions
  fishing_fleet <- create_fleet()
  set_fleet_constants(
    fishing_fleet,
    om_input[["nyr"]], om_input[["nages"]], om_input[["nlengths"]]
  )

  # Log-transform OM fishing mortality. Left assumed known, as before.
  set_variable_vector(
    fishing_fleet, "log_Fmort",
    log(om_output[["f"]][1:om_input[["nyr"]]]), "assumed_known"
  )
  set_variable_vector(fishing_fleet, "log_q", log(1.0), "assumed_known")
  set_fleet_selectivity(fishing_fleet, fishing_fleet_selectivity)
  set_fleet_observed_data(
    fishing_fleet,
    age_comp = fishing_fleet_age_comp,
    length_comp = fishing_fleet_length_comp,
    index = fishing_fleet_index
  )

  # Set age-to-length conversion matrix
  # TODO: If an age_to_length_conversion matrix is provided, the code below
  # still executes. Consider adding a check in the Rcpp interface to ensure
  # users provide a vector of inputs.
  set_variable_vector(
    fishing_fleet, "age_to_length_conversion",
    c(t(em_input[["age_to_length_conversion"]])), "assumed_known"
  )

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- create_data("index", om_input[["nyr"]])
  set_data(survey_fleet_index, survey_index)

  survey_fleet_age_comp <- create_data(
    "age_comp", om_input[["nyr"]], om_input[["nages"]]
  )
  set_data(
    survey_fleet_age_comp,
    c(t(em_input[["survey.age.obs"]][["survey1"]]) *
      em_input[["n.survey"]][["survey1"]])
  )

  survey_lengthcomp <- em_input[["survey.length.obs"]][["survey1"]]
  survey_fleet_length_comp <- create_data(
    "length_comp", om_input[["nyr"]], om_input[["nlengths"]]
  )
  set_data(
    survey_fleet_length_comp,
    c(t(em_input[["survey.length.obs"]][["survey1"]])) *
      em_input[["n.survey.lengthcomp"]][["survey1"]]
  )

  # Fleet
  # Create the survey fleet
  # turn on estimation of inflection_point and slope
  survey_fleet_selectivity <- create_selectivity("Logistic")
  set_variable_vector(
    survey_fleet_selectivity, "inflection_point",
    om_input[["sel_survey"]][["survey1"]][["A50.sel1"]], "fixed_effects"
  )
  set_variable_vector(
    survey_fleet_selectivity, "slope",
    om_input[["sel_survey"]][["survey1"]][["slope.sel1"]], "fixed_effects"
  )

  survey_fleet <- create_fleet()
  set_fleet_constants(
    survey_fleet,
    om_input[["nyr"]], om_input[["nages"]], om_input[["nlengths"]]
  )
  # Set very low survey fishing mortality
  set_variable_vector(
    survey_fleet, "log_Fmort",
    rep(-200, om_input[["nyr"]]), "assumed_known"
  )
  set_variable_vector(
    survey_fleet, "log_q",
    log(om_output[["survey_q"]][["survey1"]]), "assumed_known"
  )
  set_fleet_selectivity(survey_fleet, survey_fleet_selectivity)
  set_fleet_observed_data(
    survey_fleet,
    age_comp = survey_fleet_age_comp,
    length_comp = survey_fleet_length_comp,
    index = survey_fleet_index
  )

  set_variable_vector(
    survey_fleet, "age_to_length_conversion",
    c(t(em_input[["age_to_length_conversion"]])), "assumed_known"
  )

  # Set up priors for selectivity parameters and link to both fishery and survey selectivity
  slope_mean <- mean(c(om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]], om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]))
  slope_sd <- 3
  slope_prior <- create_distribution("dnorm")
  set_variable_vector(
    slope_prior, "expected_values", rep(slope_mean, 2), "assumed_known"
  )
  set_variable_vector(
    slope_prior, "log_sd", log(slope_sd), "assumed_known"
  )
  set_distribution_links(
    slope_prior, "prior",
    c(
      get_variable_vector_id(fishing_fleet_selectivity, "slope"),
      get_variable_vector_id(survey_fleet_selectivity, "slope")
    )
  )

  inflection_point_mean <- mean(c(om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]], om_input[["sel_survey"]][["survey1"]][["A50.sel1"]]))
  inflection_point_sd <- 3
  inflection_point_prior <- create_distribution("dnorm")
  set_variable_vector(
    inflection_point_prior, "expected_values",
    rep(inflection_point_mean, 2), "assumed_known"
  )
  set_variable_vector(
    inflection_point_prior, "log_sd", log(inflection_point_sd), "assumed_known"
  )
  set_distribution_links(
    inflection_point_prior, "prior",
    c(
      get_variable_vector_id(fishing_fleet_selectivity, "inflection_point"),
      get_variable_vector_id(survey_fleet_selectivity, "inflection_point")
    )
  )

  recruitment <- create_recruitment("BevertonHolt")
  recruitment_process <- create_recruitment("log_devs")

  # set up recruitment parameters and fix as assumed_known (default)
  # do not set up a recruitment distribution as devs will be assumed known
  # set up log_rzero (equilibrium recruitment)
  set_variable_vector(
    recruitment, "log_rzero", log(om_input[["R0"]]), "assumed_known"
  )
  # set up logit_steep
  set_variable_vector(
    recruitment, "logit_steep",
    -log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2), "assumed_known"
  )
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  set_variable_vector(
    recruitment, "log_devs",
    om_input[["logR.resid"]][2:om_input[["nyr"]]], "assumed_known"
  )
  set_recruitment_n_years(recruitment, om_input[["nyr"]])
  set_recruitment_process(recruitment, recruitment_process)

  # Growth
  ewaa_growth <- create_growth("EWAA")
  set_growth_n_years(ewaa_growth, om_input[["nyr"]])
  set_numeric_vector(ewaa_growth, "ages", om_input[["ages"]])
  set_numeric_vector(ewaa_growth, "weights", c(t(om_input[["W.mt"]])))

  # Maturity
  maturity <- create_maturity("Logistic")
  set_variable_vector(
    maturity, "inflection_point", om_input[["A50.mat"]], "assumed_known"
  )
  set_variable_vector(
    maturity, "slope", om_input[["slope.mat"]], "assumed_known"
  )

  # Population
  population <- create_population()
  set_variable_vector(
    population, "log_M",
    rep(
      log(om_input[["M.age"]][1]),
      om_input[["nyr"]] * om_input[["nages"]]
    ),
    "assumed_known"
  )
  set_variable_vector(
    population, "log_init_naa",
    log(om_output[["N.age"]][1, ]), "assumed_known"
  )
  set_population_constants(population, om_input[["nyr"]], om_input[["nages"]])
  set_numeric_vector(population, "ages", om_input[["ages"]])
  # No fleets are linked: this model has no fishery model module, so the
  # population is never evaluated. The fleet count follows from the linked
  # fleets and is no longer set by hand.
  set_population_processes(
    population,
    maturity = maturity,
    growth = ewaa_growth,
    recruitment = recruitment
  )


  # Set-up TMB
  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = get_fixed(), re = get_random())

  #' @description Test that the number of parameters in the model matches the expected number of parameters (4 selectivity parameters).
  expect_equal(length(parameters$p) + length(parameters$re), 4)

  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE, map = list()
  )

  # Test the prior nll values
  report_nll <- obj$report()$nll_components
  inflection_point_input <- c(om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]], om_input[["sel_survey"]][["survey1"]][["A50.sel1"]])
  slope_input <- c(om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]], om_input[["sel_survey"]][["survey1"]][["slope.sel1"]])
  #' @description Test the slope nll
  expect_equal(
    report_nll[1], -sum(dnorm(slope_input, mean = slope_mean, sd = 3, log = TRUE))
  )
  #' @description Test the inflection point nll
  expect_equal(
    report_nll[2], -sum(dnorm(inflection_point_input, mean = inflection_point_mean, sd = 3, log = TRUE))
  )

  # Fit MCMC using SparseNUTS
  fit <- NULL
  invisible(capture.output(
    invisible(capture.output(
      fit <- SparseNUTS::sample_snuts(
        obj,
        chains = 1,
        print = FALSE,
        refresh = 0,
        quiet = TRUE
      ),
      type = "message"
    ))
  ))
  inflection_point_est <- fit$mle$est[c(1, 3)]
  inflection_point_se <- fit$mle$se[c(1, 3)]
  slope_est <- fit$mle$est[c(2, 4)]
  slope_se <- fit$mle$se[c(2, 4)]
  for (i in 1:2) {
    #' @description Test that the posterior means for inflection point match the prior means.
    expect_equal(inflection_point_est[[i]], inflection_point_mean)
    #' @description Test that the posterior means for slope match the prior means.
    expect_equal(slope_est[[i]], slope_mean)
    #' @description Test that the posterior standard errors for inflection point match the prior standard errors.
    expect_equal(inflection_point_se[[i]], inflection_point_sd)
    #' @description Test that the posterior standard errors for slope match the prior standard errors.
    expect_equal(slope_se[[i]], slope_sd)
  }

  clear()
})
