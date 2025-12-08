# Run the google test suite

Intended for developers to run the google test suite from R.

## Usage

``` r
run_gtest(...)
```

## Arguments

- ...:

  Additional arguments to `ctest --test-dir build` such as
  `"--rerun-failed --output-on-failure"` or `--parallel 16` if you want
  it to run on multiple cores.

## Details

The output from running the tests should look something like the
following:

    Internal ctest changing into directory: C:/github/NOAA-FIMS/FIMS/build
    Test project C:/github_repos/NOAA-FIMS_org/FIMS/build
        Start 1: dlognorm.use_double_inputs
    1/5 Test #1: dlognorm.use_double_inputs .......   Passed    0.04 sec
        Start 2: dlognorm.use_int_inputs
    2/5 Test #2: dlognorm.use_int_inputs ..........   Passed    0.04 sec
        Start 3: modelTest.eta
    3/5 Test #3: modelTest.eta ....................   Passed    0.04 sec
        Start 4: modelTest.nll
    4/5 Test #4: modelTest.nll ....................   Passed    0.04 sec
        Start 5: modelTest.evaluate
    5/5 Test #5: modelTest.evaluate ...............   Passed    0.04 sec

    100% tests passed, 0 tests failed out of 5

If a test fails, the output will look something like the following:

    Internal ctest changing into directory: C:/github/NOAA-FIMS/FIMS/build
    Test project C:/github/NOAA-FIMS/FIMS/build
        Start 1: dlognorm.use_double_inputs
    1/7 Test #1: dlognorm.use_double_inputs .......   Passed    0.04 sec
        Start 2: dlognorm.use_int_inputs
    2/7 Test #2: dlognorm.use_int_inputs ..........   Passed    0.04 sec
        Start 3: modelTest.eta
    3/7 Test #3: modelTest.eta ....................   Passed    0.04 sec
        Start 4: modelTest.nll
    4/7 Test #4: modelTest.nll ....................   Passed    0.04 sec
        Start 5: modelTest.evaluate
    5/7 Test #5: modelTest.evaluate ...............   Passed    0.04 sec
        Start 6: dlognormTest.DoubleInput
    6/7 Test #6: dlognormTest.DoubleInput .........   Passed    0.04 sec
        Start 7: dlognormTest.IntInput
    7/7 Test #7: dlognormTest.IntInput ............***Failed    0.04 sec

    86% tests passed, 1 tests failed out of 7

    Total Test time (real) =   0.28 sec

    The following tests FAILED:
              7 - dlognormTest.IntInput (Failed)
    Errors while running CTest
    Output from these tests are in: C:/github/NOAA-FIMS/FIMS/build/Testing/Temporary/LastTest.log
    Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.
