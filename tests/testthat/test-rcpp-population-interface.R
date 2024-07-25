library(testthat)
test_that("Population input settings work as expected", {
  population <- new(Population)
  nyears <- 10
  nages <- 10
  population$log_M <- rep(-1, nyears * nages)
  population$estimate_M <- FALSE
  population$log_init_naa <- log(rep(1, nages))
  population$estimate_init_naa <- TRUE
  population$nages <- nages
  population$ages <- 1:nages
  population$nfleets <- 2
  population$nseasons <- 1
  population$nyears <- nyears

  expect_equal(population$get_id(), 1)
  expect_equal(population$log_M, rep(-1, nyears * nages))
  expect_false(population$estimate_M)
  expect_equal(population$log_init_naa, rep(0, nages))
  expect_true(population$estimate_init_naa)

  clear()
})
