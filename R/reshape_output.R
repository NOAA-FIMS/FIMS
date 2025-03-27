# A list of functions to reshape output from finalize()
#' Reshape JSON estimates
#'
#' This function processes the finalized FIMS JSON output and reshapes the parameter estimates 
#' into a structured tibble for easier analysis and manipulation.
#'
#' @param finalized_fims A JSON object containing the finalized FIMS output.
#' @return A tibble containing the reshaped parameter estimates.
reshape_json_estimates <- function(finalized_fims) {
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
      tibble::as_tibble(json_list[[modules_id]][["parameters"]][[.x]]) |>
        # Rename the columns of the tibble.
        dplyr::rename_with(~ c("label", "type_id", "type", "parameter")) |>
        # Expand the "parameter" column into multiple rows if it has more than one parameter.
        tidyr::unnest_longer(parameter) |>
        # Expand the nested structure in the "parameter" column into multiple columns,
        # using an underscore (`_`) as a separator for the new column names.
        tidyr::unnest_wider(
          parameter,
          names_sep = "_"
        )|>
        # TODO: update finalize() to use initial_value instead of value, 
        # then we can delete the following line.
        dplyr::rename(
          initial = parameter_value,
          estimate = parameter_estimated_value,
          estimated = parameter_estimated
        ) |>
        # Convert estimated from int to logical
        dplyr::mutate(
          estimated = ifelse(estimated == 1, TRUE, FALSE)
        )
    }
  })

  # Expand the parameter estimates with additional information from the json output
  estimates <- purrr::pmap(
    list(parameter_estimates, json_list[[modules_id]][["name"]], json_list[[modules_id]][["id"]], json_list[[modules_id]][["type"]]),
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
    dplyr::relocate(module_name, module_id, module_type, .before = everything())
    # Add gradient column
    # TODO: should we save parameter ids for final gradient? Right now, we assume that
    # the order of parameters in json_list is the same as the order of parameters in
    # estimates. Is it guaranteed to be true?
    # estimates |>
    #   dplyr::filter(parameter_estimated == 1) |>
    #   print(n=50)
    # json_list[["final_gradient"]]
}

#' Reshape JSON derived quantities
#'
#' This function processes the finalized FIMS JSON output and reshapes the derived
#' quantities into a structured tibble for easier analysis and manipulation.
#'
#' @param finalized_fims A JSON object containing the finalized FIMS output.
#' @return A tibble containing the reshaped parameter estimates.
reshape_json_derived_quantities <- function(finalized_fims){
  json_list <- jsonlite::fromJSON(finalized_fims)
  # Identify the index of the "modules" element in `json_list` by matching its name.
  # This is used to locate the relevant part of the JSON structure for further processing.
  modules_id <- which(names(json_list) == "modules")

  derived_quantitities <- purrr::map(seq_along(json_list[[modules_id]][["derived_quantities"]]), ~{
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
      derived_quantitities, 
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

#' Reshape JSON density component
#'
#' This function processes the finalized FIMS JSON output and reshapes the density
#' component into a structured tibble for easier analysis and manipulation.
#'
#' @param finalized_fims A JSON object containing the finalized FIMS output.
#' @return A tibble containing the reshaped density component.
reshape_density_component_json <- function(finalized_fims){
  density_component <- purrr::map(seq_along(json_list[[modules_id]][["density_component"]][[2]]), ~{
    # If the current module's "density_component" is NULL, return NULL to skip processing.
    if (is.null(json_list[[modules_id]][["density_component"]][[2]][[.x]])) {
      NULL
    } else {
      # Convert the current module's "density_component" into a tibble for easier manipulation.
      tibble::as_tibble(json_list[[modules_id]][["density_component"]][[2]][[.x]]) 
    }
  }) 

  expanded_density_component <- purrr::pmap(
    list(
      density_component, 
      json_list[[modules_id]][["name"]], 
      json_list[[modules_id]][["id"]], 
      json_list[[modules_id]][["type"]],
      json_list[[modules_id]][["density_component"]][[1]]
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
            module_type = ..4,
            # Add the vector type from `json_list` as a new column.
            type = ..5
          )
      }
    }
  ) |>
    # Combine all the processed tibbles into a single tibble by stacking rows.
    dplyr::bind_rows() |>
    # Reorder the columns to place `module_name`, `module_id`, and `module_type` at the beginning.
    dplyr::relocate(module_name, module_id, module_type, .before = everything())
}

#' Reshape TMB estimates
#'  
#' This function processes the TMB std and reshapes them into a structured 
#' tibble for easier analysis and manipulation.
#' 
#' @param obj An object returned from [TMB::MakeADFun()].
#' @param sdreport An object of the `sdreport` class as returned from 
#'   [TMB::sdreport()].
#' @param opt An object returned from [TMB::Optimize()].
#' @param parameter_names A character vector of parameter names. This is used to
#'   identify the parameters in the `std` object.
#' @return A tibble containing the reshaped estimates (i.e., parameters and 
#' derived quantities).
reshape_tmb_estimates <- function( 
  obj, 
  sdreport, 
  opt = NULL, 
  parameter_names){
  # Outline for the estimates table
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
    # The modeled time perioed that the value pertains to
    time = integer(),
    # The initial value use to start the optimization procedure
    initial = numeric(),
    # The estaimted parameter value, which would be the MLE estimate or the value
    # used for a given MCMC iteration
    estimate = numeric(),
    # Estimated uncertainty, reported as a standard deviation
    uncertainty = numeric(),
    # The pointwise log-likelihood used for the estimation model
    log_lik = numeric(),
    # The pointwise log-likelihood used for the test or holdout data
    log_lik_cv = numeric(),
    # The gradient component for that parameter, NA for derived quantities
    gradient = numeric(),
    # A TRUE/FALSE indicator of whether the parameter was estimated (and not fixed),
    # with NA for derived quantities
    estimated = logical()
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
        estimated = c(
          rep(TRUE, length(parameter_names)),
          rep(NA, derived_quantity_nrow)
        )
      ) |>
      # TODO: Create row_id to be used in mutating joins later if generating 
      # age, length, and time values in R.
      # tibble::rowid_to_column("row_id") |>
      # Split labels and extract module, id, label, and index
      dplyr::mutate(label_splits = strsplit(label, split = "\\.")) |>
      dplyr::rowwise() |>
      dplyr::mutate(
        module_name = ifelse(length(label_splits) > 1, label_splits[[1]], NA_character_),
        module_id = ifelse(length(label_splits) > 1, as.integer(label_splits[[3]]), NA_integer_),
        label = ifelse(length(label_splits) > 1, label_splits[[2]], label),
        parameter_id = ifelse(length(label_splits) > 1, as.integer(label_splits[[4]]), NA_integer_)
      ) |>
      dplyr::select(-label_splits) |>
      dplyr::ungroup()
  } else {
    estimates <- estimates_outline |>
      tibble::add_row( 
        label = names(obj[["par"]]),
        initial = obj[["env"]][["parameters"]][["p"]],
        estimate = obj[["env"]][["parameters"]][["p"]],
        estimated = FALSE
      ) 
  }
}

