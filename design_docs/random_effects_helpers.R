# Random Effects Helper Functions for FIMS
# These functions provide a user-friendly interface for adding and managing
# random effects in FIMS models

#' Add Random Effects to Parameters
#'
#' @description
#' This function adds random effects specifications to existing parameters in a
#' FIMS model tibble. It supports three types of random effects: parametric
#' (random effects on existing function parameters), semi-parametric (base
#' function plus random deviations), and non-parametric (time-varying process
#' with correlation structure).
#'
#' @param parameters A tibble of model parameters, typically created by
#'   [create_default_parameters()].
#' @param module_name Character. Name of the module (e.g., "Selectivity",
#'   "Recruitment", "Population").
#' @param fleet_name Character. Name of fleet (if applicable). Use `NA` for
#'   modules that are not fleet-specific (e.g., "Recruitment").
#' @param label Character. Parameter name to apply random effects to (e.g.,
#'   "inflection_point", "log_devs").
#' @param specification_type Character. Type of random effect specification:
#'   * `"parametric"`: Random effects on time-varying parameters in a function
#'   * `"semi_parametric"`: Base function plus random deviations
#'   * `"non_parametric"`: Time-varying process with correlation structure
#' @param structure Character. Correlation structure for the random effects:
#'   * `"iid"`: Independent and identically distributed
#'   * `"AR1"`: First-order autoregressive
#'   * `"RW"`: Random walk (first-order)
#'   * `"RW2"`: Random walk (second-order)
#' @param hyperparameters Named list of hyperparameters. Each element should be
#'   a list with `value` and `estimation_type`. Common hyperparameters:
#'   * `log_sigma`: Log-scale standard deviation
#'   * `rho`: Correlation parameter (for AR1)
#'   * `tau`: Precision parameter (alternative to sigma)
#' @param operation Character. For semi-parametric only: how random effects are
#'   applied to the base function. Either `"multiplicative"` or `"additive"`.
#' @param base_function Character. For semi-parametric only: name of the base
#'   function being modified (e.g., "logistic", "double_logistic").
#'
#' @return Updated parameters tibble with random effects specifications added.
#'
#' @examples
#' \dontrun{
#' # Add AR1 random effects to selectivity inflection point
#' params <- params |>
#'   add_random_effect(
#'     module_name = "Selectivity",
#'     fleet_name = "survey1",
#'     label = "inflection_point",
#'     specification_type = "parametric",
#'     structure = "AR1",
#'     hyperparameters = list(
#'       log_sigma = list(value = log(0.2), estimation_type = "fixed_effects"),
#'       rho = list(value = 0.5, estimation_type = "constant")
#'     )
#'   )
#' }
#'
#' @export
add_random_effect <- function(parameters,
                              module_name,
                              fleet_name = NA_character_,
                              label,
                              specification_type = c("parametric", "semi_parametric", "non_parametric"),
                              structure = c("iid", "AR1", "RW", "RW2"),
                              hyperparameters = list(
                                log_sigma = list(value = 0, estimation_type = "fixed_effects")
                              ),
                              operation = c("multiplicative", "additive"),
                              base_function = NULL) {
  
  # Validate inputs
  specification_type <- rlang::arg_match(specification_type)
  structure <- rlang::arg_match(structure)
  operation <- rlang::arg_match(operation)
  
  # Check that parameters is a tibble
  if (!tibble::is_tibble(parameters)) {
    cli::cli_abort("The {.arg parameters} argument must be a tibble.")
  }
  
  # Check if parameters is nested
  is_nested <- "data" %in% names(parameters)
  if (is_nested) {
    params_work <- parameters |>
      tidyr::unnest(cols = data)
  } else {
    params_work <- parameters
  }
  
  # Validate that the parameter exists
  param_filter <- params_work |>
    dplyr::filter(
      module_name == !!module_name,
      label == !!label
    )
  
  if (!is.na(fleet_name)) {
    param_filter <- param_filter |>
      dplyr::filter(fleet_name == !!fleet_name)
  }
  
  if (nrow(param_filter) == 0) {
    cli::cli_abort(c(
      "Parameter not found in parameters tibble.",
      i = "module_name: {module_name}",
      i = "fleet_name: {fleet_name}",
      i = "label: {label}"
    ))
  }
  
  # Validate semi-parametric requirements
  if (specification_type == "semi_parametric") {
    if (is.null(base_function)) {
      cli::cli_abort(c(
        "Semi-parametric random effects require specifying {.arg base_function}.",
        i = "Example: base_function = 'logistic'"
      ))
    }
  }
  
  # Add RE columns if they don't exist
  if (!"re_specification_type" %in% names(params_work)) {
    params_work <- params_work |>
      dplyr::mutate(
        re_specification_type = NA_character_,
        re_structure = NA_character_,
        re_operation = NA_character_,
        re_base_function = NA_character_
      )
  }
  
  # Add hyperparameter columns dynamically
  for (hparam_name in names(hyperparameters)) {
    value_col <- paste0("re_", hparam_name)
    est_type_col <- paste0("re_", hparam_name, "_est_type")
    
    if (!value_col %in% names(params_work)) {
      params_work[[value_col]] <- NA_real_
      params_work[[est_type_col]] <- NA_character_
    }
  }
  
  # Update the parameters tibble
  params_work <- params_work |>
    dplyr::mutate(
      # Set estimation_type to random_effects
      estimation_type = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        "random_effects",
        estimation_type
      ),
      # Set RE specification type
      re_specification_type = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        specification_type,
        re_specification_type
      ),
      # Set RE structure
      re_structure = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        structure,
        re_structure
      ),
      # Set operation (for semi-parametric)
      re_operation = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label &
          specification_type == "semi_parametric",
        operation,
        re_operation
      ),
      # Set base function (for semi-parametric)
      re_base_function = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label &
          specification_type == "semi_parametric",
        base_function,
        re_base_function
      )
    )
  
  # Set hyperparameter values
  for (hparam_name in names(hyperparameters)) {
    value_col <- paste0("re_", hparam_name)
    est_type_col <- paste0("re_", hparam_name, "_est_type")
    
    hparam_value <- hyperparameters[[hparam_name]][["value"]]
    hparam_est_type <- hyperparameters[[hparam_name]][["estimation_type"]]
    
    params_work <- params_work |>
      dplyr::mutate(
        !!value_col := dplyr::if_else(
          module_name == !!module_name &
            (is.na(fleet_name) | fleet_name == !!fleet_name) &
            label == !!label,
          hparam_value,
          .data[[value_col]]
        ),
        !!est_type_col := dplyr::if_else(
          module_name == !!module_name &
            (is.na(fleet_name) | fleet_name == !!fleet_name) &
            label == !!label,
          hparam_est_type,
          .data[[est_type_col]]
        )
      )
  }
  
  # Re-nest if input was nested
  if (is_nested) {
    params_work <- params_work |>
      tidyr::nest(.by = c(model_family, module_name, fleet_name))
  }
  
  cli::cli_inform(c(
    v = "Added {specification_type} random effect with {structure} structure to {label}.",
    i = "Module: {module_name}",
    i = "Fleet: {fleet_name %||% 'N/A'}"
  ))
  
  return(params_work)
}


#' Remove Random Effects from Parameters
#'
#' @description
#' This function removes random effects specifications from parameters in a
#' FIMS model tibble, reverting them to fixed effects.
#'
#' @inheritParams add_random_effect
#'
#' @return Updated parameters tibble with random effects removed.
#'
#' @examples
#' \dontrun{
#' params <- params |>
#'   remove_random_effect(
#'     module_name = "Selectivity",
#'     fleet_name = "survey1",
#'     label = "inflection_point"
#'   )
#' }
#'
#' @export
remove_random_effect <- function(parameters,
                                 module_name,
                                 fleet_name = NA_character_,
                                 label) {
  
  # Check if parameters is nested
  is_nested <- "data" %in% names(parameters)
  if (is_nested) {
    params_work <- parameters |>
      tidyr::unnest(cols = data)
  } else {
    params_work <- parameters
  }
  
  # Update estimation_type and clear RE-specific columns
  params_work <- params_work |>
    dplyr::mutate(
      estimation_type = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label &
          estimation_type == "random_effects",
        "fixed_effects",
        estimation_type
      ),
      re_specification_type = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        NA_character_,
        re_specification_type
      ),
      re_structure = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        NA_character_,
        re_structure
      ),
      re_operation = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        NA_character_,
        re_operation
      ),
      re_base_function = dplyr::if_else(
        module_name == !!module_name &
          (is.na(fleet_name) | fleet_name == !!fleet_name) &
          label == !!label,
        NA_character_,
        re_base_function
      )
    )
  
  # Clear hyperparameter columns if they exist
  re_hyperparam_cols <- grep("^re_(log_sigma|rho|tau)", names(params_work), value = TRUE)
  for (col in re_hyperparam_cols) {
    params_work <- params_work |>
      dplyr::mutate(
        !!col := dplyr::if_else(
          module_name == !!module_name &
            (is.na(fleet_name) | fleet_name == !!fleet_name) &
            label == !!label,
          NA_real_,
          .data[[col]]
        )
      )
  }
  
  # Re-nest if input was nested
  if (is_nested) {
    params_work <- params_work |>
      tidyr::nest(.by = c(model_family, module_name, fleet_name))
  }
  
  cli::cli_inform(c(
    v = "Removed random effects from {label}.",
    i = "Module: {module_name}",
    i = "Fleet: {fleet_name %||% 'N/A'}"
  ))
  
  return(params_work)
}


#' Add IID Random Effect
#'
#' @description
#' Convenience function to add independent and identically distributed (IID)
#' random effects to parameters. This is a wrapper around [add_random_effect()]
#' with `structure = "iid"`.
#'
#' @inheritParams add_random_effect
#' @param sigma Numeric. Standard deviation on the natural scale (will be
#'   log-transformed internally).
#' @param estimate_sigma Logical. Should sigma be estimated? If `TRUE`, sigma
#'   is a fixed effect; if `FALSE`, it's constant.
#'
#' @return Updated parameters tibble with IID random effects added.
#'
#' @examples
#' \dontrun{
#' params <- params |>
#'   add_iid_random_effect(
#'     module_name = "Recruitment",
#'     label = "log_devs",
#'     specification_type = "non_parametric",
#'     sigma = 0.4,
#'     estimate_sigma = TRUE
#'   )
#' }
#'
#' @export
add_iid_random_effect <- function(parameters,
                                  module_name,
                                  fleet_name = NA_character_,
                                  label,
                                  specification_type = "parametric",
                                  sigma = 0.2,
                                  estimate_sigma = TRUE) {
  add_random_effect(
    parameters = parameters,
    module_name = module_name,
    fleet_name = fleet_name,
    label = label,
    specification_type = specification_type,
    structure = "iid",
    hyperparameters = list(
      log_sigma = list(
        value = log(sigma),
        estimation_type = if (estimate_sigma) "fixed_effects" else "constant"
      )
    )
  )
}


#' Add AR1 Random Effect
#'
#' @description
#' Convenience function to add first-order autoregressive (AR1) random effects
#' to parameters. This is a wrapper around [add_random_effect()] with
#' `structure = "AR1"`.
#'
#' @inheritParams add_random_effect
#' @param sigma Numeric. Innovation standard deviation on the natural scale
#'   (will be log-transformed internally).
#' @param rho Numeric. AR1 correlation parameter, typically between -1 and 1.
#' @param estimate_sigma Logical. Should sigma be estimated?
#' @param estimate_rho Logical. Should rho be estimated?
#'
#' @return Updated parameters tibble with AR1 random effects added.
#'
#' @examples
#' \dontrun{
#' params <- params |>
#'   add_ar1_random_effect(
#'     module_name = "Recruitment",
#'     label = "log_devs",
#'     specification_type = "non_parametric",
#'     sigma = 0.4,
#'     rho = 0.5,
#'     estimate_sigma = TRUE,
#'     estimate_rho = FALSE
#'   )
#' }
#'
#' @export
add_ar1_random_effect <- function(parameters,
                                  module_name,
                                  fleet_name = NA_character_,
                                  label,
                                  specification_type = "non_parametric",
                                  sigma = 0.4,
                                  rho = 0.5,
                                  estimate_sigma = TRUE,
                                  estimate_rho = FALSE) {
  add_random_effect(
    parameters = parameters,
    module_name = module_name,
    fleet_name = fleet_name,
    label = label,
    specification_type = specification_type,
    structure = "AR1",
    hyperparameters = list(
      log_sigma = list(
        value = log(sigma),
        estimation_type = if (estimate_sigma) "fixed_effects" else "constant"
      ),
      rho = list(
        value = rho,
        estimation_type = if (estimate_rho) "fixed_effects" else "constant"
      )
    )
  )
}


#' Add Random Walk Random Effect
#'
#' @description
#' Convenience function to add random walk random effects to parameters. This
#' is a wrapper around [add_random_effect()] with `structure = "RW"` or
#' `"RW2"`.
#'
#' @inheritParams add_random_effect
#' @param sigma Numeric. Standard deviation of the random walk increments on
#'   the natural scale (will be log-transformed internally).
#' @param order Integer. Order of the random walk: 1 for RW (first-order
#'   differences), 2 for RW2 (second-order differences).
#' @param estimate_sigma Logical. Should sigma be estimated?
#'
#' @return Updated parameters tibble with random walk random effects added.
#'
#' @examples
#' \dontrun{
#' # First-order random walk
#' params <- params |>
#'   add_rw_random_effect(
#'     module_name = "Selectivity",
#'     fleet_name = "fleet1",
#'     label = "inflection_point",
#'     specification_type = "parametric",
#'     sigma = 0.1,
#'     order = 1,
#'     estimate_sigma = TRUE
#'   )
#'
#' # Second-order random walk (smoother)
#' params <- params |>
#'   add_rw_random_effect(
#'     module_name = "Selectivity",
#'     fleet_name = "fleet1",
#'     label = "slope",
#'     specification_type = "parametric",
#'     sigma = 0.05,
#'     order = 2,
#'     estimate_sigma = FALSE
#'   )
#' }
#'
#' @export
add_rw_random_effect <- function(parameters,
                                 module_name,
                                 fleet_name = NA_character_,
                                 label,
                                 specification_type = "parametric",
                                 sigma = 0.1,
                                 order = 1,
                                 estimate_sigma = TRUE) {
  
  # Validate order
  if (!order %in% c(1, 2)) {
    cli::cli_abort("The {.arg order} must be 1 or 2.")
  }
  
  structure <- if (order == 1) "RW" else "RW2"
  
  add_random_effect(
    parameters = parameters,
    module_name = module_name,
    fleet_name = fleet_name,
    label = label,
    specification_type = specification_type,
    structure = structure,
    hyperparameters = list(
      log_sigma = list(
        value = log(sigma),
        estimation_type = if (estimate_sigma) "fixed_effects" else "constant"
      )
    )
  )
}


#' Validate Random Effects Specifications
#'
#' @description
#' This function validates that random effects specifications in a parameters
#' tibble are complete and consistent. It checks for required fields,
#' appropriate combinations of settings, and missing hyperparameters.
#'
#' @param parameters A tibble of model parameters.
#' @param verbose Logical. If `TRUE`, prints detailed validation messages.
#'
#' @return Invisible `TRUE` if valid. Throws informative errors if validation
#'   fails.
#'
#' @examples
#' \dontrun{
#' # Validate after adding random effects
#' params <- params |>
#'   add_ar1_random_effect(
#'     module_name = "Recruitment",
#'     label = "log_devs",
#'     specification_type = "non_parametric",
#'     sigma = 0.4,
#'     rho = 0.5
#'   ) |>
#'   validate_random_effects()
#' }
#'
#' @export
validate_random_effects <- function(parameters, verbose = TRUE) {
  
  # Check if parameters is nested
  is_nested <- "data" %in% names(parameters)
  if (is_nested) {
    params_work <- parameters |>
      tidyr::unnest(cols = data)
  } else {
    params_work <- parameters
  }
  
  # Extract random effects parameters
  re_params <- params_work |>
    dplyr::filter(estimation_type == "random_effects")
  
  if (nrow(re_params) == 0) {
    if (verbose) {
      cli::cli_inform(c(
        i = "No random effects parameters found in the parameters tibble."
      ))
    }
    return(invisible(TRUE))
  }
  
  # Check 1: All RE parameters have specification_type
  missing_spec <- re_params |>
    dplyr::filter(is.na(re_specification_type))
  
  if (nrow(missing_spec) > 0) {
    cli::cli_abort(c(
      "Random effects parameters must have a specification type.",
      i = "Missing for: {unique(paste(missing_spec$module_name, missing_spec$label, sep = '::'))}"
    ))
  }
  
  # Check 2: All RE parameters have structure
  missing_structure <- re_params |>
    dplyr::filter(is.na(re_structure))
  
  if (nrow(missing_structure) > 0) {
    cli::cli_abort(c(
      "Random effects parameters must have a correlation structure.",
      i = "Missing for: {unique(paste(missing_structure$module_name, missing_structure$label, sep = '::'))}"
    ))
  }
  
  # Check 3: Semi-parametric has operation and base_function
  semi_params <- re_params |>
    dplyr::filter(re_specification_type == "semi_parametric")
  
  if (nrow(semi_params) > 0) {
    missing_operation <- semi_params |>
      dplyr::filter(is.na(re_operation))
    
    if (nrow(missing_operation) > 0) {
      cli::cli_abort(c(
        "Semi-parametric random effects must specify an operation (additive or multiplicative).",
        i = "Missing for: {unique(paste(missing_operation$module_name, missing_operation$label, sep = '::'))}"
      ))
    }
    
    missing_base <- semi_params |>
      dplyr::filter(is.na(re_base_function))
    
    if (nrow(missing_base) > 0) {
      cli::cli_abort(c(
        "Semi-parametric random effects must specify a base function.",
        i = "Missing for: {unique(paste(missing_base$module_name, missing_base$label, sep = '::'))}"
      ))
    }
  }
  
  # Check 4: Required hyperparameters are present
  # All structures require log_sigma
  if ("re_log_sigma" %in% names(re_params)) {
    missing_sigma <- re_params |>
      dplyr::filter(is.na(re_log_sigma))
    
    if (nrow(missing_sigma) > 0) {
      cli::cli_abort(c(
        "Random effects parameters must have a log_sigma hyperparameter.",
        i = "Missing for: {unique(paste(missing_sigma$module_name, missing_sigma$label, sep = '::'))}"
      ))
    }
  } else {
    cli::cli_abort(c(
      "Random effects parameters require a log_sigma column.",
      i = "Use add_random_effect() or related functions to properly set up random effects."
    ))
  }
  
  # AR1 requires rho
  ar1_params <- re_params |>
    dplyr::filter(re_structure == "AR1")
  
  if (nrow(ar1_params) > 0 && "re_rho" %in% names(ar1_params)) {
    missing_rho <- ar1_params |>
      dplyr::filter(is.na(re_rho))
    
    if (nrow(missing_rho) > 0) {
      cli::cli_abort(c(
        "AR1 random effects must have a rho hyperparameter.",
        i = "Missing for: {unique(paste(missing_rho$module_name, missing_rho$label, sep = '::'))}"
      ))
    }
  }
  
  # Success message
  if (verbose) {
    cli::cli_inform(c(
      v = "Random effects specifications validated successfully.",
      i = "Found {nrow(re_params)} random effects parameter row(s).",
      i = "Specification types: {unique(re_params$re_specification_type)}",
      i = "Structures: {unique(re_params$re_structure)}"
    ))
  }
  
  return(invisible(TRUE))
}


#' Get Summary of Random Effects
#'
#' @description
#' This function provides a summary view of all random effects specifications
#' in a parameters tibble.
#'
#' @param parameters A tibble of model parameters.
#'
#' @return A tibble summarizing random effects specifications.
#'
#' @examples
#' \dontrun{
#' summary_re(params)
#' }
#'
#' @export
summary_re <- function(parameters) {
  
  # Check if parameters is nested
  is_nested <- "data" %in% names(parameters)
  if (is_nested) {
    params_work <- parameters |>
      tidyr::unnest(cols = data)
  } else {
    params_work <- parameters
  }
  
  # Extract random effects parameters
  re_params <- params_work |>
    dplyr::filter(estimation_type == "random_effects")
  
  if (nrow(re_params) == 0) {
    cli::cli_inform(c(
      i = "No random effects parameters found."
    ))
    return(invisible(NULL))
  }
  
  # Create summary
  summary <- re_params |>
    dplyr::group_by(
      module_name, fleet_name, label,
      re_specification_type, re_structure
    ) |>
    dplyr::summarise(
      n_parameters = dplyr::n(),
      .groups = "drop"
    ) |>
    dplyr::arrange(module_name, fleet_name, label)
  
  return(summary)
}
