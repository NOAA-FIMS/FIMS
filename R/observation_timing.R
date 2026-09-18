# Fixed observation timing. Calendar calculations stay outside the AD tape.
observation_types <- function() c("catch", "index", "age_comp", "length_comp")

normalize_observation_timing <- function(data) {
  x <- data$timing
  if (inherits(x, "POSIXt") || is.factor(x) ||
    !(inherits(x, "Date") || is.numeric(x) || is.character(x))) {
    cli::cli_abort("`timing` must contain integer years, ISO dates, or Date values.")
  }
  if (inherits(x, "Date") && any(!is.na(x) & as.numeric(x) != trunc(as.numeric(x)))) {
    cli::cli_abort("Observation dates must have whole-day resolution.")
  }
  if (is.numeric(x) && !inherits(x, "Date")) {
    if (any(!is.na(x) & (!is.finite(x) | x != trunc(x)))) {
      cli::cli_abort("`timing` requires integer years; fractional years are unsupported.")
    }
    if (any(!is.na(x) & (x < 1 | x > 9999))) {
      cli::cli_abort("Supported timing years are 1 through 9999.")
    }
    raw <- ifelse(is.na(x), NA_character_, sprintf("%04d", as.integer(x)))
  } else {
    raw <- if (inherits(x, "Date")) format(x, "%Y-%m-%d") else as.character(x)
  }
  # A normalized data frame can be passed back through FIMSFrame, e.g. a peel.
  if (all(c("date", "input_precision") %in% names(data))) {
    if (!inherits(data$date, "Date")) cli::cli_abort("Normalized `date` must be an R Date vector.")
    keep <- !is.na(data$date)
    input_year <- as.integer(substr(raw, 1, 4))
    if (any(keep & !is.na(input_year) & input_year != as.integer(format(data$date, "%Y")))) {
      cli::cli_abort("Normalized `date` and `timing` disagree; update both when changing the model year.")
    }
    raw[keep] <- format(data$date[keep], "%Y-%m-%d")
    precision <- data$input_precision
    precision[is.na(precision)] <- "year"
    if (any(!precision %in% c("year", "month", "day"))) {
      cli::cli_abort("Unknown timing input precision.")
    }
    if (any(keep & ((precision == "year" & format(data$date, "%m-%d") != "01-01") |
      (precision == "month" & format(data$date, "%d") != "01")))) {
      cli::cli_abort("Normalized date does not match its input precision.")
    }
  } else {
    precision <- ifelse(nchar(raw) == 4, "year",
      ifelse(nchar(raw) == 7, "month", "day")
    )
  }
  valid <- is.na(raw) | grepl("^[0-9]{4}(-[0-9]{2}(-[0-9]{2})?)?$", raw)
  if (any(!valid)) cli::cli_abort("`timing` requires unambiguous ISO dates (YYYY, YYYY-MM, YYYY-MM-DD).")
  full <- ifelse(nchar(raw) == 4, paste0(raw, "-01-01"),
    ifelse(nchar(raw) == 7, paste0(raw, "-01"), raw)
  )
  date <- as.Date(full, format = "%Y-%m-%d")
  year <- as.integer(substr(full, 1, 4))
  invalid <- !is.na(raw) & (is.na(date) | format(date, "%Y-%m-%d") != full |
    year < 1L | year > 9999L)
  if (any(invalid)) cli::cli_abort("Invalid calendar date in timing at rows {which(invalid)}.")
  obs <- data$type %in% observation_types()
  if (any(obs & is.na(date))) cli::cli_abort("Observation timing is missing at rows {which(obs & is.na(date))}.")
  for (field in intersect(c("population", "partition"), names(data))) {
    allowed <- if (field == "partition") "pooled" else "population1"
    if (any(!is.na(data[[field]]) & data[[field]] != allowed)) {
      cli::cli_abort("Timing currently supports only {field} = {allowed}.")
    }
  }
  fishery <- unique(data$fleet[data$type == "catch"])
  interval <- data$type == "catch" |
    (data$type %in% c("age_comp", "length_comp") & data$fleet %in% fishery)
  if (any(interval & precision != "year", na.rm = TRUE)) {
    cli::cli_abort("Catch and fishery compositions require annual support (year-only timing); dated fishery samples are unsupported.")
  }
  if (any(!obs & precision != "year", na.rm = TRUE)) {
    cli::cli_abort("Biological input tables currently require year-only timing.")
  }
  data$timing <- year
  data$date <- date
  data$input_precision <- precision
  data$prediction_basis <- ifelse(obs, ifelse(interval, "catch", "survey"), NA_character_)
  data$support <- ifelse(obs, ifelse(interval, "interval", "point"), NA_character_)
  data$interval_end <- as.Date(rep(NA_character_, nrow(data)))
  data$interval_end[interval] <- as.Date(sprintf("%04d-01-01", year[interval] + 1L))
  data
}

# Verify samples before missing annual records are padded. Bin rows share a key.
validate_observation_samples <- function(data) {
  for (entry in list(c("age_comp", "age"), c("length_comp", "length"))) {
    if (any(data$type == entry[1]) && !entry[2] %in% names(data)) {
      cli::cli_abort("{entry[2]} is a required column for {entry[1]} observations.")
    }
    if (entry[2] %in% names(data) && any(data$type == entry[1] & is.na(data[[entry[2]]]))) {
      cli::cli_abort("Missing {entry[2]} bin coordinates in {entry[1]} observations.")
    }
  }
  obs <- which(data$type %in% observation_types())
  keys <- data[obs, c("fleet", "type", "date", "prediction_basis")]
  bin_keys <- keys
  for (bin in intersect(c("age", "length"), names(data))) {
    bin_keys[[bin]] <- ifelse(data$type[obs] == paste0(bin, "_comp"), data[[bin]][obs], NA)
  }
  duplicate <- duplicated(bin_keys) | duplicated(bin_keys, fromLast = TRUE)
  if ("sample_id" %in% names(data)) {
    groups <- split(seq_along(obs), observation_key(keys))
    for (g in groups) {
      if (length(unique(na.omit(data$sample_id[obs[g]]))) > 1L) duplicate[g] <- TRUE
    }
  }
  if (any(duplicate)) {
    rows <- obs[duplicate]
    detail <- paste(unique(paste(data$fleet[rows], data$type[rows], data$date[rows])), collapse = "; ")
    cli::cli_abort("Duplicate observation or composition bin in pooled partition: {detail}. Input rows: {rows}.")
  }
}

observation_key <- function(data) {
  # Length-prefixing avoids ambiguous keys when fleet names contain separators.
  parts <- lapply(data, function(x) paste0(nchar(as.character(x)), ":", x))
  do.call(paste, c(parts, sep = "|"))
}

# One row per prediction sample, in the same order as model_* data vectors.
observation_table <- function(data, fleet = NULL, type = NULL) {
  if (!methods::is(data, "FIMSFrame")) data <- FIMSFrame(data)
  d <- get_data(data)
  d <- d[d$type %in% observation_types(), , drop = FALSE]
  if (!is.null(fleet)) d <- d[d$fleet %in% fleet, , drop = FALSE]
  if (!is.null(type)) d <- d[d$type %in% type, , drop = FALSE]
  cols <- c("fleet", "type", "timing", "date", "input_precision", "prediction_basis", "support", "interval_end")
  if (!nrow(d)) {
    d <- d[, cols]
    d$observation_id <- d$partition <- d$population <- character()
    d$year_fraction <- numeric()
    d$year_i <- d$day <- d$time_id <- d$sample_i <- integer()
    return(tibble::as_tibble(d))
  }
  d <- d[, cols] |>
    dplyr::group_by(.data$fleet, .data$type, .data$date, .data$prediction_basis) |>
    dplyr::mutate(input_precision = c("year", "month", "day")[[
      max(match(.data$input_precision, c("year", "month", "day")))
    ]]) |>
    dplyr::ungroup() |>
    dplyr::distinct()
  d$observation_id <- observation_key(d[c("fleet", "type", "date", "prediction_basis")])
  d$partition <- "pooled"
  d$population <- "population1"
  start <- get_start_year(data)
  leap <- d$timing %% 4 == 0 & (d$timing %% 100 != 0 | d$timing %% 400 == 0)
  d$year_fraction <- as.numeric(d$date - as.Date(sprintf("%04d-01-01", d$timing))) / ifelse(leap, 366, 365)
  d$year_i <- d$timing - start + 1L
  d$day <- as.integer(d$date - as.Date(sprintf("%04d-01-01", start)))
  all_dates <- get_data(data)$date
  d$time_id <- match(d$date, sort(unique(all_dates))) - 1L
  d <- dplyr::group_by(d, .data$fleet, .data$type) |>
    dplyr::mutate(sample_i = dplyr::row_number()) |>
    dplyr::ungroup()
  tibble::as_tibble(d)
}

#' Get observation sample coordinates
#'
#' Returns one row per sample, including normalized dates, annual support,
#' stable observation IDs, and sample indices within each fleet/data stream.
#' Composition bins belong to one sample. Missing annual samples inserted by
#' FIMSFrame are included, so rows align with backend prediction vectors.
#'
#' @param x A FIMSFrame or a data frame accepted by FIMSFrame.
#' @return A tibble of observation coordinates. `year_i` and `sample_i` are
#'   one-based; `time_id` and `day` are zero-based. `timing` is the model year.
#'   `year_fraction` is elapsed days divided by the calendar year's length.
#' @export
get_observations <- function(x) {
  if (!methods::is(x, "FIMSFrame")) x <- FIMSFrame(x)
  observation_table(x)
}

attach_observation_timing <- function(estimates, samples) {
  if (is.null(samples) || !nrow(samples)) {
    return(estimates)
  }
  labels <- list(
    index = c("index_expected", "log_index_expected"),
    age_comp = c("agecomp_expected", "agecomp_proportion"),
    length_comp = c("lengthcomp_expected", "lengthcomp_proportion"),
    catch = c("catch_expected", "log_catch_expected")
  )
  metadata <- dplyr::bind_rows(lapply(names(labels), function(type) {
    rows <- samples[samples$type == type, ]
    dplyr::bind_rows(lapply(labels[[type]], function(label) {
      rows$label <- label
      rows
    }))
  }))
  if (!"sample_i" %in% names(estimates)) estimates$sample_i <- NA_integer_
  # Annual catch keeps year dimensions; observation streams use sample ones.
  estimates$.sample_join <- ifelse(estimates$label %in% labels$catch,
    estimates$year_i, estimates$sample_i
  )
  metadata$.sample_join <- metadata$sample_i
  metadata <- dplyr::select(metadata, -dplyr::all_of(c("type", "sample_i")))
  out <- dplyr::left_join(estimates, metadata,
    by = c("fleet", "label", ".sample_join"), suffix = c("", "_sample")
  )
  out$year_i <- dplyr::coalesce(out$year_i_sample, out$year_i)
  out$prediction_timing <- ifelse(!is.na(out$observation_id),
    ifelse(out$support == "point", "observation_date", "annual_interval"), NA_character_
  )
  dplyr::select(out, -dplyr::all_of(c(".sample_join", "year_i_sample")))
}
