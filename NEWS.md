# FIMS 0.3.0.0

* Fits to length data using an age-to-length-conversion matrix, `data1`
  includes the necessary information needed to fit to both ages and lengths.
* Adds C++ ParameterVector to allow for the estimation of time-varying
  parameters.
* Implements R wrapper functions to facilitate
  * creating the input model specifications with `create_default_*()`,
    `update_parameters()`, and `initialize_*()`;
  * adding -999 to the missing fleet, year, age, length, etc. combinations;
  * running the model with a user-supplied argument of n_of_loops, where the
    default is three, to restart the optimizer from the previous run of nlmimb;
  * summarizing the output with the `FIMSFit()` function and class.
* Implements a switch for global verbosity within FIMS through the use
  of {cli} messages and warnings.
* Updates the logging system complete with a vignette about how to use it, the
  logging system can be used for both R and C++ errors, warnings, and
  information.
* Creates the initial infrastructure to implement random effects with density
  functions.
* Implements a helper function to get the parameter names from the
  C++ code and populate the results with those names.
* Makes lpdf_vec return 0 if data is missing.

# FIMS 0.2.0.0

* Added a `pkgdown` site for FIMS
* Add code cov tests and site link
* Remove recruitment bias correction and adjustment because we plan to use `TMB` (https://github.com/NOAA-FIMS/FIMS/issues/185)
* Improve logging coverage and separate logs into multiple files
* Put recruitment deviations on the log scale when input
* Report more values with standard errors
* Code cleanup, documentation, and consistency improvements
* Remove rapidjson dependence
* Add `start_year` and `end_year` as slots to FIMSFrame
* Make `log_obs_error` a vector
* Load the Rcpp module upon package install so components don't need to be prefaced by `fims$`
* Refactor vectors to use `fims::Vector` class
* Add Newton steps to FIMS integration test to improve convergence in integration test

# FIMS 0.1.0.0

* Added a `NEWS.md` file to track changes to the package.
