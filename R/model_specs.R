#' Model component specification constructors
#'
#' These constructors describe FIMS model components without creating Rcpp
#' objects. They are intended for a future builder that translates named,
#' validated R specs into the existing low-level Rcpp interface.
#'
#' @param a50 Numeric value or FIMS role spec for age at 50 percent selectivity
#'   or maturity.
#' @param slope Numeric value or FIMS role spec for a logistic slope.
#' @param ages Numeric modeled ages.
#' @param weights Numeric vector, matrix, or array of empirical weight-at-age
#'   values.
#' @param log_rzero Numeric value or FIMS role spec for unfished recruitment.
#' @param steepness Numeric value or FIMS role spec for stock-recruit steepness.
#' @param deviations Optional FIMS role spec for recruitment deviations.
#' @param name Character component name.
#' @param selectivity A FIMS selectivity component spec.
#' @param fishing_mortality Optional numeric value or FIMS role spec.
#' @param years Numeric modeled years.
#' @param growth A FIMS growth component spec.
#' @param maturity A FIMS maturity component spec.
#' @param recruitment A FIMS recruitment component spec.
#' @param population A FIMS population component spec.
#' @param fleets A fleet spec or list of fleet specs.
#' @param observations An observation spec or list of observation specs.
#' @param likelihood Character likelihood path. Currently `"terms"` or
#'   `"legacy"`.
#'
#' @return
#' Component constructors return objects with class `fims_component_spec`.
#' [catch_at_age()] returns an object with class `fims_model_spec`.
#'
#' @name model_specs
#' @keywords model
NULL

#' @rdname model_specs
#' @export
logistic_selectivity <- function(a50, slope) {
  new_fims_component_spec(
    component = "logistic_selectivity",
    backend = "LogisticSelectivity",
    a50 = coerce_role_spec(a50, "a50"),
    slope = coerce_role_spec(slope, "slope")
  )
}

#' @rdname model_specs
#' @export
logistic_maturity <- function(a50, slope) {
  new_fims_component_spec(
    component = "logistic_maturity",
    backend = "LogisticMaturity",
    a50 = coerce_role_spec(a50, "a50"),
    slope = coerce_role_spec(slope, "slope")
  )
}

#' @rdname model_specs
#' @export
ewaa_growth <- function(ages, weights) {
  check_numeric_spec(ages, "ages")
  check_numeric_spec(as.numeric(weights), "weights")
  new_fims_component_spec(
    component = "ewaa_growth",
    backend = "EWAAGrowth",
    ages = ages,
    weights = weights
  )
}

#' @rdname model_specs
#' @export
beverton_holt <- function(log_rzero, steepness, deviations = NULL) {
  if (!is.null(deviations)) {
    check_role_spec(deviations, "deviations")
    if (deviations[["role"]] != "random_effect") {
      stop("`deviations` must be a random effect spec.", call. = FALSE)
    }
  }
  new_fims_component_spec(
    component = "beverton_holt",
    backend = "BevertonHoltRecruitment",
    log_rzero = coerce_role_spec(log_rzero, "log_rzero"),
    steepness = coerce_role_spec(steepness, "steepness"),
    deviations = deviations
  )
}

#' @rdname model_specs
#' @export
fleet <- function(name, selectivity, fishing_mortality = NULL) {
  check_name_spec(name, "name")
  check_component_or_ref_spec(selectivity, "selectivity")
  if (inherits(selectivity, "fims_component_spec") &&
      selectivity[["component"]] != "logistic_selectivity") {
    stop("`selectivity` must be a selectivity component spec.", call. = FALSE)
  }
  if (!is.null(fishing_mortality)) {
    fishing_mortality <- coerce_role_spec(fishing_mortality, "fishing_mortality")
  }
  new_fims_component_spec(
    component = "fleet",
    backend = "Fleet",
    name = name,
    selectivity = selectivity,
    fishing_mortality = fishing_mortality
  )
}

#' @rdname model_specs
#' @export
population <- function(name, ages, years, growth, maturity, recruitment) {
  check_name_spec(name, "name")
  check_numeric_spec(ages, "ages")
  check_numeric_spec(years, "years")
  check_component_or_ref_spec(growth, "growth")
  check_component_or_ref_spec(maturity, "maturity")
  check_component_or_ref_spec(recruitment, "recruitment")

  if (inherits(growth, "fims_component_spec") &&
      growth[["component"]] != "ewaa_growth") {
    stop("`growth` must be a growth component spec.", call. = FALSE)
  }
  if (inherits(maturity, "fims_component_spec") &&
      maturity[["component"]] != "logistic_maturity") {
    stop("`maturity` must be a maturity component spec.", call. = FALSE)
  }
  if (inherits(recruitment, "fims_component_spec") &&
      recruitment[["component"]] != "beverton_holt") {
    stop("`recruitment` must be a recruitment component spec.", call. = FALSE)
  }

  new_fims_component_spec(
    component = "population",
    backend = "Population",
    name = name,
    ages = ages,
    years = years,
    growth = growth,
    maturity = maturity,
    recruitment = recruitment
  )
}

#' @rdname model_specs
#' @export
catch_at_age <- function(population, fleets = list(), observations = list(),
                         likelihood = c("terms", "legacy")) {
  likelihood <- match.arg(likelihood)
  check_component_spec(population, "population")
  if (population[["component"]] != "population") {
    stop("`population` must be a population component spec.", call. = FALSE)
  }

  fleets <- as_spec_list(fleets, "fleet")
  observations <- as_spec_list(observations, "observation")

  for (fleet_spec in fleets) {
    check_component_spec(fleet_spec, "fleets")
    if (fleet_spec[["component"]] != "fleet") {
      stop("`fleets` must contain only fleet component specs.", call. = FALSE)
    }
  }
  for (observation_spec in observations) {
    if (!inherits(observation_spec, "fims_observation_spec")) {
      stop(
        "`observations` must contain only observation specs.",
        call. = FALSE
      )
    }
  }

  spec <- list(
    model = "catch_at_age",
    backend = "CatchAtAge",
    population = population,
    fleets = fleets,
    observations = observations,
    likelihood = likelihood
  )
  class(spec) <- c("fims_model_spec", "fims_spec")
  spec
}

new_fims_component_spec <- function(component, backend, ...) {
  spec <- c(list(component = component, backend = backend), list(...))
  class(spec) <- c("fims_component_spec", "fims_spec")
  spec
}

coerce_role_spec <- function(value, name) {
  if (inherits(value, "fims_role_spec")) {
    return(value)
  }
  check_numeric_spec(value, name)
  fixed_effect(value)
}

check_role_spec <- function(value, name) {
  if (!inherits(value, "fims_role_spec")) {
    stop("`", name, "` must be a FIMS role spec.", call. = FALSE)
  }
  invisible(value)
}

check_component_spec <- function(value, name) {
  if (!inherits(value, "fims_component_spec")) {
    stop("`", name, "` must be a FIMS component spec.", call. = FALSE)
  }
  invisible(value)
}

check_component_or_ref_spec <- function(value, name) {
  if (!inherits(value, "fims_component_spec") &&
      !inherits(value, "fims_ref_spec")) {
    stop(
      "`", name, "` must be a FIMS component spec or reference.",
      call. = FALSE
    )
  }
  invisible(value)
}

as_spec_list <- function(value, name) {
  if (is.null(value)) {
    return(list())
  }
  if (inherits(value, "fims_spec")) {
    return(list(value))
  }
  if (!is.list(value)) {
    stop("`", name, "` must be a spec or list of specs.", call. = FALSE)
  }
  value
}
