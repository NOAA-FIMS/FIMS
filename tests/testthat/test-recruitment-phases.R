phase_fixture <- function(parametric = FALSE) {
  d <- data_big |>
    dplyr::filter(is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4))
  d$timing <- ifelse(is.na(d$timing), NA_character_, sprintf("%04d", d$timing + 2026L))
  survey <- d[d$fleet == "survey1" & d$type %in% c("index", "age_comp", "length_comp") & d$timing == "2027", ]
  for (date in c("2027-06-30", "2027-07-01", "2027-12-31")) {
    extra <- survey
    extra$timing <- date
    d <- rbind(d, extra)
  }
  f <- FIMSFrame(d)
  p <- setup_default_parameters(f)
  if (parametric) {
    p <- dplyr::bind_rows(
      dplyr::filter(p, module_name != "Growth"),
      setup_default_Growth(f, module_type = "VonBertalanffySchnute")
    )
  }
  p$estimation_type <- "constant"
  p$distribution[p$module_name == "Recruitment"] <- NA_character_
  p$distribution_type[p$module_name == "Recruitment"] <- NA_character_
  p$estimation_type[p$module_name == "Fleet" & p$fleet == "survey1" & p$label == "log_q"] <- "fixed_effects"
  early <- setup_recruitment_schedule(f)
  early$phase <- "early"
  early$fraction <- 0.4
  late <- early
  late$phase <- "late"
  late$fraction <- 0.6
  late$date <- as.Date(sprintf("%04d-07-01", late$timing))
  list(data = f, parameters = p, schedule = setup_recruitment_schedule(f, rbind(early, late)))
}

phase_fit <- function(x) {
  fit_fims(initialize_fims(x$parameters, x$data, x$schedule),
    optimize = FALSE, get_sd = FALSE
  )
}

test_that("phases obey analytic abundance and partial-year catch", {
  #' @description Two pulses enter once, experience only post-entry mortality, and retain distinct ages after annual aging.
  x <- phase_fixture()
  on.exit(clear(), add = TRUE)
  fit <- phase_fit(x)
  report <- get_report(fit)
  nages <- get_n_ages(x$data)
  initial <- exp(x$parameters$value[which(x$parameters$label == "log_init_naa")])
  tau <- 181 / 365
  z <- report$mortality_Z[[1]][seq_len(nages)]
  f <- report$mortality_F[[1]][seq_len(nages)]
  expect_equal(report$numbers_at_age[[1]][1], 0.4 * initial[1])
  expect_equal(
    report$numbers_at_age[[1]][nages + 2],
    initial[1] * (0.4 * exp(-z[1]) + 0.6 * exp(-z[1] * (1 - tau)))
  )
  expect_equal(report$catch_numbers_at_age[[1]][1], initial[1] * f[1] / z[1] *
    (0.4 * (1 - exp(-z[1])) + 0.6 * (1 - exp(-z[1] * (1 - tau)))))
  events <- report$recruitment_events
  expect_equal(sum(events[events[, "year_i"] == 1, "recruits"]), initial[1])
  expect_equal(events[events[, "year_i"] == 1, "entry_age"], c(1, 1))
  expect_equal(unname(events[events[, "year_i"] == 4, "year_fraction"]), 0)
  cohorts <- report$recruitment_cohorts
  y2 <- cohorts[cohorts[, "year_i"] == 2 & cohorts[, "age_i"] == 2, ]
  expect_equal(y2[, "biological_age"], c(2, 2 - tau))
  expect_equal(y2[, "available_from"], c(0, 0))
  ages <- get_ages(x$data)
  inflection <- x$parameters$value[which(x$parameters$fleet == "survey1" & x$parameters$label == "inflection_point")]
  slope <- x$parameters$value[which(x$parameters$fleet == "survey1" & x$parameters$label == "slope")]
  q <- exp(x$parameters$value[which(x$parameters$fleet == "survey1" & x$parameters$label == "log_q")])
  selected <- q * plogis(slope * (ages - inflection))
  weights <- get_data(x$data) |>
    dplyr::filter(type == "weight_at_age", timing == 2027) |>
    dplyr::arrange(age) |>
    dplyr::pull(observed)
  estimates <- get_estimates(fit) |>
    dplyr::filter(fleet == "survey1", label == "index_expected", timing == 2027)
  for (i in seq_len(nrow(estimates))) {
    t <- estimates$year_fraction[i]
    numbers <- initial * exp(-z * t)
    numbers[1] <- initial[1] * 0.4 * exp(-z[1] * t)
    if (t >= tau) numbers[1] <- numbers[1] + initial[1] * 0.6 * exp(-z[1] * (t - tau))
    expect_equal(estimates$estimated[i], sum(selected * numbers * weights), tolerance = 1e-8)
  }
  expect_true(all(is.finite(get_obj(fit)$gr(get_obj(fit)$par))))
})

test_that("phase biology affects length predictions and preserves catch totals", {
  #' @description Parametric phases conserve composition mass, have finite gradients, and do not depend on survey frequency.
  x <- phase_fixture(TRUE)
  on.exit(clear(), add = TRUE)
  fit <- phase_fit(x)
  baseline <- get_report(fit)
  obj <- get_obj(fit)
  gradient <- obj$gr(obj$par)
  eps <- 1e-5
  expect_equal(as.numeric(gradient),
    (obj$fn(obj$par + eps) - obj$fn(obj$par - eps)) / (2 * eps),
    tolerance = 1e-4
  )
  expect_equal(sum(baseline$catch_numbers_at_length[[1]]), sum(baseline$catch_numbers_at_age[[1]]), tolerance = 1e-10)
  lengths <- matrix(baseline$lengthcomp_proportion[[2]], ncol = get_n_lengths(x$data), byrow = TRUE)
  expect_equal(rowSums(lengths), rep(1, nrow(lengths)), tolerance = 1e-10)
  expect_false(isTRUE(all.equal(lengths[1, ], lengths[3, ])))
  d <- get_data(x$data)
  extra <- d[d$type == "index" & d$fleet == "survey1" & d$date == as.Date("2027-07-01"), ]
  extra$date <- as.Date("2027-09-01")
  extra$input_precision <- "day"
  extra$observed <- -999
  x$data <- FIMSFrame(rbind(d, extra))
  x$schedule <- x$schedule[nrow(x$schedule):1, ]
  added <- get_report(phase_fit(x))
  for (name in c("numbers_at_age", "spawning_biomass", "catch_numbers_at_age", "catch_weight_at_age", "catch_numbers_at_length")) {
    expect_equal(added[[name]], baseline[[name]], tolerance = 1e-12)
  }
})

test_that("explicit entry ages and zero phases preserve the chosen semantics", {
  #' @description Phase entry ages are independent of class labels and a zero phase does not add recruits.
  x <- phase_fixture()
  on.exit(clear(), add = TRUE)
  x$schedule$entry_age[x$schedule$phase == "late"] <- 0.5
  x$schedule$fraction <- ifelse(x$schedule$phase == "early", 1, 0)
  report <- get_report(phase_fit(x))
  events <- report$recruitment_events
  expect_true(all(events[events[, "phase_i"] == 2, "recruits"] == 0))
  expect_true(all(events[events[, "phase_i"] == 2, "entry_age"] == 0.5))
  expect_true(all(is.finite(report$spawning_biomass[[1]])))
})

test_that("phase recruitment supports log-R and unfished consistency", {
  #' @description Fixed annual log-R budgets are allocated once and zero-fishing dynamics match the unfished path.
  x <- phase_fixture()
  on.exit(clear(), add = TRUE)
  i <- which(x$parameters$label == "log_devs")
  x$parameters$label[i] <- "log_r"
  x$parameters$value[i] <- log(c(2e6, 3e6))
  report <- get_report(phase_fit(x))
  expect_equal(report$expected_recruitment[[1]][2:3], c(2e6, 3e6))
  events <- report$recruitment_events
  expect_equal(sum(events[events[, "year_i"] == 2, "recruits"]), 2e6)
  x <- phase_fixture()
  x$parameters$value[which(x$parameters$label == "log_Fmort")] <- -1000
  missing_fishery <- get_data(x$data)
  missing_fishery$observed[missing_fishery$fleet == "fleet1" &
    missing_fishery$type %in% c("catch", "age_comp", "length_comp")] <- -999
  x$data <- FIMSFrame(missing_fishery)
  first <- get_report(phase_fit(x))
  a <- get_n_ages(x$data)
  values <- first$unfished_numbers_at_age[[1]][seq_len(a)]
  values[1] <- exp(x$parameters$value[which(x$parameters$label == "log_rzero")])
  x$parameters$value[which(x$parameters$label == "log_init_naa")] <- log(values)
  report <- get_report(phase_fit(x))
  expect_equal(report$numbers_at_age[[1]][seq_len(3 * a)], report$unfished_numbers_at_age[[1]][seq_len(3 * a)], tolerance = 1e-10)
  expect_equal(report$spawning_biomass[[1]][1:3], report$unfished_spawning_biomass[[1]][1:3], tolerance = 1e-10)
  expect_equal(report$total_catch_numbers[[1]], rep(0, 3))
})

test_that("phase gradients include mortality, recruitment, and growth", {
  #' @description Automatic derivatives of phase survival and biological integration agree with independent differences.
  x <- phase_fixture(TRUE)
  on.exit(clear(), add = TRUE)
  labels <- c("log_M", "log_rzero", "mean_length_young", "growth_coefficient")
  x$parameters$estimation_type[which(x$parameters$label %in% labels)] <- "fixed_effects"
  fit <- phase_fit(x)
  obj <- get_obj(fit)
  analytic <- as.numeric(obj$gr(obj$par))
  for (i in seq_along(obj$par)) {
    plus <- minus <- obj$par
    h <- 1e-5 * max(1, abs(obj$par[i]))
    plus[i] <- plus[i] + h
    minus[i] <- minus[i] - h
    numeric <- (obj$fn(plus) - obj$fn(minus)) / (2 * h)
    expect_lt(abs(analytic[i] - numeric) / max(1, abs(numeric)), 1e-3)
  }
})

test_that("late-year entry respects leap boundaries and terminal reporting", {
  #' @description December 31 recruits survive only one day before aging and terminal reporting does not insert future pulses.
  x <- phase_fixture()
  on.exit(clear(), add = TRUE)
  x$schedule <- setup_recruitment_schedule(x$data)
  x$schedule$date <- as.Date(sprintf("%04d-12-31", x$schedule$timing))
  report <- get_report(phase_fit(x))
  a <- get_n_ages(x$data)
  r <- report$expected_recruitment[[1]]
  z <- report$mortality_Z[[1]]
  expect_equal(report$numbers_at_age[[1]][c(1, a + 1, 2 * a + 1, 3 * a + 1)], rep(0, 4))
  expect_equal(report$numbers_at_age[[1]][a + 2], r[1] * exp(-z[1] / 365))
  expect_equal(report$numbers_at_age[[1]][2 * a + 2], r[2] * exp(-z[a + 1] / 366))
  expect_false(any(report$recruitment_events[, "year_i"] == 4))
  expect_equal(report$recruitment_events[, "year_fraction"], c(364 / 365, 365 / 366, 364 / 365))
})

test_that("phase schedules survive fitting and retrospective refits", {
  #' @description Fitted inputs retain the complete phase schedule after clear and data peels do not reset recruitment to January 1.
  x <- phase_fixture()
  on.exit(clear(), add = TRUE)
  fit <- fit_fims(initialize_fims(x$parameters, x$data, x$schedule), number_of_loops = 1)
  expect_lt(get_max_gradient(fit), 1e-4)
  clear()
  expect_equal(attr(get_input(fit), "recruitment_schedule"), x$schedule)
  peeled <- run_modified_data_fims(1, x$data, x$parameters, recruitment_schedule = x$schedule)
  expect_equal(attr(get_input(peeled), "recruitment_schedule"), x$schedule)
  expect_equal(get_report(peeled)$recruitment_events[, "year_fraction"], get_report(fit)$recruitment_events[, "year_fraction"])
})

test_that("phase catch weight integrates biological age since entry", {
  #' @description Catch-weight quadrature agrees with adaptive integration of an independently calculated growth curve.
  x <- phase_fixture(TRUE)
  on.exit(clear(), add = TRUE)
  report <- get_report(phase_fit(x))
  value <- function(label) x$parameters$value[which(x$parameters$module_name == "Growth" & x$parameters$label == label)]
  young <- value("mean_length_young")
  old <- value("mean_length_old")
  k <- value("growth_coefficient")
  ay <- value("reference_age_for_length_young")
  ao <- value("reference_age_for_length_old")
  smooth_max <- function(a, b) (a + b + sqrt((a - b)^2 + 1e-5)) / 2
  denom <- smooth_max(sqrt((1 - exp(-k * (ao - ay)))^2 + 1e-5), 1e-8)
  weight <- function(age) {
    value("length_weight_a") *
      (young + (old - young) * (1 - exp(-k * (age - ay))) / denom)^value("length_weight_b")
  }
  initial <- exp(x$parameters$value[which(x$parameters$label == "log_init_naa")])[1]
  z <- report$mortality_Z[[1]][1]
  f <- report$mortality_F[[1]][1]
  integrand <- function(t) exp(-z * t) * weight(1 + t)
  expected <- initial * f * (0.4 * integrate(integrand, 0, 1, rel.tol = 1e-10)$value +
    0.6 * integrate(integrand, 0, 1 - 181 / 365, rel.tol = 1e-10)$value)
  expect_equal(report$catch_weight_at_age[[1]][1], expected, tolerance = 1e-8)
})

test_that("public diagnostics retain phase schedules", {
  #' @description Public retrospective and likelihood workflows retain phase-age prediction policies.
  x <- phase_fixture()
  on.exit(clear(), add = TRUE)
  fit <- fit_fims(initialize_fims(x$parameters, x$data, x$schedule), number_of_loops = 1)
  retro <- run_fims_retrospective(0:1, x$data, x$parameters,
    n_cores = 1,
    recruitment_schedule = x$schedule
  )
  rows <- retro$estimates |>
    dplyr::filter(fleet == "survey1", label == "index_expected")
  expect_gt(nrow(rows), 0)
  expect_true(all(rows$maturity_timing == "phase_age"))
  profile <- run_fims_likelihood(
    model = fit, parameters = x$parameters,
    data = get_data(x$data), module_name = "Recruitment", parameter_name = "log_rzero",
    n_cores = 1, min = -0.1, max = 0.1, length = 3
  )
  rows <- profile$estimates |>
    dplyr::filter(fleet == "survey1", label == "index_expected")
  expect_gt(nrow(rows), 0)
  expect_true(all(rows$maturity_timing == "phase_age"))
})

test_that("positive compositions cannot be explained by an empty population", {
  #' @description Zero predicted fish are allowed for missing samples but not silently assigned uniform probabilities for positive data.
  x <- phase_fixture()
  on.exit(clear(), add = TRUE)
  x$parameters$value[which(x$parameters$label == "log_init_naa")] <- -1000
  expect_error(phase_fit(x), "no predicted fish")
})

test_that("projections require extended schedules and ignore survey-only dates", {
  #' @description Future recruitment uses explicit phases, while added missing surveys do not alter the projected population or catch.
  x <- phase_fixture()
  on.exit(clear(), add = TRUE)
  baseline <- get_report(phase_fit(x))
  d <- get_data(x$data)
  catch <- d[d$type == "catch" & d$timing == 2029, ]
  catch$timing <- 2030L
  catch$date <- as.Date("2030-01-01")
  catch$observed <- -999
  weights <- d[d$type == "weight_at_age" & d$timing == 2030, ]
  weights$timing <- 2031L
  weights$date <- as.Date("2031-01-01")
  x$data <- FIMSFrame(rbind(d, catch, weights))
  expect_error(setup_recruitment_schedule(x$data, x$schedule), "every modeled year")
  extra <- x$schedule[x$schedule$timing == 2029, ]
  extra$date <- as.Date(paste0("2030-", format(extra$date, "%m-%d")))
  x$schedule <- setup_recruitment_schedule(x$data, rbind(x$schedule, extra))
  x$parameters <- setup_default_parameters(x$data)
  x$parameters$estimation_type <- "constant"
  x$parameters$distribution[x$parameters$module_name == "Recruitment"] <- NA_character_
  x$parameters$distribution_type[x$parameters$module_name == "Recruitment"] <- NA_character_
  x$parameters$estimation_type[which(x$parameters$fleet == "survey1" & x$parameters$label == "log_q")] <- "fixed_effects"
  projection <- get_report(phase_fit(x))
  expect_equal(head(projection$numbers_at_age[[1]], length(baseline$numbers_at_age[[1]])), baseline$numbers_at_age[[1]])
  future <- d[d$type == "index" & d$fleet == "survey1", ][1, ]
  future$timing <- 2030L
  future$date <- as.Date("2030-07-01")
  future$input_precision <- "day"
  future$observed <- -999
  x$data <- FIMSFrame(rbind(get_data(x$data), future))
  added <- get_report(phase_fit(x))
  for (quantity in c("numbers_at_age", "catch_expected", "spawning_biomass", "recruitment_events"))
    expect_equal(added[[quantity]], projection[[quantity]])
})
