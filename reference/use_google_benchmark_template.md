# Create tests/google_benchmark/benchmark\_\*.cpp and register it in CMakeLists.txt

This helper function generates a Google Benchmark C++ template file for
a given workload and appends lines to
`tests/google_benchmark/CMakeLists.txt` to register the benchmark as a
separate executable.

## Usage

``` r
use_google_benchmark_template(
  name = "FileName_ClassName_FunctionName",
  type = c("simple", "fixture")
)
```

## Arguments

- name:

  A string representing the name for the C++ benchmark file and CMake
  executable target. It must follow the format
  `FileName_ClassName_FunctionName`, where

  - `FileName` is the C++ source file name

  - `ClassName` is the C++ class name (or a placeholder such as
    `ClassName`)

  - `FunctionName` is the C++ function name and underscores separate
    each component. For example:
    `Population_CatchAtAge_CatchNumbersAtAge`.

  The generated file will be
  `tests/google_benchmark/benchmark_<name>.cpp` and the CMake target
  will be `benchmark_<name>`.

- type:

  Benchmark template type. Use `"simple"` for direct benchmarking
  without gtest fixtures, or `"fixture"` to generate a fixture-based
  template that includes gtest.

## Value

If successful, this function invisibly returns `TRUE` to allow for the
chaining of commands. If the function is unsuccessful, an error message
is returned.

Messages from the usethis package and cli indicate where the new file
was created and that the benchmark was registered in
`tests/google_benchmark/CMakeLists.txt`.

## Examples

``` r
if (FALSE) { # \dontrun{
# Create a new benchmark file and register it in CMake:
FIMS:::use_google_benchmark_template(
  name = "Population_CatchAtAge_CatchNumbersAtAge"
)

# Create a fixture-based benchmark template:
FIMS:::use_google_benchmark_template(
  name = "Population_CatchAtAge_CatchNumbersAtAge",
  type = "fixture"
)
} # }
```
