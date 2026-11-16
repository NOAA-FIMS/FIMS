# Validate the year-major, age-fast target before clearing the active model.
normalize_mortality_process <- function(process, parameters, data) {
  if (is.null(process)) return(NULL)
  allowed <- c("type", "sd", "estimation_type", "initial")
  if (!is.list(process) || is.null(names(process)) || anyNA(names(process)) ||
      anyDuplicated(names(process)) || any(!names(process) %in% allowed) ||
      !identical(process$type, "rw1")) {
    cli::cli_abort("mortality_process must declare type = 'rw1', sd and an explicit initial rule.")
  }
  if (!is.numeric(process$sd) || length(process$sd) != 1L ||
      !is.finite(process$sd) || process$sd <= 0) {
    cli::cli_abort("Mortality innovation SD must be finite and positive.")
  }
  role <- if (is.null(process$estimation_type)) "fixed_effects" else process$estimation_type
  if (length(role) != 1L || is.na(role) || !role %in% c("constant", "fixed_effects")) {
    cli::cli_abort("Mortality SD estimation_type must be constant or fixed_effects.")
  }
  initial <- process$initial
  if (!is.list(initial) || is.null(names(initial)) || anyNA(names(initial)) ||
      anyDuplicated(names(initial)) || length(initial$type) != 1L ||
      is.na(initial$type) || !initial$type %in% c("fixed", "normal")) {
    cli::cli_abort("Mortality requires an explicit fixed or normal initial rule.")
  }
  if (initial$type == "fixed") {
    if (!identical(names(initial), "type")) cli::cli_abort("Fixed mortality initialization uses only type.")
  } else if (!setequal(names(initial), c("type", "mean", "sd")) ||
             !is.numeric(initial$mean) || length(initial$mean) != 1L || !is.finite(initial$mean) ||
             !is.numeric(initial$sd) || length(initial$sd) != 1L ||
             !is.finite(initial$sd) || initial$sd <= 0) {
    cli::cli_abort("Normal mortality initialization requires a finite log-M mean and positive log-M SD.")
  }
  rows <- parameters[parameters$module_name %in% "Population" & parameters$label %in% "log_M", ]
  years <- seq.int(get_start_year(data), get_end_year(data))
  ages <- get_ages(data)
  if (length(years) < 2L || nrow(rows) != length(years) * length(ages) ||
      anyNA(rows$timing) || anyNA(rows$age) ||
      !identical(as.numeric(rows$timing), as.numeric(rep(years, each = length(ages)))) ||
      !identical(as.numeric(rows$age), as.numeric(rep(ages, length(years)))) ||
      any(!is.finite(rows$value)) || anyNA(rows$estimation_type)) {
    cli::cli_abort("Mortality random walks require a complete finite year-major, age-fast log_M grid over at least two years.")
  }
  first <- seq_len(nrow(rows)) <= length(ages)
  estimated <- rows$estimation_type %in% c("fixed_effects", "random_effects")
  if ((initial$type == "fixed" &&
       (any(rows$estimation_type[first] != "constant") || any(!estimated[!first]))) ||
      (initial$type == "normal" && any(!estimated))) {
    cli::cli_abort("Mortality states must be estimated except the constant first-year anchors for fixed initialization.")
  }
  if (any(!is.na(rows$distribution)) || any(!is.na(rows$distribution_type))) {
    cli::cli_abort("Declare mortality densities only through mortality_process.")
  }
  list(type = "rw1", sd = process$sd, estimation_type = role, initial = initial)
}

initialize_mortality_process <- function(population, spec, n_ages) {
  density <- methods::new(GaussianProcessDistribution)
  density$set_series_count(as.integer(n_ages))
  initial <- spec$initial
  density$set_random_walk(initial$type,
    if (initial$type == "normal") initial$mean else 0,
    if (initial$type == "normal") initial$sd else 1)
  density$log_sd[] <- log(spec$sd)
  density$log_sd$set_estimation_types(spec$estimation_type)
  density$expected_values[] <- 0
  density$set_distribution_links("random_effects", population$log_M$get_id())
  density
}
