# 🧪 Testing cheat sheet

## Adding a new test

### :scroll: Create a test file using `FIMS:::use_*_template()`

- Calling [`FIMS:::use_gtest_template()`](https://noaa-fims.github.io/FIMS/reference/use_gtest_template.html) will create a new C++ test file, e.g., `tests/gtest/test_FileName_ClassName_FunctionName.cpp`, for GoogleTest and register the test in `tests/gtest/CMakeLists.txt`.

- Calling [`FIMS:::use_testthat_template()`](https://noaa-fims.github.io/FIMS/reference/use_testthat_template.html) will create a new R test file, e.g., `tests/testthat/test-function_name.R`, for {testthat}.

### 🛢️ Prepare test data for the new test

Prepare the test data in the new file or in a separate file if you plan on reusing the data in multiple test files.

- In-line test data: make the data directly in the new test file within the `setup` section.
- C++ reusable test data: create a fixture file, e.g., `tests/gtest/test_XXX_test_fixture.hpp`, to prepare reusable unit-test data, or create an integration file, e.g., `tests/gtest/integration/XXX.hpp`, to set up integration-test data for C++ tests.
- R reusable test data: add code to the `prepare_test_data()` function in [`tests/testthat/helper-integration-tests-setup-run.R`](https://github.com/NOAA-FIMS/FIMS/blob/main/tests/testthat/helper-integration-tests-setup-run.R), including code to save the new data object, e.g., `base::saveRDS(object, file = testthat::test_path("fixtures", "data_name.RDS"))`, and call `prepare_test_data()` in the `setup` section of the new test file to load the data using
  ```r
  if (!file.exists(test_path("fixtures", "data_name.RDS"))) {
    prepare_test_data()
  }
  ```
- Use pre-existing integration data, e.g., `tests/testthat/fixtures/integration_test_data_components.RData` and `tests/testthat/fixtures/integration_test_data.RData`, by loading them within the `setup` section, e.g., `load(test_path("fixtures", "integration_test_data.RData"))` or within `prepare_test_data()`, where these data objects can be updated by running `R/data1.R`.

### :pencil: Edit the code in the new test file

Follow the structure of the new test file to write appropriate correctness, edge-handling, and error-handling tests.

#### :hammer: Helper functions to set up tests

The following :hammer: helper functions are available to assist with writing integration tests in R.

- `setup_and_run_FIMS_without_wrappers()`: Set up a FIMS model without wrappers (in [`tests/testthat/helper-integration-tests-setup-run.R`](https://github.com/NOAA-FIMS/FIMS/blob/main/tests/testthat/helper-integration-tests-setup-run.R)).
- `setup_and_run_FIMS_with_wrappers()`: Set up a FIMS model with wrappers (in [`tests/testthat/helper-integration-tests-setup-run.R`](https://github.com/NOAA-FIMS/FIMS/blob/main/tests/testthat/helper-integration-tests-setup-run.R)).
- `verify_fims_deterministic()`: Compare the model output from a FIMS deterministic run against the expected "truth" from an operating model (in [`tests/testthat/helper-integration-tests-validation.R`](https://github.com/NOAA-FIMS/FIMS/blob/main/tests/testthat/helper-integration-tests-validation.R)).
- `verify_fims_nll()`: Compare the negative log likelihood (NLL) from a FIMS model against the expected NLL calculated from an operating model (in [`tests/testthat/helper-integration-tests-validation.R`](https://github.com/NOAA-FIMS/FIMS/blob/main/tests/testthat/helper-integration-tests-validation.R)).
- `validate_fims()`: Compare the output from a FIMS model where parameters are estimated against the expected "truth" from an operating model (in [`tests/testthat/helper-integration-tests-validation.R`](https://github.com/NOAA-FIMS/FIMS/blob/main/tests/testthat/helper-integration-tests-validation.R)).

#### :shushing_face: Skipping tests

The following {testthat} functions can be used at the beginning of a test file to skip tests under certain conditions:

- Add [`testthat::skip_on_ci()`](https://testthat.r-lib.org/reference/skip.html) at the beginning of a test file to skip it during continuous integration runs.
- Add [`testthat::skip_on_covr()`](https://testthat.r-lib.org/reference/skip.html) at the beginning of a test file to skip it during coverage calculation.

#### :mute: Suppressing messages

- Messages for FIMS are suppressed globally in `tests/testthat.R` to keep test reports clean in the console and on GitHub Actions. If a specific function within a test still generates messages, wrap the call in `base::suppressMessages()` to silence its output.

## :computer: Run tests

### C++

- [`FIMS:::setup_and_run_gtest()`](https://noaa-fims.github.io/FIMS/reference/setup_and_run_gtest.html): 
Set up integration test data and run the GoogleTest suite.
- [`FIMS:::run_gtest()`](https://noaa-fims.github.io/FIMS/reference/run_gtest.html): 
Run the GoogleTest suite.

### R

- Run [`devtools::load_all()`](https://devtools.r-lib.org/reference/load_all.html) and [`devtools::test()`](https://devtools.r-lib.org/reference/test.html) to execute all tests in the package.
- Run [`devtools::test(filter = "test_file_name")`](https://devtools.r-lib.org/reference/test.html) to execute a specific test file.
- Run [`devtools::install()`](https://devtools.r-lib.org/reference/install.html) before running `test-parallel*-.R`.
- [`FIMS:::run_r_unit_tests()`](https://noaa-fims.github.io/FIMS/reference/run_r_unit_tests.html): Run all R unit tests in the package.
- [`FIMS:::run_r_integration_tests()`](https://noaa-fims.github.io/FIMS/reference/run_r_integration_tests.html): Run all R integration tests in the package.
- [`FIMS:::remove_test_data()`](https://noaa-fims.github.io/FIMS/reference/remove_test_data.html): Remove test data from `tests/testthat/fixtures` to run the tests from fresh if changes in the core code affect FIMS input or output.
- [`FIMS:::check_fims()`](https://noaa-fims.github.io/FIMS/reference/check_fims.html): Run all styling, documentation, and testing for FIMS repository.

## :bug: Debug tests

### C++

- Run following lines in terminal and use gdb to debug:
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --parallel
gdb ./build/tests/gtest/population_dynamics_population.exe
c // to continue without paging
run // to see which line of code is broken
print this->log_naa // for example, print this->log_naa to see the value of log_naa; 
print i // for example, print i from the broken for loop
bt // backtrace
q // to quit
```

- Write print statements in the desired .hpp file
```cpp
nfleets = fleets.size();
std::ofstream out("debug.txt");
out <<nfleets;
```

- Start the debugger using `R -d gdb` and then type `R` to start R. See detailed instructions [here](https://webhomes.maths.ed.ac.uk/~swood34/RCdebug/RCdebug.html).

### R

- A call to `base::browser()` can be included in the body of a function to pause execution of a test and allow for interactive debugging.
- Use `?debug` to learn how to set, unset, or query the debugging flag on a function.
- Visit the [Debugging Guide](https://support.posit.co/hc/en-us/articles/205612627-Debugging-with-the-RStudio-IDE) for instructions on debugging with the RStudio IDE.

## :raising_hand: Need help?

If any of the instructions above do not work as expected, please [report an issue on GitHub](https://github.com/NOAA-FIMS/FIMS/issues).
