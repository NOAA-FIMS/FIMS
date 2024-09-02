test_that("Parameter class constructors work as expected", {
    # Test default constructor
    param_default <- new(Parameter)
    expect_equal(param_default$value, 0)
    expect_equal(param_default$min, -Inf)
    expect_equal(param_default$max, Inf)
    expect_false(param_default$estimated)
    expect_false(param_default$is_random_effect)
    expect_equal(param_default$id, 1)

    # Test constructor with value and estimated
    param_estimated <- new(Parameter, 10.5, TRUE)
    expect_equal(param_estimated$value, 10.5)
    expect_true(param_estimated$estimated)
    expect_false(param_estimated$is_random_effect)
    expect_equal(param_estimated$id, 2)

    # Test with updated value and estimated
    param_estimated$value <- 5
    param_estimated$estimated <- FALSE
    expect_equal(param_estimated$value, 5)
    expect_false(param_estimated$estimated)

    # Test constructor with value only
    param_value_only <- new(Parameter, 5.0)
    expect_equal(param_value_only$value, 5.0)
    expect_false(param_value_only$estimated)
    expect_equal(param_value_only$id, 3)
    clear()
})