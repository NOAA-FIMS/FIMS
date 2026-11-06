# Helper file for the population properties tests ----
# These functions are used by the test-integration-properties-*.R files, and
# run_deterministic() also by test-integration-ageing-error.R. Each property
# changes a few inputs and runs the model at its input values without
# optimizing. It compares the output to a value worked out in R from the
# inputs, or from other model outputs through a closed form, or to the output
# of a second run.

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

#' Find the position of a fleet in the report
#'
#' @details Moving parameter rows can change the position, so use the
#'   parameters of the run that made the report. The function stops if the
#'   name is not a fleet, because `pull_output()` would then return `NULL`,
#'   and 2 runs that both return `NULL` compare equal.
#' @param parameters The parameter tibble used for the run.
#' @param fleet_name A string with the fleet name.
#' @return An integer to pass to `pull_output()` as `fleet_number`.
fleet_position <- function(parameters, fleet_name) {
  position <- match(fleet_name, unique(stats::na.omit(parameters[["fleet"]])))
  if (is.na(position)) {
    cli::cli_abort("{.val {fleet_name}} is not a fleet in the parameters.")
  }
  position
}

#' Run FIMS once at the input values and return the report
#'
#' @details The report is used rather than `get_estimates()` because it keeps
#'   full precision. `get_estimates()` rounds derived quantities, which matters
#'   for exact comparisons of small values such as proportions and ratios. The
#'   report does not name the likelihood components (#1782), so they are named
#'   from the model output, e.g., `"fleet1_catch"`, before the model is
#'   cleared.
#' @param parameters A tibble from `setup_default_parameters()`.
#' @param data A `FIMSFrame` object.
#' @return A list from `get_report()`.
run_deterministic <- function(parameters, data) {
  fit <- parameters |>
    initialize_fims(data = data) |>
    fit_fims(optimize = FALSE)
  report <- get_report(fit)
  names(report[["nll_components"]]) <- FIMS:::label_nll_components(
    get_model_output(fit), length(report[["nll_components"]])
  )
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

#' Unfished quantities from the inputs
#'
#' @details The closed forms assume a single M for all ages and years, so
#'   `natural_mortality` has length 1 when the inputs meet that assumption.
#'   The FIMS logistic with slope s is the logistic distribution function with
#'   scale 1 / s. Unfished spawning biomass SB0 is R0 times spawning biomass
#'   per recruit, using the first-year weight and maturity.
#' @param parameters A tibble from `setup_default_parameters()`.
#' @param ages A numeric vector of model ages from `get_ages()`.
#' @param weight_at_age A matrix from `weight_at_age_matrix()`.
#' @return A list with `r0`, `natural_mortality`, `maturity` at age,
#'   `proportion_female`, equilibrium `numbers_at_age`, and SB0 as
#'   `spawning_biomass`.
unfished_quantities <- function(parameters, ages, weight_at_age) {
  r0 <- exp(pull_input(parameters, "Recruitment", "log_rzero"))
  natural_mortality <- exp(unique(
    pull_input(parameters, "Population", "log_M")
  ))
  maturity <- stats::plogis(
    ages,
    location = pull_input(parameters, "Maturity", "inflection_point"),
    scale = 1 / pull_input(parameters, "Maturity", "slope")
  )
  proportion_female <- pull_input(
    parameters, "Population", "proportion_female"
  )
  numbers_at_age <- equilibrium_numbers_at_age(
    r0, rep(natural_mortality, length(ages))
  )
  list(
    r0 = r0,
    natural_mortality = natural_mortality,
    maturity = maturity,
    proportion_female = proportion_female,
    numbers_at_age = numbers_at_age,
    spawning_biomass = sum(
      numbers_at_age * weight_at_age[1, ] * proportion_female * maturity
    )
  )
}
