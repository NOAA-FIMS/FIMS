data(package = "FIMS")

test_that("ewaa data can be added to model", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  ewaa_growth <- new(fims$EWAAgrowth)
  age_frame <- FIMSFrameAge(data_mile1)
  ewaa_data <- age_frame@weightatage
  ewaa_growth$ages <- m_ages(age_frame)
  ewaa_growth$weights <- m_weightatage(age_frame)
  expect_equal(ewaa_growth$evaluate(1), 0.00053065552)

  ewaa_growth2 <- new(fims$EWAAgrowth)
  ewaa_growth2$ages <- c(unique(ewaa_data$age), 12)
  ewaa_growth2$weights <- unique(ewaa_data$value)
  expect_error(
    ewaa_growth2$evaluate(1),
    regexp = "ages and weights must be the same length",
    ignore.case = FALSE
  )
})
