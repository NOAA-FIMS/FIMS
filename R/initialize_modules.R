.native_parameter_rows <- function(parameters, module_name, label, fleet = NULL) {
  rows <- parameters |>
    dplyr::filter(
      .data$module_name == .env$module_name,
      .data$label == .env$label
    )

  if (!is.null(fleet)) {
    rows <- rows |>
      dplyr::filter(.data$fleet == .env$fleet)
  }

  if ("time" %in% names(rows) && any(!is.na(rows$time))) {
    rows <- dplyr::arrange(rows, .data$time)
  }
  if ("age" %in% names(rows) && any(!is.na(rows$age))) {
    rows <- dplyr::arrange(rows, .data$time, .data$age)
  }

  rows
}

.native_parameter_values <- function(parameters, module_name, label,
                                     fleet = NULL, default = NULL) {
  rows <- .native_parameter_rows(parameters, module_name, label, fleet)
  if (nrow(rows) == 0L) {
    return(default)
  }
  as.numeric(rows$value)
}

.native_parameter_types <- function(parameters, module_name, label,
                                    fleet = NULL, default = "constant") {
  rows <- .native_parameter_rows(parameters, module_name, label, fleet)
  if (nrow(rows) == 0L) {
    return(default)
  }
  dplyr::coalesce(rows$estimation_type, default)
}

.native_composition_data <- function(data, fleet, type) {
  get_data(data) |>
    dplyr::filter(
      .data$fleet == .env$fleet,
      .data$type == .env$type
    ) |>
    dplyr::mutate(
      sample_size = dplyr::if_else(
        .data$value == -999,
        1,
        .data$uncertainty
      )
    ) |>
    dplyr::transmute(value = .data$value * .data$sample_size) |>
    dplyr::pull(.data$value)
}

.native_cv_from_log_sd <- function(parameters, fleet, module_type) {
  rows <- parameters |>
    dplyr::filter(
      .data$module_name == "Data",
      .data$module_type == .env$module_type,
      .data$fleet == .env$fleet,
      .data$label == "log_sd"
    )

  if (nrow(rows) == 0L) {
    cli::cli_abort(
      "Native initialization requires `log_sd` for {module_type} data from fleet `{fleet}`."
    )
  }

  log_sd <- unique(rows$value)
  if (length(log_sd) != 1L) {
    cli::cli_abort(
      "The native default-likelihood builder currently requires a single {module_type} `log_sd`."
    )
  }

  sd_log <- exp(log_sd)
  sqrt(exp(sd_log^2) - 1)
}

#' Initialize a FIMS model through the native interface
#'
#' Builds the currently supported catch-at-age model without constructing any
#' C++ interface objects. Native object IDs are retained in the returned model
#' metadata for diagnostics.
#'
#' @param parameters A parameter tibble returned by
#'   [create_default_parameters()].
#' @param data A [FIMSFrame] object or compatible data frame.
#' @return A list containing TMB parameters and native model metadata.
#' @export
initialize_fims <- function(parameters, data) {
  if (missing(parameters) || !tibble::is_tibble(parameters)) {
    cli::cli_abort("The {.var parameters} argument must be a tibble.")
  }
  if (missing(data)) {
    cli::cli_abort("The {.var data} argument is required.")
  }

  if ("data" %in% names(parameters)) {
    parameters <- tidyr::unnest(parameters, cols = "data")
  }

  valid_estimation_types <- c("constant", "fixed_effects", "random_effects")
  invalid_estimation_types <- setdiff(
    stats::na.omit(unique(parameters$estimation_type)),
    valid_estimation_types
  )
  if (length(invalid_estimation_types) > 0L) {
    cli::cli_abort(c(
      "The `estimation_type` must be one of: {valid_estimation_types}.",
      i = "Invalid values found: {invalid_estimation_types}."
    ))
  }

  fleets <- stats::na.omit(unique(parameters$fleet))
  if (length(fleets) == 0L) {
    cli::cli_abort("No fleets found in the provided {.var parameters}.")
  }

  data_table <- get_data(data)
  fishing_fleets <- intersect(
    fleets,
    unique(data_table$fleet[data_table$type == "landings"])
  )
  survey_fleets <- intersect(
    fleets,
    unique(data_table$fleet[data_table$type == "index"])
  )
  if (length(fishing_fleets) != 1L || length(survey_fleets) != 1L) {
    cli::cli_abort(
      "The native default model currently requires exactly one landings fleet and one survey-index fleet."
    )
  }

  fishing_fleet <- fishing_fleets[[1L]]
  survey_fleet <- survey_fleets[[1L]]
  n_years <- get_n_years(data)
  n_ages <- get_n_ages(data)
  n_lengths <- get_n_lengths(data)

  for (fleet in fleets) {
    log_fmort_n <- length(.native_parameter_values(
      parameters, "Fleet", "log_Fmort", fleet
    ))
    if (!log_fmort_n %in% c(1L, n_years)) {
      cli::cli_abort(
        "log_Fmort size mismatch Fleet log_Fmort size mismatch: expected 1 or {n_years}, got {log_fmort_n}."
      )
    }
  }

  log_sd_rows <- parameters |>
    dplyr::filter(.data$module_name == "Data", .data$label == "log_sd") |>
    dplyr::group_by(.data$fleet, .data$module_type) |>
    dplyr::summarise(n = dplyr::n(), .groups = "drop")
  invalid_log_sd <- log_sd_rows$n[!log_sd_rows$n %in% c(1L, n_years)]
  if (length(invalid_log_sd) > 0L) {
    cli::cli_abort(
      "The size of `log_sd` does not match the model dimensions; expected 1 or {n_years}."
    )
  }

  native_clear()

  selectivity_ids <- vapply(fleets, function(fleet) {
    module_type <- unique(parameters$module_type[
      parameters$module_name == "Selectivity" & parameters$fleet == fleet
    ])
    module_type <- stats::na.omit(module_type)
    if (!identical(module_type, "Logistic")) {
      cli::cli_abort("The native initializer currently supports logistic selectivity only.")
    }

    selectivity_logistic_create(
      inflection_point = .native_parameter_values(
        parameters, "Selectivity", "inflection_point", fleet
      ),
      slope = .native_parameter_values(
        parameters, "Selectivity", "slope", fleet
      ),
      inflection_point_estimation_type = .native_parameter_types(
        parameters, "Selectivity", "inflection_point", fleet
      ),
      slope_estimation_type = .native_parameter_types(
        parameters, "Selectivity", "slope", fleet
      )
    )
  }, integer(1L))
  names(selectivity_ids) <- fleets

  age_to_length <- if (n_lengths > 0L) {
    model_age_to_length_conversion(data)
  } else {
    numeric()
  }

  fleet_ids <- vapply(fleets, function(fleet) {
    fleet_create(
      log_fmort = .native_parameter_values(
        parameters, "Fleet", "log_Fmort", fleet
      ),
      log_q = .native_parameter_values(parameters, "Fleet", "log_q", fleet),
      selectivity_id = selectivity_ids[[fleet]],
      age_to_length_conversion = age_to_length,
      log_fmort_estimation_type = .native_parameter_types(
        parameters, "Fleet", "log_Fmort", fleet
      ),
      log_q_estimation_type = .native_parameter_types(
        parameters, "Fleet", "log_q", fleet
      )
    )
  }, integer(1L))
  names(fleet_ids) <- fleets

  recruitment_type <- stats::na.omit(unique(parameters$module_type[
    parameters$module_name == "Recruitment"
  ]))
  if (!identical(recruitment_type, "BevertonHolt")) {
    cli::cli_abort("The native initializer currently supports Beverton-Holt recruitment only.")
  }
  if (any(parameters$module_name == "Recruitment" & parameters$label == "log_r")) {
    cli::cli_abort("Native `log_r` recruitment is not implemented; use `log_devs`.")
  }

  recruitment_process <- parameters |>
    dplyr::filter(.data$module_name == "Recruitment", .data$label == "log_devs")
  recruitment_rows <- dplyr::filter(parameters, .data$module_name == "Recruitment")
  process_requested <- all(c("distribution", "distribution_type") %in% names(parameters)) &&
    any(!is.na(recruitment_rows$distribution) |
      !is.na(recruitment_rows$distribution_type))
  process_complete <- nrow(recruitment_process) > 0L &&
    all(recruitment_process$estimation_type %in% c("fixed_effects", "random_effects")) &&
    all(c("distribution", "distribution_type") %in% names(recruitment_process)) &&
    all(!is.na(recruitment_process$distribution)) &&
    all(!is.na(recruitment_process$distribution_type))
  if (process_requested && !process_complete) {
    cli::cli_abort(
      "Missing required inputs for recruitment process random or fixed effects."
    )
  }

  recruitment_id <- recruitment_beverton_holt_create(
    logit_steep = .native_parameter_values(
      parameters, "Recruitment", "logit_steep"
    ),
    log_rzero = .native_parameter_values(
      parameters, "Recruitment", "log_rzero"
    ),
    log_devs = .native_parameter_values(
      parameters, "Recruitment", "log_devs", default = numeric()
    ),
    logit_steep_estimation_type = .native_parameter_types(
      parameters, "Recruitment", "logit_steep"
    ),
    log_rzero_estimation_type = .native_parameter_types(
      parameters, "Recruitment", "log_rzero"
    ),
    log_devs_estimation_type = .native_parameter_types(
      parameters, "Recruitment", "log_devs"
    )
  )

  growth_id <- growth_ewaa_create(
    ages = get_ages(data),
    weights = model_weight_at_age(data),
    n_years = n_years
  )

  maturity_id <- maturity_logistic_create(
    inflection_point = .native_parameter_values(
      parameters, "Maturity", "inflection_point"
    ),
    slope = .native_parameter_values(parameters, "Maturity", "slope"),
    inflection_point_estimation_type = .native_parameter_types(
      parameters, "Maturity", "inflection_point"
    ),
    slope_estimation_type = .native_parameter_types(
      parameters, "Maturity", "slope"
    )
  )

  population_id <- population_create(
    log_m = .native_parameter_values(parameters, "Population", "log_M"),
    log_f_multiplier = .native_parameter_values(
      parameters,
      "Population",
      "log_f_multiplier",
      default = rep(0, n_years)
    ),
    log_init_naa = .native_parameter_values(
      parameters, "Population", "log_init_naa"
    ),
    log_m_estimation_type = .native_parameter_types(
      parameters, "Population", "log_M"
    ),
    log_f_multiplier_estimation_type = .native_parameter_types(
      parameters, "Population", "log_f_multiplier"
    ),
    log_init_naa_estimation_type = .native_parameter_types(
      parameters, "Population", "log_init_naa"
    ),
    maturity_id = maturity_id,
    growth_id = growth_id,
    recruitment_id = recruitment_id,
    fleet_ids = fleet_ids
  )

  native_create_model()

  recruitment_log_sd <- .native_parameter_values(
    parameters, "Recruitment", "log_sd", default = log(1)
  )
  native_build_default_likelihood(
    fishing_fleet_id = fleet_ids[[fishing_fleet]],
    survey_fleet_id = fleet_ids[[survey_fleet]],
    landings = model_landings(data, fishing_fleet),
    landings_cv = .native_cv_from_log_sd(
      parameters, fishing_fleet, "Landings"
    ),
    landings_age_comp = .native_composition_data(
      data, fishing_fleet, "age_comp"
    ),
    landings_length_comp = .native_composition_data(
      data, fishing_fleet, "length_comp"
    ),
    survey_index = model_index(data, survey_fleet),
    survey_cv = .native_cv_from_log_sd(parameters, survey_fleet, "Index"),
    survey_age_comp = .native_composition_data(
      data, survey_fleet, "age_comp"
    ),
    survey_length_comp = .native_composition_data(
      data, survey_fleet, "length_comp"
    ),
    recruitment_log_sd = exp(recruitment_log_sd[[1L]]),
    recruitment_log_sd_estimation_type = .native_parameter_types(
      parameters, "Recruitment", "log_sd"
    ),
    n_years = n_years,
    n_ages = n_ages,
    n_lengths = n_lengths
  )

  list(
    parameters = list(
      p = native_get_fixed(),
      re = native_get_random()
    ),
    model = list(
      population_id = population_id,
      fleet_ids = fleet_ids,
      selectivity_ids = selectivity_ids,
      recruitment_id = recruitment_id,
      growth_id = growth_id,
      maturity_id = maturity_id,
      data = data_table,
      parameter_table = parameters
    )
  )
}
