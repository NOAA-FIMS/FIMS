#' Prepare a fixed recruitment phase schedule
#'
#' Validate dates and fractions that split one annual recruitment total among
#' phases. Pass the result to initialize_fims(recruitment_schedule = ...) to
#' enable phase-specific biological ages and partial-year mortality and catch.
#'
#' @param data A FIMSFrame defining the modeled years.
#' @param schedule NULL for one January 1 phase per year, or a data frame with
#'   population, phase, date, and fraction columns. Dates must be full ISO
#'   YYYY-MM-DD strings or whole-day Date values. Fractions must be finite,
#'   nonnegative, and sum to one within 1e-8 in each year. Accepted fractions
#'   are retained without renormalization. Optional entry_age specifies the
#'   biological age at entry (default: the youngest modeled age); it must be
#'   finite, nonnegative, and constant across years within each phase.
#' @return A tibble sorted by date, containing the input columns plus timing,
#'   entry_age, phase_i, year_i, day, and year_fraction. Only modeled years are included; the extra
#'   terminal reporting year is excluded. Population must be population1.
#' @examples
#' frame <- FIMSFrame(data_big)
#' annual <- setup_recruitment_schedule(frame)
#' early <- annual
#' early$phase <- "early"
#' early$fraction <- 0.4
#' late <- annual
#' late$phase <- "late"
#' late$fraction <- 0.6
#' late$date <- as.Date(sprintf("%04d-07-01", late$timing))
#' phases <- setup_recruitment_schedule(frame, rbind(early, late))
#' head(phases)
#' @export
setup_recruitment_schedule <- function(data, schedule = NULL) {
  if (!methods::is(data, "FIMSFrame")) {
    cli::cli_abort("`data` must be a FIMSFrame.")
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
    cli::cli_abort("`schedule` requires population, phase, date, and fraction columns.")
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
  # Share calendar validation with observations, without making recruitment
  # configuration part of the observation table or likelihood.
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
  tibble::as_tibble(schedule[order(schedule$date), ])
}
