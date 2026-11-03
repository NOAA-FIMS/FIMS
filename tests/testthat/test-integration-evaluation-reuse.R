# Regression coverage for state preparation and per-evaluation recruitment reuse.
testthat::skip_if(
  Sys.getenv("RUN_SLOW_TESTS") != "true",
  "Skipping: RUN_SLOW_TESTS is not set to true."
)

test_that("repeated evaluations refresh recruitment and preserve AD gradients", {
  on.exit(FIMS::clear(), add = TRUE)
  data("data_big", package = "FIMS")
  frame <- FIMS::FIMSFrame(data_big)
  parameters <- FIMS::setup_default_parameters(data = frame)
  # Estimate mortality so the year-zero inputs to phi_0 change on the tape.
  parameters$estimation_type[parameters$label == "log_M"] <- "fixed_effects"
  input <- FIMS::initialize_fims(parameters, data = frame)
  # Test the joint objective directly, without a nested random-effects optimizer.
  obj <- TMB::MakeADFun(
    data = list(), parameters = input$parameters, map = input$map,
    random = NULL, DLL = "FIMS", silent = TRUE
  )
  par <- obj$par
  value <- obj$fn(par)
  report <- obj$report(par)
  gradient <- as.numeric(obj$gr(par))

  changed <- par + 0.01
  changed_value <- obj$fn(changed)
  changed_report <- obj$report(changed)
  expect_true(is.finite(changed_value))
  expect_false(isTRUE(all.equal(
    report$expected_recruitment, changed_report$expected_recruitment
  )))
  # Returning to the original parameters must not retain state from another run.
  expect_equal(obj$fn(par), value, tolerance = 1e-10)
  expect_equal(obj$report(par), report, tolerance = 1e-10)
  expect_equal(as.numeric(obj$gr(par)), gradient, tolerance = 1e-10)

  # Check a directional derivative including mortality against the double path.
  direction <- rep(1 / sqrt(length(par)), length(par))
  step <- 1e-4
  finite_difference <- (
    obj$fn(par + step * direction) - obj$fn(par - step * direction)
  ) / (2 * step)
  expect_equal(sum(gradient * direction), finite_difference, tolerance = 1e-5)
})
