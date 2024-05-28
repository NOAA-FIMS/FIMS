test_that("FIMSFit() creates an object of class 'FIMSFit'", {
  fit_obj <- list(version = "1.0", timing = list(time_total = 10), opt = list(num_pars = list(total = 10, fixed_effects = 5, random_effects = 5)), rep = list(ssb = 1), parList = list(), obj = NULL)
  result <- FIMSFit(
    input = list(),
    timing = c(time_total = as.difftime(10, units = "secs")),
    obj = list(
      par = NA, fn = NA, gr = NA, he = NA, hessian = NA, method = NA,
      retape = NA,
      env = list("last.par.best" = 1, "parList" = function(x) {
        list("p" = TRUE)
      }),
      report = function(x) {
        list(TRUE)
      }, simulate = NA
    )
  )

  expect_s4_class(result, "FIMSFit")
  expect_true(inherits(result, "FIMSFit"))
  expect_snapshot(
    print(result)
  )
})

test_that("FIMSFit is not returned from a strong", {
  expect_false(is.FIMSFit("not_a_FIMSFit"))
})

test_that("FIMSFit() stops for missing input", {
  expect_error(
    FIMSFit(
      obj = list(
        par = NA, fn = NA, gr = NA, he = NA, hessian = NA, method = NA,
        retape = NA,
        env = list("last.par.best" = 1, "parList" = function(x) {
          list("p" = TRUE)
        }),
        report = function(x) {
          list(TRUE)
        }, simulate = NA
      )
    ),
    "missing, with no default"
  )
})
