test_that("set_diagnostic_future_plan() uses sequential for one core", {
  previous_plan <- future::plan(future::multisession, workers = 2)
  on.exit(future::plan(previous_plan), add = TRUE)

  restore_plan <- FIMS:::set_diagnostic_future_plan(1L)

  current_plan <- future::plan("list")[[1]]

  expect_true(inherits(current_plan, "sequential"))
  expect_equal(future::nbrOfWorkers(), 1)

  future::plan(restore_plan)

  restored_plan <- future::plan("list")[[1]]

  expect_true(inherits(restored_plan, "multisession"))
  expect_equal(future::nbrOfWorkers(), 2)
})

test_that("set_diagnostic_future_plan() uses multisession for parallel work", {
  previous_plan <- future::plan(future::sequential)
  on.exit(future::plan(previous_plan), add = TRUE)

  restore_plan <- FIMS:::set_diagnostic_future_plan(2L)

  current_plan <- future::plan("list")[[1]]

  expect_true(inherits(current_plan, "multisession"))
  expect_equal(future::nbrOfWorkers(), 2)

  future::plan(restore_plan)

  restored_plan <- future::plan("list")[[1]]

  expect_true(inherits(restored_plan, "sequential"))
  expect_equal(future::nbrOfWorkers(), 1)
})
