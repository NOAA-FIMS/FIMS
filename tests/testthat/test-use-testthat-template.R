# Create temporary files for testing purposes
create_temporary_file <- function(temp_path) {
    # Create the directory for the temporary package
    dir.create(temp_path)
    # Save the current working directory and set the new one for the 
    # temporary package
    old_wd <- getwd()
    setwd(temp_path)
    # Ensure that the working directory is reset after the function exits
    on.exit(setwd(old_wd), add = TRUE)
    # Create a new package at the specified path
    pkg <- usethis::create_package(temp_path)
    # Set the project to the temporary package directory
    usethis::proj_set(temp_path)
    # Initialize the {testthat} framework for testing
    usethis::use_testthat()
    # Add a test to the package using the test template
    FIMS:::use_testthat_template("my-test")
    # Attempt to use the test template again inside a tryCatch to capture any 
    # potential errors
    error <- tryCatch(FIMS:::use_testthat_template("my-test"),
        error = function(e) {
            # Return a custom error message if an error occurs
            "An error occurred."
        }
    )
    # Return details about the created package, including paths and any errors
    output <- list(
        folder_path = temp_path,
        file_path = file.path(temp_path, "tests", "testthat", "test-my-test.R"),
        error = error
    )
}

# Generate temporary files for testing
temp_path <- file.path(tempdir(), "rcmdcheck")
output <- create_temporary_file(temp_path)
# Ensure the temporary folder is cleaned up after tests are complete
on.exit(unlink(output[["folder_path"]], recursive = TRUE), add = TRUE)

# IO correctness
test_that("use_testthat_template() works with correct inputs", {
    #' @description Test that use_testthat_template("my-test") creates the correct file.
    expect_true(file.exists(output[["file_path"]]))
})

# Edge handling
test_that("use_testthat_template() handles edge cases correctly", {
    #' @description Test that use_testthat_template() throws an error when no input is provided.
    expect_error(object = FIMS:::use_testthat_template())
})

# Error handling
test_that("use_testthat_template() throws an error if the file already exists", {
    #' @description Test that use_testthat_template("my-test") throws an error when the file already exists.
    expect_equal(output[["error"]], "An error occurred.")
})
