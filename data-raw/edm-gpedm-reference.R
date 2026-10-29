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
