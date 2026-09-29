test_that("data_big records the timing used by its operating model", {
  #' @description Survey index and compositions sample January 1; fishery observations remain annual and untimed keys retain NA.
  survey <- data_big$fleet %in% "survey1" &
    data_big$type %in% c("index", "age_comp", "length_comp")
  fishery <- data_big$fleet %in% "fleet1" &
    data_big$type %in% c("catch", "age_comp", "length_comp")
  expect_equal(sort(unique(data_big$timing[survey])), sprintf("%04d-01-01", 1:30))
  expect_true(all(grepl("^[0-9]{1,4}$", data_big$timing[fishery])))
  expect_true(all(is.na(data_big$timing[data_big$type == "age_to_length_conversion"])))
  normalized <- get_data(FIMSFrame(data_big))
  survey <- normalized$fleet %in% "survey1"
  fishery <- normalized$fleet %in% "fleet1" &
    normalized$type %in% c("catch", "age_comp", "length_comp")
  expect_true(all(format(normalized$timing[survey], "%m-%d") == "01-01"))
  expect_true(all(format(normalized$timing[fishery], "%m-%d") == "12-31"))
})
