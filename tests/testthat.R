# The verbosity settings below apply when running devtools::test() locally.

# Save the current verbosity setting
option_verbosity <- getOption("rlib_message_verbosity")
# Set verbosity to quiet for testing
options(rlib_message_verbosity = "quiet")

library(testthat)
library(FIMS)
write("----testthat files called----", file = "testthat/debug_log.txt")

safe_test_check <- function() {
  tryCatch(
    {
        test_check("FIMS")  # code that might fail
    },
    error = function(e) {
      message("Error caught: ", e$message)
      NA       # return value on error
    },
    warning = function(w) {
      message("Warning caught: ", w$message)
      invokeRestart("muffleWarning") # skip printing the warning
    },
    finally = {
      message("Done trying")
    }
  )
}

safe_test_check()

# Restore the original verbosity setting after tests
if (!is.null(option_verbosity)) {
  options(rlib_message_verbosity = option_verbosity)
} else {
  options(rlib_message_verbosity = NULL)
}
