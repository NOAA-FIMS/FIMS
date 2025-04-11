# Load necessary data for the integration test
load(test_path("fixtures", "integration_test_data.RData"))

# Set the iteration ID to 1 for accessing specific input/output list
iter_id <- 1

# Define modified parameters for different modules
modified_parameters <- readRDS(testthat::test_path(
  "fixtures",
  "parameters_model_comparison_project.RDS"
))

test_that("deterministic test of fims", {
  # Load the test data from an RDS file containing the model fit
  deterministic_age_length_comp <- readRDS(test_path("fixtures", "deterministic_age_length_comp.RDS"))

  # Call report using deterministic parameter values
  # obj[["report"]]() requires parameter list to avoid errors
  report <- get_report(deterministic_age_length_comp)
  estimates <- get_estimates(deterministic_age_length_comp)

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

  # Expected landings
  fims_landings <- report[["landings_exp"]]
  for (i in 1:length(om_output_list[[iter_id]][["L.mt"]][["fleet1"]])) {
    expect_equal(fims_landings[[1]][i], om_output_list[[iter_id]][["L.mt"]][["fleet1"]][i])
  }

  # Expect small relative error for deterministic test
  fims_object_are <- rep(0, length(em_input_list[[iter_id]][["L.obs"]][["fleet1"]]))
  for (i in 1:length(em_input_list[[iter_id]][["L.obs"]][["fleet1"]])) {
    fims_object_are[i] <- abs(fims_landings[[1]][i] - em_input_list[[iter_id]][["L.obs"]][["fleet1"]][i]) / em_input_list[[iter_id]][["L.obs"]][["fleet1"]][i]
  }

  # Expect 95% of relative error to be within 2*cv
  expect_lte(sum(fims_object_are > om_input_list[[iter_id]][["cv.L"]][["fleet1"]] * 2.0), length(em_input_list[[iter_id]][["L.obs"]][["fleet1"]]) * 0.05)

  # Compare expected landings number at age to true values
  for (i in 1:length(c(t(om_output_list[[iter_id]][["L.age"]][["fleet1"]])))) {
    expect_equal(report[["landings_naa"]][[1]][i], c(t(om_output_list[[iter_id]][["L.age"]][["fleet1"]]))[i])
  }

  # Expected landings number at age in proportion
  # QUESTION: Isn't this redundant with the non-proportion test above?
  fims_landings_naa <- matrix(report[["landings_naa"]][[1]][1:(om_input_list[[iter_id]][["nyr"]] * om_input_list[[iter_id]][["nages"]])],
    nrow = om_input_list[[iter_id]][["nyr"]], byrow = TRUE
  )
  fims_landings_naa_proportion <- fims_landings_naa / rowSums(fims_landings_naa)
  om_landings_naa_proportion <- om_output_list[[iter_id]][["L.age"]][["fleet1"]] / rowSums(om_output_list[[iter_id]][["L.age"]][["fleet1"]])

  for (i in 1:length(c(t(om_landings_naa_proportion)))) {
    expect_equal(c(t(fims_landings_naa_proportion))[i], c(t(om_landings_naa_proportion))[i])
  }

  # Expected survey index.
  fims_index <- report[["index_exp"]]
  # Using [[2]] because the survey is the 2nd fleet.
  landings_waa <- matrix(report[["landings_waa"]][[2]][1:(om_input_list[[iter_id]][["nyr"]] * om_input_list[[iter_id]][["nages"]])],
    nrow = om_input_list[[iter_id]][["nyr"]], byrow = TRUE
  )
  expect_equal(fims_landings[[2]], apply(landings_waa, 1, sum))# * om_output_list[[iter_id]][["survey_q"]][["survey1"]])

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

  # Expected landings number at age in proportion
  fims_cnaa <- matrix(report[["landings_naa"]][[2]][1:(om_input_list[[iter_id]][["nyr"]] * om_input_list[[iter_id]][["nages"]])],
    nrow = om_input_list[[iter_id]][["nyr"]], byrow = TRUE
  )
  fims_index_naa <- matrix(report[["index_naa"]][[2]][1:(om_input_list[[iter_id]][["nyr"]] * om_input_list[[iter_id]][["nages"]])],
                           nrow = om_input_list[[iter_id]][["nyr"]], byrow = TRUE
  )
  # Excluding these tests at the moment to figure out what the correct comparison values are
  # for (i in 1:length(c(t(om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]])))) {
  #   expect_lt(abs(report[["index_waa"]][[2]][i]-c(t(om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]]))[i]),0.0000000001)
  # }

  fims_cnaa_proportion <- matrix(report[["comp_pnaa"]][[2]][1:(om_input_list[[iter_id]][["nyr"]] * om_input_list[[iter_id]][["nages"]])],
                                 nrow = om_input_list[[iter_id]][["nyr"]], byrow = TRUE
  )

  om_cnaa_proportion <- 0.0 + (1.0-0.0*om_input[["nages"]])*om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]] / rowSums(om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]])

  # Ensure the proportions match for the survey age composition
  for (i in 1:length(c(t(om_cnaa_proportion)))) {
     expect_lt(abs(c(t(fims_cnaa_proportion))[i]-c(t(om_cnaa_proportion))[i]),0.00000001)
  }
})

test_that("nll test of fims", {
  # Load the test data from an RDS file containing the model fit
  deterministic_age_length_comp <- readRDS(test_path("fixtures", "deterministic_age_length_comp.RDS"))

  # Set up TMB's computational graph
  report <- get_report(deterministic_age_length_comp)
  estimates <- get_estimates(deterministic_age_length_comp)

  # recruitment likelihood
  # log_devs is of length nyr-1
  rec_nll <- -sum(dnorm(
    om_input_list[[iter_id]][["logR.resid"]][-1], rep(0, om_input_list[[iter_id]][["nyr"]] - 1),
    om_input_list[[iter_id]][["logR_sd"]], TRUE
  ))

  # fishery landings expected likelihood
  landings_nll <- landings_nll_fleet <- -sum(dlnorm(
    em_input_list[[iter_id]][["L.obs"]][["fleet1"]],
    log(om_output_list[[iter_id]][["L.mt"]][["fleet1"]]),
    sqrt(log(em_input_list[[iter_id]][["cv.L"]][["fleet1"]]^2 + 1)), TRUE
  ))

  # survey index expected likelihood
  index_nll <- index_nll_survey <- -sum(dlnorm(
    em_input_list[[iter_id]][["surveyB.obs"]][["survey1"]],
    log(om_output_list[[iter_id]][["survey_index_biomass"]][["survey1"]]),
    sqrt(log(em_input_list[[iter_id]][["cv.survey"]][["survey1"]]^2 + 1)), TRUE
  ))

  # age comp likelihoods
  fishing_acomp_observed <- em_input_list[[iter_id]][["L.age.obs"]][["fleet1"]]
  fishing_acomp_expected <- 0.0 + (1.0-0.0*om_input[["nages"]])*om_output_list[[iter_id]][["L.age"]][["fleet1"]] / rowSums(om_output_list[[iter_id]][["L.age"]][["fleet1"]])
  survey_acomp_observed <- em_input_list[[iter_id]][["survey.age.obs"]][["survey1"]]
  survey_acomp_expected <- 0.0 + (1.0-0.0*om_input[["nages"]])*om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]] / rowSums(om_output_list[[iter_id]][["survey_age_comp"]][["survey1"]])
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
  fishing_lengthcomp_expected <- 0.0 + (1.0-0.0*om_input[["nlengths"]])*om_output_list[[iter_id]][["L.length"]][["fleet1"]] / rowSums(om_output_list[[iter_id]][["L.length"]][["fleet1"]])
  survey_lengthcomp_observed <- em_input_list[[iter_id]][["survey.length.obs"]][["survey1"]]
  survey_lengthcomp_expected <- 0.0 + (1.0-0.0*om_input[["nlengths"]])*om_output_list[[iter_id]][["survey_length_comp"]][["survey1"]] / rowSums(om_output_list[[iter_id]][["survey_length_comp"]][["survey1"]])
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

  expected_jnll <- rec_nll + landings_nll + index_nll + age_comp_nll + lengthcomp_nll
  jnll <- report[["jnll"]]

  expect_lt(abs(report[["nll_components"]][1] - rec_nll),0.000000001)
  expect_lt(abs(report[["nll_components"]][2] - landings_nll_fleet), 0.000000001)
  expect_lt(abs(report[["nll_components"]][3] - age_comp_nll_fleet), 0.000000001)
  expect_lt(abs(report[["nll_components"]][4] - lengthcomp_nll_fleet), 0.000000001)
  expect_lt(abs(report[["nll_components"]][5] - index_nll_survey), 0.000000001)
  expect_lt(abs(report[["nll_components"]][6] - age_comp_nll_survey), 0.000000001)
  expect_lt(abs(report[["nll_components"]][7] - lengthcomp_nll_survey), 0.000000001)
  expect_lt(abs(jnll - expected_jnll), 0.000000001)
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
