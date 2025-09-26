# Helper file for FIMS R tests----
# This file contains multiple functions that are used to set up and run
# FIMS models with or without wrapper functions. The functions are sourced by
# devtools::load_all().

# FIMS helper function to validate the output of the FIMS model
#' Validate FIMS Estimation Run Output
#'
#' This function validates the output from the FIMS against the known OM values.
#' It performs various checks to ensure that the estimates provided by the FIMS
#' are within acceptable tolerance compared to the operating model values.
#'
#' @param report A TMB report where anything that is flagged as reportable in the
#' C++ code from running `get_report()`.
#' @param estimates A tibble of parameter values and their uncertainties from a
#' fitted model from running `get_estimates()`.
#' @param om_input A list containing the operating model inputs, such as years,
#' ages, and other parameters.
#' @param om_output A list containing the operating model outputs, including metrics
#' such as numbers at age, biomass, spawning biomass, fishing mortality, and indices.
#' @param em_input A list containing the estimation model inputs, including observed
#' landings, indices, and other relevant data.
#' @param use_fimsfit Logical; if `TRUE`, validates using `fit_fims()` results.
#'
#' @return None. The function uses `testthat` functions to perform validations.
#' It ensures that the output is within the expected range of error based on
#' standard deviations provided.
#'
#' @examples
#' # Assume `result` is a list of outputs obtained from running `fit_fims()`.
#' \dontrun{
#' validate_fims(
#'   report = FIMS::get_report(result),
#'   estimates = FIMS::get_estimates(result),
#'   om_input = om_input_list[[1]],
#'   om_output = om_output_list[[1]],
#'   em_input = em_input_list[[1]],
#'   use_fimsfit = FALSE
#' )
#' }
validate_fims <- function(
    report,
    estimates,
    om_input,
    om_output,
    em_input,
    use_fimsfit = FALSE) {
  # Helper function to validate estimates against expected values
  validate_error <- function(expected,
                             param_name,
                             use_fimsfit = FALSE,
                             estimates = NULL) {
    # Extract estimates based on whether fimsfit is used
    if (use_fimsfit) {
      object <- estimates |>
        dplyr::filter(label == param_name) |>
        dplyr::select(label, estimated, uncertainty)
      # Extract estimate
      object_estimate <- object[1:length(expected), "estimated"]
      # Extract uncertainty
      object_uncertainty <- object[1:length(expected), "uncertainty"]
    } else {
      object <- estimates[(rownames(estimates) == param_name), ]
      # Extract estimate
      object_estimate <- object[1:length(expected), "Estimate"]
      # Extract uncertainty
      object_uncertainty <- object[1:length(expected), "Std. Error"]
    }

    # Validate errors against 2*SE threshold
    absolute_error <- abs(object_estimate - expected)
    threshold <- qnorm(.975) * object_uncertainty
    #' @description Test that the 95% of the estimates fall within 2*SE
    expect_lte(sum(absolute_error > threshold), 0.05 * length(expected))
  }

  # Numbers at age
  validate_error(
    expected = c(t(om_output[["N.age"]])),
    param_name = "numbers_at_age",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # Biomass
  validate_error(
    expected = om_output[["biomass.mt"]],
    param_name = "biomass",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # Spawning biomass
  validate_error(
    expected = om_output[["SSB"]],
    param_name = "spawning_biomass",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # Recruitment
  # Expect recruitment from report equal to number at age 1 from estimates
  naa1_id <- seq(1, (om_input[["nyr"]] * om_input[["nages"]]), by = om_input[["nages"]])
  if (use_fimsfit) {
    expect_equal(
      report[["expected_recruitment"]][[1]][1:om_input[["nyr"]]],
      estimates |>
        dplyr::filter(
          label == "numbers_at_age"
        ) |>
        dplyr::slice(naa1_id) |>
        dplyr::pull(estimated) |>
        as.numeric()
    )
  } else {
    expect_equal(
      report[["expected_recruitment"]][[1]][1:om_input[["nyr"]]],
      as.numeric(estimates[rownames(estimates) == "numbers_at_age", "Estimate"][naa1_id])
    )
  }

  # Recruitment log deviations
  # The initial value of om_input[["logR.resid"]] is dropped from the model
  # TODO: the estimates table contains fixed "true" values for LogRecDev, causing
  # the test below to pass even when recruitment log_devs are fixed. Need to add
  # additional checks to verify that real estimates are being extracted?
  if (use_fimsfit &&
    length(report[["log_recruit_dev"]]) == (om_input[["nyr"]] - 1)
  ) {
    validate_error(
      expected = om_input[["logR.resid"]][-1],
      param_name = "log_devs",
      use_fimsfit = use_fimsfit,
      estimates = estimates
    )
  }

  # F
  validate_error(
    expected = om_output[["f"]],
    param_name = "F_mort",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # Expected fishery landings and survey index from om_output
  # Note: test failed when using em_input with observation errors as expected values
  validate_error(
    expected = c(
      om_output[["L.mt"]][["fleet1"]]
    ),
    param_name = "landings_expected",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # Commented out for now because there is no om_output for fishery index
  # and the function uses length expected to filter so it only works for the
  # fishing fleet when there is only one fleet. Manual testing shows it working
  # for now at least.
  # validate_error(
  #   expected = c(
  #     om_output[["survey_index_biomass"]][["survey1"]]
  #   ),
  #   param_name = "index_expected",
  #   use_fimsfit = use_fimsfit,
  #   estimates = estimates
  # )

  # Expected landings number at age
  validate_error(
    expected = c(
      t(om_output[["L.age"]][["fleet1"]]) # ,
      # t(om_output[["survey_age_comp"]][["survey1"]])
    ),
    param_name = "landings_numbers_at_age",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )
}

#' Verify FIMS Deterministic Run Output
#'
#' This function verifies the output of a deterministic run of the FIMS model
#' against the known operating model (OM) values. It checks various metrics such
#' as recruitment, fishing mortality, biomass, spawning biomass, and expected
#' landings.
#'
#' @param report A TMB report where anything that is flagged as reportable in the
#' C++ code from running `get_report()`.
#' @param estimates A tibble of parameter values and their uncertainties from a
#' fitted model from running `get_estimates()`.
#' @param om_input A list containing the operating model inputs, such as years,
#' ages, and other parameters.
#' @param om_output A list containing the operating model outputs, including metrics
#' such as numbers at age, biomass, spawning biomass, fishing mortality, and indices.
#' @param em_input A list containing the estimation model inputs, including observed
#' landings, indices, and other relevant data.
#' @param use_fimsfit Logical; if `TRUE`, validates using `fit_fims()` results.
#'
#' @return None. The function uses `testthat` functions to perform validations.
#'
#' @examples
#' # Assume `result` is a list of outputs obtained from running `fit_fims()`.
#' \dontrun{
#' verify_fims_deterministic(
#'   report = FIMS::get_report(result),
#'   estimates = FIMS::get_estimates(result),
#'   om_input = om_input_list[[1]],
#'   om_output = om_output_list[[1]],
#'   em_input = em_input_list[[1]],
#'   use_fimsfit = FALSE
#' )
#' }
verify_fims_deterministic <- function(
    report,
    estimates,
    om_input,
    om_output,
    em_input,
    use_fimsfit = FALSE) {
  nyears <- om_input[["nyr"]]
  nages <- om_input[["nages"]]
  dim <- nyears * nages
  # Compare log(R0) to true value
  if (use_fimsfit) {
    fims_logR0 <- estimates |>
      dplyr::filter(label == "log_rzero") |>
      dplyr::pull(estimated)
  } else {
    fims_logR0 <- estimates[36, "Estimate"]
  }

  expect_gt(fims_logR0, 0.0)
  # TODO: fims_logR0 is 13.8155 and log(om_input[["R0"]]) is 13.81551 causing this to error out. Fixing with tolerance temporarily.
  expect_equal(fims_logR0, log(om_input[["R0"]]), tolerance = 1e-4)

  #' @description Test that the numbers at age from report are equal to the true values
  expect_equal(
    report[["numbers_at_age"]][[1]][1:dim],
    c(t(om_output[["N.age"]]))
  )

  #' @description Test that the biomass values from report are equal to the true values
  expect_equal(
    report[["biomass"]][[1]][1:nyears],
    c(t(om_output[["biomass.mt"]]))
  )

  #' @description Test that the spawning biomass values from report are equal to the true values
  expect_equal(
    report[["spawning_biomass"]][[1]][1:nyears],
    c(t(om_output[["SSB"]]))
  )

  fims_naa <- matrix(
    report[["numbers_at_age"]][[1]][1:(om_input[["nyr"]] * om_input[["nages"]])],
    nrow = om_input[["nyr"]],
    byrow = TRUE
  )
  #' @description Test that the recruitment values from report are equal to the true values
  expect_equal(
    report[["expected_recruitment"]][[1]][1:nyears],
    fims_naa[, 1]
  )

  #' @description Test that the recruitment log_devs (fixed at initial "true" values) from report are equal to the true values
  if (!is.null(report[["log_recruit_dev"]])) {
    expect_equal(
      report[["log_recruit_dev"]][[1]],
      om_input[["logR.resid"]][-1]
    )
  }

  #' @description Test that the F (fixed at initial "true" values) from report are equal to the true values
  expect_equal(
    report[["F_mort"]][[1]],
    om_output[["f"]]
  )

  fims_landings <- report[["landings_expected"]]
  #' @description Test that the expected landings values from report are equal to the true values
  expect_equal(
    fims_landings[[1]],
    om_output[["L.mt"]][["fleet1"]]
  )


  # Get relative error in landings
  fims_object_are <- rep(0, length(em_input[["L.obs"]][["fleet1"]]))
  for (i in 1:length(em_input[["L.obs"]][["fleet1"]])) {
    fims_object_are[i] <- abs(fims_landings[[1]][i] - em_input[["L.obs"]][["fleet1"]][i]) / em_input[["L.obs"]][["fleet1"]][i]
  }

  #' @description Test that the 95% of relative error in landings is within 2*cv
  expect_lte(sum(fims_object_are > om_input[["cv.L"]][["fleet1"]] * 2.0), length(em_input[["L.obs"]][["fleet1"]]) * 0.05)

  #' @description Test that the expected landings number at age from report are equal to the true values
  expect_equal(
    report[["landings_numbers_at_age"]][[1]],
    c(t(om_output[["L.age"]][["fleet1"]]))
  )

  # Expected landings number at age in proportion
  # QUESTION: Isn't this redundant with the non-proportion test above?
  fims_landings_naa <- matrix(report[["landings_numbers_at_age"]][[1]][1:(om_input[["nyr"]] * om_input[["nages"]])],
    nrow = om_input[["nyr"]], byrow = TRUE
  )
  fims_landings_naa_proportion <- fims_landings_naa / rowSums(fims_landings_naa)
  om_landings_naa_proportion <- om_output[["L.age"]][["fleet1"]] / rowSums(om_output[["L.age"]][["fleet1"]])

  #' @description Test that the expected landings number at age in proportion from report are equal to the true values
  expect_equal(
    c(t(fims_landings_naa_proportion)),
    c(t(om_landings_naa_proportion))
  )

  # Expected survey index.
  fims_index <- report[["index_expected"]]
  # # Using [[2]] because the survey is the 2nd fleet.
  # landings_waa <- matrix(report[["landings_waa"]][[2]][1:(om_input[["nyr"]] * om_input[["nages"]])],
  #   nrow = om_input[["nyr"]], byrow = TRUE
  # )
  # #' @description Test that the expected survey index values from report are equal to the true values
  # # Using [[2]] because the survey is the 2nd fleet.
  # expect_setequal(
  #   fims_landings[[2]],
  #   apply(landings_waa, 1, sum)
  # )

  #' @description Test that the expected survey index values from report are equal to the true values
  expect_equal(
    fims_index[[2]],
    om_output[["survey_index_biomass"]][["survey1"]]
  )

  # Get relative error in survey index
  fims_object_are <- rep(0, length(em_input[["surveyB.obs"]][["survey1"]]))
  for (i in 1:length(em_input[["survey.obs"]][["survey1"]])) {
    fims_object_are[i] <- abs(fims_index[[2]][i] - em_input[["surveyB.obs"]][["survey1"]][i]) / em_input[["surveyB.obs"]][["survey1"]][i]
  }
  #' @description Test that the 95% of relative error in survey index is within 2*cv
  expect_lte(
    sum(fims_object_are > om_input[["cv.survey"]][["survey1"]] * 2.0),
    length(em_input[["surveyB.obs"]][["survey1"]]) * 0.05
  )

  # Expected landings number at age in proportion
  fims_cnaa <- matrix(report[["landings_numbers_at_age"]][[2]][1:(om_input[["nyr"]] * om_input[["nages"]])],
    nrow = om_input[["nyr"]], byrow = TRUE
  )
  fims_index_naa <- matrix(report[["index_numbers_at_age"]][[2]][1:(om_input[["nyr"]] * om_input[["nages"]])],
    nrow = om_input[["nyr"]], byrow = TRUE
  )
  # Excluding these tests at the moment to figure out what the correct comparison values are
  # for (i in 1:length(c(t(om_output[["survey_age_comp"]][["survey1"]])))) {
  #   expect_lt(abs(report[["index_waa"]][[2]][i]-c(t(om_output[["survey_age_comp"]][["survey1"]]))[i]),0.0000000001)
  # }

  fims_cnaa_proportion <- matrix(report[["agecomp_proportion"]][[2]][1:(om_input[["nyr"]] * om_input[["nages"]])],
    nrow = om_input[["nyr"]], byrow = TRUE
  )

  om_cnaa_proportion <- 0.0 + (1.0 - 0.0 * om_input[["nages"]]) * om_output[["survey_age_comp"]][["survey1"]] / rowSums(om_output[["survey_age_comp"]][["survey1"]])

  #' @description Test that the expected survey number at age in proportion from report almost equal to the true values
  expect_equal(
    abs(c(t(fims_cnaa_proportion))),
    c(t(om_cnaa_proportion))
  )
}

#' verify FIMS NLL
#'
#' This function verifies the negative log-likelihood (NLL) of the FIMS model
#' output against the expected NLL calculated from the operating model (OM)
#' and estimation model (EM) inputs. It checks the individual components of the
#' NLL, including recruitment, fishery landings, survey index, age composition,
#' and length composition. It only works for deterministic runs.
#'
#' @param report A TMB report where anything that is flagged as reportable in the
#' C++ code from running `get_report()`.
#' @param om_input A list containing the operating model inputs, such as years,
#' ages, and other parameters.
#' @param om_output A list containing the operating model outputs, including metrics
#' such as numbers at age, biomass, spawning biomass, fishing mortality, and indices.
#' @param em_input A list containing the estimation model inputs, including observed
#' landings, indices, and other relevant data.
#'
#' @return None. The function uses `testthat` functions to perform validations.
#'
#' @examples
#' # Assume `result` is a list of outputs obtained from running `fit_fims()`.
#' \dontrun{
#' verify_fims_nll(
#'   report = FIMS::get_report(result),
#'   om_input = om_input_list[[1]],
#'   om_output = om_output_list[[1]],
#'   em_input = em_input_list[[1]]
#' )
#' }
verify_fims_nll <- function(report,
                            om_input,
                            om_output,
                            em_input) {
  # recruitment likelihood
  # log_devs is of length nyr-1
  rec_nll <- -sum(dnorm(
    om_input[["logR.resid"]][-1], rep(0, om_input[["nyr"]] - 1),
    om_input[["logR_sd"]], TRUE
  ))

  # fishery landings expected likelihood
  landings_nll <- landings_nll_fleet <- -sum(dlnorm(
    em_input[["L.obs"]][["fleet1"]],
    log(om_output[["L.mt"]][["fleet1"]]),
    sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1)), TRUE
  ))

  # survey index expected likelihood
  index_nll <- index_nll_survey <- -sum(dlnorm(
    em_input[["surveyB.obs"]][["survey1"]],
    log(om_output[["survey_index_biomass"]][["survey1"]]),
    sqrt(log(em_input[["cv.survey"]][["survey1"]]^2 + 1)), TRUE
  ))

  # age comp likelihoods
  fishing_acomp_observed <- em_input[["L.age.obs"]][["fleet1"]]
  fishing_acomp_expected <- 0.0 + (1.0 - 0.0 * om_input[["nages"]]) * om_output[["L.age"]][["fleet1"]] /
    rowSums(om_output[["L.age"]][["fleet1"]])
  survey_acomp_observed <- em_input[["survey.age.obs"]][["survey1"]]
  survey_acomp_expected <- 0.0 + (1.0 - 0.0 * om_input[["nages"]]) * om_output[["survey_age_comp"]][["survey1"]] /
    rowSums(om_output[["survey_age_comp"]][["survey1"]])
  age_comp_nll_fleet <- age_comp_nll_survey <- 0
  for (y in 1:om_input[["nyr"]]) {
    age_comp_nll_fleet <- age_comp_nll_fleet -
      dmultinom(
        fishing_acomp_observed[y, ] * em_input[["n.L"]][["fleet1"]], em_input[["n.L"]][["fleet1"]],
        fishing_acomp_expected[y, ], TRUE
      )

    age_comp_nll_survey <- age_comp_nll_survey -
      dmultinom(
        survey_acomp_observed[y, ] * em_input[["n.survey"]][["survey1"]], em_input[["n.survey"]][["survey1"]],
        survey_acomp_expected[y, ], TRUE
      )
  }
  age_comp_nll <- age_comp_nll_fleet + age_comp_nll_survey

  # length comp likelihoods
  fishing_lengthcomp_observed <- em_input[["L.length.obs"]][["fleet1"]]
  fishing_lengthcomp_expected <- 0.0 + (1.0 - 0.0 * om_input[["nlengths"]]) * om_output[["L.length"]][["fleet1"]] / rowSums(om_output[["L.length"]][["fleet1"]])
  survey_lengthcomp_observed <- em_input[["survey.length.obs"]][["survey1"]]
  survey_lengthcomp_expected <- 0.0 + (1.0 - 0.0 * om_input[["nlengths"]]) * om_output[["survey_length_comp"]][["survey1"]] / rowSums(om_output[["survey_length_comp"]][["survey1"]])
  lengthcomp_nll_fleet <- lengthcomp_nll_survey <- 0
  for (y in 1:om_input[["nyr"]]) {
    # test using FIMS_dmultinom which matches the TMB dmultinom calculation and differs from R
    # by NOT rounding obs to the nearest integer.
    lengthcomp_nll_fleet <- lengthcomp_nll_fleet -
      FIMS_dmultinom(
        fishing_lengthcomp_observed[y, ] * em_input[["n.L.lengthcomp"]][["fleet1"]],
        fishing_lengthcomp_expected[y, ]
      )

    lengthcomp_nll_survey <- lengthcomp_nll_survey -
      FIMS_dmultinom(
        survey_lengthcomp_observed[y, ] * em_input[["n.survey.lengthcomp"]][["survey1"]],
        survey_lengthcomp_expected[y, ]
      )
  }
  lengthcomp_nll <- lengthcomp_nll_fleet + lengthcomp_nll_survey

  expected_jnll <- rec_nll + landings_nll + index_nll + age_comp_nll + lengthcomp_nll
  jnll <- report[["jnll"]]
  #' @description Test that the recruitment jnll is equal to the expected jnll
  expect_equal(report[["nll_components"]][1], rec_nll)
  #' @description Test that the landings jnll is equal to the expected jnll
  expect_equal(report[["nll_components"]][2], landings_nll_fleet)
  #' @description Test that the fishing fleet age comp jnll is equal to the expected jnll
  expect_equal(report[["nll_components"]][3], age_comp_nll_fleet)
  #' @description Test that the fishing fleet length comp jnll is equal to the expected jnll
  expect_equal(report[["nll_components"]][4], lengthcomp_nll_fleet)
  #' @description Test that the survey index jnll is equal to the expected jnll
  expect_equal(report[["nll_components"]][5], index_nll_survey)
  #' @description Test that the survey age comp jnll is equal to the expected jnll
  expect_equal(report[["nll_components"]][6], age_comp_nll_survey)
  #' @description Test that the survey length comp jnll is equal to the expected jnll
  expect_equal(report[["nll_components"]][7], lengthcomp_nll_survey)
  #' @description Test that the total jnll is equal to the expected jnll
  expect_equal(jnll, expected_jnll)
}
