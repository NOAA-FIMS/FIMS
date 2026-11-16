#' Simulate population trajectories and observations from a FIMS configuration
#'
#' @description
#' Generate recruitment and optional fleet log-F and age-specific log-M paths, recompute population
#' dynamics in annual order, and optionally generate observations from each path.
#' Other parameters and initial abundance are held at the supplied values. This is
#' simulation from a configuration, not a draw from a fitted posterior.
#'
#' @param parameters Parameter tibble, as for [initialize_fims()]. Supply fitted
#'   values explicitly to simulate at estimates rather than starting values.
#' @param data A [FIMSFrame()] including all years to simulate, biological inputs,
#'   fleet scenarios and observation uncertainty formulas.
#' @param recruitment_process Gaussian process list, as for [initialize_fims()].
#' @param nsim Positive integer number of replicate trajectories.
#' @param seed Optional nonnegative integer seed. When supplied, the caller's
#'   RNG state is restored. Otherwise the current R random stream is consumed.
#' @param from_year First recruitment year to simulate. Defaults to the first
#'   modeled recruitment transition (the initial population is held fixed).
#'   Earlier recruitment states condition the continuation. With the default
#'   `NULL`, observations are simulated from the initial model year too; an
#'   explicit year retains all earlier observations. With a fishing or mortality process,
#'   the initial model year is also a valid continuation year. Years beyond the
#'   configured model horizon must be added to data and parameters beforehand.
#' @param process_mode One of `"simulate"` (stationary initial draw or conditional
#'   AR1 continuation), `"fixed"` (supplied log-R states or log deviations), or `"mean"`
#'   (zero future innovations on the log residual scale). The last is a
#'   conditional mean residual path, not an arithmetic mean population forecast.
#' @param fishing_process Optional fleet random-walk list, as in [initialize_fims()].
#'   To forecast at fitted values, update its innovation SDs as well as the annual
#'   state values in `parameters`; fitted covariance values are not inferred from
#'   a stored fit. The same applies to recruitment correlation in its specification.
#' @param fishing_mode Fishing process mode (`"simulate"`, `"fixed"`, `"mean"`),
#'   defaulting to `process_mode`. Zero innovations hold the last log-F level;
#'   normal initialization uses its declared mean and fixed initialization
#'   retains the first supplied state. Whole-series draws include initial-year F.
#' @param mortality_process Optional age-specific log-M random-walk list, as in
#'   [initialize_fims()]. Supply innovation SD and initial-rule values explicitly.
#' @param mortality_mode Mortality mode (`"simulate"`, `"fixed"`, `"mean"`),
#'   defaulting to `process_mode`. Continuations retain each age's earlier states.
#'   Zero innovations continue its last log-M level; whole-series normal draws
#'   use the declared initial distribution independently at each age. Zero
#'   innovations give natural-scale medians, not arithmetic mean M or biomass.
#' @param observation_error Draw observation noise when `TRUE`. Otherwise return
#'   model predictions (lognormal medians) in the simulated column.
#' @return A list with configuration, RNG metadata and `replicates`. Each replicate
#'   contains recruitment states/means/residuals, population and fleet quantities,
#'   optional age-year `natural_mortality` tables (`log_M`, `M`, `conditioned`),
#'   and an observation table with model predictions, distribution locations,
#'   arithmetic observation means and simulated values. Observations before an
#'   explicit `from_year` are retained. Missing uncertainty designs are labeled and
#'   are not filled with invented sampling errors. Counts use their input total
#'   sample size, or a declared design size for missing composition observations.
#' @details
#' The supported scope is one population with IID/AR1 Gaussian recruitment,
#' optional independent random walks on annual fleet log-F and age-specific log-M,
#' normal/lognormal catch and index data, and multinomial compositions. Other
#' parameters and latent processes are fixed. Initial abundance is conditional;
#' the existing extra terminal report year uses mean recruitment. All process
#' paths are drawn before observations, so toggling observation error preserves
#' trajectories under the same seed. This function rebuilds the active native
#' model, like [initialize_fims()]; stored fits remain readable, but their live
#' TMB methods should not be reused after another model is initialized.
#'
#' [forecast_fims()] uses the same evaluator with an explicit continuation year.
#' Future model years, growth and other inputs must already be configured.
#' Fishing and natural mortality stay at supplied values unless walks are requested;
#' simulated or mean walk paths replace the requested future log-F/log-M values.
#' A supplied future catch observation is a sampling design, not a catch target
#' solved by this function. Parameter and initial-abundance uncertainty are
#' excluded; initial F draws follow the declared random-walk rule.
#' @export
#' @examples
#' \dontrun{
#' data <- FIMSFrame(data_big)
#' parameters <- setup_default_parameters(data)
#' sims <- simulate_fims(parameters, data,
#'   recruitment_process = list(type = "ar1", rho = 0.4), nsim = 10, seed = 123)
#' }
simulate_fims <- function(parameters, data, recruitment_process,
                          nsim = 1L, seed = NULL, from_year = NULL,
                          process_mode = c("simulate", "fixed", "mean"),
                          observation_error = TRUE, fishing_process = NULL,
                          fishing_mode = process_mode,
                          mortality_process = NULL, mortality_mode = process_mode) {
  process_mode <- match.arg(process_mode)
  fishing_mode <- match.arg(fishing_mode, c("simulate", "fixed", "mean"))
  mortality_mode <- match.arg(mortality_mode, c("simulate", "fixed", "mean"))
  if (!is.numeric(nsim) || length(nsim) != 1L || !is.finite(nsim) ||
      nsim < 1 || nsim > .Machine$integer.max || nsim != floor(nsim)) {
    cli::cli_abort("{.arg nsim} must be a positive integer.")
  }
  if (!is.logical(observation_error) || length(observation_error) != 1L ||
      is.na(observation_error)) {
    cli::cli_abort("{.arg observation_error} must be TRUE or FALSE.")
  }
  if (!is.null(seed) && (!is.numeric(seed) || length(seed) != 1L ||
      !is.finite(seed) || seed < 0 || seed > .Machine$integer.max || seed != floor(seed))) {
    cli::cli_abort("{.arg seed} must be a nonnegative integer or NULL.")
  }
  # Keep optional local seeds independent of the caller's random stream.
  had_seed <- exists(".Random.seed", envir = .GlobalEnv, inherits = FALSE)
  saved_seed <- if (had_seed) get(".Random.seed", envir = .GlobalEnv) else NULL
  if (!is.null(seed)) {
    on.exit({
      if (had_seed) assign(".Random.seed", saved_seed, envir = .GlobalEnv)
      else if (exists(".Random.seed", envir = .GlobalEnv, inherits = FALSE))
        rm(".Random.seed", envir = .GlobalEnv)
    }, add = TRUE)
  }
  is.FIMSFrame(data)
  process <- validate_recruitment_process(parameters, data, recruitment_process)
  fishing <- normalize_fishing_processes(fishing_process, parameters, data)
  mortality <- normalize_mortality_process(mortality_process, parameters, data)
  recruitment_rows <- parameters[parameters$module_name %in% "Recruitment" &
                                   parameters$label %in% c("log_devs", "log_r"), ]
  years <- recruitment_rows$timing
  whole_series <- is.null(from_year)
  if (whole_series) from_year <- years[1]
  if (!is.numeric(from_year) || length(from_year) != 1L ||
      !is.finite(from_year) ||
      !from_year %in% c(if (length(fishing) || !is.null(mortality)) get_start_year(data), years)) {
    cli::cli_abort(if (length(fishing) || !is.null(mortality)) {
      "{.arg from_year} must be a modeled process year."
    } else "{.arg from_year} must be a modeled recruitment year.")
  }
  first <- sum(years < from_year)
  fishing_first <- if (whole_series) 0L else from_year - get_start_year(data)
  observation_from <- if (whole_series) get_start_year(data) else from_year
  target <- unique(recruitment_rows$label)
  design <- simulation_observation_design(data)
  initialized <- initialize_fims(
    parameters, data, recruitment_process = process,
    fishing_process = if (length(fishing)) fishing else NULL,
    mortality_process = mortality
  )
  baseline <- initialized$model$evaluate_recruitment_path(numeric(), target)
  residuals <- if (target == "log_r") baseline$target - baseline$log_mean else baseline$target
  density <- attr(initialized, "gaussian_recruitment")
  density$observed_values[] <- residuals
  density$expected_values[] <- 0

  if (!is.null(seed)) set.seed(seed)
  if (!exists(".Random.seed", envir = .GlobalEnv, inherits = FALSE) &&
      (process_mode == "simulate" || observation_error ||
       (length(fishing) && fishing_mode == "simulate") ||
       (!is.null(mortality) && mortality_mode == "simulate"))) set.seed(NULL)
  rng_state <- if (exists(".Random.seed", envir = .GlobalEnv, inherits = FALSE))
    get(".Random.seed", envir = .GlobalEnv) else NULL
  paths <- lapply(seq_len(nsim), function(i) {
    if (process_mode == "fixed") residuals
    else density$simulate_from(first, process_mode == "simulate")
  })
  fishing_paths <- lapply(seq_len(nsim), function(i) list())
  fishing_densities <- attr(initialized, "fishing_processes")
  for (fleet in names(fishing)) {
    fishing_density <- fishing_densities[[fleet]]
    fishing_density$observed_values[] <- baseline$log_fishing_mortality[[fleet]]
    for (i in seq_len(nsim)) {
      fishing_paths[[i]][[fleet]] <- if (fishing_mode == "fixed") {
        baseline$log_fishing_mortality[[fleet]]
      } else fishing_density$simulate_from(fishing_first, fishing_mode == "simulate")
    }
  }
  mortality_paths <- rep(list(numeric()), nsim)
  if (!is.null(mortality)) {
    mortality_density <- attr(initialized, "gaussian_mortality")
    mortality_density$observed_values[] <- baseline$log_natural_mortality
    mortality_paths <- lapply(seq_len(nsim), function(i) {
      if (mortality_mode == "fixed") baseline$log_natural_mortality else
        mortality_density$simulate_from(fishing_first * get_n_ages(data),
                                         mortality_mode == "simulate")
    })
  }
  replicates <- lapply(seq_len(nsim), function(i) {
    # Fixed direct log-R states must stay fixed when new F or M paths change the
    # biological mean; holding their old residuals would change recruitment.
    recruitment_path <- if (process_mode == "fixed" && target == "log_r")
      numeric() else paths[[i]]
    snapshot <- initialized$model$evaluate_all_process_paths(
      recruitment_path, target, fishing_paths[[i]], mortality_paths[[i]]
    )
    realized_residuals <- if (process_mode == "fixed" && target == "log_r")
      snapshot$target - snapshot$log_mean else paths[[i]]
    realized_log_recruitment <- if (target == "log_r") snapshot$target else
      snapshot$log_mean + realized_residuals
    checked <- c(snapshot$target, snapshot$log_mean,
                 snapshot$population$numbers_at_age,
                 snapshot$population$biomass,
                 snapshot$population$spawning_biomass)
    if (any(!is.finite(checked))) {
      cli::cli_abort("Simulation replicate {i} produced non-finite states or predictions.")
    }
    result <- list(
      recruitment = tibble::tibble(
        timing = years, target = target, value = snapshot$target,
        log_mean = snapshot$log_mean, residual = realized_residuals,
        log_recruitment = realized_log_recruitment,
        conditioned = years < from_year
      ),
      population = snapshot$population,
      fleets = snapshot$fleets,
      observations = simulation_observations(
        snapshot, design, observation_from, observation_error
      )
    )
    if (length(fishing)) {
      result$fishing_mortality <- dplyr::bind_rows(lapply(names(fishing), function(fleet) {
        log_f <- snapshot$log_fishing_mortality[[fleet]]
        fixed_initial <- fishing[[fleet]]$initial$type == "fixed"
        tibble::tibble(
          fleet = fleet, timing = seq.int(get_start_year(data), get_end_year(data)),
          log_Fmort = log_f, Fmort = exp(log_f),
          conditioned = seq_along(log_f) <= fishing_first |
            (seq_along(log_f) == 1L & fixed_initial)
        )
      }))
    }
    if (!is.null(mortality)) {
      n_ages <- get_n_ages(data)
      timing <- rep(seq.int(get_start_year(data), get_end_year(data)), each = n_ages)
      result$natural_mortality <- tibble::tibble(
        timing = timing, age = rep(get_ages(data), get_n_years(data)),
        log_M = snapshot$log_natural_mortality, M = exp(log_M),
        conditioned = timing < get_start_year(data) + fishing_first |
          (timing == get_start_year(data) & mortality$initial$type == "fixed")
      )
    }
    result
  })
  result <- list(
    configuration = list(
      parameters = parameters, data = get_data(data), recruitment_process = process,
      from_year = from_year, observation_from_year = observation_from,
      process_mode = process_mode,
      observation_error = observation_error, parameter_uncertainty = FALSE,
      initial_state_uncertainty = FALSE, other_process_uncertainty = FALSE,
      nsim = nsim,
      years = seq.int(get_start_year(data), get_end_year(data) + 1L),
      ages = get_ages(data), terminal_year_recruitment = "biological mean"
    ),
    rng = list(seed = seed, kind = RNGkind(), initial_state = rng_state),
    replicates = replicates
  )
  if (length(fishing)) {
    result$configuration$fishing_process <- fishing
    result$configuration$fishing_mode <- fishing_mode
    result$configuration$fishing_from_year <- get_start_year(data) + fishing_first
    result$configuration$fishing_initial_uncertainty <- fishing_first == 0L &&
      fishing_mode == "simulate" && any(vapply(fishing,
        function(x) x$initial$type == "normal", logical(1)))
  }
  if (!is.null(mortality)) {
    result$configuration$mortality_process <- mortality
    result$configuration$mortality_mode <- mortality_mode
    result$configuration$mortality_from_year <- get_start_year(data) + fishing_first
    result$configuration$mortality_initial_uncertainty <- fishing_first == 0L &&
      mortality_mode == "simulate" && mortality$initial$type == "normal"
  }
  result
}

#' @rdname simulate_fims
#' @export
forecast_fims <- function(parameters, data, recruitment_process, from_year,
                          nsim = 1L, seed = NULL,
                          process_mode = c("simulate", "fixed", "mean"),
                          observation_error = TRUE, fishing_process = NULL,
                          fishing_mode = process_mode,
                          mortality_process = NULL, mortality_mode = process_mode) {
  simulate_fims(parameters, data, recruitment_process, nsim, seed, from_year,
                 match.arg(process_mode), observation_error,
                 fishing_process = fishing_process, fishing_mode = fishing_mode,
                 mortality_process = mortality_process, mortality_mode = mortality_mode)
}

# Resolve observation layout once; use the same fleet-specific bins as setup.
simulation_observation_design <- function(data) {
  rows <- get_data(data) |>
    dplyr::filter(.data$type %in% c("catch", "index", "age_comp", "length_comp"))
  parsed <- parse_data_distribution(rows$uncertainty)
  rows$family <- parsed$family
  rows$link <- parsed$link
  scalar <- function(x) if (is.numeric(x) && length(x) == 1L) x else NA_real_
  rows$sd <- vapply(seq_len(nrow(rows)), function(i) {
    if (is.na(rows$family[i])) NA_real_
    else if (rows$family[i] == "dnorm") scalar(parsed$sd[[i]])
    else if (rows$family[i] == "dlnorm") scalar(parsed$sdlog[[i]])
    else NA_real_
  }, numeric(1))
  rows$sample_size <- vapply(parsed$size, scalar, numeric(1))
  ages <- get_ages(data)
  bins <- resolve_fleet_length_bins(get_data(data))
  year <- rows$timing - get_start_year(data)
  rows$position <- year + 1L
  rows$model_bin <- TRUE
  for (fleet in unique(rows$fleet)) {
    a <- which(rows$fleet == fleet & rows$type == "age_comp")
    rows$position[a] <- year[a] * length(ages) + match(rows$age[a], ages)
    l <- which(rows$fleet == fleet & rows$type == "length_comp")
    rows$position[l] <- year[l] * length(bins[[fleet]]) + match(rows$length[l], bins[[fleet]])
    rows$model_bin[l] <- !is.na(rows$position[l])
  }
  if (any(!is.na(rows$family) & !rows$family %in% c("dnorm", "dlnorm", "dmultinom"))) {
    cli::cli_abort("Simulation supports normal, lognormal, and multinomial observations.")
  }
  comp <- rows$type %in% c("age_comp", "length_comp")
  if (any(comp & !is.na(rows$family) & rows$family != "dmultinom") ||
      any(!comp & rows$family %in% "dmultinom")) {
    cli::cli_abort("Simulation requires multinomial compositions and normal/lognormal scalar observations.")
  }
  rows
}

# Draw observations only after all population paths have been generated.
simulation_observations <- function(snapshot, design, from_year, noise) {
  out <- design
  out$expected <- out$location <- out$observation_mean <- NA_real_
  out$simulated <- out$observed
  out$status <- ifelse(out$timing < from_year, "conditioned", "expected")
  for (i in seq_len(nrow(out))) {
    if (!out$model_bin[i]) {
      out$status[i] <- "outside_model_bins"
      out$simulated[i] <- NA_real_
      next
    }
    quantities <- snapshot$fleets[[out$fleet[i]]]
    link <- out$link[i]
    if (is.na(link)) link <- switch(out$type[i],
      catch = "catch_expected", index = "index_expected",
      age_comp = "agecomp_proportion", length_comp = "lengthcomp_proportion")
    values <- quantities[[link]]
    position <- out$position[i]
    if (is.null(values) || position > length(values) || position < 1) {
      cli::cli_abort("Observation prediction link {.val {link}} is unavailable or has incompatible dimensions.")
    }
    location <- values[position]
    out$location[i] <- location
    out$expected[i] <- if (out$family[i] %in% "dlnorm") exp(location) else location
    out$observation_mean[i] <- if (out$family[i] %in% "dlnorm")
      exp(location + out$sd[i]^2 / 2) else out$expected[i]
  }
  active <- out$timing >= from_year & out$model_bin
  scalar <- active & out$type %in% c("index", "catch")
  for (i in which(scalar)) {
    if (!noise) {
      out$simulated[i] <- out$expected[i]
    } else if (is.na(out$family[i]) || !is.finite(out$sd[i]) || out$sd[i] <= 0) {
      out$simulated[i] <- NA_real_
      out$status[i] <- "missing_uncertainty"
    } else {
      out$simulated[i] <- if (out$family[i] == "dnorm")
        stats::rnorm(1, out$location[i], out$sd[i]) else
        stats::rlnorm(1, out$location[i], out$sd[i])
      out$status[i] <- "simulated"
    }
  }
  indices <- which(out$model_bin & out$type %in% c("age_comp", "length_comp"))
  groups <- split(indices, interaction(out$fleet[indices], out$type[indices],
                                      out$timing[indices], drop = TRUE))
  for (idx in groups) {
    if (!length(idx)) next
    probability <- out$expected[idx]
    if (any(!is.finite(probability)) || any(probability < 0) ||
        abs(sum(probability) - 1) > 1e-6) {
      cli::cli_abort("Composition predictions must be nonnegative probabilities summing to one.")
    }
    observed <- out$observed[idx]
    count_units <- all(out$unit[idx] == "number")
    size <- if (count_units && all(is.finite(observed) & observed >= 0))
      sum(observed) else unique(out$sample_size[idx])
    valid_size <- length(size) == 1 && is.finite(size) && size >= 0 && size == floor(size)
    out$observation_mean[idx] <- if (count_units) {
      if (valid_size) size * probability else NA_real_
    } else probability
    if (out$timing[idx[1]] < from_year) next
    # Proportion expectations need no sample size when no noise is requested.
    if (!noise && !count_units) {
      out$simulated[idx] <- probability
      out$status[idx] <- "expected"
      next
    }
    if (!valid_size || (noise && anyNA(out$family[idx]))) {
      out$simulated[idx] <- NA_real_
      out$status[idx] <- "missing_uncertainty"
      next
    }
    if (size == 0 && !count_units) {
      out$simulated[idx] <- NA_real_
      out$status[idx] <- "missing_uncertainty"
      next
    }
    counts <- if (noise) as.numeric(stats::rmultinom(1, size, probability)) else size * probability
    out$simulated[idx] <- if (count_units) counts else counts / size
    out$observation_mean[idx] <- if (count_units) size * probability else probability
    out$status[idx] <- if (noise) "simulated" else "expected"
  }
  out
}
