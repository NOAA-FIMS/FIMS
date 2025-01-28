# Prepare data
fims_frame <- FIMS::FIMSFrame(data1)
# Ensure that the fims_frame object is removed after tests are completed
on.exit(rm(fims_frame), add = TRUE)

# Initialize an EWAAgrowth object
ewaa_growth <- methods::new(EWAAgrowth)
# Assign age data to the EWAAgrowth object
ewaa_growth$ages <- get_ages(fims_frame)
# Assign weight data to the EWAAgrowth object
ewaa_growth$weights <- m_weight_at_age(fims_frame)
on.exit(ewaa_growth)

# Set up a different EWAAgrowth object
ewaa_growth2 <- methods::new(EWAAgrowth)
on.exit(ewaa_growth2)

# IO correctness
test_that("EWAAgrowth evaluate() works with correct input data", {
  #' @description Test that EWAAgrowth evaluate(1) returns the first value in the weight-at-age data.
  expect_equal(ewaa_growth$evaluate(1), 0.00053065552)
})

test_that("EWAAgrowth get_id() works with correct input data", {
  #' @description Test that id of ewaa_growth is 1.
  expect_equal(ewaa_growth$get_id(), 1)
  #' @description Test that id of ewaa_growth2 is 2.
  expect_equal(ewaa_growth2$get_id(), 2)
})
# Clear any previous FIMS settings
clear()

# Edge handling 
# Question: The following test has been commented out due to it causing R to crash. 
# Should we remove the test entirely or revisit and fix it in the future?
# test_that("EWAAgrowth evaluate() doesn't work when missing weights", {
#   # Initialize an EWAAgrowth object
#   ewaa_growth <- methods::new(EWAAgrowth)
#   # Assign age data to the EWAAgrowth object
#   ewaa_growth$ages <- get_ages(fims_frame)
#   #' @description Test that EWAAgrowth evaluate(1) throws an error when weights are missing.
#   expect_error(ewaa_growth$evaluate(1))
#   # Clear any previous FIMS settings
#   clear()
# })

# Error handling
test_that("EWAAgrowth evaluate() returns expected error for mismatched input lengths", {
  # Initialize an EWAAgrowth object
  ewaa_growth <- methods::new(EWAAgrowth)
  # Assign age data and intentionally mismatch the length of ages and weights
  ewaa_growth$ages <- c(get_ages(fims_frame), 13)
  # Assign weight data to the EWAAgrowth object
  ewaa_growth$weights <- m_weight_at_age(fims_frame)
  #' @description Test that EWAAgrowth evaluate() throws an error when the lengths of ages and weights don't match.
  expect_error(
    ewaa_growth$evaluate(1),
    regexp = "ages and weights must be the same length",
    ignore.case = FALSE
  )
  # Clear any previous FIMS settings
  clear()
})
