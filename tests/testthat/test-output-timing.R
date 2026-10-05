test_that("time dimensions resolve explicit dates in flattened array order", {
  #' @description Dates replace year indices, including short years, terminal years and recruitment offsets.
  dates <- c("0003-01-01", "0004-04-14", "0005-01-01")
  dimensions <- list(header = list("n_years", "n_ages"), dimensions = list(2L, 2L), timing = dates)
  result <- FIMS:::dimensions_to_tibble(dimensions)
  expect_equal(result$timing, rep(as.Date(dates[1:2]), each = 2))
  expect_equal(result$age_i, rep(1:2, 2))
  dimensions <- list(header = list("n_years+1"), dimensions = list(3L), timing = dates)
  expect_equal(FIMS:::dimensions_to_tibble(dimensions)$timing, as.Date(dates))
  dimensions <- list(header = list("n_years-1"), dimensions = list(1L), timing = dates)
  expect_equal(FIMS:::dimensions_to_tibble(dimensions)$timing, as.Date(dates[2]))
  expect_true(is.na(FIMS:::dimensions_to_tibble(list(header = list(NULL), dimensions = list(1L)))$timing))
  dimensions$timing <- NULL
  expect_error(FIMS:::dimensions_to_tibble(dimensions), "explicit timing dates")
})

test_that("Mohn's rho compares matching terminal dates", {
  #' @description Retrospective comparisons use actual dates without numeric year indices.
  estimates <- tibble::tibble(
    label = "spawning_biomass", retrospective_peel = c(0L, 0L, 0L, 1L, 1L, 1L),
    timing = as.Date(c("1995-01-01", "1996-01-01", "1997-01-01", "1995-01-01", "1996-01-01", "1997-01-01")),
    estimated = c(100, 200, 300, 110, 220, 360)
  )
  expect_equal(calculate_mohns_rho(list(years_to_remove = c(0L, 1L), estimates = estimates), "spawning_biomass"), 0.1)
})

test_that("short calendar years and missing samples are reported as dates", {
  #' @description The model exports exact early dates; missing samples retain annual December 31 labels.
  x <- dplyr::filter(annual_test_data(), is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4))
  x$timing <- as.Date(ifelse(is.na(x$timing), NA_character_, sprintf("%04d-01-01", x$timing)))
  selected <- x$fleet == "survey1" & x$type == "index" & !is.na(x$timing)
  x$observed[selected & as.integer(format(x$timing, "%Y")) == 2L] <- -999
  x$timing[selected & as.integer(format(x$timing, "%Y")) == 3L] <- as.Date("0003-06-10")
  frame <- FIMSFrame(x)
  fit <- fit_fims(initialize_fims(setup_default_parameters(frame), frame), optimize = FALSE, get_sd = FALSE)
  estimates <- get_estimates(fit)
  survey <- match("survey1", get_fleets(frame))
  rows <- dplyr::filter(estimates, module_name == "Fleet", module_id == survey, label == "index_expected")
  expect_equal(rows$timing, as.Date(c("0001-01-01", "0002-12-31", "0003-06-10")))
  metadata <- jsonlite::fromJSON(get_model_output(fit), simplifyVector = FALSE)
  expect_true("0003-06-10" %in% unlist(metadata$time_axes))
  expect_true(all(!is.na(augment(fit)$timing)))
  clear()
})


test_that("retrospective removal uses calendar years rather than elapsed days", {
  #' @description Peeling a year removes all samples in that calendar year while retaining annual catch.
  x <- dplyr::filter(annual_test_data(), is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4))
  x$timing <- as.Date(ifelse(is.na(x$timing), NA_character_, sprintf("%04d-01-01", x$timing + 1994L)))
  selected <- x$type == "index" & !is.na(x$timing) & format(x$timing, "%Y") == "1997"
  x$timing[selected] <- as.Date("1997-06-10")
  captured <- NULL
  testthat::local_mocked_bindings(
    initialize_fims = function(parameters, data) {
      captured <<- data
      list()
    },
    fit_fims = function(input, ...) input,
    .package = "FIMS"
  )
  FIMS:::run_modified_data_fims(1L, FIMSFrame(x), NULL)
  rows <- get_data(captured)
  sampled <- rows$type %in% c("index", "age_comp", "length_comp") & rows$observed != -999
  expect_equal(max(rows$timing[sampled]), as.Date("1996-01-01"))
  expect_true(any(rows$type == "catch" & rows$timing == as.Date("1997-01-01"), na.rm = TRUE))
})


test_that("dates are optional inputs to data constructors, not population settings", {
  #' @description Low-level data constructors accept dates directly and validate their dimensions and calendar format.
  clear()
  expect_s4_class(methods::new(Index, 2L, c("1996-04-14", "1997-06-10")), "Rcpp_Index")
  expect_s4_class(methods::new(Catch, 2L, c("1996-01-01", "1997-01-01")), "Rcpp_Catch")
  expect_s4_class(methods::new(AgeComp, 2L, 3L, c("1996-04-14", "1997-06-10")), "Rcpp_AgeComp")
  expect_s4_class(methods::new(LengthComp, 2L, 3L, c("1996-04-14", "1997-06-10")), "Rcpp_LengthComp")
  expect_error(methods::new(Index, 2L, "1996-04-14"), "one date per annual row")
  expect_error(methods::new(Index, 1L, "1997-02-29"), "invalid calendar date")
  expect_error(methods::new(Index, 1L, "199x-01-01"), "ISO")
  clear()
})
