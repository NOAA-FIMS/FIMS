# A separable Gaussian objective has exact values, derivatives and uncertainty.
quadra_gaussian_fixture <- function(random = FALSE) {
  clear()
  sel <- create_selectivity("Logistic")
  set_variable_vector(sel, "inflection_point", 3, "fixed_effects")
  set_variable_vector(
    sel, "slope", -2,
    if (random) "random_effects" else "fixed_effects"
  )
  for (field in c("inflection_point", "slope")) {
    prior <- create_distribution("dnorm")
    set_variable_vector(prior, "expected_values", 1, "assumed_known")
    set_variable_vector(prior, "log_sd", log(2), "assumed_known")
    set_distribution_links(prior, "prior", get_variable_vector_id(sel, field))
  }
  CreateTMBModel()
  list(p = get_fixed(), re = get_random())
}

test_that("Quadra matches analytic and TMB joint derivatives through XPtr", {
  on.exit(clear())
  for (random in c(FALSE, TRUE)) {
    parameters <- quadra_gaussian_fixture(random)
    obj <- TMB::MakeADFun(list(), parameters, DLL = "FIMS", silent = TRUE)
    values <- unlist(parameters, use.names = FALSE)
    for (shift in c(0, 0.25, -0.5, 0)) {
      p <- parameters$p + shift
      re <- parameters$re + shift
      result <- quadra_evaluate(p, re)
      expected <- -sum(dnorm(values + shift, 1, 2, log = TRUE))
      expect_equal(result$objective, expected, tolerance = 1e-10)
      expect_equal(result$gradient, (values + shift - 1) / 4, tolerance = 1e-10)
      expect_equal(result$objective, obj$fn(values + shift), tolerance = 1e-10)
      expect_equal(result$gradient, as.numeric(obj$gr(values + shift)), tolerance = 1e-10)
    }
    sd <- quadra_sdreport(parameters$p, parameters$re)
    expect_equal(unname(sd$hessian), diag(0.25, 2), tolerance = 1e-10)
    expect_equal(unname(sd$cov.all), diag(4, 2), tolerance = 1e-10)
    expect_true(sd$pdHess)
    fit <- quadra_fit(parameters$p, parameters$re)
    expect_true(fit$converged)
    expect_equal(c(fit$par, fit$random), c(1, 1), tolerance = 1e-6)
    expect_equal(quadra_evaluate(parameters$p, parameters$re)$objective,
      -sum(dnorm(values, 1, 2, log = TRUE)),
      tolerance = 1e-10
    )
  }
})

test_that("Quadra rejects invalid inputs and clears cached model state", {
  on.exit(clear())
  clear()
  expect_error(quadra_evaluate(numeric(), numeric()), "Build the model")
  parameters <- quadra_gaussian_fixture()
  expect_error(quadra_evaluate(NA_real_, numeric()), "finite numeric")
  expect_error(quadra_evaluate(1, numeric()), "parameter count")
  expect_error(quadra_fit(max_iterations = 1.5), "positive integer")
  expect_error(quadra_fit(gradient_tolerance = Inf), "positive number")
  original <- quadra_evaluate()$objective
  CreateTMBModel()
  expect_equal(quadra_evaluate()$objective, original)
  clear()
  expect_error(quadra_evaluate(numeric(), numeric()), "Build the model")
  quadra_gaussian_fixture(TRUE)
  expect_equal(quadra_evaluate()$objective, original)
})

test_that("Quadra Laplace fits preserve subsequent evaluation", {
  on.exit(clear())
  parameters <- quadra_gaussian_fixture(TRUE)
  result <- quadra_fit(method = "laplace", max_iterations = 50L)
  expect_true(result$converged)
  expect_equal(result$par, 1, tolerance = 1e-5)
  expect_equal(result$random, 1, tolerance = 1e-5)
  expect_equal(result$objective, -dnorm(1, 1, 2, log = TRUE), tolerance = 1e-6)
  expect_equal(quadra_evaluate(parameters$p, parameters$re)$objective,
    -sum(dnorm(c(3, -2), 1, 2, log = TRUE)),
    tolerance = 1e-8
  )
  expect_equal(quadra_sdreport(result$par, result$random)$std.error, c(2, 2),
    tolerance = 1e-8
  )
})

test_that("catch-at-age models agree across backends and return FIMSFit", {
  on.exit(clear())
  data <- FIMSFrame(data_big)
  parameters <- setup_default_parameters(data)
  parameters$estimation_status <- ifelse(
    parameters$label == "log_rzero", "fixed_effects", "assumed_known"
  )
  parameters <- parameters[!(parameters$module_name == "Recruitment" &
    parameters$label == "log_sd"), ]
  rec <- parameters$module_name == "Recruitment"
  parameters$distribution[rec] <- NA_character_
  parameters$distribution_type[rec] <- NA_character_
  input <- initialize_fims(data = data, parameters = parameters)
  obj <- TMB::MakeADFun(list(), input$parameters, DLL = "FIMS", silent = TRUE)
  result <- quadra_evaluate()
  expect_true(is.finite(result$objective))
  expect_equal(result$objective, obj$fn(obj$par), tolerance = 1e-8)
  expect_equal(result$gradient, as.numeric(obj$gr(obj$par)), tolerance = 1e-7)
  fit <- fit_fims(input, backend = "quadra", control = list(iter.max = 100L))
  expect_s4_class(fit, "FIMSFit")
  expect_equal(fit@input$backend, "quadra")
  expect_true(is.finite(fit@opt$objective))
  expect_equal(fit@opt$objective, fit@report$jnll, tolerance = 1e-7)
  expect_s3_class(fit@sdreport, "quadra_sdreport")
  expect_true(fit@sdreport$pdHess)
  expect_lt(fit@max_gradient, 1e-4)
  tmb <- fit_fims(input, number_of_loops = 0)
  expect_equal(fit@opt$par, unname(tmb@opt$par), tolerance = 1e-5)
  expect_equal(fit@sdreport$cov.fixed, tmb@sdreport$cov.fixed, tolerance = 1e-5)
  expect_error(fit_fims(within(input, map <- list(p = factor(NA))),
    backend = "quadra"
  ), "parameter maps")
  CreateTMBModel()
  expect_error(fit_fims(input, backend = "quadra"), "rebuilt")
})
