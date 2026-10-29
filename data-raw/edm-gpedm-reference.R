# Regenerate from the repository root with GPEDM installed from:
# remotes::install_github(
#   "tanyalrogers/GPEDM@2e4bb3c124daa6042e32d988a3b383683d1fa088"
# )
# Source example: https://github.com/tanyalrogers/GPEDM/blob/2e4bb3c124daa6042e32d988a3b383683d1fa088/README.md
# Section: "Other types of predictions", tlogtest2.

reference_commit <- "2e4bb3c124daa6042e32d988a3b383683d1fa088"
stopifnot(
  identical(utils::packageDescription("GPEDM")[["RemoteSha"]], reference_commit)
)

example_data <- new.env()
utils::data("thetalog2pop", package = "GPEDM", envir = example_data)
population_data <- example_data[["thetalog2pop"]] |>
  dplyr::filter(.data[["Population"]] == "PopA") |>
  dplyr::arrange(.data[["Time"]])
training_data <- population_data[1:40, ]
test_data <- population_data[41:50, ]

reference_fit <- GPEDM::fitGP(
  data = as.data.frame(training_data),
  y = "Abundance",
  E = 2,
  tau = 1,
  time = "Time",
  newdata = as.data.frame(test_data)
)

reference <- list(
  source = list(
    repository = "https://github.com/tanyalrogers/GPEDM",
    commit = reference_commit,
    version = as.character(utils::packageVersion("GPEDM")),
    example = "README: Other types of predictions, tlogtest2"
  ),
  embedding_dimension = 2L,
  time_lag = 1L,
  forecast_horizon = 1L,
  training_series = training_data[["Abundance"]],
  test_series = test_data[["Abundance"]],
  training_predictors = unname(reference_fit[["inputs"]][["x"]]),
  test_predictors = unname(GPEDM::makelags(
    y = test_data[["Abundance"]], E = 2, tau = 1
  )),
  scaling = reference_fit[["scaling"]],
  parameters = reference_fit[["pars"]],
  in_sample = reference_fit[["insampresults"]],
  out_of_sample = reference_fit[["outsampresults"]]
)

saveRDS(
  reference,
  file = file.path("tests", "testthat", "fixtures", "edm-gpedm-reference.rds"),
  version = 3
)

# Fixed, unequal length-scales keep both coordinates relevant and separate
# prediction correctness from optimizer behavior. These extend the README
# workflow with explicit predictors for horizons that makelags() cannot express.
make_reference_case <- function(horizon, tau, uncertainty) {
  time <- seq_len(90L)
  series <- sin(0.71 * time) + 0.6 * cos(0.23 * time) +
    0.25 * sin(1.37 * time)
  training_series <- series[1:60]
  test_series <- series[61:90]
  training_uncertainty <- 0.03 + seq_along(training_series) / 1000
  test_uncertainty <- 0.07 + seq_along(test_series) / 2000

  make_predictors <- function(values) {
    cbind(
      lag1 = dplyr::lag(values, n = horizon),
      lag2 = dplyr::lag(values, n = horizon + tau)
    )
  }
  training_predictors <- make_predictors(training_series)
  test_predictors <- make_predictors(test_series)
  training_rows <- which(stats::complete.cases(
    cbind(training_series, training_predictors)
  ))
  test_rows <- which(stats::complete.cases(cbind(test_series, test_predictors)))
  fixed_parameters <- c(phi1 = 0.35, phi2 = 1.2, ve = 0.08, sigma2 = 1.4)
  fit <- GPEDM::fitGP(
    y = training_series,
    x = training_predictors,
    fixedpars = unname(fixed_parameters),
    xnew = test_predictors[test_rows, , drop = FALSE],
    ynew = test_series[test_rows]
  )

  # Independent ablations demonstrate that neither lag can be silently dropped.
  ablated_predictions <- vapply(seq_len(2L), function(dimension) {
    ablated_parameters <- fixed_parameters
    ablated_parameters[[dimension]] <- 1e-12
    ablated_fit <- GPEDM::fitGP(
      y = training_series,
      x = training_predictors,
      fixedpars = unname(ablated_parameters),
      xnew = test_predictors[test_rows, , drop = FALSE],
      ynew = test_series[test_rows]
    )
    ablated_fit[["outsampresults"]][["predmean"]]
  }, numeric(length(test_rows)))

  source <- reference[["source"]]
  source[["example"]] <- paste(
    "Synthetic extension of README tlogtest2 with explicit lag predictors",
    "and fixed phi=(0.35, 1.2), ve=0.08, sigma2=1.4"
  )
  list(
    source = source,
    embedding_dimension = 2L,
    time_lag = tau,
    forecast_horizon = horizon,
    with_uncertainty = uncertainty,
    training_series = training_series,
    test_series = test_series,
    training_rows = training_rows,
    test_rows = test_rows,
    training_predictors = unname(training_predictors[training_rows, ]),
    test_predictors = unname(test_predictors[test_rows, ]),
    training_uncertainty = training_uncertainty,
    test_uncertainty = test_uncertainty,
    training_embedded_uncertainty = unname(
      make_predictors(training_uncertainty)[training_rows, ]
    ),
    test_embedded_uncertainty = unname(
      make_predictors(test_uncertainty)[test_rows, ]
    ),
    scaling = fit[["scaling"]],
    parameters = fit[["pars"]],
    in_sample = fit[["insampresults"]][["predmean"]][training_rows],
    out_of_sample = fit[["outsampresults"]][["predmean"]],
    ablated_predictions = ablated_predictions
  )
}

reference_cases <- list(
  two_active_lags = make_reference_case(1L, 1L, FALSE),
  longer_horizon_and_lag = make_reference_case(3L, 2L, FALSE),
  uncertainty = make_reference_case(1L, 1L, TRUE),
  longer_horizon_with_uncertainty = make_reference_case(3L, 2L, TRUE)
)
saveRDS(
  reference_cases,
  file = file.path("tests", "testthat", "fixtures", "edm-gpedm-cases.rds"),
  version = 3
)
