# Test suite for check_arg_data
test_that("check_arg_data() throws error for invalid inputs", {
  # Test with an invalid data type
  expect_error(
    check_arg_data(data = 1:10),
    regexp = "argument must be an object created by"
  )

  # Test with NULL input
  expect_error(
    check_arg_data(data = NULL),
    regexp = "argument must be an object created by"
  )

  # Test with an unrelated object
  expect_error(
    check_arg_data(data = data.frame(a=1)),
    regexp = "argument must be an object created by."
  )
})

test_that("check_arg_data() passes for valid FIMSFrame input", {
  # Create a mock FIMSFrame object
  data("data_mile1")
  data <- FIMSFrame(data_mile1)

  # Expect no error for valid input
  expect_silent(check_arg_data(data = data))
})

# Test suite for check_valid_input()
test_that("check_valid_input() throws error for invalid inputs", {
  valid_options <- c("OptionA", "OptionB", "OptionC")

  # Test with non-character input
  expect_error(
    check_valid_input(input = 123, valid_options = valid_options, arg_name = "test_arg"),
    regexp = "The test_arg argument must be a non-empty character string."
  )

  # Test with character vector of length > 1
  expect_error(
    check_valid_input(input = c("OptionA", "OptionB"), valid_options = valid_options, arg_name = "test_arg"),
    regexp = "The test_arg argument must be a non-empty character string."
  )

  # Test with input not in valid options
  expect_error(
    check_valid_input(input = "InvalidOption", valid_options = valid_options, arg_name = "test_arg"),
    regexp = "argument is missing from the supported options: OptionA, OptionB, and OptionC"
  )
})

test_that("check_valid_input() passes for valid inputs", {
  valid_options <- c("OptionA", "OptionB", "OptionC")

  # Expect no error for valid input
  expect_silent(check_valid_input(input = "OptionA", valid_options = valid_options, arg_name = "test_arg"))
  expect_silent(check_valid_input(input = "OptionB", valid_options = valid_options, arg_name = "test_arg"))
})
