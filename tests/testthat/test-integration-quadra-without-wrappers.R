load(testthat::test_path("fixtures", "integration_test_data.RData"))

result_quadra_comparison <- setup_and_run_FIMS_without_wrappers(
  iter_id = 1,
  om_input_list = om_input_list,
  om_output_list = om_output_list,
  em_input_list = em_input_list,
  estimation_mode = FALSE,
  random_effects = c(recruitment = "log_devs"),
  compare_backends = TRUE
)

result_quadra_fit_comparison <- setup_and_run_FIMS_without_wrappers(
  iter_id = 1,
  om_input_list = om_input_list,
  om_output_list = om_output_list,
  em_input_list = em_input_list,
  estimation_mode = FALSE,
  random_effects = c(recruitment = "log_devs"),
  compare_optimizers = TRUE
)

result_quadra_laplace_comparison <- setup_and_run_FIMS_without_wrappers(
  iter_id = 1,
  om_input_list = om_input_list,
  om_output_list = om_output_list,
  em_input_list = em_input_list,
  estimation_mode = FALSE,
  random_effects = c(recruitment = "log_devs"),
  compare_laplace = TRUE
)

test_that("Quadra and TMB joint objectives agree without wrappers", {
  comparison <- result_quadra_comparison$backend_comparison

  expect_true(is.finite(comparison$tmb_joint_objective))
  expect_true(is.finite(comparison$quadra_joint_objective))
  expect_equal(
    comparison$quadra_joint_objective,
    comparison$tmb_joint_objective,
    tolerance = 1e-8
  )
  expect_length(
    comparison$quadra_gradient,
    length(result_quadra_comparison$parameters$p) +
      length(result_quadra_comparison$parameters$re)
  )
  expect_true(all(is.finite(comparison$quadra_gradient)))
  n_random <- length(result_quadra_comparison$parameters$re)
  random_indices <- seq.int(
    length(comparison$quadra_gradient) - n_random + 1L,
    length(comparison$quadra_gradient)
  )
  expect_equal(
    comparison$quadra_gradient[random_indices],
    comparison$tmb_gradient[random_indices],
    tolerance = 1e-7
  )
  expect_lt(
    comparison$quadra_graph_plan$random_active_vertices,
    comparison$quadra_graph_plan$total_vertices
  )
  expect_lt(
    comparison$quadra_graph_plan$laplace_active_vertices,
    comparison$quadra_graph_plan$total_vertices
  )
  expect_lt(
    comparison$quadra_graph_plan$restricted_gradient_max_difference,
    1e-10
  )
})

test_that("Quadra joint replay is faster than the TMB Laplace hot path", {
  comparison <- result_quadra_comparison$backend_comparison

  expect_gt(comparison$tmb_evaluation_time_seconds, 0)
  expect_gt(comparison$quadra_evaluation_time_seconds, 0)
  expect_lt(
    comparison$quadra_evaluation_time_seconds,
    comparison$tmb_evaluation_time_seconds
  )
})

test_that("Quadra L-BFGS and TMB nlminb reach the same joint optimum", {
  comparison <- result_quadra_fit_comparison$backend_comparison
  tmb_fit <- comparison$tmb_fit
  quadra_fit <- comparison$quadra_fit
  quadra_parameters <- c(quadra_fit$par, quadra_fit$random)

  expect_true(tmb_fit$converged)
  expect_true(quadra_fit$converged)
  expect_lt(quadra_fit$objective, quadra_fit$initial_objective)
  expect_equal(quadra_fit$objective, tmb_fit$objective, tolerance = 1e-5)
  expect_equal(quadra_parameters, tmb_fit$par, tolerance = 1e-3)
  expect_lt(quadra_fit$gradient_norm, tmb_fit$gradient_norm)
  expect_gt(quadra_fit$compact_tape_vertices, 0)
  expect_gt(quadra_fit$compact_tape_bytes, 0)
  expect_true(quadra_fit$full_graph_released)
  expect_lt(quadra_fit$compact_validation_objective_difference, 1e-10)
  expect_lt(quadra_fit$compact_validation_gradient_max_difference, 1e-10)
})

test_that("TMB construction remains valid after compact Quadra fitting", {
  expect_true(CreateTMBModel())
})

test_that("Quadra restricted exact Hessian reproduces the TMB Laplace objective", {
  comparison <- result_quadra_laplace_comparison$backend_comparison
  quadra_laplace <- comparison$quadra_laplace_evaluation
  tmb_laplace <- result_quadra_laplace_comparison$obj$fn(
    result_quadra_laplace_comparison$obj$par
  )

  expect_true(quadra_laplace$converged)
  expect_true(quadra_laplace$logdet_ok)
  expect_equal(
    as.numeric(quadra_laplace$objective),
    as.numeric(tmb_laplace),
    tolerance = 1e-3
  )
  expect_identical(quadra_laplace$factorization$backend, "dense_ldlt")
  expect_identical(quadra_laplace$factorization$structure, "dense")
  expect_equal(
    quadra_laplace$factorization$nnz,
    quadra_laplace$factorization$random_size^2
  )
})

# These objects retain several large TMB/Quadra tapes. Release them before the
# remaining test files run in the same R process.
rm(
  result_quadra_comparison,
  result_quadra_fit_comparison,
  result_quadra_laplace_comparison
)
gc()
