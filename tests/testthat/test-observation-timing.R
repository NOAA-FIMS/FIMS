# A short annual fishery with independent survey sample dates.
timing_fixture <- function() {
  d <- data_big |>
    dplyr::filter(is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4))
  d$timing <- ifelse(is.na(d$timing), NA_character_, sprintf("%04d", d$timing + 2026L))
  d
}

test_that("dates normalize without changing annual rate units or sample identity", {
  #' @description Date precision, leap dates, annual intervals, and round trips are preserved.
  d <- timing_fixture()
  i <- which(d$type == "index")[1]
  d$timing[i] <- "2028-02-29"
  f <- FIMSFrame(d)
  obs <- get_observations(f)
  leap <- obs[obs$type == "index" & obs$date == as.Date("2028-02-29"), ]
  expect_equal(leap$day, 365L + 59L)
  expect_equal(leap$year_i, 2L)
  expect_equal(leap$year_fraction, 59 / 366)
  expect_equal(leap$input_precision, "day")
  catch <- obs[obs$type == "catch", ]
  expect_true(all(catch$support == "interval"))
  expect_equal(as.integer(catch$interval_end - catch$date), c(365L, 366L, 365L))
  expect_equal(get_data(FIMSFrame(get_data(f))), get_data(f))
  expect_equal(get_observations(FIMSFrame(d[nrow(d):1, ])), obs)
  d$timing[i] <- "2027-07"
  expect_equal(get_observations(FIMSFrame(d))$date[
    get_observations(FIMSFrame(d))$input_precision == "month"
  ], as.Date("2027-07-01"))
})

test_that("invalid dates and duplicate samples fail with useful context", {
  #' @description Invalid dates, unsupported intervals/partitions, and duplicate bins fail early.
  d <- timing_fixture()
  i <- which(d$type == "index")[1]
  for (bad in c("2027-02-29", "2028-13-01", "2028-04-31", "0000-01-01")) {
    x <- d
    x$timing[i] <- bad
    expect_error(FIMSFrame(x), "Invalid calendar date")
  }
  x <- d
  x$timing[i] <- NA_character_
  expect_error(FIMSFrame(x), "Observation timing is missing")
  x <- d
  x$timing[i] <- "07/01/2027"
  expect_error(FIMSFrame(x), "ISO")
  x <- d
  x$timing[which(x$type == "catch")[1]] <- "2027-01-01"
  expect_error(FIMSFrame(x), "annual support")
  expect_error(FIMSFrame(dplyr::bind_rows(d, d[i, ])), "Duplicate observation.*pooled")
  bin <- which(d$type == "age_comp")[1]
  expect_error(FIMSFrame(dplyr::bind_rows(d, d[bin, ])), "composition bin")
  x <- d
  x$partition <- "female"
  expect_error(FIMSFrame(x), "only partition = pooled")
  expect_error(FIMSFrame(dplyr::mutate(data_big, timing = 1.5)), "fractional years")
})

test_that("Date input and independent composition dates are supported", {
  #' @description Date vectors work for surveys and bin completeness is checked per date.
  d <- timing_fixture()
  survey <- d[d$fleet == "survey1", ]
  survey$timing <- as.Date(paste0(survey$timing, "-07-01"))
  # Biological tables remain annual; use survey observations plus annual weights.
  survey <- survey[survey$type %in% c("index", "age_comp", "length_comp"), ]
  normalized <- normalize_observation_timing(survey)
  expect_true(all(normalized$input_precision == "day"))
  d$timing[d$fleet == "survey1" & d$type == "age_comp"] <-
    paste0(d$timing[d$fleet == "survey1" & d$type == "age_comp"], "-04-01")
  extra <- d[d$fleet == "survey1" & d$type == "age_comp" & d$timing == "2027-04-01", ]
  extra$timing <- "2027-09-01"
  f <- FIMSFrame(dplyr::bind_rows(d, extra))
  expect_equal(nrow(observation_table(f, "survey1", "age_comp")), 4L)
  expect_equal(length(model_age_comp(f, "survey1")), 4L * get_n_ages(f))
  expect_error(FIMSFrame(dplyr::bind_rows(d, extra[-1, ])), "missing age values")
})

test_that("multiple survey samples reach likelihoods and reports independently", {
  #' @description Sample counts, annual mappings, likelihood alignment, and persistent IDs agree end to end.
  d <- timing_fixture()
  index <- which(d$fleet == "survey1" & d$type == "index" & d$timing == "2027")
  d$timing[index] <- "2027-03-01"
  extra <- d[index, ]
  extra$timing <- "2027-09-01"
  extra$observed <- extra$observed * 1.2
  # Give age and length compositions different schedules and different counts.
  age <- d[d$fleet == "survey1" & d$type == "age_comp" & d$timing == "2027", ]
  age$timing <- "2027-03-01" # Shares the index date; length dates differ.
  len <- d[d$fleet == "survey1" & d$type == "length_comp" & d$timing == "2028", ]
  len$timing <- "2028-08-01"
  age2 <- age
  age2$timing <- "2028-06-01"
  f <- FIMSFrame(dplyr::bind_rows(d, extra, age, age2, len))
  parameters <- setup_default_parameters(data = f)
  # The legacy random-effect report joins by numeric value; use distinct
  # deviations so that separate issue does not obscure timing assertions.
  deviations <- parameters$module_name == "Recruitment" & parameters$label == "log_devs"
  deviations[is.na(deviations)] <- FALSE
  parameters$value[deviations] <- seq_len(sum(deviations)) * 0.01
  expect_no_warning(input <- initialize_fims(parameters, data = f))
  expect_true(all(attr(input, "observation_timing")$weight_timing == "annual_lookup"))
  expect_true(all(attr(input, "observation_timing")$length_mapping_timing == "fixed_annual"))
  on.exit(clear(), add = TRUE)
  fit <- fit_fims(input, optimize = FALSE, get_sd = FALSE)
  estimates <- get_estimates(fit)
  raw <- jsonlite::fromJSON(get_model_output(fit), simplifyVector = FALSE)
  raw_fleet <- Filter(function(x) identical(x$fleet, "survey1"), raw$fleets)[[1]]
  raw_index <- Filter(function(x) identical(x$type, "index"), raw_fleet$observation_timing)
  expect_equal(vapply(raw_index, `[[`, "", "observation_id"), observation_table(f, "survey1", "index")$observation_id)
  expect_equal(vapply(raw_index, `[[`, 0.0, "year_fraction"), observation_table(f, "survey1", "index")$year_fraction, tolerance = 1e-6)
  expect_equal(vapply(raw_index, `[[`, "", "date"), format(observation_table(f, "survey1", "index")$date, "%Y-%m-%d"))
  predictions <- estimates[estimates$fleet == "survey1" & estimates$label == "index_expected", ]
  expect_equal(nrow(predictions), 4L)
  expect_equal(predictions$year_i, c(1L, 1L, 2L, 3L))
  expect_equal(predictions$observed, model_index(f, "survey1"), tolerance = 1e-6)
  expect_equal(predictions$observation_id, observation_table(f, "survey1", "index")$observation_id)
  expect_gt(predictions$expected[1], predictions$expected[2])
  expect_true(all(predictions$prediction_timing == "observation_date"))
  for (stream in c("agecomp_expected", "lengthcomp_expected")) {
    rows <- estimates[estimates$fleet == "survey1" & estimates$label == stream, ]
    expect_equal(length(unique(rows$observation_id)), if (stream == "agecomp_expected") 5L else 4L)
    expect_false(anyNA(rows$date))
  }
  expect_true(all(is.finite(get_obj(fit)$gr(get_obj(fit)$par))))
  irregular_report <- get_report(fit)
  # Independent analytic predictions use annual selected abundance and total Z.
  report <- irregular_report
  fleet_i <- which(vapply(raw$fleets, function(x) x$fleet, "") == "survey1")
  na <- get_n_ages(f)
  for (stream in c("index", "age_comp", "length_comp")) {
    samples <- observation_table(f, "survey1", stream)
    for (i in seq_len(nrow(samples))) {
      cells <- (samples$year_i[i] - 1L) * na + seq_len(na)
      survival <- exp(-report$mortality_Z[[1]][cells] * samples$year_fraction[i])
      if (stream == "index") {
        expected <- sum(report$index_weight_at_age[[fleet_i]][cells] * survival)
        expect_equal(report$index_expected[[fleet_i]][i], expected, tolerance = 1e-10)
      } else {
        expected <- report$index_numbers_at_age[[fleet_i]][cells] * survival
        prefix <- if (stream == "age_comp") "agecomp" else "lengthcomp"
        if (stream == "length_comp") {
          conversion <- matrix(report$age_to_length_conversion[[fleet_i]], nrow = na, byrow = TRUE)
          expected <- as.numeric(expected %*% conversion)
        }
        expected <- expected / sum(expected)
        bins <- length(expected)
        expect_equal(report[[paste0(prefix, "_proportion")]][[fleet_i]][(i - 1L) * bins + seq_len(bins)], expected, tolerance = 1e-10)
      }
    }
  }
  # Re-evaluation at perturbed parameters exercises AD and cache invalidation.
  obj <- get_obj(fit)
  par <- obj$par
  initial_objective <- obj$fn(par)
  analytic <- obj$gr(par)
  h <- 1e-5
  selected <- unique(round(seq(1, length(par), length.out = min(8, length(par)))))
  numeric_gradient <- vapply(selected, function(j) {
    plus <- minus <- par
    plus[j] <- plus[j] + h
    minus[j] <- minus[j] - h
    (obj$fn(plus) - obj$fn(minus)) / (2 * h)
  }, 0.0)
  expect_equal(as.numeric(analytic[selected]), numeric_gradient, tolerance = 1e-4)
  expect_equal(obj$fn(par), initial_objective, tolerance = 1e-8)

  before_clear <- predictions$observation_id
  clear()
  after <- get_estimates(fit)
  expect_equal(after$observation_id[after$fleet == "survey1" & after$label == "index_expected"], before_clear)
  annual_input <- initialize_fims(parameters, data = FIMSFrame(timing_fixture()))
  annual_fit <- fit_fims(annual_input, optimize = FALSE, get_sd = FALSE)
  for (quantity in c("numbers_at_age", "catch_expected", "spawning_biomass")) {
    expect_equal(irregular_report[[quantity]], get_report(annual_fit)[[quantity]])
  }
})

test_that("Rcpp rejects invalid sample coordinate vectors", {
  #' @description Invalid stream lengths, ordering, and annual indices fail before model execution.
  on.exit(clear(), add = TRUE)
  fleet <- methods::new(Fleet)
  fleet$n_years$set(3L)
  set_timing <- function(years, days, ids = seq_along(years) - 1L, fractions = rep(0, length(years))) {
    fleet$SetObservationTiming(
      "index", as.integer(years), as.integer(days),
      as.integer(ids), paste0("sample", seq_along(years)),
      rep("2027-01-01", length(years)), fractions
    )
  }
  expect_error(set_timing(c(0L, 1L), 0L), "equal nonzero lengths")
  expect_error(set_timing(3L, 0L), "out of range")
  for (fraction in c(-0.1, 1, NA_real_, Inf)) {
    expect_error(set_timing(0L, 0L, fractions = fraction), "out of range")
  }
  expect_error(set_timing(c(0L, 0L), c(5L, 5L)), "strictly increasing")
  expect_error(set_timing(c(1L, 0L), c(5L, 6L)), "strictly increasing")
  expect_error(set_timing(NA_integer_, 0L), "out of range")
  expect_no_error(set_timing(c(0L, 0L, 2L), c(5L, 10L, 800L)))
})

test_that("fleet names and sample IDs survive JSON escaping", {
  #' @description Quotes and backslashes in fleet names remain valid report identifiers.
  d <- timing_fixture()
  name <- 'survey "spring"\\A'
  d$fleet[d$fleet == "survey1"] <- name
  f <- FIMSFrame(d)
  parameters <- setup_default_parameters(f)
  on.exit(clear(), add = TRUE)
  fit <- fit_fims(initialize_fims(parameters, f), optimize = FALSE, get_sd = FALSE)
  report <- jsonlite::fromJSON(get_model_output(fit), simplifyVector = FALSE)
  fleet <- Filter(function(x) identical(x$fleet, name), report$fleets)
  expect_length(fleet, 1L)
  ids <- vapply(fleet[[1]]$observation_timing, `[[`, "", "observation_id")
  expected <- get_observations(f)
  expect_setequal(ids, expected$observation_id[expected$fleet == name])
})


test_that("early model years work when the platform does not pad date years", {
  #' @description Emulate unpadded %Y output and preserve annual input round trips.
  iso <- observation_date_iso
  environment(iso) <- list2env(list(
    format = function(x, format = "", ...) {
      result <- base::format(x, format = format, ...)
      if (inherits(x, "Date") && grepl("%Y", format, fixed = TRUE)) {
        result <- sub("^0+([0-9])", "\\1", result)
      }
      result
    }
  ), parent = environment(iso))
  testthat::local_mocked_bindings(observation_date_iso = iso, .package = "FIMS")
  dates <- as.Date(c("0001-01-01", "0004-02-29", "0099-12-31",
    "0100-01-01", "0999-01-01", "1000-01-01", NA))
  expect_equal(observation_date_iso(dates), c("0001-01-01", "0004-02-29",
    "0099-12-31", "0100-01-01", "0999-01-01", "1000-01-01", NA))
  f <- FIMSFrame(data_big)
  expect_equal(get_data(FIMSFrame(get_data(f))), get_data(f))
  obs <- get_observations(f)
  expect_true(any(grepl("0001-01-01", obs$observation_id, fixed = TRUE)))
  survey <- data.frame(type = "index", fleet = "survey1", timing = dates[1:3])
  normalized <- normalize_observation_timing(survey)
  expect_equal(normalized$timing, c(1L, 4L, 99L))
  expect_equal(normalized$date, dates[1:3])
  survey$timing <- c("0001-02-29", "0004-02-29", "0099-12-31")
  expect_error(normalize_observation_timing(survey), "Invalid calendar date")
})
