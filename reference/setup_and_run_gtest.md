# Setup and run the google test suite

Intended for developers to set up their local environment and run the
google test suite from R.

## Usage

``` r
setup_and_run_gtest(...)
```

## Arguments

- ...:

  Additional arguments to `ctest --test-dir build` such as
  `"--rerun-failed --output-on-failure"` or `--parallel 16` if you want
  it to run on multiple cores.

## Examples

``` r
if (FALSE) { # \dontrun{
setup_and_run_gtest("--rerun-failed --output-on-failure")
} # }
```
