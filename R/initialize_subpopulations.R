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
    required <- c("name", "recruitment_fraction", "reproductive_contribution")
    allowed <- c(required, "init_naa", "log_init_naa", "init_estimation_type", "growth", "maturity", "selectivity")
    if (!is.list(group) || !all(required %in% names(group)) ||
      any(!names(group) %in% allowed) || anyDuplicated(names(group))) {
      cli::cli_abort(c(
        "Each subpopulation needs {.field name}, {.field recruitment_fraction}, and {.field reproductive_contribution}.",
        "i" = "Optional overrides are {.field growth}, {.field maturity}, and {.field selectivity}. Use module configurations, not IDs: {.fn initialize_fims} rebuilds the registry."
      ))
    }
    result <- group[required]
    if (is.null(group$init_naa) == is.null(group$log_init_naa)) {
      cli::cli_abort("Supply exactly one of {.field init_naa} or {.field log_init_naa} for each subpopulation.")
    }
    if (!is.null(group$log_init_naa)) {
      initial <- group$log_init_naa
      if (!tibble::is_tibble(initial) || !all(c("value", "estimation_type") %in% names(initial))) {
        cli::cli_abort("{.field log_init_naa} must be a parameter tibble with value and estimation_type, ordered by modeled age.")
      }
      if (!is.null(group$init_estimation_type)) {
        cli::cli_abort("Set estimation_type in the log_init_naa tibble, not init_estimation_type.")
      }
      result$init_naa <- exp(initial$value)
      result$init_estimation_type <- initial$estimation_type
    } else {
      result$init_naa <- group$init_naa
      result$init_estimation_type <- if (is.null(group$init_estimation_type)) "constant" else group$init_estimation_type
    }
    if (!is.numeric(result$init_naa) || length(result$init_naa) != length(get_ages(data)) ||
      any(!is.finite(result$init_naa)) || any(result$init_naa <= 0)) {
      cli::cli_abort("Each subpopulation's init_naa must contain one finite, strictly positive value per modeled age.")
    }
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
