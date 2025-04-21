# To remove the NOTE
# no visible binding for global variable
utils::globalVariables(c(
  "module_name", "module_id", "module_type",
  "parameter_min", "parameter_max", "label", "label_splits"
))

# A list of functions to reshape output from finalize()
#' Reshape JSON estimates
#'
#' @description
#' This function processes the finalized FIMS JSON output and reshapes the
#' parameter estimates into a structured tibble for easier analysis and
#' manipulation.
#'
#' @param finalized_fims A JSON object containing the finalized FIMS output.
#' @param opt An object returned from an optimizer, typically from
#'   [stats::nlminb()], used to fit a TMB model.
#' @return A tibble containing the reshaped parameter estimates.
reshape_json_estimates <- function(finalized_fims, opt = list()) {
  json_list <- jsonlite::fromJSON(finalized_fims)
  # Identify the index of the "modules" element in `json_list` by matching its name.
  # This is used to locate the relevant part of the JSON structure for further processing.
  modules_id <- which(names(json_list) == "modules")

  # Extract and process the "parameters" from each module in `json_list`
  parameter_estimates <- purrr::map(seq_along(json_list[[modules_id]][["parameters"]]), ~ {
    # If the current module's "parameters" is NULL, return NULL to skip processing.
    if (is.null(json_list[[modules_id]][["parameters"]][[.x]])) {
      NULL
    } else {
      # Convert the current module's "parameters" into a tibble for easier manipulation.
      temp <- tibble::as_tibble(json_list[[modules_id]][["parameters"]][[.x]]) |>
        # Rename the columns of the tibble.
        dplyr::rename_with(~ c("label", "type_id", "type", "parameter")) |>
        # Expand the "parameter" column into multiple rows if it has more than one parameter.
        tidyr::unnest_longer(parameter) |>
        # Expand the nested structure in the "parameter" column into multiple columns,
        # using an underscore (`_`) as a separator for the new column names.
        tidyr::unnest_wider(
          parameter,
          names_sep = "_"
        ) |>
        # Rename the columns of the expanded "parameter" column.
        dplyr::rename(
          initial = parameter_value,
          estimate = parameter_estimated_value,
          estimation_type = parameter_estimationtypeis
        )
      # If the `opt` has a length of 0, set the `estimated` column to FALSE.
      # TODO: ask Matthew if we can set estimated (in JSON) to FALSE when optimization = 0
      if (length(opt) == 0) {
        temp <- temp |>
          dplyr::mutate(estimation_type = "constant")
      }

      temp <- temp |>
        dplyr::rowwise() |>
        # if estimated == FALSE, then copy initial value to estimate
        dplyr::mutate(
          estimate = ifelse(estimation_type == "constant", initial, estimate)
        ) |>
        dplyr::ungroup()
    }
  })

  # Expand the parameter estimates with additional information from the json output
  estimates <- purrr::pmap(
    # Combine the parameter estimates with the module name, ID, and type from
    # `json_list`.
    list(
      parameter_estimates,
      json_list[[modules_id]][["name"]],
      json_list[[modules_id]][["id"]],
      json_list[[modules_id]][["type"]]
    ),
    ~ {
      # Skip processing if the current estimate is NULL.
      if (is.null(..1)) {
        NULL
      } else {
        ..1 |>
          dplyr::mutate(
            # Add the module name from `json_list` as a new column of the current estimates.
            module_name = ..2,
            # Add the module ID from `json_list` as a new column.
            module_id = ..3,
            # Add the module type from `json_list` as a new column.
            module_type = ..4
          )
      }
    }
  ) |>
    # Combine all the processed tibbles into a single tibble by stacking rows.
    dplyr::bind_rows() |>
    # Reorder the columns to place `module_name`, `module_id`, and `module_type` at the beginning.
    dplyr::relocate(module_name, module_id, module_type, .before = tidyselect::everything()) |>
    # Remove columns `parameter_min` and `parameter_max` as we currently don't
    # have bounds on parameters.
    dplyr::select(-c(parameter_min, parameter_max))
}

#' Reshape TMB estimates
#'
#' @description
#' This function processes the TMB std and reshapes them into a structured
#' tibble for easier analysis and manipulation.
#'
#' @param obj An object returned from [TMB::MakeADFun()].
#' @param sdreport An object of the `sdreport` class as returned from
#'   [TMB::sdreport()].
#' @param opt An object returned from an optimizer, typically from
#'   [stats::nlminb()], used to fit a TMB model.
#' @param parameter_names A character vector of parameter names. This is used to
#'   identify the parameters in the `std` object.
#' @return A tibble containing the reshaped estimates (i.e., parameters and
#' derived quantities).
reshape_tmb_estimates <- function(obj,
                                  sdreport = NULL,
                                  opt = NULL,
                                  parameter_names,
                                  random_names) {
  # Outline for the estimates table
  # TODO: The fleet_name, age, length, and time columns are currently emplty. Matthew
  # has started adding information to the JSON output in the dev-model-families branch.
  # We can populate these columns once the dev-model-families branch is merged
  # into dev.
  estimates_outline <- dplyr::tibble(
    # The FIMS Rcpp module
    module_name = character(),
    # The unique ID of the module
    module_id = integer(),
    # The name of the parameter or derived quantity
    label = character(),
    # The unique ID of the parameter
    parameter_id = integer(),
    # The fleet name associated with the parameter or derived quantity
    fleet_name = character(),
    # The age associated with the parameter or derived quantity
    age = numeric(),
    # The length associated with the parameter or derived quantity
    length = numeric(),
    # The modeled time period that the value pertains to
    time = integer(),
    # The initial value use to start the optimization procedure
    initial = numeric(),
    # The estimated parameter value, which would be the MLE estimate or the value
    # used for a given MCMC iteration
    estimate = numeric(),
    # Estimated uncertainty, reported as a standard deviation
    uncertainty = numeric(),
    # The pointwise log-likelihood used for the estimation model
    log_like = numeric(),
    # The pointwise log-likelihood used for the test or holdout data
    log_like_cv = numeric(),
    # The gradient component for that parameter, NA for derived quantities
    gradient = numeric(),
    # A TRUE/FALSE indicator of whether the parameter was estimated (and not fixed),
    # with NA for derived quantities
    estimation_type = character()
  )

  if (length(sdreport) > 0) {
    std <- summary(sdreport)
    # Number of rows for derived quantities: based on the difference
    # between the total number of rows in std and the length of parameter_names.
    derived_quantity_nrow <- nrow(std) - length(parameter_names)
    # Create a tibble with the data from the std, and then apply transformations.
    estimates <- estimates_outline |>
      tibble::add_row(
        label = dimnames(std)[[1]],
        estimate = std[, "Estimate"],
        uncertainty = std[, "Std. Error"],
        # Use obj[["env"]][["parameters"]][["p"]] as this will return both initial
        # fixed and random effects while obj[["par"]] only returns initial fixed
        # effects
        initial = c(
          obj[["env"]][["parameters"]][["p"]],
          rep(NA_real_, derived_quantity_nrow)
        ),
        gradient = c(
          obj[["gr"]](opt[["par"]]),
          rep(NA_real_, derived_quantity_nrow)
        ),
        estimation_type = c(
          rep("fixed_effects", length(parameter_names)),
          rep(NA, derived_quantity_nrow)
        )
      )
  } else {
    estimates <- estimates_outline |>
      tibble::add_row(
        label = names(obj[["par"]]),
        initial = obj[["env"]][["parameters"]][["p"]],
        estimate = obj[["env"]][["parameters"]][["p"]],
        estimation_type = "constant"
      )
  }

  estimates <- estimates |>
    # Split labels and extract module, id, label, and parameter id
    dplyr::mutate(label_splits = strsplit(label, split = "\\.")) |>
    dplyr::rowwise() |>
    # TODO: the code could be simplified using tidyr::separate_wider_*().
    # However, doing so would require avoiding pre-specification of these columns
    # in the estimates_outline tibble. Consider updating the code if we decide
    # not to create the `estimates_outline` tibble in advance.
    dplyr::mutate(
      module_name = ifelse(length(label_splits) > 1, label_splits[[1]], NA_character_),
      module_id = ifelse(length(label_splits) > 1, as.integer(label_splits[[3]]), NA_integer_),
      label = ifelse(length(label_splits) > 1, label_splits[[2]], label),
      parameter_id = ifelse(length(label_splits) > 1, as.integer(label_splits[[4]]), NA_integer_)
    ) |>
    dplyr::select(-label_splits) |>
    dplyr::ungroup()
}

#' Reshape JSON derived quantities
#'
#' This function processes the finalized FIMS JSON output and reshapes the derived
#' quantities into a structured tibble for easier analysis and manipulation.
#'
#' @param finalized_fims A JSON object containing the finalized FIMS output.
#' @return A tibble containing the reshaped parameter estimates.
reshape_json_derived_quantities <- function(finalized_fims) {
  json_list <- jsonlite::fromJSON(finalized_fims)
  # Identify the index of the "modules" element in `json_list` by matching its name.
  # This is used to locate the relevant part of the JSON structure for further processing.
  modules_id <- which(names(json_list) == "modules")

  derived_quantities <- purrr::map(seq_along(json_list[[modules_id]][["derived_quantities"]]), ~ {
    # If the current module's "derived_quantities" is NULL, return NULL to skip processing.
    if (is.null(json_list[[modules_id]][["derived_quantities"]][[.x]])) {
      NULL
    } else {
      # Convert the current module's "derived_quantities" into a tibble for easier manipulation.
      tibble::as_tibble(json_list[[modules_id]][["derived_quantities"]][[.x]]) |>
        # Expand the "values" column into multiple rows.
        tidyr::unnest_longer(values)
    }
  })

  expanded_derived_quantities <- purrr::pmap(
    list(
      derived_quantities,
      json_list[[modules_id]][["name"]],
      json_list[[modules_id]][["id"]],
      json_list[[modules_id]][["type"]]
    ),
    ~ {
      # Skip processing if the current derived_quantity is NULL.
      if (is.null(..1)) {
        NULL
      } else {
        ..1 |>
          dplyr::mutate(
            # Add the module name from `json_list` as a new column of the current estimates.
            module_name = ..2,
            # Add the module ID from `json_list` as a new column.
            module_id = ..3,
            # Add the module type from `json_list` as a new column.
            module_type = ..4
          )
      }
    }
  ) |>
    # Combine all the processed tibbles into a single tibble by stacking rows.
    dplyr::bind_rows() |>
    # Reorder the columns to place `module_name`, `module_id`, and `module_type` at the beginning.
    dplyr::relocate(module_name, module_id, module_type, .before = tidyselect::everything())
}

#' Reshape JSON 'values' components
#'
#' This function processes the model input from FIMS JSON output ('values') and
#' reshapes the initial values for data inputs into a structured tibble.
#'
#' @param finalized_fims A JSON object containing the finalized FIMS output.
#' @return A tibble containing the reshaped initial data values.
#'
reshape_json_values <- function(finalized_fims) {
  json_list <- jsonlite::fromJSON(finalized_fims)
  # Identify the index of the "modules" element in `json_list` by matching its name.
  # This is used to locate the relevant part of the JSON structure for further processing.
  modules_id <- which(names(json_list) == "modules")

  values <- purrr::map(seq_along(json_list[[modules_id]][["values"]]), ~ {
    # If the current module's "values" is NULL, return NULL to skip processing.
    if (is.null(json_list[[modules_id]][["values"]][[.x]])) {
      NULL
    } else {
      # Convert the current module's "values" into a tibble for easier manipulation.
      tibble::as_tibble(json_list[[modules_id]][["values"]][[.x]])
    }
  })

  expanded_values <- purrr::pmap(
    list(
      values,
      json_list[[modules_id]][["name"]],
      json_list[[modules_id]][["id"]],
      json_list[[modules_id]][["type"]]
    ),
    ~ {
      # Skip processing if the current derived_quantity is NULL.
      if (is.null(..1)) {
        NULL
      } else {
        ..1 |>
          dplyr::mutate(
            # Add the module name from `json_list` as a new column of the current estimates.
            module_name = ..2,
            # Add the module ID from `json_list` as a new column.
            module_id = ..3,
            # Add the module type from `json_list` as a new column.
            module_type = ..4
          )
      }
    }
  ) |>
    # Combine all the processed tibbles into a single tibble by stacking rows.
    dplyr::bind_rows() |>
    # Reorder the columns to place `module_name`, `module_id`, and `module_type` at the beginning.
    dplyr::relocate(module_name, module_id, module_type, .before = everything())
}


#' Reshape JSON 'fits' components
#'
#' This function processes the finalized FIMS JSON output and extracts/formats
#' log_like, distribution, init, and expected from
#' density_components, observed_values, and expected_values
#' into a structured tibble for easier analysis and manipulation. Values are
#' subsequently used to generate the 'fits' tibble from get_fits().
#'
#' @param finalized_fims A JSON object containing the finalized FIMS output.
#' @return A tibble containing the reshaped data fitting values.
#'
reshape_json_fits <- function(finalized_fims) {
  json_list <- jsonlite::fromJSON(finalized_fims)
  # Identify the index of the "modules" element in `json_list`.
  # This is used to locate the relevant part of the JSON structure.
  modules_id <- which(names(json_list) == "modules")

  log_like <- purrr::map(seq_along(json_list[[modules_id]][["density_component"]][["values"]]), ~ {
    # If the current module's "density_component" is NULL, return NULL to skip processing.
    if (is.null(json_list[[modules_id]][["density_component"]][["values"]][[.x]])) {
      NULL
    } else {
      # Convert the current module's "density_component" into a tibble for easier manipulation.
      tibble::as_tibble(json_list[[modules_id]][["density_component"]][["values"]][[.x]])
    }
  })

  init <- purrr::map(seq_along(json_list[[modules_id]][["observed_values"]][["values"]]), ~ {
    # If the current module's "observed_values" is NULL, return NULL to skip processing.
    if (is.null(json_list[[modules_id]][["observed_values"]][["values"]][[.x]])) {
      NULL
    } else {
      # Convert the current module's "observed_values" into a tibble for easier manipulation.
      tibble::as_tibble(json_list[[modules_id]][["observed_values"]][["values"]][[.x]])
    }
  })

  expected <- purrr::map(seq_along(json_list[[modules_id]][["expected_values"]][["values"]]), ~ {
    # If the current module's "expected_values" is NULL, return NULL to skip processing.
    if (is.null(json_list[[modules_id]][["expected_values"]][["values"]][[.x]])) {
      NULL
    } else {
      # Convert the current module's "expected_values" into a tibble for easier manipulation.
      tibble::as_tibble(json_list[[modules_id]][["expected_values"]][["values"]][[.x]])
    }
  })

  prelim_fits <- purrr::pmap(
    list(
      log_like,
      init,
      expected,
      json_list[[modules_id]][["name"]],
      json_list[[modules_id]][["id"]],
      json_list[[modules_id]][["type"]]
    ),
    ~ {
      # Skip processing if the current log_like is NULL.
      if (is.null(..1)) {
        NULL
      } else {
        ..1 |>
          dplyr::rename(log_like = value) |>
          cbind(..2) |>
          dplyr::rename(init = value) |>
          cbind(..3) |>
          dplyr::rename(expected = value) |>
          dplyr::mutate(
            # Add the module name from `json_list` as a new column of the current estimates.
            module_name = ..4,
            # Add the module ID from `json_list` as a new column.
            module_id = ..5,
            # Add the module type from `json_list` as a new column.
            module_type = ..6
          )
      }
    }
  ) |>
    # Combine all the processed tibbles into a single tibble by stacking rows.
    dplyr::bind_rows() |>
    # Reorder the columns to place `module_name`, `module_id`, and `module_type` at the beginning.
    dplyr::relocate(module_name, module_id, module_type, .before = everything()) |>
    tibble::as_tibble()
}
