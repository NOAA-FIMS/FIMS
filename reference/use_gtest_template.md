# Create tests/gtest/test\_\*.cpp test file and register it in CMakeLists.txt

This helper function generates a GoogleTest (gtest) C++ template file
for a given function and appends lines to `CMakeLists.txt` to register
the test.

## Usage

``` r
use_gtest_template(name = "FileName_ClassName_FunctionName")
```

## Arguments

- name:

  A string representing the combined name for the C++ test file and
  CMake executable target. It must follow the format
  `FileName_ClassName_FunctionName`, where

  - `FileName` is the C++ source file name (e.g., "Logistic")

  - `ClassName` is the C++ class name (e.g., "LogisticSelectivity")

  - `FunctionName` is the C++ function name (e.g., "Evaluate") and use
    underscores to separate each component. For example:
    `Logistic_LogisticSelectivity_Evaluate`.

  If the function is not a member of a class, use a placeholder for
  `ClassName`. For example: `FIMSMath_ClassName_Logistic`.

  The default is `FileName_ClassName_FunctionName`.

## Value

If successful, this function invisibly returns `TRUE` to allow for the
chaining of commands. If the function is unsuccessful, an error message
is returned.

Three messages are also returned from the usethis package, which is used
by this function. The first states where the FIMS project is on your
computer. The second states the file path of the newly created file. The
file will not be automatically opened. The third states the test has
been registered in `tests/gtest/CMakeLists.txt`.

## Examples

``` r
if (FALSE) { # \dontrun{
# Create a new test file named
# "test_Logistic_LogisticSelectivity_Evaluate.cpp" for
# `LogisticSelectivity::evaluate()` in
# `inst/include/population_dynamics/selectivity/functors/logistic.hpp`.

FIMS:::use_gtest_template(name = "Logistic_LogisticSelectivity_Evaluate")
} # }
```
