# tests for input objects
data(package = "FIMS")

test_that("Can create the S4 FIMSFrame classes", {
  age_frame <- FIMSFrameAge(data_mile1)
  fims_frame <- FIMSFrame(data_mile1)
  expect_s4_class(age_frame, "FIMSFrameAge")
  expect_s4_class(fims_frame, "FIMSFrame")
  # A data frame is an S3 object with class data.frame
  expect_s3_class(age_frame@data, "data.frame")
  expect_vector(age_frame@ages, ptype = integer())
  expect_s3_class(age_frame@weightatage, "data.frame")
  expect_s3_class(fims_frame@data, "data.frame")
  expect_silent(plot(fims_frame))
  expect_silent(plot(age_frame))
  dev.off()
})

test_that("Show method works as expected", {
  expect_message(show(fims_frame))
})


test_that("Validators work as expected", {
  bad_input <- data.frame(test = 1, test2 = 2)
  expect_error(FIMSFrame(bad_input))
})
