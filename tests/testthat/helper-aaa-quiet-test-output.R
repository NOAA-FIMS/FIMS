# The verbosity settings below apply when running testthat::test_package(),
# which mimics the "Show testthat output" step in the call-r-cmd-check GitHub
# Action workflow.

# Save the current verbosity setting
option_verbosity <- getOption("rlib_message_verbosity")
# Set verbosity to quiet for testing
#options(rlib_message_verbosity = "quiet")
