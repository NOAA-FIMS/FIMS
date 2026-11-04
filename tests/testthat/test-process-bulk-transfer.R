test_that("process distributions transfer scales and initialize zeros in bulk", {
  #' @description Both process families preserve vector scales, estimation types, and zero initialization for both recruitment parameterizations.
  on.exit(FIMS::clear(), add = TRUE)
  values <- function(x) {
    vapply(seq_len(x$size()), function(i) x$get(i - 1L)$value, numeric(1))
  }
  types <- function(x) {
    vapply(seq_len(x$size()), function(i) {
      x$get(i - 1L)$estimation_type$get()
    }, character(1))
  }
  module <- methods::new(FIMS::BevertonHoltRecruitment)
  for (par in c("log_devs", "log_r")) {
    module$field(par)$resize(3L)
    for (family in list(stats::gaussian(), FIMS::lognormal())) {
      distribution <- FIMS::initialize_process_distribution(
        module, par, family,
        sd = list(value = c(0.2, 0.5, 1),
                  estimation_type = c("fixed_effects", NA, "random_effects"))
      )
      expect_equal(values(distribution$log_sd), log(c(0.2, 0.5, 1)))
      expect_equal(types(distribution$log_sd),
        c("fixed_effects", "constant", "random_effects"))
      expect_equal(values(distribution$observed_values), numeric(3))
      expect_equal(values(distribution$expected_values), numeric(3))
      broadcast <- FIMS::initialize_process_distribution(
        module, par, family,
        sd = list(value = c(0.2, 0.5, 1), estimation_type = "constant")
      )
      expect_equal(types(broadcast$log_sd), rep("constant", 3))
    }
  }
})
