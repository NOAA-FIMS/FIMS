#' Run Retrospective Analysis for a FIMS Model
#'
#' @title Run Retrospective Analysis
#'
#' @description
#' Performs retrospective analysis by fitting a FIMS model object multiple times,
#' each time removing an increasing number of terminal years from the data.
#' This diagnostic tool is used to detect retrospective patterns or bias in
#' model estimates.
#'
#' @details
#' Retrospective analysis involves refitting a model after sequentially removing
#' one or more years of data from the end of the time series. This helps assess
#' whether the model estimates are stable and whether there is a consistent
#' pattern of revision as new data are added. The function runs models in
#' parallel for computational efficiency.
#'
#' The first element of `years_to_remove` should typically be 0 to represent
#' the reference model with no data removed. This allows for direct comparison
#' of retrospective peels to the base model.
#'
#' Retrospective patterns can indicate model misspecification, time-varying
#' processes not captured by the model, or data quality issues. Mohn's rho,
#' calculated by [calculate_mohns_rho()], provides a summary statistic for
#' retrospective bias.
#'
#' @param years_to_remove A numeric vector specifying the number of terminal
#'   years to remove for each retrospective peel. For example, `0:5` creates
#'   six model runs: the reference model (0 years removed) and five
#'   retrospective peels (1-5 years removed). Must contain non-negative integers.
#'   Minimum length is 1
#' @param data A data frame or FIMSFrame object containing the complete dataset
#'   used in the base model run. This should include all data types required
#'   by FIMS (landings, indices, composition data, biological data, etc.).
#' @param parameters A FIMS parameters object created by
#'   [setup_default_parameters()], containing the model configuration
#'   and initial parameter values for the base model.
#' @param n_cores An integer specifying the number of CPU cores to use for
#'   parallel processing. If `NULL` (default), uses `parallel::detectCores() - 1`.
#'   Set to 1 for sequential processing. Must be a positive integer.
#'
#' @return
#' A list with two named elements:
#' * `years_to_remove` - The input vector of years removed for each peel
#' * `estimates` - A data frame containing model estimates from all retrospective
#'   runs, with the following key columns:
#'   * `label` - Type of estimate (e.g., "spawning_biomass")
#'   * `year_i` - Year index for the estimate
#'   * `age_i` - Age index (if applicable)
#'   * `estimated` - Point estimate value
#'   * `uncertainty` - Standard error of the estimate
#'   * `retrospective_peel` - Number of years removed for this peel
#'
#' @references
#' Mohn, R. 1999. The retrospective problem in sequential population analysis:
#' An investigation using cod fishery and simulated data. ICES Journal of
#' Marine Science 56: 473-488.
#'
#' Hurtado-Ferro, F., et al. 2015. Looking in the rear-view mirror: bias and
#' retrospective patterns in integrated, age-structured stock assessment models.
#' ICES Journal of Marine Science 72(1): 99-110.
#'
#' @seealso
#' * [stockplotr::plot_spawning_biomass()] for visualizing retrospective results
#' * [calculate_mohns_rho()] for calculating Mohn's rho statistic
#' * [setup_default_parameters()] for creating parameter objects
#'
#' @family diagnostic_functions
#'
#' @export
#'
#' @importFrom rlang .data
#'
#' @keywords diagnostics
#'
#' @examples
#' \dontrun{
#' library(FIMS)
#'
#' # Use built-in dataset from FIMS
#' data("data_big")
#' data_4_model <- FIMSFrame(data_big)
#'
#' # Create a parameters object
#' parameters <- setup_default_parameters(data = data_4_model)
#'
#' # Run base model
#' base_model <- parameters |>
#'   initialize_fims(data = data_4_model) |>
#'   fit_fims(optimize = TRUE)
#'
#' # Run retrospective analysis with 5 peels
#' retro_fit <- run_fims_retrospective(
#'   years_to_remove = 0:5,
#'   data = data_big,
#'   parameters = parameters,
#'   n_cores = 1
#' )
#'
#' # View structure of results
#' names(retro_fit)
#' head(retro_fit$estimates)
#' }
#'
run_fims_retrospective <- function(
  years_to_remove,
  data,
  parameters,
  n_cores = NULL
) {
  # Validate years_to_remove
  if (length(years_to_remove) == 0) {
    cli::cli_abort("years_to_remove must have at least one value")
  }

  if (any(years_to_remove < 0)) {
    cli::cli_abort("years_to_remove must contain non-negative values")
  }

  # Set number of cores to use
  if (is.null(n_cores)) {
    n_cores_to_use <- parallel::detectCores() - 1
  } else {
    # Validate n_cores before conversion
    if (!is.numeric(n_cores) || n_cores %% 1 != 0 || n_cores <= 0) {
      cli::cli_abort("n_cores must be a positive integer. Input was {n_cores}")
    }
    n_cores_to_use <- as.integer(n_cores)
  }
  dplyr::case_when(
    n_cores_to_use == 1 ~ future::plan(future::sequential),
    n_cores_to_use > 1 & Sys.info()["sysname"] == "Windows" ~ future::plan(future::multisession, workers = n_cores_to_use),
    n_cores_to_use > 1 & Sys.info()["sysname"] != "Windows" ~ future::plan(future::multicore, workers = n_cores_to_use)
  )

  if (n_cores_to_use == 1) {
    cli::cli_alert_info("...Running sequentially on a single core")
  } else {
    cli::cli_alert_info("...Running in parallel on {n_cores_to_use} cores")
  }
  on.exit(future::plan(future::sequential), add = TRUE)

  # Run retro analyses in parallel
  estimates_list <- furrr::future_map(
    .x = years_to_remove,
    .f = function(years) {
      fit <- run_modified_data_fims(
        years_to_remove = years,
        data = data,
        parameters = parameters
      )
      FIMS::get_estimates(fit)
    },
    .options = furrr::furrr_options(seed = TRUE, globals = TRUE)
  )

  for (i in seq_along(estimates_list)) {
    estimates_list[[i]]$retrospective_peel <- years_to_remove[i]
  }
  estimates_df <- do.call(rbind, estimates_list)

  return(list("years_to_remove" = years_to_remove, "estimates" = estimates_df))
}
