# Setup ----
# Load or prepare any necessary data for testing

test_that("Simplex projection works inside FIMS TMB framework", {
  clear()

  # Create delay embedding object for library
  lib_de <- methods::new(DelayEmbedding)
  lib_series <- c(10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0)
  E <- 3L
  tau <- 1L
  lib_de$construct(lib_series, E, tau)

  # Create delay embedding object for test
  test_de <- methods::new(DelayEmbedding)
  test_series <- c(50.0, 60.0, 70.0, 80.0)
  test_de$construct(test_series, E, tau)

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

test_that("SMap projection works inside FIMS TMB framework", {
  clear()

  # Create delay embedding object for library
  lib_de <- methods::new(DelayEmbedding)
  lib_series <- c(10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0)
  E <- 3L
  tau <- 1L
  lib_de$construct(lib_series, E, tau)

  # Create delay embedding object for test
  test_de <- methods::new(DelayEmbedding)
  test_series <- c(50.0, 60.0, 70.0, 80.0)
  test_de$construct(test_series, E, tau)

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
  lib_de$construct(lib_series, E, tau)

  # Create delay embedding object for test
  test_de <- methods::new(DelayEmbedding)
  test_series <- c(50.0, 60.0, 70.0, 80.0)
  test_de$construct(test_series, E, tau)

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
