# Validate the opt-in fleet log-F random walks without mutating the active model.
normalize_fishing_processes <- function(processes, parameters, data) {
  if (is.null(processes)) return(list())
  if (!is.list(processes) || length(processes) == 0 || is.null(names(processes)) ||
      anyNA(names(processes)) || any(!nzchar(names(processes))) ||
      anyDuplicated(names(processes))) {
    cli::cli_abort("{.arg fishing_process} must be a nonempty named list of fleet specifications.")
  }
  years <- seq.int(get_start_year(data), get_end_year(data))
  if (length(years) < 2L) cli::cli_abort("Fishing random walks require at least two annual states.")
  catch_fleets <- unique(get_data(data)$fleet[get_data(data)$type %in% "catch"])
  allowed <- c("type", "sd", "estimation_type", "initial", "sd_group")
  fleet_names <- names(processes)
  output <- lapply(fleet_names, function(fleet) {
    spec <- processes[[fleet]]
    if (!fleet %in% catch_fleets || !is.list(spec) || is.null(names(spec)) ||
        anyNA(names(spec)) || anyDuplicated(names(spec)) ||
        any(!names(spec) %in% allowed) || !identical(spec$type, "rw1")) {
      cli::cli_abort("Each fishing specification must name a catch fleet and declare type = 'rw1'.")
    }
    if (!is.numeric(spec$sd) || length(spec$sd) != 1 || !is.finite(spec$sd) || spec$sd <= 0) {
      cli::cli_abort("Fishing random walks require one finite positive innovation SD.")
    }
    estimation_type <- if (is.null(spec$estimation_type)) "fixed_effects" else spec$estimation_type
    if (length(estimation_type) != 1 || is.na(estimation_type) ||
        !estimation_type %in% c("constant", "fixed_effects")) {
      cli::cli_abort("Fishing innovation SD estimation_type must be constant or fixed_effects.")
    }
    group <- spec$sd_group
    if (!is.null(group) && (!is.character(group) || length(group) != 1L ||
        is.na(group) || !nzchar(trimws(group)))) {
      cli::cli_abort("Fishing sd_group must be one nonempty string.")
    }
    initial <- spec$initial
    if (!is.list(initial) || is.null(names(initial)) || anyNA(names(initial)) ||
        anyDuplicated(names(initial)) || length(initial$type) != 1 ||
        is.na(initial$type) || !initial$type %in% c("fixed", "normal")) {
      cli::cli_abort("Specify an explicit fishing initial rule: fixed or normal.")
    }
    if (initial$type == "fixed") {
      if (!identical(names(initial), "type")) {
        cli::cli_abort("A fixed fishing initial rule uses the first log_Fmort parameter value; provide only type.")
      }
    } else {
      if (!setequal(names(initial), c("type", "mean", "sd")) ||
          !is.numeric(initial$mean) || length(initial$mean) != 1 || !is.finite(initial$mean) ||
          !is.numeric(initial$sd) || length(initial$sd) != 1 ||
          !is.finite(initial$sd) || initial$sd <= 0) {
        cli::cli_abort("A normal fishing initial rule requires a finite log-F mean and positive log-F SD.")
      }
    }
    rows <- parameters |>
      dplyr::filter(.data$module_name == "Fleet", .data$fleet == .env$fleet,
                    .data$label == "log_Fmort")
    if (nrow(rows) != length(years) || anyNA(rows$timing) ||
        !identical(as.numeric(rows$timing), as.numeric(years)) ||
        any(!is.finite(rows$value)) || anyNA(rows$estimation_type)) {
      cli::cli_abort("Fishing random walks require finite log_Fmort states in contiguous annual order.")
    }
    estimated <- rows$estimation_type %in% c("fixed_effects", "random_effects")
    if (initial$type == "fixed") {
      if (rows$estimation_type[1] != "constant" || any(!estimated[-1])) {
        cli::cli_abort("A fixed fishing initial state must be constant; all later states must be estimated.")
      }
    } else if (any(!estimated)) {
      cli::cli_abort("A normal fishing initial rule requires all log_Fmort states to be estimated.")
    }
    if (any(!is.na(rows$distribution_type)) || any(!is.na(rows$distribution))) {
      cli::cli_abort("Declare fishing process distributions only in fishing_process; duplicate parameter-row distributions are not supported.")
    }
    result <- list(type = "rw1", sd = spec$sd, estimation_type = estimation_type, initial = initial)
    if (!is.null(group)) result$sd_group <- group
    result
  })
  groups <- vapply(output, function(x) if (is.null(x$sd_group)) NA_character_ else x$sd_group, character(1))
  for (group in unique(stats::na.omit(groups))) {
    members <- output[which(groups == group)]
    if (length(unique(vapply(members, `[[`, numeric(1), "sd"))) != 1L ||
        length(unique(vapply(members, `[[`, character(1), "estimation_type"))) != 1L) {
      cli::cli_abort("Members of fishing sd_group {.val {group}} must have identical SDs and estimation types.")
    }
  }
  stats::setNames(output, fleet_names)
}

# Reuse the target's registration. Only the scalar innovation log-SD is new.
initialize_fishing_process <- function(module, spec) {
  density <- methods::new(GaussianProcessDistribution)
  initial <- spec$initial
  density$set_random_walk(initial$type,
                           if (initial$type == "normal") initial$mean else 0,
                           if (initial$type == "normal") initial$sd else 1)
  density$log_sd[] <- log(spec$sd)
  density$log_sd$set_estimation_types(spec$estimation_type)
  density$expected_values[] <- 0
  density$observed_values[] <- vapply(seq_len(length(module$log_Fmort)),
                                     function(i) module$log_Fmort[i]$value, numeric(1))
  density$set_distribution_links("random_effects", module$log_Fmort$get_id())
  density
}

# Tie only innovation log-SDs; latent states and initial distributions stay separate.
fishing_process_map <- function(parameters, distributions, specifications) {
  parameter_names <- names(get_parameter_names(as.list(parameters$p)))
  index <- seq_along(parameters$p)
  groups <- vapply(specifications, function(x) {
    if (is.null(x$sd_group) || x$estimation_type == "constant") NA_character_ else x$sd_group
  }, character(1))
  for (group in unique(stats::na.omit(groups))) {
    members <- which(groups == group)
    positions <- vapply(members, function(i) {
      prefix <- paste0("gaussian_process.", distributions[[names(specifications)[i]]]$get_id(), ".log_sd.")
      position <- which(startsWith(parameter_names, prefix))
      if (length(position) != 1L) cli::cli_abort("Cannot resolve grouped fishing innovation SD registration.")
      position
    }, integer(1))
    index[positions] <- positions[1L]
  }
  if (anyDuplicated(index)) list(p = factor(index)) else NULL
}
