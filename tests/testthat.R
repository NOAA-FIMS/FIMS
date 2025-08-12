# The verbosity settings below apply when running devtools::test() locally.

# Save the current verbosity setting
option_verbosity <- getOption("rlib_message_verbosity")
# Set verbosity to quiet for testing
#options(rlib_message_verbosity = "quiet")

library(testthat)
library(FIMS)

test_check("FIMS")

# Restore the original verbosity setting after tests
if (!is.null(option_verbosity)) {
  options(rlib_message_verbosity = option_verbosity)
} else {
  options(rlib_message_verbosity = NULL)
}
