# Package index

## Data

Example dataset used in FIMS demonstrations and examples.

- [`data_big`](https://NOAA-FIMS.github.io/FIMS/reference/data_big.md) :
  FIMS input data frame

- [`fims_input_types`](https://NOAA-FIMS.github.io/FIMS/reference/fims_input_types.md)
  : Vector of acceptable FIMS input types

- [`plot()`](https://NOAA-FIMS.github.io/FIMS/reference/plot.md) :

  Plot a `FIMSFrame` object

## Run FIMS

Primary functions used when setting up or running a FIMS model.

- [`FIMSFrame()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFrame.md)
  :

  Class constructors for `FIMSFrame` and associated child classes

- [`get_data()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  [`get_fleets()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  [`get_n_fleets()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  [`get_n_years()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  [`get_start_year()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  [`get_end_year()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  [`get_ages()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  [`get_n_ages()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  [`get_lengths()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  [`get_n_lengths()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFrame.md)
  : Get a slot in a FIMSFrame object

- [`model_landings()`](https://NOAA-FIMS.github.io/FIMS/reference/model_.md)
  [`model_index()`](https://NOAA-FIMS.github.io/FIMS/reference/model_.md)
  [`model_age_comp()`](https://NOAA-FIMS.github.io/FIMS/reference/model_.md)
  [`model_length_comp()`](https://NOAA-FIMS.github.io/FIMS/reference/model_.md)
  [`model_weight_at_age()`](https://NOAA-FIMS.github.io/FIMS/reference/model_.md)
  [`model_age_to_length_conversion()`](https://NOAA-FIMS.github.io/FIMS/reference/model_.md)
  : Get a vector of data to be passed to a FIMS module from a FIMSFrame
  object

- [`create_default_configurations()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_configurations.md)
  : Create a default FIMS configuration tibble

- [`create_default_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/create_default_parameters.md)
  : Create default parameters for a FIMS model

- [`initialize_data_distribution()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_data_distribution.md)
  [`initialize_process_distribution()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_data_distribution.md)
  [`initialize_process_structure()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_data_distribution.md)
  : Set up a new distribution for a data type or a process

- [`lognormal()`](https://NOAA-FIMS.github.io/FIMS/reference/lognormal.md)
  [`multinomial()`](https://NOAA-FIMS.github.io/FIMS/reference/lognormal.md)
  : Distributions not available in the stats package

- [`initialize_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_fims.md)
  : Initialize C++ modules via Rcpp for a FIMS model

- [`FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/FIMSFit.md) :

  Class constructors for class `FIMSFit` and associated child classes

- [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)
  : Fit a FIMS model (BETA)

- [`get_input()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_report()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_obj()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_opt()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_max_gradient()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_sdreport()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_estimates()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_number_of_parameters()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_timing()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_version()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  [`get_model_output()`](https://NOAA-FIMS.github.io/FIMS/reference/get_FIMSFit.md)
  : Get a slot in a FIMSFit object

- [`is.FIMSFit()`](https://NOAA-FIMS.github.io/FIMS/reference/is.FIMSFit.md)
  : Check if an object is of class FIMSFit

- [`is_fims_verbose()`](https://NOAA-FIMS.github.io/FIMS/reference/is_fims_verbose.md)
  : Should FIMS be verbose?

## Package development

Primary functions used when developing FIMS package.

- [`remove_test_data()`](https://NOAA-FIMS.github.io/FIMS/reference/remove_test_data.md)
  : Remove test data
- [`run_gtest()`](https://NOAA-FIMS.github.io/FIMS/reference/run_gtest.md)
  : Run the google test suite
- [`run_r_integration_tests()`](https://NOAA-FIMS.github.io/FIMS/reference/run_r_integration_tests.md)
  : Run R integration tests
- [`run_r_unit_tests()`](https://NOAA-FIMS.github.io/FIMS/reference/run_r_unit_tests.md)
  : Run R unit tests
- [`setup_and_run_gtest()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_and_run_gtest.md)
  : Setup and run the google test suite
- [`setup_gtest()`](https://NOAA-FIMS.github.io/FIMS/reference/setup_gtest.md)
  : Set up your local environment to run the google tests locally
- [`use_gtest_template()`](https://NOAA-FIMS.github.io/FIMS/reference/use_gtest_template.md)
  : Create tests/gtest/test\_\*.cpp test file and register it in
  CMakeLists.txt
- [`use_testthat_template()`](https://NOAA-FIMS.github.io/FIMS/reference/use_testthat_template.md)
  : Create tests/testthat/test-\*.R test file
- [`reshape_json_estimates()`](https://NOAA-FIMS.github.io/FIMS/reference/reshape_json_estimates.md)
  : Reshape JSON estimates
- [`reshape_tmb_estimates()`](https://NOAA-FIMS.github.io/FIMS/reference/reshape_tmb_estimates.md)
  : Reshape TMB estimates

## Exported C++ functions and classes

Links to the relevant doxygen documentation for the C++ code.

- [`Cpp_classes`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`AgeComp`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`BevertonHoltRecruitment`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`CatchAtAge`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`DlnormDistribution`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`DmultinomDistribution`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`DnormDistribution`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`DoubleLogisticSelectivity`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`EWAAGrowth`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`Fleet`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`Index`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`Landings`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`LengthComp`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`LogDevsRecruitmentProcess`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`LogRRecruitmentProcess`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`LogisticMaturity`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`LogisticSelectivity`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`Parameter`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`ParameterVector`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`Population`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`RealVector`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`SharedInt`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`SharedReal`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  [`SharedString`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_classes.md)
  : C++ Classes Exported via Rcpp
- [`Cpp_functions`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`clear`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`get_fixed`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`get_log`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`get_log_errors`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`get_log_warnings`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`get_parameter_names`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`get_random`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`get_random_names`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`inv_logit`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`log_error`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`log_info`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`log_warning`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`logit`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`set_fixed`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`set_log_throw_on_error`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`set_random`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  [`CreateTMBModel`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
  : C++ Functions Exported via Rcpp
