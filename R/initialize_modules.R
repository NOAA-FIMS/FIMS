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

  if ("timing" %in% names(rows) && any(!is.na(rows$timing))) {
    rows <- dplyr::arrange(rows, .data$timing)
  }
  if ("age" %in% names(rows) && any(!is.na(rows$age))) {
    if ("timing" %in% names(rows)) {
      rows <- dplyr::arrange(rows, .data$timing, .data$age)
    } else {
      rows <- dplyr::arrange(rows, .data$age)
    }
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

.native_distribution_from_data <- function(data, fleet, type) {
  specifications <- get_data(data) |>
    dplyr::filter(
      .data$fleet == .env$fleet,
      .data$type == .env$type
    ) |>
    dplyr::pull(.data$uncertainty)

  if (length(specifications) == 0L) {
    cli::cli_abort(
      "The native initializer requires uncertainty specifications for {type} data from fleet `{fleet}`."
    )
  }
  parsed <- parse_data_distribution(specifications)
  families <- unique(stats::na.omit(parsed$family))
  if (length(families) != 1L || !families %in% c("dnorm", "dlnorm")) {
    cli::cli_abort(
      "The native initializer supports one continuous family (`dnorm` or `dlnorm`) per {type} data set from fleet `{fleet}`."
    )
  }
  family <- families[[1L]]
  expected_link <- if (family == "dnorm") {
    paste0(type, "_expected")
  } else {
    paste0("log_", type, "_expected")
  }
  links <- unique(stats::na.omit(parsed$link))
  if (!identical(links, expected_link)) {
    cli::cli_abort(
      "The {family} distribution for {type} data must link to `{expected_link}`."
    )
  }
  scale_name <- if (family == "dnorm") "sd" else "sdlog"
  scale <- vapply(parsed[[scale_name]], function(value) {
    as.numeric(eval(value, envir = parent.frame()))
  }, numeric(1L))
  list(family = family, scale = scale)
}

.native_validate_composition_distribution <- function(data, fleet, type) {
  specifications <- get_data(data) |>
    dplyr::filter(
      .data$fleet == .env$fleet,
      .data$type == .env$type
    ) |>
    dplyr::pull(.data$uncertainty)
  if (length(specifications) == 0L) {
    return(invisible(NULL))
  }
  parsed <- parse_data_distribution(specifications)
  families <- unique(stats::na.omit(parsed$family))
  expected_link <- if (type == "age_comp") {
    "agecomp_proportion"
  } else {
    "lengthcomp_proportion"
  }
  links <- unique(stats::na.omit(parsed$link))
  if (!identical(families, "dmultinom") || !identical(links, expected_link)) {
    cli::cli_abort(
      "Native {type} data require `dmultinom(prob = {expected_link}, size = ...)`."
    )
  }
  invisible(NULL)
}

.native_model_composition <- function(data, fleet, type) {
  has_composition <- get_data(data) |>
    dplyr::filter(
      .data[["fleet"]] == .env[["fleet"]],
      .data[["type"]] == .env[["type"]]
    ) |>
    nrow() |>
    as.logical()
  if (!has_composition) {
    return(numeric())
  }
  if (identical(type, "age_comp")) {
    return(model_age_comp(data, fleet))
  }
  model_length_comp(data, fleet)
}

#' Initialize a FIMS model through the native interface
#'
#' Builds the currently supported catch-at-age model without constructing any
#' C++ interface objects. Native object IDs are retained in the returned model
#' metadata for diagnostics.
#'
#' @param parameters A parameter tibble returned by
#'   [setup_default_parameters()].
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

  # Parameter tables created before the native-interface transition used
  # `time` for model timing. Preserve that public input contract while using
  # the current domain name internally; timing is a model coordinate, not an
  # elapsed runtime measurement.
  if ("time" %in% names(parameters) && !"timing" %in% names(parameters)) {
    parameters <- dplyr::rename(parameters, timing = "time")
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
    unique(data_table$fleet[data_table$type == "catch"])
  )
  survey_fleets <- intersect(
    fleets,
    unique(data_table$fleet[data_table$type == "index"])
  )
  if (length(fishing_fleets) != 1L || length(survey_fleets) != 1L) {
    cli::cli_abort(
      "The native default model currently requires exactly one catch fleet and one survey-index fleet."
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
      parameters, "Recruitment", "log_devs",
      default = numeric()
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
    parameters, "Recruitment", "log_sd",
    default = log(1)
  )
  catch_distribution <- .native_distribution_from_data(
    data, fishing_fleet, "catch"
  )
  index_distribution <- .native_distribution_from_data(
    data, survey_fleet, "index"
  )
  for (fleet in fleets) {
    .native_validate_composition_distribution(data, fleet, "age_comp")
    .native_validate_composition_distribution(data, fleet, "length_comp")
  }
  native_build_default_likelihood(
    fishing_fleet_id = fleet_ids[[fishing_fleet]],
    survey_fleet_id = fleet_ids[[survey_fleet]],
    landings = model_catch(data, fishing_fleet),
    landings_distribution = catch_distribution$family,
    landings_sd = catch_distribution$scale,
    landings_age_comp = .native_model_composition(
      data, fishing_fleet, "age_comp"
    ),
    landings_length_comp = .native_model_composition(
      data, fishing_fleet, "length_comp"
    ),
    survey_index = model_index(data, survey_fleet),
    survey_distribution = index_distribution$family,
    survey_sd = index_distribution$scale,
    survey_age_comp = .native_model_composition(data, survey_fleet, "age_comp"),
    survey_length_comp = .native_model_composition(
      data, survey_fleet, "length_comp"
    ),
    recruitment_log_sd = recruitment_log_sd[[1L]],
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
