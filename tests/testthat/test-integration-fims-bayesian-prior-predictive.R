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

  # Extract fishing fleet landings data (observed) and initialize index module
  catch <- em_input[["L.obs"]][["fleet1"]]
  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_index <- methods::new(Index, om_input[["nyr"]])
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) fishing_fleet_index$index_data$set(x - 1, catch[x])
  )

  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*nages
  fishing_fleet_age_comp <- methods::new(AgeComp, om_input[["nyr"]], om_input[["nages"]])
  # Here we fill in the values for the object with the observed age comps for fleet one
  # we multiply these proportions by the sample size for likelihood weighting
  purrr::walk(
    1:(om_input[["nyr"]] * om_input[["nages"]]),
    \(x) fishing_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(em_input[["L.age.obs"]][["fleet1"]])) * em_input[["n.L"]][["fleet1"]])[x]
    )
  )

  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- methods::new(LengthComp, om_input[["nyr"]], om_input[["nlengths"]])
  purrr::walk(
    1:(om_input[["nyr"]] * om_input[["nlengths"]]),
    \(x) fishing_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(em_input[["L.length.obs"]][["fleet1"]])) * em_input[["n.L.lengthcomp"]][["fleet1"]])[x]
    )
  )
  # Fleet
  # Create the fishing fleet
  fishing_fleet_selectivity <- methods::new(LogisticSelectivity)
  fishing_fleet_selectivity$inflection_point[1]$value <- om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]]
  # turn on estimation of inflection_point
  fishing_fleet_selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  fishing_fleet_selectivity$slope[1]$value <- om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]]
  # turn on estimation of slope
  fishing_fleet_selectivity$slope[1]$estimation_type$set("fixed_effects")

  # Initialize the fishing fleet module
  # Initialize the fishing fleet module
  fishing_fleet <- methods::new(Fleet)
  # Set number of years
  fishing_fleet$n_years$set(om_input[["nyr"]])
  # Set number of age classes
  fishing_fleet$n_ages$set(om_input[["nages"]])
  # Set number of length bins
  fishing_fleet$n_lengths$set(om_input[["nlengths"]])

  fishing_fleet$log_Fmort$resize(om_input[["nyr"]])
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    fishing_fleet$log_Fmort[y]$value <- log(om_output[["f"]][y])
  }
  fishing_fleet$log_q[1]$value <- log(1.0)
  fishing_fleet$SetSelectivityID(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedIndexDataID(fishing_fleet_index$get_id())
  fishing_fleet$SetObservedAgeCompDataID(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetObservedLengthCompDataID(fishing_fleet_length_comp$get_id())

  # Set age-to-length conversion matrix
  # TODO: If an age_to_length_conversion matrix is provided, the code below
  # still executes. Consider adding a check in the Rcpp interface to ensure
  # users provide a vector of inputs.
  fishing_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:(om_input[["nages"]] * om_input[["nlengths"]])) {
    # Set the age-length conversion matrix values
    fishing_fleet$age_to_length_conversion[i]$value <- c(t(em_input[["age_to_length_conversion"]]))[i]
  }

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- methods::new(Index, om_input[["nyr"]])
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) survey_fleet_index$index_data$set(x - 1, survey_index[x])
  )
  survey_fleet_age_comp <- methods::new(AgeComp, om_input[["nyr"]], om_input[["nages"]])
  purrr::walk(
    1:(om_input[["nyr"]] * om_input[["nages"]]),
    \(x) survey_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(em_input[["survey.age.obs"]][["survey1"]])) * em_input[["n.survey"]][["survey1"]])[x]
    )
  )
  survey_lengthcomp <- em_input[["survey.length.obs"]][["survey1"]]
  survey_fleet_length_comp <- methods::new(LengthComp, om_input[["nyr"]], om_input[["nlengths"]])
  purrr::walk(
    1:(om_input[["nyr"]] * om_input[["nlengths"]]),
    \(x) survey_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(survey_lengthcomp)) * em_input[["n.survey.lengthcomp"]][["survey1"]])[x]
    )
  )
  # Fleet
  # Create the survey fleet
  survey_fleet_selectivity <- methods::new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point[1]$value <- om_input[["sel_survey"]][["survey1"]][["A50.sel1"]]
  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  survey_fleet_selectivity$slope[1]$value <- om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]
  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimation_type$set("fixed_effects")

  survey_fleet <- methods::new(Fleet)
  survey_fleet$n_ages$set(om_input[["nages"]])
  survey_fleet$n_years$set(om_input[["nyr"]])
  survey_fleet$n_lengths$set(om_input[["nlengths"]])
  survey_fleet$log_Fmort$resize(om_input[["nyr"]])
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    survey_fleet$log_Fmort[y]$value <- (-200)
  }
  survey_fleet$log_q[1]$value <- log(om_output[["survey_q"]][["survey1"]])
  survey_fleet$SetSelectivityID(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexDataID(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompDataID(survey_fleet_age_comp$get_id())
  survey_fleet$SetObservedLengthCompDataID(survey_fleet_length_comp$get_id())

  survey_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:(om_input[["nages"]] * om_input[["nlengths"]])) {
    survey_fleet$age_to_length_conversion[i]$value <-
      c(t(em_input[["age_to_length_conversion"]]))[i]
  }

  # Set up priors for selectivity parameters and link to both fishery and survey selectivity
  slope_mean <- mean(c(om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]], om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]))
  slope_sd <- 3
  slope_prior <- methods::new(DnormDistribution)
  slope_prior$expected_values$resize(2)
  slope_prior$expected_values[1]$value <- slope_mean
  slope_prior$expected_values[2]$value <- slope_mean
  slope_prior$log_sd$resize(1)
  slope_prior$log_sd[1]$value <- log(slope_sd)
  slope_prior$set_distribution_links("prior", c(fishing_fleet_selectivity$slope$get_id(), survey_fleet_selectivity$slope$get_id()))

  inflection_point_mean <- mean(c(om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]], om_input[["sel_survey"]][["survey1"]][["A50.sel1"]]))
  inflection_point_sd <- 3
  inflection_point_prior <- methods::new(DnormDistribution)
  inflection_point_prior$expected_values$resize(2)
  inflection_point_prior$expected_values[1]$value <- inflection_point_mean
  inflection_point_prior$expected_values[2]$value <- inflection_point_mean
  inflection_point_prior$log_sd$resize(1)
  inflection_point_prior$log_sd[1]$value <- log(inflection_point_sd)
  inflection_point_prior$set_distribution_links("prior", c(fishing_fleet_selectivity$inflection_point$get_id(), survey_fleet_selectivity$inflection_point$get_id()))


  recruitment <- methods::new(BevertonHoltRecruitment)
  recruitment_process <- new(LogDevsRecruitmentProcess)

  # set up recruitment parameters and fix as constant (default)
  # do not set up a recruitment distribution as devs will be held constant
  # set up log_rzero (equilibrium recruitment) 
  recruitment$log_rzero[1]$value <- log(om_input[["R0"]])
  # set up logit_steep
  recruitment$logit_steep[1]$value <- -log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2)
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  recruitment$log_devs$resize(om_input[["nyr"]] - 1)
  for (y in 1:(om_input[["nyr"]] - 1)) {
    recruitment$log_devs[y]$value <- om_input[["logR.resid"]][y + 1]
  }
  recruitment$n_years$set(om_input[["nyr"]])
  recruitment$SetRecruitmentProcessID(recruitment_process$get_id())

  # Growth
  ewaa_growth <- methods::new(EWAAGrowth)
  ewaa_growth$n_years$set(om_input[["nyr"]])
  ewaa_growth$ages$resize(om_input[["nages"]])
  purrr::walk(
    seq_along(om_input[["ages"]]),
    \(x) ewaa_growth$ages$set(x - 1, om_input[["ages"]][x])
  )
  ewaa_growth$weights$resize(om_input[["nages"]])
  purrr::walk(
    seq(ewaa_growth$weights$size()),
    # Weights are only by age in the OM not by age and year. The modular math
    # will repeat 1:n_ages over and over again for each year.
    \(x) ewaa_growth$weights$set(x - 1, om_input[["W.mt"]][x])
  )

  # Maturity
  maturity <- methods::new(LogisticMaturity)
  maturity$inflection_point[1]$value <- om_input[["A50.mat"]]
  maturity$inflection_point[1]$estimation_type$set("constant")
  maturity$slope[1]$value <- om_input[["slope.mat"]]
  maturity$slope[1]$estimation_type$set("constant")

  # Population
  population <- methods::new(Population)
  population$log_M$resize(om_input[["nyr"]] * om_input[["nages"]])
  for (i in 1:(om_input[["nyr"]] * om_input[["nages"]])) {
    population$log_M[i]$value <- log(om_input[["M.age"]][1])
  }
  population$log_init_naa$resize(om_input[["nages"]])
  for (i in 1:om_input$nages) {
    population$log_init_naa[i]$value <- log(om_output[["N.age"]][1, i])
  }
  population$n_ages$set(om_input[["nages"]])
  population$ages$resize(om_input[["nages"]])
  purrr::walk(
    seq_along(om_input[["ages"]]),
    \(x) population$ages$set(x - 1, om_input[["ages"]][x])
  )
  population$n_fleets$set(sum(om_input[["fleet_num"]], om_input[["survey_num"]]))
  population$n_years$set(om_input[["nyr"]])
  population$SetRecruitmentID(recruitment$get_id())
  population$SetGrowthID(ewaa_growth$get_id())
  population$SetMaturityID(maturity$get_id())


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
  fit <- SparseNUTS::sample_snuts(obj, chains = 1)
  inflection_point_est <- fit$mle$est[c(1,3)] |> unname()
  inflection_point_se <- fit$mle$se[c(1,3)] |> unname()
  slope_est <- fit$mle$est[c(2,4)] |> unname()
  slope_se <- fit$mle$se[c(2,4)] |> unname()
  for(i in 1:2){
    #' @description Test that the posterior means for inflection point match the prior means.
    expect_equal(inflection_point_est[i], inflection_point_mean)
    #' @description Test that the posterior means for slope match the prior means.
    expect_equal(slope_est[i], slope_mean)
    #' @description Test that the posterior standard errors for inflection point match the prior standard errors.
    expect_equal(inflection_point_se[i], inflection_point_sd)
    #' @description Test that the posterior standard errors for slope match the prior standard errors.
    expect_equal(slope_se[i], slope_sd)
  } 

  clear()
})
