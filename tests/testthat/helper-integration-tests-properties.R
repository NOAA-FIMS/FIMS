# Helper file for the population properties tests ----
# These functions are used by the test-integration-properties-*.R files. Each
# property runs the model at its input values without optimizing and compares
# the output to a value worked out in R from the inputs, or from other model
# outputs through a closed form.

#' Pull the values of one input from a parameter tibble
#'
#' @param parameters A tibble from `setup_default_parameters()`.
#' @param module A string with the module name, e.g., `"Recruitment"`.
#' @param input_label A string with the parameter label, e.g., `"log_rzero"`.
#' @param fleet_name A string with the fleet name. The default `NA` keeps
#'   every fleet.
#' @return A numeric vector ordered by year and then age.
pull_input <- function(parameters, module, input_label, fleet_name = NA) {
  parameters |>
    dplyr::filter(
      module_name == .env$module,
      label == .env$input_label,
      is.na(.env$fleet_name) | fleet == .env$fleet_name
    ) |>
    dplyr::arrange(timing, age) |>
    dplyr::pull(value)
}

#' Pull the values of one output from a TMB report
#'
#' @details The report holds 1 vector per population or per fleet.
#'   `initialize_fims()` creates fleets in the order they first appear in the
#'   parameters, and the report lists fleet outputs in that order.
#' @param report A list from `get_report()`.
#' @param output_label A string with the output label, e.g.,
#'   `"spawning_biomass"`.
#' @param fleet_number The position of the fleet in the report. The default
#'   of 1 is also the only population.
#' @return A numeric vector ordered by year and then age or length.
pull_output <- function(report, output_label, fleet_number = 1) {
  report[[output_label]][[fleet_number]]
}

#' Run FIMS once at the input values and return the report
#'
#' @details The report is used rather than `get_estimates()` because it keeps
#'   full precision. `get_estimates()` rounds derived quantities, which matters
#'   for exact comparisons of small values such as proportions and ratios.
#' @param parameters A tibble from `setup_default_parameters()`.
#' @param data A `FIMSFrame` object.
#' @return A list from `get_report()`.
run_deterministic <- function(parameters, data) {
  fit <- parameters |>
    initialize_fims(data = data) |>
    fit_fims(optimize = FALSE)
  report <- get_report(fit)
  clear()
  report
}

#' Equilibrium numbers at age with no fishing
#'
#' @details Recruits enter at the first model age. Each later age has survived
#'   the natural mortality of every younger age once. The plus group holds all
#'   older ages, which is the sum of a geometric series with ratio exp(-M) of
#'   the plus group.
#' @param recruitment Numbers at the first model age.
#' @param natural_mortality A numeric vector of natural mortality, 1 per age.
#' @return A numeric vector of numbers at age.
equilibrium_numbers_at_age <- function(recruitment, natural_mortality) {
  n_ages <- length(natural_mortality)
  numbers_at_age <- recruitment *
    exp(-cumsum(c(0, natural_mortality[-n_ages])))
  numbers_at_age[n_ages] <- numbers_at_age[n_ages] /
    (1 - exp(-natural_mortality[n_ages]))
  numbers_at_age
}

#' Weight at age from a data tibble as a year by age matrix
#'
#' @details The model uses 1 weight at age per year and age. The function
#'   stops if the data hold more than 1 weight-at-age row for a year and age,
#'   e.g., rows for more than 1 fleet, because the matrix would then not be
#'   the weights the model uses.
#' @param data A tibble of data such as `data_big`.
#' @return A matrix where rows are years, including the year after the last,
#'   and columns are ages.
weight_at_age_matrix <- function(data) {
  weight_at_age <- data |>
    dplyr::filter(type == "weight_at_age") |>
    dplyr::arrange(timing, age)
  if (anyDuplicated(weight_at_age[c("timing", "age")]) > 0) {
    cli::cli_abort(c(
      "x" = "The data have more than 1 {.code weight_at_age} row for a year
      and age.",
      "i" = "These helpers expect 1 weight at age per year and age.",
      "i" = "Check {.code dplyr::count(data, type, fleet, timing, age)} for
      the {.code weight_at_age} rows."
    ))
  }
  matrix(
    weight_at_age[["observed"]],
    ncol = dplyr::n_distinct(weight_at_age[["age"]]),
    byrow = TRUE
  )
}
