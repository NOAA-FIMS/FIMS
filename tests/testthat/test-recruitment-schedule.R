recruitment_schedule_frame <- function() {
  d <- data_big
  d$timing <- d$timing + 2026L
  FIMSFrame(d)
}

test_that("default recruitment schedules retain annual boundaries", {
  #' @description Default phases cover modeled years without a terminal-year pulse.
  frame <- FIMSFrame(data_big)
  schedule <- setup_recruitment_schedule(frame)
  expect_equal(nrow(schedule), get_n_years(frame))
  expect_equal(schedule$timing, seq.int(get_start_year(frame), get_end_year(frame)))
  expect_true(all(schedule$fraction == 1 & schedule$year_fraction == 0))
  expect_equal(schedule$year_i, seq_len(get_n_years(frame)))
  expect_equal(setup_recruitment_schedule(frame, schedule), schedule)
})

test_that("multiple phases preserve calendar coordinates and allocations", {
  #' @description Fractions and leap-year dates are preserved independently of input order.
  frame <- recruitment_schedule_frame()
  x <- setup_recruitment_schedule(frame)
  x$phase <- "early"
  x$fraction <- 0.4
  late <- x
  late$phase <- "late"
  late$fraction <- 0.6
  late$date <- as.Date(sprintf("%04d-07-01", late$timing))
  input <- rbind(x, late)
  result <- setup_recruitment_schedule(frame, input)
  expect_equal(setup_recruitment_schedule(frame, input[nrow(input):1, ]), result)
  expect_equal(result$year_fraction[result$timing == 2028], c(0, 182 / 366))
  expect_equal(result$year_fraction[result$timing == 2027], c(0, 181 / 365))
  expect_equal(result$fraction, rep(c(0.4, 0.6), get_n_years(frame)))
  input$date <- observation_date_iso(input$date)
  expect_equal(setup_recruitment_schedule(frame, input), result)
  input$fraction <- ifelse(input$phase == "early", 1, 0)
  expect_no_error(setup_recruitment_schedule(frame, input))
})

test_that("invalid recruitment schedules fail before model initialization", {
  #' @description Reject missing coverage, invalid dates, populations, phases, and allocations.
  frame <- recruitment_schedule_frame()
  x <- setup_recruitment_schedule(frame)
  expect_error(setup_recruitment_schedule(data_big), "FIMSFrame")
  expect_error(setup_recruitment_schedule(frame, data.frame()), "requires")
  expect_error(setup_recruitment_schedule(frame, x[0, ]), "empty")
  expect_error(setup_recruitment_schedule(frame, x[-1, ]), "every modeled year")
  expect_error(setup_recruitment_schedule(frame, rbind(x, x[1, ])), "Duplicate")
  for (bad in c(-0.1, NA, Inf, 0.9)) {
    y <- x
    y$fraction[1] <- bad
    expect_error(setup_recruitment_schedule(frame, y), "fractions")
  }
  y <- x
  y$population[1] <- "population2"
  expect_error(setup_recruitment_schedule(frame, y), "population1")
  for (bad in c("", NA, "different")) {
    y <- x
    y$phase[1] <- bad
    expect_error(setup_recruitment_schedule(frame, y), "phase")
  }
  for (bad in c("2027", "2027-02-29", "2027-04-31", "2057-01-01")) {
    y <- x
    y$date <- observation_date_iso(y$date)
    y$date[1] <- bad
    expect_error(setup_recruitment_schedule(frame, y))
  }
  y <- x
  y$date[1] <- y$date[1] + 0.5
  expect_error(setup_recruitment_schedule(frame, y), "whole-day")
})

test_that("initialization retains annual schedules without changing predictions", {
  #' @description Explicit defaults reproduce implicit defaults and failed schedules leave the active model intact.
  d <- data_big |>
    dplyr::filter(is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4))
  frame <- FIMSFrame(d)
  p <- setup_default_parameters(frame)
  on.exit(clear(), add = TRUE)
  annual <- setup_recruitment_schedule(frame)
  input <- initialize_fims(p, frame)
  expect_equal(attr(input, "recruitment_schedule"), annual)
  fit <- fit_fims(input, optimize = FALSE, get_sd = FALSE)
  obj <- get_obj(fit)
  expected <- list(
    objective = obj$fn(obj$par), gradient = obj$gr(obj$par),
    report = get_report(fit)
  )
  invalid <- annual
  invalid$fraction[1] <- 0.9
  expect_error(initialize_fims(p, frame, invalid), "sum to one")
  expect_equal(obj$fn(obj$par), expected$objective)
  fit <- fit_fims(initialize_fims(p, frame, annual), optimize = FALSE, get_sd = FALSE)
  obj <- get_obj(fit)
  expect_equal(list(
    objective = obj$fn(obj$par), gradient = obj$gr(obj$par),
    report = get_report(fit)
  ), expected, tolerance = 1e-12)
  clear()
  expect_equal(attr(get_input(fit), "recruitment_schedule"), annual)
})

test_that("Rcpp recruitment schedules validate phase coordinates and entry ages", {
  #' @description Direct interface calls validate timing and biological entry ages.
  on.exit(clear(), add = TRUE)
  clear()
  r <- methods::new(BevertonHoltRecruitment)
  r$n_years$set(2L)
  expect_no_error(r$SetRecruitmentSchedule(0:1, c(0, 0), c(1, 1), c("annual", "annual")))
  expect_error(r$SetRecruitmentSchedule(0:1, 0, c(1, 1), c("annual", "annual")), "lengths")
  expect_error(r$SetRecruitmentSchedule(c(-1L, 1L), c(0, 0), c(1, 1), c("annual", "annual")), "coordinates")
  expect_error(r$SetRecruitmentSchedule(0:1, c(0, 1), c(1, 1), c("annual", "annual")), "coordinates")
  expect_error(r$SetRecruitmentSchedule(0:1, c(0, 0), c(NA, 1), c("annual", "annual")), "coordinates")
  expect_error(r$SetRecruitmentSchedule(0:1, c(0, 0), c(1, 1), c(NA, "annual")), "missing")
  expect_error(r$SetRecruitmentSchedule(1:0, c(0, 0), c(1, 1), c("annual", "annual")), "ordered")
  expect_error(r$SetRecruitmentSchedule(0:1, c(0, 0), c(0.5, 1), c("annual", "annual")), "summing")
  expect_error(r$SetRecruitmentSchedule(0:1, c(0, 0), c(1, 1), c("a", "b")), "consistent")
  expect_no_error(r$SetRecruitmentSchedule(0:1, c(0.5, 0.5), c(1, 1), c("annual", "annual")))
  expect_no_error(r$SetRecruitmentSchedule(
    c(0L, 0L, 1L, 1L), c(0, 0.5, 0, 0.5),
    rep(0.5, 4), rep(c("early", "late"), 2)
  ))
  expect_no_error(r$SetRecruitmentEntryAges(rep(1, 4)))
  expect_error(r$SetRecruitmentEntryAges(c(1, 1)), "length")
  expect_error(r$SetRecruitmentEntryAges(c(1, NA, 1, 1)), "finite")
  expect_error(r$SetRecruitmentEntryAges(c(1, 1, 2, 1)), "constant")
})
