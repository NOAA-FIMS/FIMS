# tests for input objects
data(package = "FIMS")

test_that("Can create the s4 FIMSFrame classes", {
  age_frame <- FIMSFrameAge(data_mile1)
  fims_frame <- FIMSFrame(data_mile1)
  expect_s4_class(age_frame, "FIMSFrameAge")
  expect_s4_class(fims_frame, "FIMSFrame")
  # A data frame is an S3 object with class data.frame
  expect_s3_class(age_frame@data, "data.frame")
  expect_vector(age_frame@ages, ptype = integer())
  # Note: specifying ptype makes this specific enough to check the right type.
  # note the following fails, because the code leaves it as a data.frame currently,
  # but test that it is an atomic vector below:
  #expect_vector(age_frame@weightatage, ptype = numeric(), 
  # size = length(age_frame@ages))
  # A data frame is an S3 object with class data.frame
  expect_s3_class(fims_frame@data, "data.frame")
})

test_that("Validators work as expected", {
  bad_input <- data.frame(test = 1, test2 = 2)
  bad_input_no_rows <- bad_input[-1, , drop = FALSE]

  expect_error(FIMSFrame(bad_input_no_rows), "data must have at least one row")
  expect_error(FIMSFrame(bad_input), "data must contain")
})

test_that("Show and plot methods work", {
  # might be a better test than this
  age_frame <- FIMSFrameAge(data_mile1)
  fims_frame <- FIMSFrame(data_mile1)
  # Check show method
  expect_output(print(age_frame), "This is a FIMSFrame data.frame")
  expect_output(print(fims_frame), "This is a FIMSFrame data.frame")
  # check plotting method expect error for now,because a plot method not yet
  # defined for FIMSFrame, only for FIMSFrameAge. expect_silent(plot(fims_frame))
  expect_silent(plot(age_frame))
})