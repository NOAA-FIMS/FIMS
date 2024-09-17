test_that("fimsfit creates an object of class 'fimsfit'", {
  fit_obj <- list(version = "1.0", timing = list(time_total = 10), opt = list(num_pars = list(total = 10, fixed_effects = 5, random_effects = 5)), rep = list(ssb = 1), parList = list(), obj = NULL)
  class(fit_obj) <- "list"
  result <- fimsfit(fit_obj)

  expect_s3_class(result, "fimsfit")
  expect_true(inherits(result, "list"))
  expect_equal(class(result), c("fimsfit", "list"))
})

test_that("fimsfit returns a warning for non-list input", {
  expect_warning(fimsfit("not_a_list"), "Object passed to fimsfit is not a list -- something went wrong in fitting?")
})

test_that("fimsfit stops for missing version", {
  fit_obj <- list(timing = list(time_total = 10), opt = list(num_pars = list(total = 10, fixed_effects = 5, random_effects = 5)), rep = list(ssb = 1), parList = list(), obj = NULL)
  class(fit_obj) <- "list"
  expect_error(fimsfit(fit_obj), "No version found, something went wrong")
})

test_that("is.fimsfit detects fimsfit objects correctly", {
  fit_obj <- list(version = "1.0", timing = list(time_total = 10), opt = list(num_pars = list(total = 10, fixed_effects = 5, random_effects = 5)), rep = list(ssb = 1), parList = list(), obj = NULL)
  class(fit_obj) <- c("fimsfit", "list")

  expect_true(is.fimsfit(fit_obj))
  expect_false(is.fimsfit(list(version = "1.0")))
})

test_that("is.fimsfits detects lists of fimsfit objects correctly", {
  fit_obj <- list(version = "1.0", timing = list(time_total = 10), opt = list(num_pars = list(total = 10, fixed_effects = 5, random_effects = 5)), rep = list(ssb = 1), parList = list(), obj = NULL)
  class(fit_obj) <- c("fimsfit", "list")

  fit_list <- list(fit_obj, fit_obj)
  non_fit_list <- list(fit_obj, list(version = "1.0"))

  expect_true(is.fimsfits(fit_list))
  expect_false(is.fimsfits(non_fit_list))
  expect_false(is.fimsfits("not_a_list"))
})

test_that("print.fimsfit prints summary correctly", {
  fit_obj <- list(version = "1.0",
                  timing = list(time_total = 10),
                  opt = list(num_pars = list(total = 77,
                                             fixed_effects = 77,
                                             random_effects = 0),
                             max_gradient = 0.0001,
                             objective = 1498.433),
                  rep = list(ssb = list(rep(10000, 5)),
                             jnll = 1498.33),
                  parList = list()
                  )
  class(fit_obj) <- c("fimsfit", "list")

  expect_snapshot(
    print.fimsfit(fit_obj)
  )
})
