test_that("dates are prepared alongside each data object's annual rows", {
  #' @description Data dates preserve leap-day and early-year values, with December 31 for padded years.
  x <- dplyr::filter(data_big, type == "index", fleet == "survey1") |>
    dplyr::select(-age)
  x$timing <- as.Date(sprintf("%04d-01-01", x$timing + 1994L))
  x$timing[2] <- as.Date("1996-04-14")
  frame <- FIMSFrame(x)
  dates <- FIMS:::data_timing(frame, "survey1", "index")
  expect_equal(dates[1:3], c("1995-01-01", "1996-04-14", "1997-01-01"))
  x <- x[-2, ]
  expect_equal(FIMS:::data_timing(FIMSFrame(x), "survey1", "index")[2], "1996-12-31")
})

test_that("ambiguous observation dimensions fail before reaching C++", {
  #' @description Multiple samples in one annual likelihood cell and missing sample dates are rejected.
  x <- dplyr::filter(data_big, type == "index", fleet == "survey1") |>
    dplyr::select(-age)
  x$timing <- as.Date(sprintf("%04d-01-01", x$timing + 1994L))
  extra <- x[1, ]
  extra$timing <- as.Date("1995-06-10")
  expect_error(FIMS:::data_timing(
    FIMSFrame(dplyr::bind_rows(x, extra)),
    "survey1", "index"
  ), "one dated sample per year")
  expect_error(FIMS:::data_timing(
    FIMSFrame(dplyr::bind_rows(x, x[1, ])),
    "survey1", "index"
  ), "one dated sample per year")
  x$timing[1] <- NA
  expect_error(
    FIMS:::data_timing(FIMSFrame(x), "survey1", "index"),
    "one dated sample per year"
  )
})


test_that("composition samples retain annual and unique-bin limits", {
  #' @description Both composition types reject a second annual sample and duplicate bins on the same date.
  for (type in c("age_comp", "length_comp")) {
    x <- data_big
    x$timing <- as.Date(ifelse(is.na(x$timing), NA_character_,
      sprintf("%04d-01-01", x$timing + 1994L)
    ))
    frame <- FIMSFrame(x)
    samples <- dplyr::filter(get_data(frame), .data$type == .env$type, fleet == "survey1")
    duplicate <- frame
    duplicate@data <- dplyr::bind_rows(get_data(frame), samples[1, ])
    expect_error(
      FIMS:::data_timing(duplicate, "survey1", type),
      "one dated sample per year"
    )
    second <- samples[samples$timing == as.Date("1995-01-01"), ]
    second$timing <- as.Date("1995-06-10")
    duplicate@data <- dplyr::bind_rows(get_data(frame), second)
    expect_error(
      FIMS:::data_timing(duplicate, "survey1", type),
      "one dated sample per year"
    )
  }
})


test_that("dated observations reach TMB with independent survival and growth", {
  #' @description Dates change only the matching sample predictions, leave annual dynamics unchanged, and retain correct AD gradients.
  x <- dplyr::filter(
    data_big,
    is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4)
  )
  x$timing <- as.Date(ifelse(is.na(x$timing), NA_character_,
    sprintf("%04d-01-01", x$timing + 1994L)
  ))
  run <- function(input) {
    frame <- FIMSFrame(input)
    parameters <- setup_default_parameters(frame) |>
      dplyr::filter(module_name != "Growth") |>
      dplyr::bind_rows(setup_default_Growth(frame, module_type = "VonBertalanffySchnute"))
    fit_fims(initialize_fims(parameters, frame), optimize = FALSE, get_sd = FALSE)
  }
  baseline <- get_report(run(x))
  for (type in c("index", "age_comp", "length_comp")) {
    selected <- x$fleet == "survey1" & x$type == type &
      !is.na(x$timing) & format(x$timing, "%Y") == "1996"
    x$timing[selected] <- as.Date(switch(type,
      index = "1996-06-10",
      age_comp = "1996-09-01",
      length_comp = "1996-04-14"
    ))
  }
  fit <- run(x)
  dated <- get_report(fit)
  expect_equal(dated$numbers_at_age, baseline$numbers_at_age)
  expect_equal(dated$catch_expected, baseline$catch_expected)
  survey <- match("survey1", get_fleets(FIMSFrame(x)))
  n_ages <- get_n_ages(FIMSFrame(x))
  cells <- n_ages + seq_len(n_ages)
  z <- baseline$mortality_Z[[1]][cells]
  index_fraction <- as.numeric(as.Date("1996-06-10") - as.Date("1996-01-01")) / 366
  age_fraction <- as.numeric(as.Date("1996-09-01") - as.Date("1996-01-01")) / 366
  expect_equal(
    dated$index_numbers_at_age[[survey]][cells],
    baseline$index_numbers_at_age[[survey]][cells] * exp(-z * index_fraction)
  )
  expected_age <- baseline$agecomp_proportion[[survey]][cells] * exp(-z * age_fraction)
  expect_equal(dated$agecomp_proportion[[survey]][cells], expected_age / sum(expected_age))
  n_lengths <- length(unique(stats::na.omit(x$length[x$fleet == "survey1" & x$type == "length_comp"])))
  cells <- n_lengths + seq_len(n_lengths)
  expect_false(isTRUE(all.equal(
    dated$lengthcomp_proportion[[survey]][cells],
    baseline$lengthcomp_proportion[[survey]][cells]
  )))
  expect_equal(
    dated$lengthcomp_proportion[[survey]][-cells],
    baseline$lengthcomp_proportion[[survey]][-cells]
  )
  obj <- get_obj(fit)
  expect_true(is.finite(obj$fn(obj$par)))
  gradient <- as.numeric(obj$gr(obj$par))
  expect_true(all(is.finite(gradient)))
  growth <- grep("Growth", names(obj$par), ignore.case = TRUE)
  expect_gt(length(growth), 0)
  for (i in head(growth, 3)) {
    upper <- lower <- obj$par
    upper[i] <- upper[i] + 1e-5
    lower[i] <- lower[i] - 1e-5
    finite_difference <- (obj$fn(upper) - obj$fn(lower)) / 2e-5
    expect_equal(gradient[i], as.numeric(finite_difference), tolerance = 1e-3)
  }
  obj$fn(obj$par)

  #' @description JSON dates survive reading without FIMSFrame or live C++ objects.
  json <- get_model_output(fit)
  estimates <- get_estimates(fit)
  expect_s3_class(estimates$timing, "Date")
  expect_false(any(c("year", "year_i") %in% names(estimates)))
  for (label in c("index_expected", "agecomp_expected", "lengthcomp_expected")) {
    rows <- dplyr::filter(estimates, .data$module_name == "Fleet", .data$module_id == survey, .data$label == .env$label)
    actual <- unique(rows$timing[format(rows$timing, "%Y") == "1996"])
    expect_equal(actual, as.Date(switch(label,
      index_expected = "1996-06-10",
      agecomp_expected = "1996-09-01",
      lengthcomp_expected = "1996-04-14"
    )))
  }
  annual <- dplyr::filter(estimates, label %in% c("numbers_at_age", "mortality_Z"))
  expect_true(all(format(annual$timing, "%m-%d") == "01-01"))
  expect_equal(max(estimates$timing[estimates$label == "numbers_at_age"]), as.Date("1998-01-01"))
  recruitment <- dplyr::filter(estimates, label == "log_devs")
  expect_equal(min(recruitment$timing), as.Date("1996-01-01"))
  expect_true(all(is.na(estimates$timing[estimates$label == "logit_steep"])))
  clear()
  decoded <- FIMS:::reshape_json_estimates(json)
  expect_equal(decoded$timing, estimates$timing)
})


test_that("year-only observations average the year and retain their interpretation", {
  #' @description Annual fishery compositions use catch and surveys use mean surviving numbers, with dates preserved through JSON.
  x <- dplyr::filter(data_big, is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4))
  frame <- FIMSFrame(x)
  expect_identical(FIMSFrame(get_data(frame)), frame)
  expect_equal(FIMS:::data_timing(frame, "survey1", "index"), c("0001-12-31", "0002-12-31", "0003-12-31"))
  fit <- fit_fims(initialize_fims(setup_default_parameters(frame), frame), optimize = FALSE, get_sd = FALSE)
  report <- get_report(fit)
  nages <- get_n_ages(frame)
  cells <- seq_len(nages * 3)
  fishery <- match("fleet1", get_fleets(frame))
  survey <- match("survey1", get_fleets(frame))
  proportions <- function(x) as.vector(apply(matrix(x, nrow = 3, byrow = TRUE), 1, function(y) y / sum(y)))
  expect_equal(report$agecomp_proportion[[fishery]], proportions(report$catch_numbers_at_age[[fishery]]))
  annual <- report$index_numbers_at_age[[survey]]
  annual_age <- report$agecomp_proportion[[survey]]
  json <- get_model_output(fit)
  estimates <- get_estimates(fit)
  rows <- dplyr::filter(estimates, module_name == "Fleet", module_id == survey, label == "index_expected")
  expect_equal(rows$timing, as.Date(c("0001-12-31", "0002-12-31", "0003-12-31")))
  expect_false("timing_type" %in% names(estimates))
  expect_true(all(format(estimates$timing[estimates$label == "catch_expected"], "%m-%d") == "12-31"))
  clear()
  expect_equal(FIMS:::reshape_json_estimates(json)$timing, estimates$timing)
  x$timing <- as.Date(ifelse(is.na(x$timing), NA_character_, sprintf("%04d-01-01", x$timing)))
  dated_frame <- FIMSFrame(x)
  dated <- fit_fims(initialize_fims(setup_default_parameters(dated_frame), dated_frame), optimize = FALSE, get_sd = FALSE)
  baseline <- get_report(dated)
  z <- baseline$mortality_Z[[1]][cells]
  expect_equal(annual, baseline$index_numbers_at_age[[survey]] * (1 - exp(-z)) / z)
  expect_equal(annual_age, proportions(annual))
  expect_equal(report$catch_expected, baseline$catch_expected)
  expect_equal(report$numbers_at_age, baseline$numbers_at_age)
  expect_false(isTRUE(all.equal(report$agecomp_proportion[[fishery]], baseline$agecomp_proportion[[fishery]])))
  clear()
})


test_that("annual growth averages agree with independently sampled dates", {
  #' @description Annual weight and length predictions approximate a dense midpoint integral and retain AD gradients.
  x <- dplyr::filter(data_big, is.na(timing) | timing <= 2 | (type == "weight_at_age" & timing == 3))
  x$timing <- as.character(x$timing + 1995L)
  run <- function(input) {
    clear()
    frame <- FIMSFrame(input)
    parameters <- setup_default_parameters(frame) |>
      dplyr::filter(module_name != "Growth") |>
      dplyr::bind_rows(setup_default_Growth(frame, module_type = "VonBertalanffySchnute"))
    fit_fims(initialize_fims(parameters, frame), optimize = FALSE, get_sd = FALSE)
  }
  fit <- run(x)
  annual <- get_report(fit)
  obj <- get_obj(fit)
  gradient <- as.numeric(obj$gr(obj$par))
  expect_true(all(is.finite(gradient)))
  for (i in head(grep("Growth", names(obj$par), ignore.case = TRUE), 3)) {
    upper <- lower <- obj$par
    upper[i] <- upper[i] + 1e-5
    lower[i] <- lower[i] - 1e-5
    expect_equal(gradient[i], as.numeric((obj$fn(upper) - obj$fn(lower)) / 2e-5), tolerance = 1e-3)
  }
  survey <- match("survey1", get_fleets(FIMSFrame(x)))
  weights <- lengths <- NULL
  for (k in seq_len(16)) {
    dated <- x
    selected <- dated$fleet == "survey1" & dated$type %in% c("index", "length_comp")
    years <- as.integer(dated$timing[selected])
    start <- as.Date(sprintf("%04d-01-01", years))
    days <- as.numeric(as.Date(sprintf("%04d-01-01", years + 1L)) - start)
    dated$timing[selected] <- format(start + floor((k - 0.5) * days / 16), "%Y-%m-%d")
    report <- get_report(run(dated))
    weights <- cbind(weights, report$index_weight_at_age[[survey]])
    lengths <- cbind(lengths, report$index_numbers_at_length[[survey]])
  }
  expect_equal(annual$index_weight_at_age[[survey]], rowMeans(weights), tolerance = 0.003)
  expect_equal(annual$index_numbers_at_length[[survey]], rowMeans(lengths), tolerance = 0.003)
  expected <- matrix(rowMeans(lengths), nrow = 2, byrow = TRUE)
  expected <- as.vector(t(expected / rowSums(expected)))
  expect_equal(annual$lengthcomp_proportion[[survey]], expected, tolerance = 0.003)
  clear()
})
