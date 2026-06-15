#' Calculate Mohn's rho for retrospective analysis
#'
#' @title Calculate Mohn's rho for retrospective analysis
#'
#' @description
#' Calculates Mohn's rho, a diagnostic statistic that summarizes the bias in
#' retrospective analysis. The statistic measures the average relative difference
#' between estimates from retrospective peels and the reference model at the
#' corresponding terminal year of each peel. A value close to zero suggests
#' minimal retrospective bias, while larger absolute values indicate potential
#' model misspecification or data conflicts.
#'
#' The formula for Mohn's rho is:
#' \deqn{\rho = \frac{1}{n} \sum_{i=1}^{n} \frac{x_{peel,i} - x_{base,i}}{x_{base,i}}}
#' where n is the number of retrospective peels, \eqn{x_{peel,i}} is the
#' estimated value from peel i at its terminal year, and \eqn{x_{base,i}} is
#' the value from the base model at that same year.
#'
#' @param retro_fit List returned by the [run_fims_retrospective()] function
#' containing `years_to_remove` (vector of years peeled) and `estimates`
#' (data frame with retrospective results)
#' @param quantity Character string specifying which quantity to calculate
#' Mohn's rho for (e.g., "spawning_biomass", "expected_recruitment")
#'
#' @return A numeric value representing Mohn's rho. Values close to zero indicate
#' minimal retrospective bias. Positive values suggest the model retrospectively
#' underestimates the quantity, while negative values suggest overestimation.
#'
#' @references
#' Mohn, R. (1999). The retrospective problem in sequential population analysis:
#' An investigation using cod fishery and simulated data. ICES Journal of Marine
#' Science, 56(4), 473-488.
#'
#' Hurtado-Ferro, F., Szuwalski, C.S., Valero, J.L., Anderson, S.C., Cunningham,
#' C.J., Johnson, K.F., Licandeo, R., McGilliard, C.R., Monnahan, C.C., Muradian,
#' M.L., Ono, K., Vert-Pre, K.A., Whitten, A.R., and Punt, A.E. (2015). Looking
#' in the rear-view mirror: bias and retrospective patterns in integrated,
#' age-structured stock assessment models. ICES Journal of Marine Science, 72(1),
#' 99-110.
#'
#' @export
#'
#' @importFrom rlang .data
#'
#' @examples
#' \dontrun{
#' library(FIMS)
#' # Use built-in dataset from FIMS
#' data("data_big")
#' # Prepare data for FIMS model
#' data_4_model <- FIMSFrame(data_big)
#' # Create parameters
#' parameters <- data_4_model |>
#'   create_default_configurations() |>
#'   create_default_parameters(data = data_4_model)
#'
#' # Run retrospective analysis
#' retro_fit <- run_fims_retrospective(
#'   years_to_remove = 0:5,
#'   data = data_big,
#'   parameters = parameters,
#'   n_cores = 1
#' )
#'
#' # Calculate Mohn's rho for spawning biomass
#' rho_ssb <- calculate_mohns_rho(retro_fit, quantity = "spawning_biomass")
#'
#' # Calculate Mohn's rho for recruitment
#' rho_rec <- calculate_mohns_rho(retro_fit, quantity = "expected_recruitment")
#' }
calculate_mohns_rho <- function(retro_fit, quantity) {
  # Input validation
  if (!is.list(retro_fit)) {
    cli::cli_abort("{.arg retro_fit} must be a list")
  }

  if (!all(c("years_to_remove", "estimates") %in% names(retro_fit))) {
    cli::cli_abort("{.arg retro_fit} must contain 'years_to_remove' and 'estimates' elements")
  }

  if (!is.character(quantity) || length(quantity) != 1) {
    cli::cli_abort("{.arg quantity} must be a single character string")
  }

  # Check that the first model has 0 years peeled (i.e., it's the reference model)
  if (0 != retro_fit[["years_to_remove"]][[1]]) {
    cli::cli_abort("{.arg retro_fit} must contain reference year run")
  }

  # Filter estimates for the specified quantity
  retro_estimates <- retro_fit[["estimates"]] |>
    dplyr::filter(.data$label %in% quantity)

  # Check that the quantity exists in the estimates
  if (nrow(retro_estimates) == 0) {
    cli::cli_abort("Quantity {.val {quantity}} not found in estimates. Check the {.field label} column")
  }

  # Initialize vector to store relative differences
  mohn_values <- numeric()

  # Calculate vector of ending year for each model
  end_year <- max(retro_estimates$year_i, na.rm = TRUE) - retro_fit$years_to_remove

  # Calculate relative differences for each peel (excluding reference model)
  for (i in 2:length(retro_fit$years_to_remove)) {
    # Get value for ending year of peeled model
    peel_value <- retro_estimates |>
      dplyr::filter(.data$year_i == end_year[i]) |>
      dplyr::filter(.data$retrospective_peel == retro_fit$years_to_remove[i]) |>
      dplyr::pull(.data$estimated)

    # Get value from the same year for the reference model
    ref_value <- retro_estimates |>
      dplyr::filter(.data$year_i == end_year[i]) |>
      dplyr::filter(.data$retrospective_peel == retro_fit$years_to_remove[1]) |>
      dplyr::pull(.data$estimated)

    # Check for missing or invalid values
    if (length(peel_value) == 0 || length(ref_value) == 0) {
      cli::cli_warn("Missing value for peel {i-1} at year {end_year[i]}, skipping this peel")
      next
    }

    if (is.na(peel_value) || is.na(ref_value)) {
      cli::cli_warn("NA value encountered for peel {i-1} at year {end_year[i]}, skipping this peel")
      next
    }

    if (ref_value == 0) {
      cli::cli_warn("Reference value is zero for peel {i-1} at year {end_year[i]}, skipping this peel")
      next
    }

    # Calculate the relative difference
    # (store in the i-1 position because model 1 is the reference model)
    mohn_values[i - 1] <- (peel_value - ref_value) / ref_value
  }

  # Check if we have any valid values
  if (length(mohn_values) == 0) {
    cli::cli_abort("No valid retrospective peels found to calculate Mohn's rho")
  }

  # Calculate mean rho by averaging across peels (not including the reference year)
  mohn_rho <- mean(mohn_values, na.rm = TRUE)

  return(mohn_rho)
}
