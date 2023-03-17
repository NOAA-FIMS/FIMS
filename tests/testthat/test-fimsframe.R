# tests for input objects
data(package = "FIMS")
age_frame <- FIMSFrameAge(data_mile1)
fims_frame <- FIMSFrame(data_mile1)

test_that("Can create the S4 FIMSFrame classes", {
  expect_s4_class(age_frame, "FIMSFrameAge")
  expect_s4_class(fims_frame, "FIMSFrame")
  # A data frame is an S3 object with class data.frame
  expect_s3_class(age_frame@data, "data.frame")
  expect_vector(age_frame@ages, ptype = integer())
  expect_s3_class(age_frame@weightatage, "data.frame")
  expect_s3_class(fims_frame@data, "data.frame")

  # A helper function that creates a figure from code
  save_png <- function(code, width = 1000, height = 1000) {
    path <- tempfile(fileext = ".png")
    on.exit(unlink(path), add = TRUE)
    png(path, width = width, height = height)
    code
    dev.off()
  }

  expect_silent(save_png(plot(fims_frame)))
  expect_silent(save_png(plot(age_frame)))
})

test_that("Accessors work as expected in FIMSFrame", {
  expect_vector(fleets(fims_frame), ptype = numeric())
  expect_vector(nyrs(fims_frame), ptype = numeric())
  expect_length(nyrs(fims_frame), 1)
})

test_that("Accessors work as expected in FIMSFrameAge", {
  # expect_s3_class(data(age_frame), "data.frame")
  expect_vector(fleets(age_frame), ptype = numeric())
  expect_vector(nyrs(age_frame), ptype = numeric())
  expect_length(nyrs(age_frame), 1)
  expect_vector(ages(age_frame), ptype = integer())
  expect_s3_class(weightatage(age_frame), "data.frame")
})

test_that("Show method works as expected", {
  expect_output(show(fims_frame))
})


test_that("Validators work as expected", {
  bad_input <- data.frame(test = 1, test2 = 2)
  expect_warning(expect_error(FIMSFrame(bad_input)))
})
