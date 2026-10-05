# Use annual years explicitly in tests that shift or truncate the calendar.
# data_big also contains dated observations, so its timing column is character.
annual_test_data <- function() {
  data <- FIMS::data_big
  data$timing <- as.integer(sub("-.*$", "", data$timing))
  data
}
