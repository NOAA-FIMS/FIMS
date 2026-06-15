#' Run Likelihood Profile for a FIMS Model
#'
#' @title Run Likelihood Profile
#'
#' @description
#' Conducts likelihood profile analysis for a FIMS model by fixing a specified
#' parameter at a range of values and re-optimizing all other parameters. This
#' diagnostic tool helps assess parameter uncertainty, identifiability, and
#' potential local minima in the likelihood surface.
#'
#' @details
#' Likelihood profiling involves systematically varying a parameter of interest
#' across a range of values while optimizing all other parameters. At each fixed
#' value, the negative log-likelihood is recorded, creating a profile of the
#' likelihood surface. This reveals whether the parameter is well-estimated
#' (sharp, parabolic profile) or poorly identified (flat profile), and can
#' identify multiple modes or asymmetric confidence regions.
#'
#' The function profiles over values specified relative to the estimated value
#' from the base model. For example, with `min = -2` and `max = 2`, the profile
#' spans from 2 units below to 2 units above the estimated value. The profiled
#' values are evenly spaced on the parameter scale (not log scale).
#'
#' Models are run in parallel for computational efficiency. The function
#' automatically handles parameter identification using module names when needed
#' to distinguish between multiple instances of the same parameter type.
#'
#' @param model A FIMSFit object returned by [FIMS::fit_fims()]. Used to extract
#'   the estimated value of the parameter being profiled
#' @param parameters A FIMS parameters object created by
#'   [FIMS::create_default_parameters()], containing the model configuration
#'   and initial parameter values
#' @param data A data frame, tibble, or FIMSFrame object containing the model
#'   data. This should include all data types required by FIMS
#' @param module_name A character string specifying the module containing the
#'   parameter to profile. Default is `NULL`. Required when the parameter name
#'   exists in multiple modules (e.g., multiple fleets). Examples include
#'   `"fleet1"`, `"survey1"`, or `"recruitment"`
#' @param parameter_name A character string specifying the parameter to profile.
#'   Default is `"log_rzero"`. Must match a parameter name in the FIMS model.
#'   Common options include `"log_rzero"`, `"log_sigma_recruit"`, `"logit_steep"`,
#'   or selectivity parameters
#' @param n_cores An integer specifying the number of CPU cores to use for
#'   parallel processing. If `NULL` (default), uses `parallel::detectCores() - 1`.
#'   Set to 1 for sequential processing. Must be a positive integer
#' @param min A numeric value specifying the minimum offset from the estimated
#'   parameter value. Default is `-2`. The profile range starts at
#'   `estimated_value + min`
#' @param max A numeric value specifying the maximum offset from the estimated
#'   parameter value. Default is `2`. The profile range ends at
#'   `estimated_value + max`. Must be greater than `min`
#' @param length An integer specifying the number of points in the likelihood
#'   profile. Default is `5`. Use an odd number to include the estimated value.
#'   Must be a positive integer. Values above 50 will generate a warning
#'
#' @return
#' A list with two named elements:
#' * `vec` - Numeric vector of parameter values used in the profile
#' * `estimates` - Data frame containing model estimates for each profiled value,
#'   with the following key columns:
#'   * `label` - Type of estimate (e.g., "spawning_biomass", "recruitment")
#'   * `year_i` - Year index for the estimate
#'   * `age_i` - Age index (if applicable)
#'   * `estimated` - Point estimate value
#'   * `uncertainty` - Standard error of the estimate
#'   * `lpdf` - Log probability density (negative log-likelihood component)
#'   * `value_{parameter_name}` - The fixed parameter value for this profile point
#'
#' @references
#' Venzon, D.J. and Moolgavkar, S.H. 1988. A method for computing
#' profile-likelihood-based confidence intervals. Applied Statistics 37: 87-94.
#'
#' Maunder, M.N. and Punt, A.E. 2013. A review of integrated analysis in
#' fisheries stock assessment. Fisheries Research 142: 61-74.
#'
#' @seealso
#' * [plot_likelihood()] for visualizing likelihood profiles
#' * [FIMS::fit_fims()] for fitting FIMS models
#' * [FIMS::create_default_parameters()] for creating parameter objects
#'
#' @family diagnostic_functions
#'
#' @export
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
#' parameters <- data_4_model |>
#'   create_default_configurations() |>
#'   create_default_parameters(data = data_4_model)
#'
#' # Run the base model with optimization
#' base_model <- parameters |>
#'   initialize_fims(data = data_4_model) |>
#'   fit_fims(optimize = TRUE)
#'
#' # Run likelihood profile for log_rzero
#' like_fit <- run_fims_likelihood(
#'   model = base_model,
#'   parameters = parameters,
#'   data = data_big,
#'   parameter_name = "log_rzero",
#'   n_cores = 3,
#'   min = -1,
#'   max = 1,
#'   length = 21
#' )
#'
#' # View the profiled parameter values
#' like_fit$vec
#'
#' # View structure of estimates
#' head(like_fit$estimates)
#' }
#'
run_fims_likelihood <- function(
  model,
  parameters,
  data,
  module_name = NULL,
  parameter_name = "log_rzero",
  n_cores = NULL,
  min = -2,
  max = 2,
  length = 5
  # TODO: check inputs to make sure they make sense
) {
  # checking inputs
  if (length < 1 | as.integer(length) != length) {
    cli::cli_abort("Input length should be a positive integer ")
  }

  if (length > 50) {
    cli::cli_warn("Input length is {length}, are you sure you want it so large?")
  }

  if (min >= max) {
    cli::cli_abort("Input min should be less than max.")
  }

  if (class(model) != "FIMSFit") {
    cli::cli_abort("Input model needs to be a FIMSFit object.")
  }
  # calculate vector
  values <- seq(min, max, length = length)

  if (!0 %in% values) {
    cli::cli_warn("Inputs min and max don't span 0. Are you sure this is right?")
  }

  init <- FIMS::get_estimates(model) |>
    dplyr::filter(.data[["label"]] == parameter_name) |>
    dplyr::pull(.data[["estimated"]]) # NOTE: input and estimated value are slightly different (even though its fixed) input = 13.8155, estimated = 13.857

  if (!is.null(module_name)) {
    module_names <- parameters |>
      tidyr::unnest(cols = data) |>
      dplyr::pull(.data[["module_name"]]) |>
      unique()
    if (!module_name %in% module_names) {
      cli::cli_abort("Input module_name not found in parameters tibble.")
    }
    parameter_row <- parameters |>
      tidyr::unnest(cols = data) |>
      dplyr::filter(.data[["module_name"]] == module_name & .data[["label"]] == parameter_name)
  } else {
    parameter_row <- parameters |>
      tidyr::unnest(cols = data) |>
      dplyr::filter(.data[["label"]] == parameter_name)
  }

  if (nrow(parameter_row) == 0) {
    cli::cli_abort("Input parameter_name did not match any rows in parameter tibble.")
  }

  if (nrow(parameter_row) > 1) {
    cli::cli_abort("Input parameter_name matched too many rows in parameter tibble: {length(parameter_row)}. Try adding a module_name.")
  }

  vec <- values + init
  # report the values
  cli::cli_alert_info(
    "parameter values being profiles over: {paste(vec, collapse = ', ')}"
  )

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

  # Ensure cleanup happens
  on.exit(future::plan(future::sequential), add = TRUE)

  # run FIMS in parallel for each of the likelihood profile values
  estimates_list <- furrr::future_map(
    .x = vec,
    .f = function(value, parameter_name, module_name, parameters, data) {
      # Run the model
      fit <- run_modified_pars_fims(
        new_value = value,
        parameter_name = parameter_name,
        module_name = module_name,
        parameters = parameters,
        data = data
      )
      # Extract estimates immediately while still in worker
      FIMS::get_estimates(fit)
    },
    parameter_name = parameter_name,
    module_name = module_name,
    parameters = parameters,
    data = data,
    .options = furrr::furrr_options(seed = TRUE, globals = TRUE)
  )

  # adding the fixed parameter value to the estimates tibble for each of the models
  for (i in seq_along(estimates_list)) {
    # create a new column name based on the profile parameter
    # this could be extended to profile over multiple dimensions parameters
    colname <- paste0("value_", parameter_name)
    estimates_list[[i]][[colname]] <- vec[i]
  }
  # combine the separate tibbles in the list into one longer tibble
  estimates_df <- do.call(rbind, estimates_list)

  return(list("vec" = vec, "estimates" = estimates_df))
}
