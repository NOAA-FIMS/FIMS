#' Build optional biological modules for named subpopulations
#'
#' @noRd
initialize_subpopulation_modules <- function(groups, parameters, data, fleets) {
  if (!is.list(groups)) {
    cli::cli_abort("{.arg subpopulations} must be a list of group configurations.")
  }
  build <- function(spec, module_name, fleet = NULL) {
    allowed <- c("parameters", "data", if (module_name == "Growth") "weights")
    if (!is.list(spec) || any(!names(spec) %in% allowed) ||
      (length(spec) > 0 && (is.null(names(spec)) || any(names(spec) == ""))) ||
      anyDuplicated(names(spec))) {
      cli::cli_abort("{module_name} overrides accept named {.field parameters}, {.field data}{if (module_name == 'Growth') ', and weights' else ''} settings.")
    }
    module_parameters <- if (is.null(spec[["parameters"]])) parameters else spec[["parameters"]]
    module_data <- if (is.null(spec[["data"]])) data else spec[["data"]]
    if (!tibble::is_tibble(module_parameters)) {
      cli::cli_abort("Override {.field parameters} must be a parameter tibble.")
    }
    if (!methods::is(module_data, "FIMSFrame") ||
      !identical(get_ages(module_data), get_ages(data)) ||
      !identical(get_start_year(module_data), get_start_year(data)) ||
      !identical(get_end_year(module_data), get_end_year(data))) {
      cli::cli_abort("Override {.field data} must use the population's modeled ages and years.")
    }
    module <- switch(module_name,
      Growth = initialize_growth(module_parameters, module_data),
      Maturity = initialize_maturity(module_parameters, module_data),
      Selectivity = initialize_selectivity(module_parameters, module_data, fleet)
    )
    if (!is.null(spec[["weights"]])) {
      weights <- spec[["weights"]]
      if (!is.numeric(weights) || any(!is.finite(weights)) || any(weights < 0)) {
        cli::cli_abort("Override {.field weights} must be finite, nonnegative numbers.")
      }
      # Only EWAA exposes a weights field. Derived growth uses its parameters.
      if (!methods::is(module, "Rcpp_EWAAGrowth")) {
        cli::cli_abort("The {.field weights} shortcut requires EWAA growth.")
      }
      module$weights[] <- weights
    }
    module$get_id()
  }
  lapply(groups, function(group) {
    required <- c("name", "initial_fraction", "recruitment_fraction", "reproductive_contribution")
    allowed <- c(required, "growth", "maturity", "selectivity")
    if (!is.list(group) || !all(required %in% names(group)) ||
      any(!names(group) %in% allowed) || anyDuplicated(names(group))) {
      cli::cli_abort(c(
        "Each subpopulation needs {.field name}, {.field initial_fraction}, {.field recruitment_fraction}, and {.field reproductive_contribution}.",
        "i" = "Optional overrides are {.field growth}, {.field maturity}, and {.field selectivity}. Use module configurations, not IDs: {.fn initialize_fims} rebuilds the registry."
      ))
    }
    result <- group[required]
    if (!is.null(group[["growth"]])) {
      result[["growth_id"]] <- build(group[["growth"]], "Growth")
    }
    if (!is.null(group[["maturity"]])) {
      result[["maturity_id"]] <- build(group[["maturity"]], "Maturity")
    }
    selectivity <- group[["selectivity"]]
    if (!is.null(selectivity) && length(selectivity) > 0) {
      if (!is.list(selectivity) || is.null(names(selectivity)) ||
        any(names(selectivity) == "") || anyDuplicated(names(selectivity)) ||
        any(!names(selectivity) %in% names(fleets))) {
        cli::cli_abort("Subpopulation {.field selectivity} must be a list named by known fleets.")
      }
      ids <- vapply(names(selectivity), function(name) {
        build(selectivity[[name]], "Selectivity", name)
      }, numeric(1))
      names(ids) <- vapply(fleets[names(selectivity)], function(module) {
        as.character(module$get_id())
      }, character(1))
      result[["selectivity_ids"]] <- ids
    }
    result
  })
}
