FIMS testing plan for milestone 1 is documented using the test case documentation template from the [FIMS developer Handbook](https://noaa-fims.github.io/collaborative_workflow/testing.html#test-case-documentation-examples).

## General test case documentation

The test case documentation below is a general case to apply to many functions/modules. For individual functions/modules, please make detailed test cases for specific options, noting "same as the general test case" where appropriate.

- Test ID: General test case
- Feature to be tested
    - The function/module returns correct output values given different input values
    - The function/module returns error messages when users give wrong types of inputs
    - The function/module notifies with an error if the input value is outside the bound of the input parameter

- Approach
    - Prepare expected true values using R
    - Run tests in R using testthat and compare output values with expected values
    - Push tests to the working repository and run tests using GitHub Actions
    - Submit pull request for code review

- Evaluation criteria
    - The tests pass if the output values equal to the expected true values or the difference between an output value and an expected true value does not exceed an tolerance value (i.e., absolute error) 
    - The tests pass if the function/module returns error messages when users give wrong types of inputs
    - The tests pass if the function/module returns error messages when users provides an input value that is outside the bound of the input parameter

- Test deliverables
    - Test logs on GitHub Actions. Document results of logs in the feature pull request

## Logistic selectivity
- Test ID: The logistic selectivity function (test_population_dynamics_selectivity_logistic.cpp) 

- Feature to be tested
    - In addition to the features listed in the general test case, the function can be used to create multiple selectivity objects with different a50, slope, and age inputs
    - The IDs of the selectivity objects can be created, modified, or deleted properly when multiple objects are created, modified, or deleted

- Approach
    - R functional testing
    - Same as the general test case
    - Use object$evaluate to test if the object returns correct output values given different input values 
    
- Evaluation criteria
    - Same as the general test case 
    - Set tolerance value between FIMS result and expected value to 0.0001

- Test deliverables
    - Same as the general test case
