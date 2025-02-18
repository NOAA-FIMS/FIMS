# Load necessary data for the integration test
load(test_path("fixtures", "integration_test_data.RData"))

# Set the iteration ID to 1 for accessing specific input/output list
iter_id <- 1

# Define modified parameters for different modules
fit_agecomp <- readRDS(testthat::test_path(
  "fixtures",
  "parameters_model_comparison_project.RDS"
))

test_that("deterministic test of fims", {
  # Run FIMS using the setup_and_run_FIMS_with_wrappers function
  result <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = FALSE,
    modified_parameters = modified_parameters
  )

  # Call report using deterministic parameter values
  # obj[["report"]]() requires parameter list to avoid errors
  report <- get_report(result)
  estimates <- get_estimates(result)

  # Compare log(R0) to true value
  fims_logR0 <- estimates |>
    dplyr::filter(label == "log_rzero") |>
    dplyr::pull(estimate)
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
  # Run FIMS using the setup_and_run_FIMS_with_wrappers function
  result <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = FALSE,
    modified_parameters = modified_parameters
  )

  # Set up TMB's computational graph
  report <- get_report(result)
  estimates <- get_estimates(result)

  # log(R0)
  fims_logR0 <- estimates |>
    dplyr::filter(label == "log_rzero") |>
    dplyr::pull(estimate)
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

test_that("estimation test with age and length comp using wrappers", {
  # Load the test data from an RDS file containing the model fit
  fit_age_length_comp <- readRDS(test_path("fixtures", "fit_age_length_comp.RDS"))

  # Compare FIMS results with model comparison project OM values
  validate_fims(
    report = get_report(fit_age_length_comp),
    estimates = get_estimates(fit_age_length_comp),
    om_input = om_input_list[[iter_id]],
    om_output = om_output_list[[iter_id]],
    em_input = em_input_list[[iter_id]],
    use_fimsfit = TRUE
  )
})

test_that("estimation test with age comp only using wrappers", {
  # Load the test data from an RDS file containing the model fit
  fit_agecomp <- readRDS(test_path("fixtures", "fit_agecomp.RDS"))

  # Compare FIMS results with model comparison project OM values
  validate_fims(
    report = get_report(fit_agecomp),
    estimates = get_estimates(fit_agecomp),
    om_input = om_input_list[[iter_id]],
    om_output = om_output_list[[iter_id]],
    em_input = em_input_list[[iter_id]],
    use_fimsfit = TRUE
  )

  # Load the test data from an RDS file containing the model fit
  fit_agecomp_na <- readRDS(test_path("fixtures", "fit_agecomp_na.RDS"))

  # Compare FIMS results with model comparison project OM values
  validate_fims(
    report = get_report(fit_agecomp_na),
    estimates = get_estimates(fit_agecomp_na),
    om_input = om_input_list[[iter_id]],
    om_output = om_output_list[[iter_id]],
    em_input = em_input_list[[iter_id]],
    use_fimsfit = TRUE
  )
})

test_that("estimation test with length comp only using wrappers", {
  # Load the test data from an RDS file containing the model fit
  fit_lengthcomp <- readRDS(test_path("fixtures", "fit_lengthcomp.RDS"))

  # Compare FIMS results with model comparison project OM values
  validate_fims(
    report = get_report(fit_lengthcomp),
    estimates = get_estimates(fit_lengthcomp),
    om_input = om_input_list[[iter_id]],
    om_output = om_output_list[[iter_id]],
    em_input = em_input_list[[iter_id]],
    use_fimsfit = TRUE
  )

  # Load the test data from an RDS file containing the model fit
  fit_lengthcomp_na <- readRDS(test_path("fixtures", "fit_lengthcomp_na.RDS"))

  # Compare FIMS results with model comparison project OM values
  validate_fims(
    report = get_report(fit_lengthcomp_na),
    estimates = get_estimates(fit_lengthcomp_na),
    om_input = om_input_list[[iter_id]],
    om_output = om_output_list[[iter_id]],
    em_input = em_input_list[[iter_id]],
    use_fimsfit = TRUE
  )
})

test_that("estimation test with age and length comp with NAs", {
  # Load the test data from an RDS file containing the model fit
  fit_age_length_comp_na <- readRDS(test_path("fixtures", "fit_age_length_comp_na.RDS"))

  # Compare FIMS results with model comparison project OM values
  validate_fims(
    report = get_report(fit_age_length_comp_na),
    estimates = get_estimates(fit_age_length_comp_na),
    om_input = om_input_list[[iter_id]],
    om_output = om_output_list[[iter_id]],
    em_input = em_input_list[[iter_id]],
    use_fimsfit = TRUE
  )
})
