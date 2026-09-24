# A small fixed-input model. The oracle below never reads FIMS predictions,
# reported mortality, recruitment, or cohort state to construct expected values.
analytic_phase_case <- function(early_fraction = 0.4, extract = FALSE) {
  years <- 2027:2029
  dates <- as.Date(paste0(rep(years, each = 7),
    rep(c("-01-01", "-01-02", "-04-01", "-06-30", "-07-01", "-07-02", "-12-31"), 3)))
  data <- data_big |>
    dplyr::filter(type %in% c("catch", "index", "age_comp", "weight_at_age"),
                  is.na(age) | age <= 3,
                  timing <= 3 | (type == "weight_at_age" & timing == 4))
  data$timing <- sprintf("%04d", data$timing + 2026L)
  data$observed[data$type == "weight_at_age"] <- data$age[data$type == "weight_at_age"]
  data$observed[data$type == "age_comp"] <- 1 / 3
  survey <- data[data$fleet == "survey1" & data$timing == "2027", ]
  data <- data[data$fleet != "survey1", ]
  for (date in as.character(dates)) {
    sample <- survey
    sample$timing <- date
    if (early_fraction == 0 && substr(date, 6, 10) < "07-01") {
      # Keep the dummy observations possible when no age-1 fish have entered.
      composition <- which(sample$type == "age_comp")
      sample$observed[composition] <- ifelse(sample$age[composition] == 1, 0, 0.5)
    }
    data <- rbind(data, sample)
  }
  data <- FIMSFrame(data)
  parameters <- setup_default_parameters(data)
  parameters$estimation_type <- "constant"
  parameters$distribution[parameters$module_name == "Recruitment"] <- NA_character_
  parameters$distribution_type[parameters$module_name == "Recruitment"] <- NA_character_
  parameters$value[which(parameters$label == "log_init_naa")] <- log(c(1000, 200, 100))
  parameters$value[which(parameters$label == "log_M")] <- log(0.2)
  parameters$value[which(parameters$module_name == "Selectivity" & parameters$label == "slope")] <- 0
  parameters$value[which(parameters$fleet == "fleet1" & parameters$label == "log_Fmort")] <- log(c(0.3, 0.4, 0.2))
  parameters$value[which(parameters$fleet == "survey1" & parameters$label == "log_Fmort")] <- -1000
  parameters$value[which(parameters$fleet == "survey1" & parameters$label == "log_q")] <- log(0.4)
  parameters$estimation_type[which(parameters$fleet == "survey1" & parameters$label == "log_q")] <- "fixed_effects"
  recruitment <- which(parameters$label == "log_devs")
  parameters$label[recruitment] <- "log_r"
  parameters$value[recruitment] <- log(c(1200, 1400))
  early <- setup_recruitment_schedule(data)
  early$phase <- "early"
  early$fraction <- early_fraction
  late <- early
  late$phase <- "late"
  late$fraction <- 1 - early_fraction
  late$date <- as.Date(paste0(years, "-07-01"))
  schedule <- setup_recruitment_schedule(data, rbind(early, late))
  obj <- NULL
  on.exit({
    if (!is.null(obj)) TMB::FreeADFun(obj)
    clear()
  }, add = TRUE)
  fit <- fit_fims(initialize_fims(parameters, data, schedule),
                  optimize = FALSE, get_sd = FALSE)
  obj <- get_obj(fit)
  list(report = get_report(fit), dates = dates,
       estimates = if (extract) get_estimates(fit) else NULL,
       output = get_model_output(fit))
}

analytic_phase_oracle <- function(early_fraction = 0.4) {
  # Calendar constants are explicit: July 1 is day 181 in 2027/2029 and
  # day 182 in leap-year 2028. Dates refer to the start of each day.
  year_days <- c(365, 366, 365)
  july_days <- c(181, 182, 181)
  recruitment <- c(1000, 1200, 1400)
  fishing <- c(0.3, 0.4, 0.2) * 0.5 # slope zero => selectivity = 1/2
  mortality <- 0.2 + fishing
  older <- c(200, 100)
  annual <- catches <- deaths <- survivors <- matrix(NA_real_, 3, 3)
  survey <- list()
  for (year in 1:3) {
    r <- recruitment[year]
    z <- mortality[year]
    tau <- july_days[year] / year_days[year]
    annual[year, ] <- c(r * early_fraction, older)
    # Integrate exp(-Z t) over each cohort's actual time in the population.
    exposure <- c(r * (early_fraction * -expm1(-z) +
      (1 - early_fraction) * -expm1(-z * (1 - tau))), older * -expm1(-z)) / z
    catches[year, ] <- fishing[year] * exposure
    deaths[year, ] <- 0.2 * exposure
    survivors[year, ] <- c(r * (early_fraction * exp(-z) +
      (1 - early_fraction) * exp(-z * (1 - tau))), older * exp(-z))
    times <- c(0, 1, if (year == 2) 91 else 90,
               july_days[year] + c(-1, 0, 1), year_days[year] - 1) / year_days[year]
    numbers <- t(vapply(times, function(time) {
      young <- r * early_fraction * exp(-z * time)
      if (time >= tau) young <- young + r * (1 - early_fraction) * exp(-z * (time - tau))
      c(young, older * exp(-z * time))
    }, numeric(3)))
    survey[[year]] <- list(numbers = numbers, index = as.numeric(numbers %*% (1:3)) * 0.4 * 0.5)
    older <- c(survivors[year, 1], sum(survivors[year, 2:3]))
  }
  list(annual = annual, catches = catches, deaths = deaths, survivors = survivors,
       survey = survey, terminal_older = older, recruitment = recruitment,
       july = july_days / year_days, mortality = mortality)
}

test_that("fixed recruitment agrees with independent survival and catch equations", {
  #' @description Annual abundance, plus-group aging, partial-year catch, and recruitment allocation obey an input-only analytic oracle in ordinary and leap years.
  for (fraction in c(0.4, 1, 0)) {
    actual <- analytic_phase_case(fraction)
    expected <- analytic_phase_oracle(fraction)
    report <- actual$report
    annual <- matrix(report$numbers_at_age[[1]], ncol = 3, byrow = TRUE)
    catch <- matrix(report$catch_numbers_at_age[[1]], ncol = 3, byrow = TRUE)
    expect_equal(annual[1:3, ], expected$annual, tolerance = 1e-10)
    expect_equal(annual[4, 2:3], expected$terminal_older, tolerance = 1e-10)
    expect_equal(catch, expected$catches, tolerance = 1e-10)
    expect_equal(as.numeric(report$catch_expected[[1]]),
                 as.numeric(expected$catches %*% (1:3)), tolerance = 1e-10)
    events <- report$recruitment_events
    for (year in 1:3) {
      rows <- events[events[, "year_i"] == year, , drop = FALSE]
      expect_equal(nrow(rows), 2L)
      expect_equal(unname(rows[, "recruits"]),
                   expected$recruitment[year] * c(fraction, 1 - fraction), tolerance = 1e-10)
      expect_equal(sum(rows[, "recruits"]), expected$recruitment[year], tolerance = 1e-10)
      expect_equal(unname(rows[, "year_fraction"]), c(0, expected$july[year]))
      # No fish disappear at aging: the next year's older classes retain
      # recruits + starting older fish minus fishery catch and natural deaths.
      budget <- expected$recruitment[year] + sum(expected$annual[year, 2:3])
      expect_equal(sum(annual[year + 1, 2:3]) + sum(catch[year, ]) +
                     sum(expected$deaths[year, ]), budget, tolerance = 1e-10)
    }
  }
})

test_that("surveys include pulses exactly on entry dates and never before", {
  #' @description January/July boundaries, April exclusion of July recruits, daily survival, and age proportions follow independent abundance predictions across a leap year and zero-weight phases.
  for (fraction in c(0.4, 1, 0)) {
    actual <- analytic_phase_case(fraction)
    expected <- analytic_phase_oracle(fraction)
    index <- as.numeric(actual$report$index_expected[[2]])
    expect_length(index, 21L)
    expect_equal(index, unlist(lapply(expected$survey, `[[`, "index")), tolerance = 1e-10)
    proportions <- matrix(actual$report$agecomp_proportion[[2]], ncol = 3, byrow = TRUE)
    numbers <- do.call(rbind, lapply(expected$survey, `[[`, "numbers"))
    expect_equal(proportions, numbers / rowSums(numbers), tolerance = 1e-10)
    for (year in 1:3) {
      values <- index[(year - 1) * 7 + 1:7]
      days <- c(365, 366, 365)[year]
      daily_survival <- exp(-expected$mortality[year] / days)
      # June 30 -> July 1: surviving existing biomass plus one new pulse.
      expect_equal(values[5] - values[4] * daily_survival,
                   expected$recruitment[year] * (1 - fraction) * 0.4 * 0.5,
                   tolerance = 1e-10)
      # July 1 -> July 2: survival only; the pulse is not added a second time.
      expect_equal(values[6], values[5] * daily_survival, tolerance = 1e-10)
      if (fraction == 0) {
        # Before July, the youngest class is empty, including January and April.
        expect_equal(proportions[(year - 1) * 7 + 1:4, 1], rep(0, 4))
      }
    }
  }
})


test_that("zero survey mortality leaves model output valid and extractable", {
  #' @description A non-extractive survey has zero catch and log catch of negative infinity; JSON uses the existing -999 sentinel without corrupting finite predictions.
  actual <- analytic_phase_case(extract = TRUE)
  expect_true(jsonlite::validate(actual$output))
  expect_equal(as.numeric(actual$report$catch_expected[[2]]), rep(0, 3))
  expect_equal(as.numeric(actual$report$log_catch_expected[[2]]), rep(-Inf, 3))
  log_catch <- actual$estimates |>
    dplyr::filter(fleet == "survey1", label == "log_catch_expected")
  expect_equal(log_catch$estimated, rep(-999, 3))
  index <- actual$estimates |>
    dplyr::filter(fleet == "survey1", label == "index_expected") |>
    dplyr::arrange(date)
  expected <- analytic_phase_oracle()
  expect_equal(index$estimated, unlist(lapply(expected$survey, `[[`, "index")),
               tolerance = 1e-9) # JSON currently prints ten decimal places.
})
