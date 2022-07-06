  library(Rcpp)
data(package = "FIMS")

test_that("ewaa data can be added to model",{

  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  ewaa_growth <- new(fims$EWAAgrowth)
  age_frame <- FIMSFrameAge(data_mile1)
  ewaa_data <- age_frame@weightatage
  ewaa_growth$ages <- unique(ewaa_data$age)
  ewaa_growth$weights <- unique(ewaa_data$value)
  expect_equal(ewaa_growth$evaluate(1), 0.005306555)

})
