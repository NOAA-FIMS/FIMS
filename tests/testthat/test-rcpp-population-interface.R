library(testthat)
test_that("Population input settings work as expected", {
  population <- new(Population)
  nyears <- 10
  nages <- 10
  population$log_M <- methods::new(ParameterVector, rep(-1, nyears * nages), nyears * nages)
  population$log_init_naa <- methods::new(ParameterVector, log(rep(1, nages)), nages)
  population$log_init_naa$set_all_estimable(TRUE)
  population$nages <- nages
  population$ages <- 1:nages
  population$nfleets <- 2
  population$nseasons <- 1
  population$nyears <- nyears
  population$proportion_female <- rep(0.5, nages)
  population$estimate_prop_female <- FALSE

  expect_equal(population$get_id(), 1)
  expect_equal(population$log_M, rep(-1, nyears * nages))
  expect_false(population$estimate_M)
  expect_equal(population$log_init_naa, rep(0, nages))
  expect_true(population$estimate_init_naa)
  expect_false(population$estimate_prop_female)
  expect_equal(population$proportion_female, rep(0.5, nages))

  clear()
})
