# tests for input objects
data("data1", package = "FIMS")
fims_frame <- FIMSFrame(data1)

test_that("Can create the S4 FIMSFrame classes", {
  expect_s4_class(fims_frame, "FIMSFrame")
  # A data frame is an S3 object with class data.frame
  expect_s3_class(get_data(fims_frame), "data.frame")

  expect_s3_class(get_data(fims_frame), "data.frame")

  # A helper function that creates a figure from code
  save_png <- function(code, width = 1000, height = 1000) {
    path <- tempfile(fileext = ".png")
    on.exit(unlink(path), add = TRUE)
    png(path, width = width, height = height)
    code
    dev.off()
  }

  expect_silent(save_png(plot(fims_frame)))
})

test_that("Accessors work as expected in FIMSFrame", {
  expect_s3_class(get_data(fims_frame), "data.frame")

  expect_vector(get_fleets(fims_frame), ptype = numeric())

  expect_type(get_n_years(fims_frame), "integer")
  expect_length(get_n_years(fims_frame), 1)

  expect_type(get_start_year(fims_frame), "integer")
  expect_length(get_start_year(fims_frame), 1)

  expect_type(get_end_year(fims_frame), "integer")
  expect_length(get_end_year(fims_frame), 1)

  expect_s3_class(get_data(fims_frame), "data.frame")

  expect_vector(get_fleets(fims_frame), ptype = numeric())

  expect_type(get_n_years(fims_frame), "integer")
  expect_length(get_n_years(fims_frame), 1)

  expect_type(get_start_year(fims_frame), "integer")
  expect_length(get_start_year(fims_frame), 1)

  expect_type(get_end_year(fims_frame), "integer")
  expect_length(get_end_year(fims_frame), 1)


  expect_vector(get_ages(fims_frame), ptype = integer())

  expect_type(get_n_ages(fims_frame), "integer")
  expect_length(get_n_ages(fims_frame), 1)

  expect_vector(m_weight_at_age(fims_frame), ptype = numeric())

})

test_that("Show method works as expected", {
  expect_output(show(fims_frame))
})

test_that("Validators work as expected", {
  bad_input <- data.frame(test = 1, test2 = 2)
  expect_error(FIMSFrame(bad_input))
})

n_years <- get_n_years(fims_frame)
n_ages <- get_n_ages(fims_frame)

fleet_names_age_comp <- dplyr::filter(
  .data = as.data.frame(get_data(fims_frame)),
  type == "age"
) |>
  dplyr::distinct(name) |>
  dplyr::pull(name)
n_age_comp <- length(fleet_names_age_comp)

fleet_names_index <- dplyr::filter(
  .data = as.data.frame(get_data(fims_frame)),
  type == "index"
) |>
  dplyr::distinct(name) |>
  dplyr::pull(name)
n_index <- length(fleet_names_index)

test_that("Can add index data to model", {
  index_dat <- vector(mode = "list", length = n_index)
  names(index_dat) <- fleet_names_index

  for (index_i in 1:n_index) {
    index <- Index
    index_dat[[fleet_names_index[index_i]]] <- new(index, n_years)
    expect_silent(index_dat[[fleet_names_index[index_i]]] <-
      m_index(fims_frame, fleet_names_index[index_i]))
  }

  clear()
})

test_that("Can add agecomp data to model", {
  age_comp_dat <- vector(mode = "list", length = n_age_comp)
  names(age_comp_dat) <- fleet_names_age_comp

  for (fleet_f in 1:n_age_comp) {
    age_comp_dat[[fleet_names_age_comp[fleet_f]]] <- new(AgeComp, n_years, n_ages)
    expect_silent(age_comp_dat[[fleet_names_age_comp[fleet_f]]]$age_comp_data <-
      m_agecomp(fims_frame, fleet_names_age_comp[fleet_f]))
  }

  clear()
})
