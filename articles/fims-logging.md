# Introducing the FIMS Logging System

## Importance of Logging

Logging in FIMS is important because it allows developers to understand
what is happening within a model. It is especially useful when problems
arise. The FIMS logging system has been designed to handle common
issues, such as undefined modules, dimension issues, and software errors
that may occur when a model is not properly defined. This logging system
is accessible from both R and C++ and messages are provided using a JSON
format.

## How FIMS Logging works

At run time, FIMS records log entries in C++ and exposes them to R as
JSON strings. From R, you can inspect all entries
([`get_log()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)),
or only warnings and errors
([`get_log_warnings()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md),
[`get_log_errors()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)).

For implementation details (log schema, severity macros, write-on-exit
behavior, and signal handling), see the following resources:

- The source file:
  [inst/include/common/def.hpp](https://github.com/NOAA-FIMS/FIMS/blob/main/inst/include/common/def.hpp)
- The
  [documentation](https://noaa-fims.github.io/FIMS/doxygen/classfims_1_1FIMSLog.html)
  for the same header

The log is initiated when
[`CreateTMBModel()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md)
is run, where the first entry is always the call to this function, e.g.,

``` r

empty_model <- CreateTMBModel()
log_string <- get_log()
log_data_frame <- jsonlite::fromJSON(log_string)
log_data_frame
```

    ##                  timestamp level
    ## 1 Wed Jul  1 12:49:28 2026  info
    ## 2 Wed Jul  1 12:49:28 2026  info
    ## 3 Wed Jul  1 12:49:28 2026  info
    ## 4 Wed Jul  1 12:49:28 2026  info
    ##                                                  message id   user
    ## 1 Creating model and checking for required components...  0 runner
    ## 2                            Model successfully created.  1 runner
    ## 3 Creating model and checking for required components...  2 runner
    ## 4                            Model successfully created.  3 runner
    ##                                      wd
    ## 1 /home/runner/work/FIMS/FIMS/vignettes
    ## 2 /home/runner/work/FIMS/FIMS/vignettes
    ## 3 /home/runner/work/FIMS/FIMS/vignettes
    ## 4 /home/runner/work/FIMS/FIMS/vignettes
    ##                                                              file
    ## 1 /home/runner/work/FIMS/FIMS/inst/include/common/information.hpp
    ## 2 /home/runner/work/FIMS/FIMS/inst/include/common/information.hpp
    ## 3 /home/runner/work/FIMS/FIMS/inst/include/common/information.hpp
    ## 4 /home/runner/work/FIMS/FIMS/inst/include/common/information.hpp
    ##                                                                                routine
    ## 1                bool fims_info::Information<Type>::CreateModel() [with Type = double]
    ## 2                bool fims_info::Information<Type>::CreateModel() [with Type = double]
    ## 3 bool fims_info::Information<Type>::CreateModel() [with Type = TMBad::global::ad_aug]
    ## 4 bool fims_info::Information<Type>::CreateModel() [with Type = TMBad::global::ad_aug]
    ##   line
    ## 1  825
    ## 2  842
    ## 3  825
    ## 4  842

``` r

clear()
```

This particular log entry originates from the use of `FIMS_INFO_LOG()`
inside of
[`CreateTMBModel()`](https://NOAA-FIMS.github.io/FIMS/reference/Cpp_functions.md),
see
[inst/include/interface/rcpp/rcpp_interface.hpp](https://github.com/NOAA-FIMS/FIMS/blob/main/inst/include/interface/rcpp/rcpp_interface.hpp).
In the example, the entry for the “level” column is `"info"` because
`FIMS_INFO_LOG()` was called, whereas if `FIMS_WARNING_LOG()` was called
instead, then the “level” entry would be `"warning"`. The log entry also
specifies the C++ file where the log entry originated from.
Additionally, the “routine” and “line” columns specify the function and
line, respectively, within the file that led to the creation of the log
entry.

See the [introduction to FIMS
vignette](https://NOAA-FIMS.github.io/FIMS/articles/fims-demo.md) for a
log of a complete model run.

## Using the Logging System in R

The FIMS Logging System is also available from R, with a caveat! Logging
from R gives less information than logging from C++. When a log entry
originates from R, file, routine, and line information are absent.
Further implementations may rectify this issue. Below are examples of
adding log entries from R.

``` r

log_info("info entry from R script")
log_warning("warning entry from R script")
jsonlite::fromJSON(get_log())
```

    ##                  timestamp   level                     message id   user
    ## 1 Wed Jul  1 12:49:28 2026    info    info entry from R script  0 runner
    ## 2 Wed Jul  1 12:49:28 2026 warning warning entry from R script  1 runner
    ##                                      wd
    ## 1 /home/runner/work/FIMS/FIMS/vignettes
    ## 2 /home/runner/work/FIMS/FIMS/vignettes
    ##                                          file        routine line
    ## 1 /home/runner/work/FIMS/FIMS/vignettes/R_env R_script_entry   -1
    ## 2 /home/runner/work/FIMS/FIMS/vignettes/R_env R_script_entry   -1

``` r

clear()
```

The example above just logs information and warnings but you can also
log errors. Notice that the “routine” information in the error log is
not super informative even though it is filled with a lot of
information. The R stack trace in this column is to large to capture the
root cause of the error. For that reason, it’s advised to make the error
message as detailed as possible when calling `log_error(x)` from R. Note
that the formatting is better when written to a file rather than printed
to the screen so the following code chunk is not rendered here but you
can on your own computer.

``` r

error <- log_error("error entry from R script")
jsonlite::fromJSON(get_log_errors())
```

There are several logging functions that are exported to R. Below is a
list of all of the functions as well as an example of how you can find
out more information about each of them. Note that because these are C++
functions that are exported to R, we have chosen to document them in the
C++ code using doxygen and just provide links to that documentation when
users call [`methods::show()`](https://rdrr.io/r/methods/show.html) on
them.

``` r

ls("package:FIMS") |>
  grep(pattern = "_log|log_", value = TRUE) |>
  cli::cli_bullets()
```

    ## get_log

    ## get_log_errors

    ## get_log_warnings

    ## inv_logit

    ## log_error

    ## log_info

    ## log_warning

    ## set_log_throw_on_error

``` r

# Get documentation for log_error
methods::show(log_error)
```

    ## internal C++ function <0x55e47111abe0>
    ##     docstring : See https://noaa-fims.github.io/FIMS/doxygen/rcpp__interface_8hpp.html.
    ##     signature : void log_error(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >)

Another useful option is `set_throw_on_error()`. If this option is set
to TRUE, FIMS will abort immediately when an error occurs and if
`write_log(TRUE)` was previously called, the log file will be written
before FIMS terminates the session.

``` r

set_log_throw_on_error(TRUE)
log_error("throwing now!")
```
