recruitment_input_fixture <- function() {
  observations <- data_big |>
    dplyr::filter(is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4))
  observations$timing <- ifelse(is.na(observations$timing), NA_character_,
    sprintf("%04d", observations$timing + 2026L))
  recruitment <- data.frame(
    type = "recruitment_fraction",
    timing = paste0(rep(2027:2029, each = 2), rep(c("-01-01", "-07-01"), 3)),
    phase = rep(c("early", "late"), 3), observed = rep(c(0.4, 0.6), 3),
    entry_age = 1
  )
  list(observations = observations, recruitment = recruitment,
       data = dplyr::bind_rows(observations, recruitment))
}

recruitment_input_parameters <- function(frame) {
  p <- setup_default_parameters(frame)
  p$estimation_type <- "constant"
  p$distribution[p$module_name == "Recruitment"] <- NA_character_
  p$distribution_type[p$module_name == "Recruitment"] <- NA_character_
  p$estimation_type[which(p$fleet == "survey1" & p$label == "log_q")] <- "fixed_effects"
  p
}

test_that("recruitment timing travels in the input table without creating observations", {
  #' @description Configuration survives normalization, row reordering, and table round trips without changing fleet, bin, horizon, or observation coordinates.
  x <- recruitment_input_fixture()
  base <- FIMSFrame(x$observations)
  expect_no_warning(frame <- FIMSFrame(x$data))
  expect_true("recruitment_fraction" %in% fims_input_types)
  expect_equal(get_fleets(frame), get_fleets(base))
  expect_equal(get_ages(frame), get_ages(base))
  expect_equal(get_n_years(frame), get_n_years(base))
  expect_equal(get_observations(frame), get_observations(base))
  expect_equal(get_data(FIMSFrame(get_data(frame))), get_data(frame))
  expect_equal(get_data(FIMSFrame(x$data[nrow(x$data):1, ])), get_data(frame))
  expect_equal(setup_default_parameters(frame), setup_default_parameters(base))
  phases <- setup_recruitment_schedule(frame)
  expect_equal(phases$fraction, rep(c(0.4, 0.6), 3))
  expect_equal(phases$year_fraction, c(0, 181 / 365, 0, 182 / 366, 0, 181 / 365))
  expect_equal(nrow(get_data(frame)[get_data(frame)$type == "recruitment_fraction", ]), 6L)
  # An optional entry age really is optional when omitted from the input.
  x$data$entry_age <- NULL
  expect_true(all(setup_recruitment_schedule(FIMSFrame(x$data))$entry_age == min(get_ages(base))))
})

test_that("timing-only schedule inputs and timing edits preserve calendar dates", {
  #' @description Full timing values take precedence over derived date metadata, and new configuration can be appended to normalized observations.
  x <- recruitment_input_fixture()
  base <- FIMSFrame(x$observations)
  explicit <- data.frame(population = "population1", phase = x$recruitment$phase,
                          timing = x$recruitment$timing, fraction = x$recruitment$observed,
                          entry_age = 1)
  expected <- setup_recruitment_schedule(FIMSFrame(x$data))
  expect_equal(setup_recruitment_schedule(base, explicit), expected)
  explicit$timing <- as.Date(explicit$timing)
  expect_equal(setup_recruitment_schedule(base, explicit), expected)
  normalized <- get_data(base)
  normalized$timing <- as.character(normalized$timing)
  expect_equal(setup_recruitment_schedule(FIMSFrame(dplyr::bind_rows(normalized, x$recruitment))), expected)
  edited <- get_data(FIMSFrame(x$data))
  edited$timing <- as.character(edited$timing)
  late <- which(edited$type == "recruitment_fraction" & edited$phase == "late")
  edited$timing[late] <- paste0(2027:2029, "-10-01")
  revised <- setup_recruitment_schedule(FIMSFrame(edited))
  expect_equal(revised$date[revised$phase == "late"], as.Date(paste0(2027:2029, "-10-01")))
  expect_equal(revised$fraction, expected$fraction)
  explicit <- expected
  explicit$timing <- as.character(explicit$timing)
  explicit$timing[explicit$phase == "late"] <- paste0(2027:2029, "-10-01")
  expect_equal(setup_recruitment_schedule(base, explicit), revised)
  # Survey edits also use timing, without requiring edits to the derived date.
  survey <- which(edited$type == "index" & edited$fleet == "survey1")
  edited$timing[survey] <- paste0(2027:2029, "-04-01")
  samples <- get_observations(FIMSFrame(edited))
  expect_equal(samples$date[samples$type == "index"], as.Date(paste0(2027:2029, "-04-01")))
})

test_that("table-based recruitment gives the same model as an explicit schedule", {
  #' @description Inputs alone recover identical objectives, gradients, reports, and stored schedules; conflicting sources fail before clearing the live model.
  x <- recruitment_input_fixture()
  base <- FIMSFrame(x$observations)
  frame <- FIMSFrame(x$data)
  phases <- setup_recruitment_schedule(frame)
  p <- recruitment_input_parameters(frame)
  on.exit(clear(), add = TRUE)
  explicit <- fit_fims(initialize_fims(p, base, phases), optimize = FALSE, get_sd = FALSE)
  obj <- get_obj(explicit)
  expected <- list(report = get_report(explicit), nll = obj$fn(obj$par), gradient = obj$gr(obj$par))
  TMB::FreeADFun(obj)
  for (input in list(frame, x$data, get_data(frame))) {
    fit <- fit_fims(initialize_fims(p, input), optimize = FALSE, get_sd = FALSE)
    obj <- get_obj(fit)
    expect_equal(list(report = get_report(fit), nll = obj$fn(obj$par), gradient = obj$gr(obj$par)), expected)
    expect_equal(attr(get_input(fit), "recruitment_schedule"), phases)
    # Existing refit code can pass the retained schedule if it agrees.
    expect_equal(setup_recruitment_schedule(frame, phases), phases)
    conflict <- phases
    conflict$fraction <- rep(c(0.5, 0.5), 3)
    expect_error(initialize_fims(p, input, conflict), "conflicts")
    expect_equal(obj$fn(obj$par), expected$nll)
    TMB::FreeADFun(obj)
  }
})

test_that("malformed recruitment rows are rejected rather than silently dropped", {
  #' @description Reject incomplete years, duplicate phases/dates, invalid fractions, ambiguous timing, and observation fields on recruitment configuration.
  x <- recruitment_input_fixture()
  make <- function(rows) FIMSFrame(dplyr::bind_rows(x$observations, rows))
  expect_error(make(x$recruitment[-1, ]), "sum to one|consistent")
  expect_error(make(x$recruitment[1:4, ]), "every modeled year")
  expect_error(make(rbind(x$recruitment, x$recruitment[1, ])), "Duplicate")
  bad <- x$recruitment
  bad$timing[1] <- "2026-01-01"
  expect_error(make(bad), "every modeled year")
  for (value in c("2027", "2027-07", "2027-02-29", NA_character_)) {
    bad <- x$recruitment
    bad$timing[1] <- value
    expect_error(make(bad), "full ISO|Invalid calendar")
  }
  for (value in c(-0.1, NA, Inf, 0.9)) {
    bad <- x$recruitment
    bad$observed[1] <- value
    expect_error(make(bad), "fractions")
  }
  bad <- x$recruitment
  bad$phase <- NULL
  expect_error(make(bad), "phase")
  for (field in c("fleet", "age", "length", "uncertainty")) {
    bad <- x$recruitment
    bad[[field]] <- if (field %in% c("age", "length")) 1 else "unexpected"
    expect_error(make(bad), "must leave")
  }
  bad <- x$recruitment
  bad$unit <- "number"
  expect_error(make(bad), "unit = proportion")
})

test_that("retrospectives keep recruitment from the data without a schedule argument", {
  #' @description Removing survey years retains the full recruitment configuration and phase-aware predictions during a refit.
  x <- recruitment_input_fixture()
  frame <- FIMSFrame(x$data)
  p <- recruitment_input_parameters(frame)
  on.exit(clear(), add = TRUE)
  fit <- run_modified_data_fims(1, frame, p)
  expect_equal(attr(get_input(fit), "recruitment_schedule"), setup_recruitment_schedule(frame))
  events <- get_report(fit)$recruitment_events
  expect_equal(events[events[, "year_i"] <= 3, "year_fraction"],
               c(0, 181 / 365, 0, 182 / 366, 0, 181 / 365))
})

test_that("public diagnostic refits retain table-based recruitment", {
  #' @description Retrospective and likelihood workflows accept a single input table and preserve its phase prediction policies.
  x <- recruitment_input_fixture()
  frame <- FIMSFrame(x$data)
  p <- recruitment_input_parameters(frame)
  on.exit(clear(), add = TRUE)
  fit <- fit_fims(initialize_fims(p, frame), number_of_loops = 1)
  retro <- run_fims_retrospective(0:1, frame, p, n_cores = 1)
  rows <- retro$estimates |>
    dplyr::filter(fleet == "survey1", label == "index_expected")
  expect_gt(nrow(rows), 0L)
  expect_true(all(rows$maturity_timing == "phase_age"))
  profile <- run_fims_likelihood(model = fit, parameters = p,
    data = get_data(frame), module_name = "Recruitment", parameter_name = "log_rzero",
    n_cores = 1, min = -0.1, max = 0.1, length = 3)
  rows <- profile$estimates |>
    dplyr::filter(fleet == "survey1", label == "index_expected")
  expect_gt(nrow(rows), 0L)
  expect_true(all(rows$maturity_timing == "phase_age"))
})

test_that("annual recruitment rows preserve the default annual model", {
  #' @description A single January phase in the data has exactly the same predictions as implicit annual recruitment.
  x <- recruitment_input_fixture()
  rows <- x$recruitment[x$recruitment$phase == "early", ]
  rows$phase <- "annual"
  rows$observed <- 1
  base <- FIMSFrame(x$observations)
  frame <- FIMSFrame(dplyr::bind_rows(x$observations, rows))
  p <- recruitment_input_parameters(frame)
  on.exit(clear(), add = TRUE)
  fit <- fit_fims(initialize_fims(p, base), optimize = FALSE, get_sd = FALSE)
  expected <- get_report(fit)
  TMB::FreeADFun(get_obj(fit))
  fit <- fit_fims(initialize_fims(p, frame), optimize = FALSE, get_sd = FALSE)
  expect_equal(get_report(fit), expected, tolerance = 1e-12)
  TMB::FreeADFun(get_obj(fit))
})

test_that("projection years require recruitment rows without defining the horizon", {
  #' @description Recruitment configuration cannot extend the model by itself; once catch extends it, missing recruitment years fail and complete inputs succeed.
  x <- recruitment_input_fixture()
  future <- x$recruitment[5:6, ]
  future$timing <- sub("2029", "2030", future$timing)
  expect_error(FIMSFrame(dplyr::bind_rows(x$data, future)), "every modeled year")
  catch <- x$observations[x$observations$type == "catch" & x$observations$timing == "2029", ]
  catch$timing <- "2030"
  catch$observed <- -999
  weights <- x$observations[x$observations$type == "weight_at_age" & x$observations$timing == "2030", ]
  weights$timing <- "2031"
  extended <- dplyr::bind_rows(x$data, catch, weights)
  expect_error(FIMSFrame(extended), "every modeled year")
  frame <- FIMSFrame(dplyr::bind_rows(extended, future))
  expect_equal(get_n_years(frame), 4L)
  expect_equal(nrow(setup_recruitment_schedule(frame)), 8L)
})
