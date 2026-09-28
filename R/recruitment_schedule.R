#' Prepare a fixed recruitment phase schedule
#'
#' Validate timing and fractions that split one annual recruitment total among
#' phases. Add `type = "recruitment_fraction"` rows to the ordinary input data
#' frame, with entry dates in `timing`, fractions in `observed`, phase names in
#' `phase`, and optional `entry_age`. Initialization reads these rows automatically.
#'
#' @param data A FIMSFrame defining the modeled years and optional recruitment rows.
#' @param schedule Optional explicit schedule for existing callers, with
#'   population, phase, timing, and fraction columns. The previous `date` column
#'   is also accepted. Timing must be full ISO YYYY-MM-DD strings or whole-day
#'   Date values. NULL reads recruitment_fraction rows from data, or uses one
#'   January 1 phase per year when no rows are supplied. If both sources are
#'   supplied, they must agree. Fractions must be finite, nonnegative, and sum to
#'   one within 1e-8 in each year; they are not renormalized. Optional entry_age
#'   defaults to the youngest modeled age and must be finite, nonnegative, and
#'   constant across years within each phase.
#' @return A tibble sorted by date, containing the input columns plus timing,
#'   entry_age, phase_i, year_i, day, and year_fraction. Only modeled years are included; the extra
#'   terminal reporting year is excluded. Population must be population1.
#' @examples
#' observations <- data_big
#' observations$timing <- sprintf("%04d", observations$timing)
#' observations$timing[is.na(data_big$timing)] <- NA_character_
#' years <- seq_len(get_n_years(FIMSFrame(data_big)))
#' recruitment <- data.frame(
#'   type = "recruitment_fraction", phase = "annual",
#'   timing = sprintf("%04d-01-01", years), observed = 1
#' )
#' frame <- FIMSFrame(dplyr::bind_rows(observations, recruitment))
#' head(setup_recruitment_schedule(frame))
#' @export
setup_recruitment_schedule <- function(data, schedule = NULL) {
  if (!methods::is(data, "FIMSFrame")) {
    cli::cli_abort("`data` must be a FIMSFrame.")
  }
  embedded <- recruitment_schedule_from_data(get_data(data))
  reference <- NULL
  if (is.null(schedule)) {
    schedule <- embedded
  } else if (!is.null(embedded)) {
    reference <- setup_recruitment_schedule(data)
  }
  # `timing` is the public input column. Keep accepting the previous explicit
  # schedule's `date` column and its normalized output for existing callers.
  if (is.data.frame(schedule) && "timing" %in% names(schedule)) {
    if (!"date" %in% names(schedule)) {
      schedule$date <- schedule$timing
    } else {
      # Match the input-table convention: explicit timing edits supersede
      # normalized date metadata, while annual coordinates remain unchanged.
      timing <- schedule$timing
      explicit <- if (inherits(timing, "Date")) rep(TRUE, length(timing)) else
        !is.na(timing) & nchar(as.character(timing)) > 4L
      if (any(explicit)) {
        if (inherits(timing, "Date") &&
            any(!is.finite(as.numeric(timing)) | as.numeric(timing) != trunc(as.numeric(timing))))
          cli::cli_abort("Recruitment dates must be finite whole-day dates.")
        dates <- if (inherits(schedule$date, "Date")) observation_date_iso(schedule$date) else schedule$date
        dates[explicit] <- if (inherits(timing, "Date")) observation_date_iso(timing[explicit]) else timing[explicit]
        schedule$date <- dates
      }
    }
  }
  years <- seq.int(get_start_year(data), get_end_year(data))
  if (is.null(schedule)) {
    schedule <- data.frame(
      population = "population1", phase = "annual",
      date = sprintf("%04d-01-01", years), fraction = 1
    )
  }
  required <- c("population", "phase", "date", "fraction")
  if (!is.data.frame(schedule) || !all(required %in% names(schedule))) {
    cli::cli_abort("`schedule` requires population, phase, timing, and fraction columns (`date` is also accepted).")
  }
  if (!"entry_age" %in% names(schedule)) schedule$entry_age <- min(get_ages(data))
  required <- c(required, "entry_age")
  schedule <- as.data.frame(schedule[required])
  if (!nrow(schedule)) cli::cli_abort("Recruitment schedule must not be empty.")
  if (!is.character(schedule$population) || anyNA(schedule$population) ||
    any(schedule$population != "population1")) {
    cli::cli_abort("Recruitment schedules support only population1.")
  }
  if (!is.character(schedule$phase) || anyNA(schedule$phase) ||
    any(!nzchar(trimws(schedule$phase)))) {
    cli::cli_abort("Recruitment phase identifiers must be nonempty strings.")
  }
  schedule$phase <- enc2utf8(schedule$phase)
  dates <- schedule$date
  if (inherits(dates, "Date")) {
    if (anyNA(dates) || any(!is.finite(as.numeric(dates))) ||
      any(as.numeric(dates) != trunc(as.numeric(dates)))) {
      cli::cli_abort("Recruitment dates must be finite whole-day dates.")
    }
    dates <- observation_date_iso(dates)
  }
  if (!is.character(dates) || anyNA(dates) ||
    any(!grepl("^[0-9]{4}-[0-9]{2}-[0-9]{2}$", dates))) {
    cli::cli_abort("Recruitment dates require full ISO dates (YYYY-MM-DD).")
  }
  # Share calendar validation with observations; recruitment configuration
  # does not contribute to the observation table or likelihood.
  calendar <- normalize_observation_timing(data.frame(
    timing = dates, type = "index", fleet = "recruitment"
  ))
  if (!setequal(unique(calendar$timing), years)) {
    cli::cli_abort("Recruitment schedules must cover every modeled year and no other years.")
  }
  if (anyDuplicated(calendar$date)) {
    cli::cli_abort("Duplicate recruitment dates within a population are unsupported.")
  }
  if (anyDuplicated(data.frame(year = calendar$timing, phase = schedule$phase))) {
    cli::cli_abort("Each recruitment phase must occur once per year.")
  }
  phases <- split(schedule$phase, calendar$timing)
  if (!all(vapply(phases, setequal, logical(1), y = phases[[1]]))) {
    cli::cli_abort("Recruitment phase identifiers must be consistent across modeled years.")
  }
  if (!is.numeric(schedule$entry_age) || anyNA(schedule$entry_age) ||
    any(!is.finite(schedule$entry_age) | schedule$entry_age < 0)) {
    cli::cli_abort("Recruitment entry_age must be finite and nonnegative.")
  }
  if (any(vapply(split(schedule$entry_age, schedule$phase),
    function(x) length(unique(x)) != 1L, logical(1)))) {
    cli::cli_abort("Recruitment entry_age must be constant within each phase across years.")
  }
  fractions <- schedule$fraction
  if (!is.numeric(fractions) || anyNA(fractions) ||
    any(!is.finite(fractions) | fractions < 0)) {
    cli::cli_abort("Recruitment fractions must be finite nonnegative numbers.")
  }
  totals <- tapply(fractions, calendar$timing, sum)
  if (any(abs(totals - 1) > 1e-8)) {
    cli::cli_abort("Recruitment fractions must sum to one in each modeled year.")
  }
  schedule$phase_i <- match(schedule$phase, sort(unique(schedule$phase), method = "radix"))
  schedule$date <- calendar$date
  schedule$timing <- calendar$timing
  schedule$year_i <- calendar$timing - years[1] + 1L
  schedule$day <- as.integer(calendar$date - as.Date(sprintf("%04d-01-01", years[1])))
  leap <- calendar$timing %% 4 == 0 &
    (calendar$timing %% 100 != 0 | calendar$timing %% 400 == 0)
  schedule$year_fraction <- as.numeric(calendar$date -
    as.Date(sprintf("%04d-01-01", calendar$timing))) / ifelse(leap, 366, 365)
  result <- tibble::as_tibble(schedule[order(schedule$date), ])
  if (!is.null(reference) && !isTRUE(all.equal(result, reference, check.attributes = FALSE))) {
    cli::cli_abort("Recruitment schedule conflicts with recruitment_fraction rows in `data`; supply one consistent schedule.")
  }
  result
}


# Recruitment configuration uses the ordinary long input table but never
# contributes observations, fleet dimensions, or likelihood terms.
recruitment_schedule_from_data <- function(data) {
  rows <- data[data$type %in% "recruitment_fraction", , drop = FALSE]
  if (!nrow(rows)) return(NULL)
  if (!"phase" %in% names(rows))
    cli::cli_abort("recruitment_fraction rows require a `phase` column.")
  for (column in intersect(c("fleet", "age", "length", "uncertainty"), names(rows))) {
    if (any(!is.na(rows[[column]])))
      cli::cli_abort("recruitment_fraction rows must leave `{column}` missing; use `entry_age` for biological entry age.")
  }
  if (any(!is.na(rows$unit) & rows$unit != "proportion"))
    cli::cli_abort("recruitment_fraction rows require unit = proportion (or missing).")
  rows <- normalize_observation_timing(rows)
  if (anyNA(rows$date) || any(rows$input_precision != "day"))
    cli::cli_abort("Recruitment timing requires full ISO dates (YYYY-MM-DD) or whole-day Date values.")
  population <- if ("population" %in% names(rows)) rows$population else rep(NA_character_, nrow(rows))
  population[is.na(population)] <- "population1"
  schedule <- data.frame(population = population, phase = rows$phase,
                         date = rows$date, fraction = rows$observed)
  if ("entry_age" %in% names(rows)) schedule$entry_age <- rows$entry_age
  schedule
}

append_recruitment_input <- function(frame, rows) {
  schedule <- setup_recruitment_schedule(frame, recruitment_schedule_from_data(rows))
  rows <- normalize_observation_timing(rows)
  rows <- rows[match(schedule$date, rows$date), , drop = FALSE]
  rows$population <- schedule$population
  rows$phase <- schedule$phase
  rows$entry_age <- schedule$entry_age
  rows$unit <- "proportion"
  frame@data <- dplyr::bind_rows(frame@data, tibble::as_tibble(rows)) |>
    dplyr::arrange(.data$fleet, .data$type, .data$timing, .data$date)
  methods::validObject(frame)
  frame
}
