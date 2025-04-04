# Helper file for FIMS R tests----
# This file contains multiple functions that are used to set up and run  
# FIMS models with or without wrapper functions. The functions are sourced by 
# devtools::load_all().

# FIMS helper function to validate the output of the FIMS model
#' Validate FIMS Model Output
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
#' such as numbers at age, biomass, spawning biomass, fishing mortality, and survey indices.
#' @param em_input A list containing the estimation model inputs, including observed
#' landingses, survey indices, and other relevant data.
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
        dplyr::select(label, estimate, uncertainty)
      # Extract estimate
      object_estimate <- object[1:length(expected), "estimate"]
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
    expect_lte(sum(absolute_error > threshold), 0.05 * length(expected))
  }

  # Numbers at age
  validate_error(
    expected = c(t(om_output[["N.age"]])),
    param_name = "NAA",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # Biomass
  validate_error(
    expected = om_output[["biomass.mt"]],
    param_name = "Biomass",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # Spawning biomass
  validate_error(
    expected = om_output[["SSB"]],
    param_name = "SSB",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # Recruitment
  # Expect recruitment from report equal to number at age 1 from estimates
  naa1_id <- seq(1, (om_input[["nyr"]] * om_input[["nages"]]), by = om_input[["nages"]])
  if (use_fimsfit) {
    expect_equal(
      report[["recruitment"]][[1]][1:om_input[["nyr"]]],
      estimates |>
        dplyr::filter(
          label == "NAA" 
        ) |>
        dplyr::slice(naa1_id) |>
        dplyr::pull(estimate) |>
        as.numeric()
    )
  } else {
    expect_equal(
      report[["recruitment"]][[1]][1:om_input[["nyr"]]],
      as.numeric(estimates[rownames(estimates) == "NAA", "Estimate"][naa1_id])
    )
  }
  
  # Recruitment log deviations
  # The initial value of om_input[["logR.resid"]] is dropped from the model
  # TODO: the estimates table contains fixed "true" values for LogRecDev, causing
  # the test below to pass even when recruitment log_devs are fixed. Need to add
  # additional checks to verify that real estimates are being extracted?
  validate_error(
    expected = om_input[["logR.resid"]][-1],
    param_name = "LogRecDev",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # F
  validate_error(
    expected = om_output[["f"]],
    param_name = "FMort",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  # Expected fishery landings and survey index from om_output
  # Note: test failed when using em_input with observation errors as expected values
  validate_error(
    expected = c(
      om_output[["L.mt"]][["fleet1"]]
    ),
    param_name = "ExpectedLandings",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )

  #Commented out for now because ther is now om_output for fishery index
  #and the function uses length expected to filter so it only works for the
  #fishing fleet when there is only one fleet. Manual testing shows it working
  #for now at least.
  # validate_error(
  #   expected = c(
  #     om_output[["survey_index_biomass"]][["survey1"]]
  #   ),
  #   param_name = "ExpectedIndex",
  #   use_fimsfit = use_fimsfit,
  #   estimates = estimates
  # )

  # Expected survey number at age
  validate_error(
    expected = c(
      t(om_output[["L.age"]][["fleet1"]])#,
      #t(om_output[["survey_age_comp"]][["survey1"]])
    ),
    param_name = "CNAA",
    use_fimsfit = use_fimsfit,
    estimates = estimates
  )
}