load(test_path("fixtures", "integration_test_data.RData"))

# Set the iteration ID to 1 for accessing specific input/output list
iter_id <- 1

# Extract model input and output data for the specified iteration
om_input <- om_input_list[[iter_id]]
om_output <- om_output_list[[iter_id]]
em_input <- em_input_list[[iter_id]]

# Define modified parameters for different modules
modified_parameters <- vector(mode = "list", length = length(iter_id))
modified_parameters[[iter_id]] <- list(
  fleet1 = list(
    Fleet.log_Fmort.value = log(om_output_list[[iter_id]][["f"]])
  ),
  survey1 = list(
    LogisticSelectivity.inflection_point.value = 1.5,
    LogisticSelectivity.slope.value = 2,
    Fleet.log_q.value = log(om_output_list[[iter_id]][["survey_q"]][["survey1"]])
  ),
  recruitment = list(
    BevertonHoltRecruitment.log_rzero.value = log(om_input_list[[iter_id]][["R0"]]),
    BevertonHoltRecruitment.log_devs.value = om_input_list[[iter_id]][["logR.resid"]][-1],
    # TODO: integration tests fail after setting BevertonHoltRecruitment.log_devs.estimated
    # to TRUE. We need to debug the issue, then update the line below accordingly.
    BevertonHoltRecruitment.log_devs.estimated = TRUE,
    DnormDistribution.log_sd.value = om_input_list[[iter_id]][["logR_sd"]]
  ),
  maturity = list(
    LogisticMaturity.inflection_point.value = om_input_list[[iter_id]][["A50.mat"]],
    LogisticMaturity.inflection_point.estimated = FALSE,
    LogisticMaturity.slope.value = om_input_list[[iter_id]][["slope.mat"]],
    LogisticMaturity.slope.estimated = FALSE
  ),
  population = list(
    Population.log_init_naa.value = log(om_output_list[[iter_id]][["N.age"]][1, ])
  )
)

test_that("deterministic test of fims with recruitment re", {
  result <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = FALSE,
    random_effects = TRUE,
    modified_parameters = modified_parameters
  )

  # Call report using deterministic parameter values
  # obj[["report"]]() requires parameter list to avoid errors
  report <- result@report
  obj <- result@obj

  # Compare log(R0) to true value
  fims_logR0 <- as.numeric(result@obj[["par"]][36])
  expect_gt(fims_logR0, 0.0)
  expect_equal(fims_logR0, log(om_input_list[[iter_id]][["R0"]]))

  # Compare numbers at age to true value
  for (i in 1:length(c(t(om_output_list[[iter_id]][["N.age"]])))) {
    expect_equal(report[["naa"]][[1]][i], c(t(om_output_list[[iter_id]][["N.age"]]))[i])
  }

  # Compare biomass to true value
  for (i in 1:length(om_output_list[[iter_id]][["biomass.mt"]])) {
    expect_equal(report[["biomass"]][[1]][i], om_output_list[[iter_id]][["biomass.mt"]][i])
  }

  # Compare spawning biomass to true value
  for (i in 1:length(om_output_list[[iter_id]][["SSB"]])) {
    expect_equal(report[["ssb"]][[1]][i], om_output_list[[iter_id]][["SSB"]][i])
  }

  # Compare recruitment to true value
  fims_naa <- matrix(report[["naa"]][[1]][1:(om_input_list[[iter_id]][["nyr"]] * om_input_list[[iter_id]][["nages"]])],
    nrow = om_input_list[[iter_id]][["nyr"]], byrow = TRUE
  )

  # loop over years to compare recruitment by year
  for (i in 1:om_input_list[[iter_id]][["nyr"]]) {
    expect_equal(fims_naa[i, 1], om_output_list[[iter_id]][["N.age"]][i, 1])
  }

  # confirm that recruitment matches the numbers in the first age
  # by comparing to fims_naa (what's reported from FIMS)
  expect_equal(
    fims_naa[1:om_input_list[[iter_id]][["nyr"]], 1],
    report[["recruitment"]][[1]][1:om_input_list[[iter_id]][["nyr"]]]
  )

  # confirm that recruitment matches the numbers in the first age
  # by comparing to the true values from the OM
  for (i in 1:om_input_list[[iter_id]][["nyr"]]) {
    expect_equal(report[["recruitment"]][[1]][i], om_output_list[[iter_id]][["N.age"]][i, 1])
  }

  # recruitment log_devs (fixed at initial "true" values)
  # the initial value of om_input[["logR.resid"]] is dropped from the model
  expect_equal(report[["log_recruit_dev"]][[1]], om_input_list[[iter_id]][["logR.resid"]][-1])
  # check input to ensure log_devs are being read in as random effects
  expect_equal(length(obj$env$parList()$p), 49)
  expect_equal(length(obj$env$parList()$re), 29)
  expect_equal(result@number_of_parameters[["fixed_effects"]], 49)
  expect_equal(result@number_of_parameters[["random_effects"]], 29)
  expect_equal(result@number_of_parameters[["total"]], 78)


  # F (fixed at initial "true" values)
  expect_equal(report[["F_mort"]][[1]], om_output_list[[iter_id]][["f"]])

  # Expected catch
  fims_index <- report[["exp_index"]]
  for (i in 1:length(om_output_list[[iter_id]][["L.mt"]][["fleet1"]])) {
    expect_equal(fims_index[[1]][i], om_output_list[[iter_id]][["L.mt"]][["fleet1"]][i])
  }

  # Expect small relative error for deterministic test
  fims_object_are <- rep(0, length(em_input_list[[iter_id]][["L.obs"]][["fleet1"]]))
  for (i in 1:length(em_input_list[[iter_id]][["L.obs"]][["fleet1"]])) {
    fims_object_are[i] <- abs(fims_index[[1]][i] - em_input_list[[iter_id]][["L.obs"]][["fleet1"]][i]) / em_input_list[[iter_id]][["L.obs"]][["fleet1"]][i]
  }

  # Expect 95% of relative error to be within 2*cv
  expect_lte(sum(fims_object_are > om_input_list[[iter_id]][["cv.L"]][["fleet1"]] * 2.0), length(em_input_list[[iter_id]][["L.obs"]][["fleet1"]]) * 0.05)

  # Compare expected catch number at age to true values
  for (i in 1:length(c(t(om_output_list[[iter_id]][["L.age"]][["fleet1"]])))) {
    expect_equal(report[["cnaa"]][[1]][i], c(t(om_output_list[[iter_id]][["L.age"]][["fleet1"]]))[i])
  }

  # Expected catch number at age in proportion
  # QUESTION: Isn't this redundant with the non-proportion test above?
  fims_cnaa <- matrix(report[["cnaa"]][[1]][1:(om_input_list[[iter_id]][["nyr"]] * om_input_list[[iter_id]][["nages"]])],
    nrow = om_input_list[[iter_id]][["nyr"]], byrow = TRUE
  )
  fims_cnaa_proportion <- fims_cnaa / rowSums(fims_cnaa)
  om_cnaa_proportion <- om_output_list[[iter_id]][["L.age"]][["fleet1"]] / rowSums(om_output_list[[iter_id]][["L.age"]][["fleet1"]])

  for (i in 1:length(c(t(om_cnaa_proportion)))) {
    expect_equal(c(t(fims_cnaa_proportion))[i], c(t(om_cnaa_proportion))[i])
  }

  # Expected survey index.
  # Using [[2]] because the survey is the 2nd fleet.
  cwaa <- matrix(report[["cwaa"]][[2]][1:(om_input_list[[iter_id]][["nyr"]] * om_input_list[[iter_id]][["nages"]])],
    nrow = om_input_list[[iter_id]][["nyr"]], byrow = TRUE
  )
  expect_equal(fims_index[[2]], apply(cwaa, 1, sum) * om_output_list[[iter_id]][["survey_q"]][["survey1"]])

  for (i in 1:length(om_output_list[[iter_id]][["survey_index_biomass"]][["survey1"]])) {
    expect_equal(fims_index[[2]][i], om_output_list[[iter_id]][["survey_index_biomass"]][["survey1"]][i])
  }

  fims_object_are <- rep(0, length(em_input_list[[iter_id]][["surveyB.obs"]][["survey1"]]))
  for (i in 1:length(em_input_list[[iter_id]][["survey.obs"]][["survey1"]])) {
    fims_object_are[i] <- abs(fims_index[[2]][i] - em_input_list[[iter_id]][["surveyB.obs"]][["survey1"]][i]) / em_input_list[[iter_id]][["surveyB.obs"]][["survey1"]][i]
  }
  # Expect 95% of relative error to be within 2*cv
  expect_lte(
    sum(fims_object_are > om_input_list[[iter_id]][["cv.survey"]][["survey1"]] * 2.0),
    length(em_input_list[[iter_id]][["surveyB.obs"]][["survey1"]]) * 0.05
  )

  # Expected catch number at age in proportion
  fims_cnaa <- matrix(report[["cnaa"]][[2]][1:(om_input_list[[iter_id]][["nyr"]] * om_input_list[[iter_id]][["nages"]])],
    nrow = om_input_list[[iter_id]][["nyr"]], byrow = TRUE
  )

  for (i in 1:length(c(t(om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]])))) {
    expect_equal(report[["cnaa"]][[2]][i], c(t(om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]]))[i])
  }

  fims_cnaa_proportion <- fims_cnaa / rowSums(fims_cnaa)
  om_cnaa_proportion <- om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]] / rowSums(om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]])

  for (i in 1:length(c(t(om_cnaa_proportion)))) {
    expect_equal(c(t(fims_cnaa_proportion))[i], c(t(om_cnaa_proportion))[i])
  }
})

test_that("nll test of fims", {
  result <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = FALSE,
    random_effects = TRUE,
    modified_parameters = modified_parameters
  )

  # Set up TMB's computational graph
  obj <- result@obj
  report <- result@report

  # Calculate standard errors
  # sdr <- result@sdreport
  # sdr_fixed <- result[["sdr_fixed"]]

  # log(R0)
  fims_logR0 <- as.numeric(result@obj[["par"]][36])
  expect_equal(fims_logR0, log(om_input_list[[iter_id]][["R0"]]))

  # recruitment likelihood
  # log_devs is of length nyr-1
  rec_nll <- -sum(dnorm(
    om_input_list[[iter_id]][["logR.resid"]][-1], rep(0, om_input_list[[iter_id]][["nyr"]] - 1),
    om_input_list[[iter_id]][["logR_sd"]], TRUE
  ))

  # catch and survey index expected likelihoods
  index_nll_fleet <- -sum(dlnorm(
    em_input_list[[iter_id]][["L.obs"]][["fleet1"]],
    log(om_output_list[[iter_id]][["L.mt"]][["fleet1"]]),
    sqrt(log(em_input_list[[iter_id]][["cv.L"]][["fleet1"]]^2 + 1)), TRUE
  ))
  index_nll_survey <- -sum(dlnorm(
    em_input_list[[iter_id]][["surveyB.obs"]][["survey1"]],
    log(om_output_list[[iter_id]][["survey_index_biomass"]][["survey1"]]),
    sqrt(log(em_input_list[[iter_id]][["cv.survey"]][["survey1"]]^2 + 1)), TRUE
  ))
  index_nll <- index_nll_fleet + index_nll_survey
  # age comp likelihoods
  fishing_acomp_observed <- em_input_list[[iter_id]][["L.age.obs"]][["fleet1"]]
  fishing_acomp_expected <- om_output_list[[iter_id]][["L.age"]][["fleet1"]] / rowSums(om_output_list[[iter_id]][["L.age"]][["fleet1"]])
  survey_acomp_observed <- em_input_list[[iter_id]][["survey.age.obs"]][["survey1"]]
  survey_acomp_expected <- om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]] / rowSums(om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]])
  age_comp_nll_fleet <- age_comp_nll_survey <- 0
  for (y in 1:om_input_list[[iter_id]][["nyr"]]) {
    age_comp_nll_fleet <- age_comp_nll_fleet -
      dmultinom(
        fishing_acomp_observed[y, ] * em_input_list[[iter_id]][["n.L"]][["fleet1"]], em_input_list[[iter_id]][["n.L"]][["fleet1"]],
        fishing_acomp_expected[y, ], TRUE
      )

    age_comp_nll_survey <- age_comp_nll_survey -
      dmultinom(
        survey_acomp_observed[y, ] * em_input_list[[iter_id]][["n.survey"]][["survey1"]], em_input_list[[iter_id]][["n.survey"]][["survey1"]],
        survey_acomp_expected[y, ], TRUE
      )
  }
  age_comp_nll <- age_comp_nll_fleet + age_comp_nll_survey

  # length comp likelihoods
  fishing_lengthcomp_observed <- em_input_list[[iter_id]][["L.length.obs"]][["fleet1"]]
  fishing_lengthcomp_expected <- om_output_list[[iter_id]][["L.length"]][["fleet1"]] / rowSums(om_output_list[[iter_id]][["L.length"]][["fleet1"]])
  survey_lengthcomp_observed <- em_input_list[[iter_id]][["survey.length.obs"]][["survey1"]]
  survey_lengthcomp_expected <- om_output_list[[iter_id]][["survey_length_comp"]][["survey1"]] / rowSums(om_output_list[[iter_id]][["survey_length_comp"]][["survey1"]])
  lengthcomp_nll_fleet <- lengthcomp_nll_survey <- 0
  for (y in 1:om_input_list[[iter_id]][["nyr"]]) {
    # test using FIMS_dmultinom which matches the TMB dmultinom calculation and differs from R
    # by NOT rounding obs to the nearest integer.
    lengthcomp_nll_fleet <- lengthcomp_nll_fleet -
      FIMS_dmultinom(
        fishing_lengthcomp_observed[y, ] * em_input_list[[iter_id]][["n.L.lengthcomp"]][["fleet1"]],
        fishing_lengthcomp_expected[y, ]
      )

    lengthcomp_nll_survey <- lengthcomp_nll_survey -
      FIMS_dmultinom(
        survey_lengthcomp_observed[y, ] * em_input_list[[iter_id]][["n.survey.lengthcomp"]][["survey1"]],
        survey_lengthcomp_expected[y, ]
      )
  }
  lengthcomp_nll <- lengthcomp_nll_fleet + lengthcomp_nll_survey

  expected_jnll <- rec_nll + index_nll + age_comp_nll + lengthcomp_nll
  jnll <- report[["jnll"]]

  expect_equal(report[["nll_components"]][1], rec_nll)
  expect_equal(report[["nll_components"]][2], index_nll_fleet)
  expect_equal(report[["nll_components"]][3], age_comp_nll_fleet)
  expect_equal(report[["nll_components"]][4], lengthcomp_nll_fleet)
  expect_equal(report[["nll_components"]][5], index_nll_survey)
  expect_equal(report[["nll_components"]][6], age_comp_nll_survey)
  expect_equal(report[["nll_components"]][7], lengthcomp_nll_survey)
  expect_equal(jnll, expected_jnll)
})

test_that("estimation test of fims using wrapper functions", {
  result <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = TRUE,
    random_effects = TRUE,
    modified_parameters = modified_parameters
  )

  # TODO:: naa tests fail when log_dev estimation turned on
  # # Compare FIMS results with model comparison project OM values
  # validate_fims(
  #   report = get_report(result),
  #   estimates = get_estimates(result),
  #   om_input = om_input_list[[iter_id]],
  #   om_output = om_output_list[[iter_id]],
  #   em_input = em_input_list[[iter_id]],
  #   use_fimsfit = TRUE
  # )
})

test_that("estimation test with recruitment re on logr", {
  fims_data <- FIMS::FIMSFrame(data1)

  # Clear any previous FIMS settings
  clear()

  fleets <- list(
    fleet1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    ),
    survey1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    )
  )

  default_parameters <- fims_data |>
    create_default_parameters(
      fleets = fleets,
      recruitment = list(
        form = "BevertonHoltRecruitment",
        process_distribution = c(log_r = "DnormDistribution", fit_as_random = TRUE),
        process_expected = "log_expected_recruitment"),
    )

  modified_parameters <- list(
    fleet1 = list(
      Fleet.log_Fmort.value = log(om_output_list[[1]][["f"]])
    ),
    survey1 = list(
      LogisticSelectivity.inflection_point.value = 1.5,
      LogisticSelectivity.slope.value = 2,
      Fleet.log_q.value = log(om_output_list[[1]][["survey_q"]][["survey1"]])
    ),
    recruitment = list(
      BevertonHoltRecruitment.log_rzero.value = log(om_input_list[[1]][["R0"]]),
      BevertonHoltRecruitment.log_rzero.estimated = TRUE,
      BevertonHoltRecruitment.log_rzero.random = FALSE,
      BevertonHoltRecruitment.log_r.value = rep(0, (om_input_list[[1]]$nyr-1)),
      BevertonHoltRecruitment.log_r.estimated = TRUE,
      BevertonHoltRecruitment.log_r.random = TRUE,
      BevertonHoltRecruitment.log_devs.value = rep(1, (om_input_list[[1]]$nyr-1)),
      BevertonHoltRecruitment.log_devs.estimated = FALSE,
      DnormDistribution.log_sd.value = om_input_list[[1]][["logR_sd"]],
      DnormDistribution.log_sd.estimated = FALSE
    ),
    maturity = list(
      LogisticMaturity.inflection_point.value = om_input_list[[1]][["A50.mat"]],
      LogisticMaturity.inflection_point.estimated = FALSE,
      LogisticMaturity.slope.value = om_input_list[[1]][["slope.mat"]],
      LogisticMaturity.slope.estimated = FALSE
    ),
    population = list(
      Population.log_init_naa.value = log(om_output_list[[1]][["N.age"]][1, ])
    )
  )

 parameters <- default_parameters |>
    update_parameters(
      modified_parameters = modified_parameters
    )

  parameter_list <- initialize_fims(
    parameters = parameters,
    data = fims_data
  )
  fit_log_r <- fit_fims(parameter_list, optimize = FALSE)

  clear()


  # TODO:: naa tests fail when log_dev estimation turned on
  # # Compare FIMS results with model comparison project OM values
  # validate_fims(
  #   report = fit@report,
  #   sdr = fit@estimates,
  #   sdr_report = fit@estimates,
  #   om_input = om_input_list[[iter_id]],
  #   om_output = om_output_list[[iter_id]],
  #   em_input = em_input_list[[iter_id]],
  #   use_fimsfit = TRUE
  # )

 # fit with log_devs as re
  # Clear any previous FIMS settings
  clear()

  fleet1 <- survey1 <- list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Index = "DlnormDistribution",
      AgeComp = "DmultinomDistribution",
      LengthComp = "DmultinomDistribution"
    )
  )

  default_parameters <- fims_data |>
    create_default_parameters(
      fleets = list(fleet1 = fleet1, survey1 = survey1),
      recruitment = list(
        form = "BevertonHoltRecruitment",
        process_distribution = c(log_devs = "DnormDistribution", fit_as_random = TRUE)),
      growth = list(form = "EWAAgrowth"),
      maturity = list(form = "LogisticMaturity")
    )

  modified_parameters <- list(
    fleet1 = list(
      Fleet.log_Fmort.value = log(om_output_list[[1]][["f"]])
    ),
    survey1 = list(
      LogisticSelectivity.inflection_point.value = 1.5,
      LogisticSelectivity.slope.value = 2,
      Fleet.log_q.value = log(om_output_list[[1]][["survey_q"]][["survey1"]])
    ),
    recruitment = list(
      BevertonHoltRecruitment.log_rzero.value = log(om_input_list[[1]][["R0"]]),
      BevertonHoltRecruitment.log_rzero.estimated = TRUE,
      BevertonHoltRecruitment.log_rzero.random = FALSE,
      BevertonHoltRecruitment.log_r.value = -999,
      BevertonHoltRecruitment.log_r.estimated = FALSE,
      BevertonHoltRecruitment.log_r.random = FALSE,
      BevertonHoltRecruitment.log_devs.value = rep(1, (om_input_list[[1]]$nyr-1)),
      BevertonHoltRecruitment.log_devs.estimated = TRUE,
      BevertonHoltRecruitment.log_devs.random = TRUE,
      DnormDistribution.log_sd.value = om_input_list[[1]][["logR_sd"]],
      DnormDistribution.log_sd.estimated = FALSE
    ),
    maturity = list(
      LogisticMaturity.inflection_point.value = om_input_list[[1]][["A50.mat"]],
      LogisticMaturity.inflection_point.estimated = FALSE,
      LogisticMaturity.slope.value = om_input_list[[1]][["slope.mat"]],
      LogisticMaturity.slope.estimated = FALSE
    ),
    population = list(
      Population.log_init_naa.value = log(om_output_list[[1]][["N.age"]][1, ])
    )
  )

 parameters <- default_parameters |>
    update_parameters(
      modified_parameters = modified_parameters
    )

  parameter_list <- initialize_fims(
    parameters = parameters,
    data = fims_data
  )
  fit_log_devs <- fit_fims(parameter_list, optimize = TRUE)

  clear()

expect_equal(fit_log_r@report$nll_components[-1], fit_log_devs@report$nll_components[-1], tolerance = .001)
expect_equal(fit_log_r@report$recruitment, fit_log_devs@report$recruitment, tolerance = .01)


})


#raw - helper functions still not working

om_input <- om_input_list[[iter_id]] # Operating model input for the current iteration
  om_output <- om_output_list[[iter_id]] # Operating model output for the current iteration
  em_input <- em_input_list[[iter_id]] # Estimation model input for the current iteration

  # Clear any previous FIMS settings
  clear()

  # Extract fishing fleet landings data (observed) and initialize index module
  catch <- em_input[["L.obs"]][["fleet1"]]
  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_index <- methods::new(Index, om_input[["nyr"]])
  fishing_fleet_index$index_data <- catch
  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*nages
  fishing_fleet_age_comp <- methods::new(AgeComp, om_input[["nyr"]], om_input[["nages"]])
  # Here we fill in the values for the object with the observed age comps for fleet one
  # we multiply these proportions by the sample size for likelihood weighting
  fishing_fleet_age_comp$age_comp_data <- c(t(em_input[["L.age.obs"]][["fleet1"]])) * em_input[["n.L"]][["fleet1"]]

  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- methods::new(LengthComp, om_input[["nyr"]], om_input[["nlengths"]])
  fishing_fleet_length_comp$length_comp_data <- c(t(em_input[["L.length.obs"]][["fleet1"]])) * em_input[["n.L.lengthcomp"]][["fleet1"]]

  # Fleet
  # Create the fishing fleet
  fishing_fleet_selectivity <- methods::new(LogisticSelectivity)
  fishing_fleet_selectivity$inflection_point[1]$value <- om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]]
  fishing_fleet_selectivity$inflection_point[1]$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  fishing_fleet_selectivity$inflection_point[1]$estimated <- TRUE
  fishing_fleet_selectivity$slope[1]$value <- om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]]
  # turn on estimation of slope
  fishing_fleet_selectivity$slope[1]$is_random_effect <- FALSE
  fishing_fleet_selectivity$slope[1]$estimated <- TRUE

  # Initialize the fishing fleet module
  fishing_fleet <- methods::new(Fleet)
  # Set number of years
  fishing_fleet$nyears <- om_input[["nyr"]]
  # Set number of age classes
  fishing_fleet$nages <- om_input[["nages"]]
  # Set number of length bins
  fishing_fleet$nlengths <- om_input[["nlengths"]]

  fishing_fleet$log_Fmort$resize(om_input[["nyr"]])
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    fishing_fleet$log_Fmort[y]$value <- log(om_output[["f"]][y])
  }
  fishing_fleet$log_Fmort$set_all_estimable(TRUE)
  fishing_fleet$log_q[1]$value <- log(1.0)
  fishing_fleet$estimate_q <- FALSE
  fishing_fleet$random_q <- FALSE
  fishing_fleet$SetSelectivity(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedIndexData(fishing_fleet_index$get_id())
  fishing_fleet$SetObservedAgeCompData(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetObservedLengthCompData(fishing_fleet_length_comp$get_id())

  # Set up fishery index data using the lognormal
  fishing_fleet_index_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_index_distribution$log_sd$resize(om_input[["nyr"]])
  for (y in 1:om_input[["nyr"]]) {
    # Compute lognormal SD from OM coefficient of variation (CV)
    fishing_fleet_index_distribution$log_sd[y]$value <- log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1)))
  }
  fishing_fleet_index_distribution$log_sd$set_all_estimable(FALSE)
  # Set Data using the IDs from the modules defined above
  fishing_fleet_index_distribution$set_observed_data(fishing_fleet$GetObservedIndexDataID())
  fishing_fleet_index_distribution$set_distribution_links("data", fishing_fleet$log_expected_index$get_id())

  # Set up fishery age composition data using the multinomial
  fishing_fleet_agecomp_distribution <- methods::new(DmultinomDistribution)
  fishing_fleet_agecomp_distribution$set_observed_data(fishing_fleet$GetObservedAgeCompDataID())
  fishing_fleet_agecomp_distribution$set_distribution_links("data", fishing_fleet$proportion_catch_numbers_at_age$get_id())

  # Set up fishery length composition data using the multinomial
  fishing_fleet_lengthcomp_distribution <- methods::new(DmultinomDistribution)
  fishing_fleet_lengthcomp_distribution$set_observed_data(fishing_fleet$GetObservedLengthCompDataID())
  fishing_fleet_lengthcomp_distribution$set_distribution_links("data", fishing_fleet$proportion_catch_numbers_at_length$get_id())

  # Set age-to-length conversion matrix
  # TODO: If an age_to_length_conversion matrix is provided, the code below
  # still executes. Consider adding a check in the Rcpp interface to ensure
  # users provide a vector of inputs.
  fishing_fleet$age_length_conversion_matrix <- methods::new(
    ParameterVector,
    c(t(em_input[["age_to_length_conversion"]])),
    om_input[["nages"]] * om_input[["nlengths"]]
  )
  # Turn off estimation for length-at-age
  fishing_fleet$age_length_conversion_matrix$set_all_estimable(FALSE)
  fishing_fleet$age_length_conversion_matrix$set_all_random(FALSE)

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- methods::new(Index, om_input[["nyr"]])
  survey_fleet_index$index_data <- survey_index
  survey_fleet_age_comp <- methods::new(AgeComp, om_input[["nyr"]], om_input[["nages"]])
  survey_fleet_age_comp$age_comp_data <- c(t(em_input[["survey.age.obs"]][["survey1"]])) * em_input[["n.survey"]][["survey1"]]
  survey_lengthcomp <- em_input[["survey.length.obs"]][["survey1"]]
  survey_fleet_length_comp <- methods::new(LengthComp, om_input[["nyr"]], om_input[["nlengths"]])
  survey_fleet_length_comp$length_comp_data <- c(t(survey_lengthcomp)) * em_input[["n.survey.lengthcomp"]][["survey1"]]
  # Fleet
  # Create the survey fleet
  survey_fleet_selectivity <- methods::new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point[1]$value <- om_input[["sel_survey"]][["survey1"]][["A50.sel1"]]
  survey_fleet_selectivity$inflection_point[1]$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimated <- TRUE
  survey_fleet_selectivity$slope[1]$value <- om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]
  survey_fleet_selectivity$slope[1]$is_random_effect <- FALSE
  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimated <- TRUE

  survey_fleet <- methods::new(Fleet)
  survey_fleet$is_survey <- TRUE
  survey_fleet$nages <- om_input[["nages"]]
  survey_fleet$nyears <- om_input[["nyr"]]
  survey_fleet$nlengths <- om_input[["nlengths"]]
  survey_fleet$log_q[1]$value <- log(om_output[["survey_q"]][["survey1"]])
  survey_fleet$log_q[1]$estimated <- TRUE
  survey_fleet$estimate_q <- TRUE
  survey_fleet$random_q <- FALSE
  survey_fleet$SetSelectivity(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompData(survey_fleet_age_comp$get_id())
  survey_fleet$SetObservedLengthCompData(survey_fleet_length_comp$get_id())

  # Set up survey index data using the lognormal
  survey_fleet_index_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution$log_sd$resize(om_input[["nyr"]])
  for (y in 1:om_input$nyr) {
    survey_fleet_index_distribution$log_sd[y]$value <- log(sqrt(log(em_input[["cv.survey"]][["survey1"]]^2 + 1)))
  }
  survey_fleet_index_distribution$log_sd$set_all_estimable(FALSE)
  # Set Data using the IDs from the modules defined above
  survey_fleet_index_distribution$set_observed_data(survey_fleet$GetObservedIndexDataID())
  survey_fleet_index_distribution$set_distribution_links("data", survey_fleet$log_expected_index$get_id())

  # Age composition distribution
  survey_fleet_agecomp_distribution <- methods::new(DmultinomDistribution)
  survey_fleet_agecomp_distribution$set_observed_data(survey_fleet$GetObservedAgeCompDataID())
  survey_fleet_agecomp_distribution$set_distribution_links("data", survey_fleet$proportion_catch_numbers_at_age$get_id())

  # Length composition distribution
  survey_fleet_lengthcomp_distribution <- methods::new(DmultinomDistribution)
  survey_fleet_lengthcomp_distribution$set_observed_data(survey_fleet$GetObservedLengthCompDataID())
  survey_fleet_lengthcomp_distribution$set_distribution_links("data", survey_fleet$proportion_catch_numbers_at_length$get_id()) # Set age to length conversion matrix
  survey_fleet$age_length_conversion_matrix <- methods::new(
    ParameterVector,
    c(t(em_input[["age_to_length_conversion"]])),
    om_input[["nages"]] * om_input[["nlengths"]]
  )
  # Turn off estimation for length-at-age
  survey_fleet$age_length_conversion_matrix$set_all_estimable(FALSE)
  survey_fleet$age_length_conversion_matrix$set_all_random(FALSE)

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- methods::new(BevertonHoltRecruitment)

  # NOTE: in first set of parameters below (for recruitment),
  # $is_random_effect (default is FALSE) and $estimated (default is FALSE)
  # are defined even if they match the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- log(om_input[["R0"]])
  recruitment$log_rzero[1]$is_random_effect <- FALSE
  recruitment$log_rzero[1]$estimated <- TRUE
  # set up logit_steep
  recruitment$logit_steep[1]$value <- -log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2)
  recruitment$logit_steep[1]$is_random_effect <- FALSE
  recruitment$logit_steep[1]$estimated <- FALSE
  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))
  recruitment$log_devs$resize(om_input[["nyr"]] - 1)
  for (y in 1:(om_input[["nyr"]] - 1)) {
    recruitment$log_devs[y]$value <- 1
      }
  recruitment$log_devs$set_all_estimable(FALSE)
  recruitment$log_r$resize(om_input[["nyr"]] - 1)
  for (y in 1:(om_input[["nyr"]] - 1)) {
    recruitment$log_r[y]$value <- 0
  }
  recruitment$log_r$set_all_estimable(TRUE)
  recruitment$log_r$set_all_random(TRUE)
  recruitment$nyears <- om_input[["nyr"]]

  # recruitment_distribution2 <- initialize_process_distribution(recruitment, 
  #   par = "log_r", expected = "log_expected_recruitment",
  #   family = gaussian(), 
  #   sd = list(value = om_input[["logR_sd"]], estimated = FALSE),
  #   is_random_effect = TRUE)
 
  recruitment_distribution <- methods::new(DnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd <- methods::new(ParameterVector, 1)
  recruitment_distribution$log_sd[1]$value <- log(om_input[["logR_sd"]])
  recruitment_distribution$log_sd[1]$estimated <- TRUE
  recruitment_distribution$x$resize(om_input[["nyr"]] - 1)
  recruitment_distribution$expected_values$resize(om_input[["nyr"]] - 1)
  for (i in 1:(om_input[["nyr"]] - 1)) {
  #  recruitment_distribution$x[i]$value <- 0
    recruitment_distribution$expected_values[i]$value <- 0
  }
  recruitment_distribution$set_distribution_links("random_effects", 
    c(recruitment$log_r$get_id(), recruitment$log_expected_recruitment$get_id()))
  
  # Growth
  ewaa_growth <- methods::new(EWAAgrowth)
  ewaa_growth$ages <- om_input[["ages"]]
  ewaa_growth$weights <- om_input[["W.mt"]]

  # Maturity
  maturity <- methods::new(LogisticMaturity)
  maturity$inflection_point[1]$value <- om_input[["A50.mat"]]
  maturity$inflection_point[1]$is_random_effect <- FALSE
  maturity$inflection_point[1]$estimated <- FALSE
  maturity$slope[1]$value <- om_input[["slope.mat"]]
  maturity$slope[1]$is_random_effect <- FALSE
  maturity$slope[1]$estimated <- FALSE

  # Population
  population <- methods::new(Population)
  population$log_M$resize(om_input[["nyr"]] * om_input[["nages"]])
  for (i in 1:(om_input[["nyr"]] * om_input[["nages"]])) {
    population$log_M[i]$value <- log(om_input[["M.age"]][1])
  }
  population$log_M$set_all_estimable(FALSE)
  population$log_init_naa$resize(om_input[["nages"]])
  for (i in 1:om_input$nages) {
    population$log_init_naa[i]$value <- log(om_output[["N.age"]][1, i])
  }
  population$log_init_naa$set_all_estimable(TRUE)
  population$nages <- om_input[["nages"]]
  population$ages <- om_input[["ages"]]
  population$nfleets <- sum(om_input[["fleet_num"]], om_input[["survey_num"]])
  population$nseasons <- 1
  population$nyears <- om_input[["nyr"]]
  population$SetRecruitment(recruitment$get_id())
  population$SetGrowth(ewaa_growth$get_id())
  population$SetMaturity(maturity$get_id())

  # Set-up TMB
  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = get_fixed(),
                     re = get_random())
  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE, map = map, random = "re",
  )

  # Optimization with nlminb
  opt <- NULL
  if (estimation_mode == TRUE) {
    opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
      control = list(eval.max = 10000, iter.max = 10000, trace = 0)
    )
  }
  # Call report using MLE parameter values, or
  # the initial values if optimization is skipped
  report <- obj[["report"]](obj[["env"]][["last.par.best"]])

  sdr <- TMB::sdreport(obj)
  sdr_report <- summary(sdr, "report")
  sdr_fixed <- summary(sdr, "fixed")

  clear()
