# Internal helper: validate the expected FIMSFrame input class.
assert_is_fims_frame <- function(data) {
  if (!inherits(data, "FIMSFrame")) {
    cli::cli_abort(c(
      "i" = "{.var data} should be a {.cls FIMSFrame} object.",
      "x" = "{.var data} is a {.cls {class(data)}} object."
    ))
  }
}

# Internal helper: validate that fleet is a single character string.
assert_single_fleet <- function(fleet) {
  if (!is.character(fleet) || length(fleet) != 1) {
    cli::cli_abort(c(
      "i" = "{.var fleet} must be a single character string.",
      "x" = "{.var fleet} is of class {.cls {class(fleet)}} and has length {length(fleet)}."
    ))
  }
}

# Internal helper: check that fleet appears in the input data.
assert_fleet_in_data <- function(data, fleet) {
  fleets <- get_fleets(data)
  if (!fleet %in% fleets) {
    cli::cli_abort(c(
      "i" = "{.var fleet} is not present in the {.var data}.",
      "x" = "Available fleet names are: {.val {fleets}}."
    ))
  }
}

# Internal helper: check that a requested module type exists for a fleet.
assert_module_type_available <- function(data, fleet, module_type) {
  available_types <- get_data(data) |>
    dplyr::filter(.data$fleet == .env$fleet) |>
    dplyr::pull(.data$type) |>
    unique() |>
    snake_to_pascal()

  if (!module_type %in% available_types) {
    cli::cli_abort(c(
      "i" = "{.var fleet} does not have {.var module_type} data in the input data.",
      "x" = "Please check the input data or specify a different {.var fleet} or {.var module_type}."
    ))
  }
}

# Internal helper: validate numeric type and allowed lengths.
assert_numeric_length <- function(x, x_name, valid_lengths, requirement) {
  if (!is.numeric(x) || !length(x) %in% valid_lengths) {
    local_bullets <- c(
      "i" = "{.var {x_name}} argument must be {requirement}.",
      "x" = "{.var {x_name}} has a length of {length(x)}.",
      "x" = "{.var {x_name}} is of the class {class(x)}."
    )
    names(local_bullets)[2] <- ifelse(length(x) > max(valid_lengths), "x", "i")
    names(local_bullets)[3] <- ifelse(inherits(x, "numeric"), "i", "x")
    cli::cli_abort(local_bullets)
  }
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