# Setup ----
# Load or prepare any necessary data for testing

test_that("Simplex projection works inside FIMS TMB framework", {
  clear()

  # Create delay embedding object for library
  lib_de <- methods::new(DelayEmbedding)
  lib_series <- c(10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0)
  E <- 3L
  tau <- 1L
  forecast_horizon <- 1L
  lib_de$construct(lib_series, E, tau, forecast_horizon)

  # Create delay embedding object for test
  test_de <- methods::new(DelayEmbedding)
  test_series <- c(50.0, 60.0, 70.0, 80.0)
  test_de$construct(test_series, E, tau, forecast_horizon)

  # Create SimplexProjection module
  sp <- methods::new(SimplexProjection)
  sp$embedding_dimension <- E
  sp$n_neighbors <- E + 1L
  sp$lib_de_id <- lib_de$get_id()
  sp$test_de_id <- test_de$get_id()
  sp$forecast_horizon <- 1L

  # Set up fishery model wrapper to trigger to_json/finalize loop
  caa <- methods::new(CatchAtAge)

  # Setup and trace the TMB model
  expect_no_error(CreateTMBModel())

  # Create ADFun and evaluate
  parameters <- list(
    p = get_fixed(),
    re = get_random()
  )
  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE
  )
  obj$report()

  # Run finalization via caa output
  caa_out <- caa$get_output()
  
  # finalized will be true and predictions populated
  expect_equal(length(sp$predictions$get_values()), test_de$n_rows)
  expect_true(all(is.finite(sp$predictions$get_values())))

  clear()
})

for (predictor_name in c("SimplexProjection", "SMapProjection", "GPEdmProjection")) {
  test_that(paste(predictor_name, "preserves TMB registration diagnostics"), {
    #' @description EDM registration logs invalid IDs before wrong object types and succeeds with valid embeddings after rejected registrations.
    clear()
    withr::defer(clear())
    library_embedding <- methods::new(DelayEmbedding)
    test_embedding <- methods::new(DelayEmbedding)
    series <- c(0.4, 0.9, 0.3, 0.8, 0.5, 0.7, 0.2, 0.6, 0.4, 0.9)
    library_embedding$construct(series, 2L, 1L, 1L)
    test_embedding$construct(series[5:10], 2L, 1L, 1L)
    predictor_class <- switch(predictor_name,
      SimplexProjection = SimplexProjection,
      SMapProjection = SMapProjection,
      GPEdmProjection = GPEdmProjection
    )
    predictor <- methods::new(predictor_class)
    predictor[["embedding_dimension"]] <- 2L
    library_id <- library_embedding$get_id()
    test_id <- test_embedding$get_id()
    wrong_type_id <- predictor$get_id()
    for (ids in list(
      c(0L, test_id), c(library_id, 0L), c(0L, 0L), c(wrong_type_id, 0L),
      c(wrong_type_id, test_id), c(library_id, wrong_type_id),
      c(wrong_type_id, wrong_type_id)
    )) {
      predictor[["lib_de_id"]] <- ids[[1L]]
      predictor[["test_de_id"]] <- ids[[2L]]
      expect_no_error(CreateTMBModel())
      message <- if (0L %in% ids) {
        "invalid lib_de_id or test_de_id."
      } else {
        "objects are not DelayEmbeddingInterface."
      }
      errors <- jsonlite::fromJSON(get_log_errors())
      expect_equal(
        tail(errors[["message"]], 2L),
        rep(paste0(predictor_name, "Interface: ", message), 2L)
      )
    }

    predictor[["lib_de_id"]] <- library_id
    predictor[["test_de_id"]] <- test_id
    expected <- predictor$predict(library_id, test_id)
    expect_no_error(CreateTMBModel())
    objective <- TMB::MakeADFun(
      data = list(),
      parameters = list(p = get_fixed(), re = get_random()),
      DLL = "FIMS",
      silent = TRUE
    )
    objective$report()
    predictor$finalize()
    expect_equal(predictor[["predictions"]]$get_values(), expected)
  })
}

test_that("SMap projection works inside FIMS TMB framework", {
  clear()

  # Create delay embedding object for library
  lib_de <- methods::new(DelayEmbedding)
  lib_series <- c(10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0)
  E <- 3L
  tau <- 1L
  forecast_horizon <- 1L
  lib_de$construct(lib_series, E, tau, forecast_horizon)

  # Create delay embedding object for test
  test_de <- methods::new(DelayEmbedding)
  test_series <- c(50.0, 60.0, 70.0, 80.0)
  test_de$construct(test_series, E, tau, forecast_horizon)

  # Create SMapProjection module
  smap <- methods::new(SMapProjection)
  smap$embedding_dimension <- E
  smap$theta <- 1.0
  smap$kernel <- "exponential"
  smap$lib_de_id <- lib_de$get_id()
  smap$test_de_id <- test_de$get_id()
  smap$forecast_horizon <- 1L

  caa <- methods::new(CatchAtAge)

  # Setup and trace the TMB model
  expect_no_error(CreateTMBModel())

  parameters <- list(
    p = get_fixed(),
    re = get_random()
  )
  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE
  )
  obj$report()

  # Run finalization
  caa_out <- caa$get_output()

  expect_equal(length(smap$predictions$get_values()), test_de$n_rows)
  expect_true(all(is.finite(smap$predictions$get_values())))

  clear()
})

test_that("GPEdm projection works inside FIMS TMB framework", {
  clear()

  # Create delay embedding object for library
  lib_de <- methods::new(DelayEmbedding)
  lib_series <- c(10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0)
  E <- 3L
  tau <- 1L
  forecast_horizon <- 1L
  lib_de$construct(lib_series, E, tau, forecast_horizon)

  # Create delay embedding object for test
  test_de <- methods::new(DelayEmbedding)
  test_series <- c(50.0, 60.0, 70.0, 80.0)
  test_de$construct(test_series, E, tau, forecast_horizon)

  # Create GPEdmProjection module
  gp <- methods::new(GPEdmProjection)
  gp$embedding_dimension <- E
  gp$sigma2 <- 1.0
  gp$ve <- 0.1
  gp$phi <- c(0.1, 0.2, 0.3)
  gp$lib_de_id <- lib_de$get_id()
  gp$test_de_id <- test_de$get_id()
  gp$forecast_horizon <- 1L

  caa <- methods::new(CatchAtAge)

  # Setup and trace the TMB model
  expect_no_error(CreateTMBModel())

  parameters <- list(
    p = get_fixed(),
    re = get_random()
  )
  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE
  )
  obj$report()

  # Run finalization
  caa_out <- caa$get_output()

  expect_equal(length(gp$predictions$get_values()), test_de$n_rows)
  expect_true(all(is.finite(gp$predictions$get_values())))

  clear()
})
