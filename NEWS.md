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
