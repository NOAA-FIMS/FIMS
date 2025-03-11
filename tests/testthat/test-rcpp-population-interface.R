library(testthat)
test_that("Population input settings work as expected", {
  population <- methods::new(Population)
  nyears <- 10
  nages <- 10
  log_M_length <- nyears * nages
  population$log_M$resize(log_M_length)
  purrr::walk(
    seq_along(1:log_M_length),
    \(x) population$log_M[x]$value <- -1
  )
  population$log_init_naa$resize(nages)
  purrr::walk(
    seq_along(1:nages),
    \(x) population$log_init_naa[x]$value <- 0
  )
  population$log_init_naa$set_all_estimable(TRUE)
  population$nages$set(nages)
  population$ages$resize(nages)
  purrr::walk(
    seq_along(1:nages),
    \(x) population$ages$set(x - 1, x)
  )
  population$nfleets$set(2)
  population$nseasons$set(1)
  population$nyears$set(nyears)

  expect_equal(population$get_id(), 1)
  for (i in 1:(nyears * nages)) {
    expect_equal(population$log_M[i]$value, -1)
    expect_false(population$log_M[i]$estimated)
  }
  for (i in 1:nyears) {
    expect_equal(population$log_init_naa[i]$value, 0)
    expect_true(population$log_init_naa[i]$estimated)
  }

  clear()
})
