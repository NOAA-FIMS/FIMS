#' Create a default FIMS configuration tibble
#'
#' @description
#' This function generates a default configuration tibble for a Fisheries
#' Integrated Modeling System (FIMS) model based on the data input. It
#' automatically creates configuration entries for data modules (e.g., landings,
#' index, compositions) and, depending on the model family, standard population
#' dynamics modules (recruitment, growth, maturity) and selectivity modules for
#' fleets.
#'
#' @details
#' The function inspects the data to find unique combinations of fleet
#' names and data types. It then maps these to the appropriate FIMS module names
#' and joins them with a predefined template of default settings. When the
#' `model_family` is "catch_at_age", it also adds default configurations for:
#' \itemize{
#'   \item **Selectivity:** A logistic selectivity module for each unique fleet.
#'   \item **Recruitment:** A Beverton--Holt recruitment module.
#'   \item **Growth:** An empirical weight-at-age (EWAA) growth module.
#'   \item **Maturity:** A logistic maturity module.
#' }
#' The final output is a nested tibble, which serves as a starting point for 
#' building a complete FIMS model configuration.
#'
#' @param data An S4 object of class `FIMSFrame`. FIMS input data.
#' @param model_family A string specifying the model family.
#'   Defaults to `"catch_at_age"`.
#'
#' @return A `tibble` with default model configurations. The tibble has a nested
#'   structure with the following top-level columns.
#' \describe{
#'   \item{\code{model_family}:}{The specified model family (e.g.,
#'     "catch_at_age").}
#'   \item{\code{module_name}:}{The name of the FIMS module (e.g.,
#'     "Data", "Selectivity", "Recruitment", "Growth", "Maturity").}
#'   \item{\code{fleet_name}:}{The name of the fleet the module applies to. This
#'     will be `NA` for non-fleet-specific modules like "Recruitment".}
#'   \item{\code{data}:}{A list-column containing a `tibble` with detailed
#'     configurations. Unnesting this column reveals:
#'     \describe{
#'       \item{\code{module_type}:}{The specific type of the module (e.g.,
#'         "Logistic" for a "Selectivity" module).}
#'       \item{\code{distribution_link}:}{The component the distribution module
#'         links to.}
#'       \item{\code{distribution_type}:}{The type of distribution (e.g., "Data",
#'         "process").}
#'       \item{\code{distribution}:}{The name of distribution (e.g.,
#'         "Dlnorm", `Dmultinom`).}
#'     }
#'   }
#' }
#'
#' @export
#'
#' @examples
#' # Load the example dataset and create a FIMS data frame
#' data("data1")
#' fims_frame <- FIMSFrame(data1)
#'
#' # Create the default model configuration tibble
#' default_configurations <- create_default_configurations(data = fims_frame)
#' 
#' # Unnest the data column to see detailed configurations
#' default_configurations_unnest <- default_configurations |>
#'   tidyr::unnest(cols = data) |>
#'   print()
#' 
#' # Model fleet1 with double logistic selectivity
#' configurations_double_logistic <- default_configurations_unnest |>
#'   dplyr::rows_update(
#'     tibble::tibble(
#'       module_name = "Selectivity",
#'       fleet_name = "fleet1",
#'       module_type = "DoubleLogistic"
#'     ),
#'   by = c("module_name", "fleet_name")
#'   ) |>
#'   print()
create_default_configurations <- function(data, model_family = c("catch_at_age")) {
  # Check if the input object is a FIMSFrame, aborting if not.
  if (!inherits(data, "FIMSFrame")) {
    cli::cli_abort(
      c(
        "{.var data} must be a {.cls FIMSFrame} object.",
        "i" = "Please convert your data before using this function."
      )
    )
  }

  # Ensures the user input matches the options provided,
  #   if not, then match.arg() throws an error
  model_family <- match.arg(model_family)

  # Extract unique combinations of fleet names and data types from the data.
  # This forms the basis for determining which modules are needed for each fleet.
  unique_fleet_types <- data |>
    get_data() |>
    dplyr::distinct(name, type) |>
    # Convert type from snake_case to PascalCase
    dplyr::mutate(module_type = snake_to_pascal(type)) |>
    # Set module_type to NA for weight-at-age and age-to-length-conversion
    dplyr::mutate(module_type = dplyr::case_when(
      type == "weight-at-age" ~ NA_character_,
      type == "age-to-length-conversion"  ~ NA_character_,
      TRUE ~ module_type
    )) |>
    # Remove any combinations where the type did not match a known module.
    dplyr::filter(!is.na(module_type)) |>
    dplyr::rename(fleet_name = name) |>
    dplyr::select(-type)

  # Define a template for data modules (comps, landings, index).
  # This specifies the default distribution for each type of data.
  data_config_template <- dplyr::tribble(
    ~module_name, ~module_type, ~distribution_link, ~distribution_type, ~distribution,
    "Data", "Landings",   "Landings", "Data", "Dlnorm",
    "Data", "Index",      "Index",    "Data", "Dlnorm",
    "Data", "AgeComp",    "AgeComp",  "Data", "Dmultinom",
    "Data", "LengthComp", "LengthComp", "Data", "Dmultinom"
  )

  # Create data module configurations by joining the unique fleet types
  # with the corresponding template entries.
  fleet_data_config <- unique_fleet_types |>
    dplyr::left_join(data_config_template, by = "module_type")

  # Initialize placeholders for conditional configurations.
  selectivity_config <- tibble::tibble()
  other_config <- tibble::tibble()

  # If model_family is "catch_at_age", create selectivity configurations for
  # fleets and other configurations for population dynamics.
  if (model_family == "catch_at_age") {
    # Create these rows by getting distinct fleet names and joining them
    # with the selectivity template.
    selectivity_config <- unique_fleet_types |>
      dplyr::distinct(fleet_name) |>
      dplyr::mutate(
        module_name = "Selectivity",
        module_type = "Logistic"
      )

    # Define a template for standard, non-fleet-specific modules.
    other_config <- dplyr::tribble(
      ~module_name, ~module_type, ~distribution_link, ~distribution_type, ~distribution,
      "Recruitment", "BevertonHolt", "log_devs", "process", "Dnorm",
      "Growth", "EWAA", NA_character_, NA_character_, NA_character_,
      "Maturity", "Logistic", NA_character_, NA_character_, NA_character_
    )
  }

  # Combine all configuration pieces into a single tibble.
  # The `dplyr::bind_rows` function intelligently handles differing columns
  # by filling missing values with NA.
  final_config <- dplyr::bind_rows(
    fleet_data_config,
    selectivity_config,
    other_config
  ) |>
    # Add model_family column
    dplyr::mutate(model_family = model_family) |>
    # Arrange for readability.
    dplyr::arrange(fleet_name, module_name) |>
    # Reorder columns
    dplyr::select(
      model_family, module_name, module_type, fleet_name, everything()
    ) |>
    # Nest the configuration details into a list-column called 'data'.
    # This creates the final, structured output format expected by FIMS.
    tidyr::nest(.by = c(model_family, module_name, fleet_name))
}

#' Convert snake_case strings to PascalCase
#'
#' This function takes a vector of strings in snake_case format and converts 
#' them to PascalCase.
#'
#' @param snake_strings A vector of strings in snake_case format.
#' @return A vector of strings in PascalCase format.
#' @examples
#' snake_to_pascal(c("age_comp", "length_comp"))
#' snake_to_pascal("index")
#' @noRd
snake_to_pascal <- function(snake_strings) {
  purrr::map_chr(snake_strings, \(x) {
    parts <- strsplit(x, "_")[[1]]
    paste(
      toupper(substring(parts, 1, 1)),
      substring(parts, 2),
      sep = "",
      collapse = ""
    )
  })
}
