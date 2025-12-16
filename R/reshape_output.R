# This file contains many functions to reshape output from get_output()
# To remove the NOTE `no visible binding for global variable`
utils::globalVariables(c(
  "module_name", "module_id", "module_type",
  "parameter_min", "parameter_max", "label", "label_splits"
))

#' Reshape JSON estimates
#'
#' @description
#' This function processes the finalized FIMS JSON output and reshapes the
#' parameter estimates into a structured tibble for easier analysis and
#' manipulation.
#'
#' @param model_output A JSON object containing the finalized FIMS output as
#'   returned from `get_output()`, which is an internal function to each model
#'   family.
#' @return A tibble containing the reshaped parameter estimates.
reshape_json_estimates <- function(model_output) {
  json_list <- jsonlite::fromJSON(model_output, simplifyVector = FALSE)
  read_list <- purrr::map(
    json_list[!names(json_list) %in% c(
      "name", "type", "estimation_framework", "id", "objective_function_value",
      "max_gradient_component", "gradient",
      "population_ids", "fleet_ids", "log"
    )],
    \(x) tidyr::unnest_wider(tibble::tibble(json = x), json)
  )

  # Process the module parameters
  module_information <- purrr::map_df(
    read_list[
      !names(read_list) %in%
        c(
          "name", "type", "estimation_framework", "id",
          "objective_function_value", "populations", "fleets", "data",
          "density_components", "population_ids", "fleet_ids"
        )
    ],
    .f = \(y) dplyr::mutate(
      y,
      parameters = purrr::map(
        parameters,
        \(x) purrr::map_df(x, dimension_folded_to_tibble)
      )
    )
  ) |>
    tidyr::unnest(parameters)

  # Process the fleet-level information
  fleet_density_data <- read_list[["fleets"]] |>
    dplyr::select(module_id, data_ids) |>
    dplyr::mutate(
      data_ids = purrr::map(
        data_ids,
        \(y) tibble::enframe(unlist(y), name = "data_type", value = "data_id")
      )
    ) |>
    tidyr::unnest(data_ids) |>
    dplyr::filter(data_id != -999) |>
    dplyr::mutate(
      name = paste(data_type, "expected", sep = "_")
    ) |>
    dplyr::left_join(
      y = read_list[["density_components"]] |>
        dplyr::filter(
          observed_data_id != -999
        ) |>
        dplyr::rename(distribution = "module_type") |>
        dplyr::select(-dplyr::starts_with("module")),
      by = c("data_id" = "observed_data_id")
    ) |>
    dplyr::mutate(
      density_component = purrr::map(density_component, density_to_tibble)
    ) |>
    tidyr::unnest(density_component) |>
    dplyr::select(-dplyr::starts_with("data_")) |>
    dplyr::group_by(module_id, name) |>
    dplyr::mutate(join_by = dplyr::row_number()) |>
    dplyr::ungroup()

  fleet_information <- read_list[["fleets"]] |>
    tidyr::pivot_longer(
      cols = c(parameters, derived_quantities),
      names_to = "delete_me",
      values_to = "parameters"
    ) |>
    dplyr::select(-delete_me, -data_ids) |>
    # Remove column ids that are not currently needed
    dplyr::select(-dplyr::matches("^n.+s$")) |>
    dplyr::mutate(
      parameters = purrr::map(
        parameters,
        \(x) purrr::map_df(x, dimension_folded_to_tibble)
      )
    ) |>
    tidyr::unnest(parameters) |>
    dplyr::group_by(module_id, name) |>
    dplyr::mutate(join_by = dplyr::row_number()) |>
    dplyr::ungroup() |>
    dplyr::left_join(
      fleet_density_data,
      by = c("module_id", "name", "join_by"),
      suffix = c("", "_density")
    ) |>
    dplyr::select(-join_by)

  # Process the data components
  # TODO: Data component needs actual uncertainty instead of 0
  data_information <- read_list[["data"]] |>
    dplyr::mutate(
      dimensionality = purrr::map(dimensionality, \(x) dimensions_to_tibble(x))
    ) |>
    tidyr::unnest(c(dimensionality, value, uncertainty)) |>
    dplyr::mutate(value = unlist(value), uncertainty = unlist(uncertainty))

  # Process the density components
  # This is done above for fleet information but we will need to do it for
  # parameter-level information once we have a link to the parameter id
  density_information <- read_list[["density_components"]] |>
    dplyr::mutate(
      density_component = purrr::map(density_component, density_to_tibble)
    ) |>
    tidyr::unnest(density_component)

  # Process the population data
  population_information <- read_list[["populations"]] |>
    tidyr::pivot_longer(
      cols = c(parameters, derived_quantities),
      names_to = "delete_me",
      values_to = "parameters"
    ) |>
    # TODO: Think about these ids when we have more than one population
    dplyr::select(-delete_me, -dplyr::ends_with("_id"), -population) |>
    dplyr::mutate(
      parameters = purrr::map(
        parameters,
        \(x) purrr::map_df(x, dimension_folded_to_tibble)
      )
    ) |>
    tidyr::unnest(parameters)

  # TODO: Bring in TMB estimates
  # TODO: Change some column names
  # Bring everything together
  out <- dplyr::bind_rows(
    # density_information,
    fleet_information,
    module_information,
    population_information
  ) |>
    dplyr::select(
      module_name, module_id, module_type,
      "label" = name,
      type, type_id, "parameter_id" = id,
      fleet, dplyr::ends_with("_i"),
      "input" = value, estimated = "estimated_value", "expected" = expected_values,
      "observed" = observed_values, uncertainty, estimation_type,
      distribution, input_type, lpdf = "lpdf_value", likelihood,
      dplyr::everything()
    )
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
                                  parameter_names) {
  # Outline for the estimates table
  estimates_outline <- tibble::tibble(
    # The FIMS Rcpp module
    module_name = character(),
    # The unique ID of the module
    module_id = integer(),
    # The name of the parameter or derived quantity
    label = character(),
    # The unique ID of the parameter
    parameter_id = integer(),
    # The initial value use to start the optimization procedure
    initial = numeric(),
    # The estimated parameter value, which would be the MLE estimate or the value
    # used for a given MCMC iteration
    estimate = numeric(),
    # Estimated uncertainty, reported as a standard deviation
    uncertainty = numeric(),
    # The pointwise log-likelihood used for the test or holdout data
    log_like_cv = numeric(),
    # The gradient component for that parameter, NA for derived quantities
    gradient = numeric()
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
        )
      )
  } else {
    estimates <- estimates_outline |>
      tibble::add_row(
        label = names(obj[["par"]]),
        initial = obj[["env"]][["parameters"]][["p"]],
        estimate = obj[["env"]][["parameters"]][["p"]]
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
      module_id = ifelse(length(label_splits) > 1, as.integer(label_splits[[2]]), NA_integer_),
      label = ifelse(length(label_splits) > 1, label_splits[[3]], label),
      parameter_id = ifelse(length(label_splits) > 1, as.integer(label_splits[[4]]), NA_integer_)
    ) |>
    dplyr::select(-label_splits) |>
    dplyr::ungroup()
}

#' Converts a dimension-folder section into a tibble
#'
#' This is an internal helper function that processes a complex list
#' structure read in from a json file containing dimensionality information, a
#' name, and either explicit values with a type or estimated values with
#' uncertainty.
#'
#' @param section A section of the json file represented as a list.
#' @return
#' A tibble containing the json output in a formatted structure listing the
#' dimensionality as columns rather than just row and column lengths.
#' @noRd
#'
#' @examples
#' # A simple example for a value with uncertainty:
#' section_derived <- list(
#'   name = "derived_quantity_name",
#'   dimensionality = list(
#'     unit = "m", symbol = "L", scale = 1.0, type = "length"
#'   ),
#'   value = 10.5,
#'   uncertainty = 0.5
#' )
#' dimension_folded_to_tibble(section_derived)
dimension_folded_to_tibble <- function(section) {
  if (length(section) == 0) {
    return(NA)
  }
  while (length(section) == 1) {
    unlist(section, recursive = FALSE)
  }
  temp <- dimensions_to_tibble(section[["dimensionality"]]) |>
    dplyr::mutate(name = section[["name"]])
  if ("type" %in% names(section)) {
    temp |>
      dplyr::mutate(
        # TODO: Need to rename
        type_id = section[["id"]],
        type = section[["type"]]
      ) |>
      dplyr::bind_cols(
        tibble::tibble(data = section[["values"]]) |>
          tidyr::unnest_wider(data)
      ) |>
      dplyr::select(-min, -max)
  } else {
    temp |>
      dplyr::bind_cols(
        estimated_value = unlist(section[["value"]]),
        uncertainty = unlist(section[["uncertainty"]]),
        estimation_type = "derived_quantity"
      )
  }
}

#' Covert the dimension information from a FIMS json output into a tibble
#'
#' Dimensions in the json output are stored as a list of length two, with the
#' header information containing the name of the dimension and the dimensions
#' containing integers specifying the length for each dimension. The result
#' helps interpret how the FIMS output is structured given it is dimension
#' folded into a single vector in the json output.
#'
#' @details
#' The dimension index is returned not the actual year of the model. For
#' example, if the model starts in year 1900, then year_i of 1, which is what
#' is returned from this function will need to map to 1900 and that will need
#' to be done externally.
#' This function will accommodate dimensions of year-1 and year+1 where the
#' indexing of the former will start at 2 instead of 1.
#' @param data A list containing the header and dimensions information from a
#'   FIMS json output object.
#' @return
#' A tibble containing ordered rows for each combination of the dimensions.
#' @noRd
#' @examples
#' dummy_dimensions <- list(
#'   header = list("n_years", "n_ages"),
#'   dimensions = list(30L, 12L)
#' )
#' dimensions_to_tibble(dummy_dimensions)
#' # Example with n_years+1
#' dummy_dimensions <- list(
#'   header = list("n_years+1", "n_ages"),
#'   dimensions = list(31L, 12L)
#' )
#' dimensions_to_tibble(dummy_dimensions)
dimensions_to_tibble <- function(data) {
  #' Replace headers like "n_years" with "year_i".
  #' Example: "n_ages+1" with "age_i"
  #' This matches names starting with 'n' (with or without an underscore)
  #' and shortens them to a simple indexed form.
  better_names <- unlist(data[["header"]]) |>
    gsub(pattern = "^n_?(.+?)s([-\\+]\\d+)?$", replacement = "\\1_i")
  names(data[["dimensions"]]) <- better_names
  if (length(better_names) == 0) {
    # When the header is NULL
    return(tibble::add_row(tibble::tibble()))
  }
  if ("na" %in% better_names && length(better_names) == 1) {
    # When the dimensions are na because there is no associated indexing
    return(tibble::add_row(tibble::tibble()))
  }
  # Accommodate any -1 by creating a different start value
  test <- grepl("-\\d", data[["header"]])
  addition <- gsub(".+-(\\d)", "\\1", data[["header"]])
  addition[!test] <- 0
  start <- 1 + as.numeric(addition)
  data[["dimensions"]][test] <- as.numeric(data[["dimensions"]][test]) +
    as.numeric(addition)
  # Create the returned tibble by first sequencing from 1:n for each dimension
  purrr::map2(start, data[["dimensions"]], seq) |>
    purrr::set_names(names(data[["dimensions"]])) |>
    expand.grid() |>
    tibble::as_tibble() |>
    dplyr::arrange(!!!rlang::syms(better_names))
}

#' Convert the density component information into a tibble
#'
#' The density component information is stored in a single column but contains
#' a list of five elements. This function helps to widen that list into a
#' tibble and expand the `values`, `expected_values`, and `observed_values`
#' into long columns because they are all of the same length.
#'
#' @param data A list of lists from the json output that is titled
#'   `density_component`.
#' @return
#' A tibble is returned.
#' @noRd
#'
#' @examples
#' dummy_density <- list(
#'   name = "lpdf_vec",
#'   lpdf_value = -102.079,
#'   values = list(
#'     -1.39915, -2.44735, -2.93024, -3.21848, -2.95698, -3.51745
#'   ),
#'   expected_values = list(
#'     5.0854, 6.13354, 6.61636, 6.90467, 6.64311, 7.20302
#'   ),
#'   observed_values = list(
#'     161.646, 461.089, 747.29, 996.971, 767.548, 1343.86
#'   )
#' )
#' density_to_tibble(dummy_density)
#' @noRd
density_to_tibble <- function(data) {
  data |>
    tibble::as_tibble() |>
    tidyr::unnest(c(value, expected_values, observed_values)) |>
    dplyr::rename(likelihood = value)
}
