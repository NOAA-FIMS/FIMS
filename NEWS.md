# FIMS 0.7.1

* Fixes json output when there are non numeric values, infinity values, and -999
* Allows the output columns to be merged with reshape and dplyr when character
  values are combined with strings
* Fixes spawning biomass output in print method to report value

# FIMS 0.7.0

* Turns on the code for model families that allows for more model types in the
  future than just the catch-at-age model that is currently implemented through
  the use of FisheryModelBase, which is a unified parent class for all model
  families. Moved derived quantities (e.g., numbers at age, spawning biomass,
  catch at age) from Fleet and Population objects into FisheryModelBase.
  Refactored Population and Fleet classes to be trait only objects
* Converts nested list of lists used for model control to a tibble with one line
  per parameter that can be un-nested and nested.
* Increased the documentation of FIMS by using formulas in doxygen, building the
  doxygen on the NOAA-FIMS github.io site instead of its own web page, writing
  more information in the testing README, etc.
* Removes seasonality logic given it was not being used
* Removes date columns in input data in exchange for timing
* Uses {stockplotr} for the figures in the "fims-demo" vignette

# FIMS 0.6.3

* Fixes bugs in fims_math.hpp when using TMBad library to use the standard
  library functions (exp, log, cos, sqrt, pow) when necessary.

# FIMS 0.6.2

* Fixes bug where `unfished_biomass` was not initialized.
* Adds TMBad flag to windows Make file.

# FIMS 0.6.1

* Fixes FIMS log file where the user was not known on certain operating systems.

# FIMS 0.6.0

* Initializes backend code to support model families.
* Returns ungrouped tibble from `get_estimates()`.

# FIMS 0.5.3

* Fix json output to use full stop rather than underscore for names, a bug
  introduced in v0.5.0. Also fixes `reshape_tmb_estimates()` where the splits
  were incorrectly referenced because module_id comes before the label.

# FIMS 0.5.2

* Fixes constructor in ParameterVector to error if the size is too small.

# FIMS 0.5.1

* Fixes fix-pr-commands.yml permissions and to use the parent workflow.

# FIMS 0.5.0

* Implements random effects
* Adds fits tibble to output with `get_fits()`
* Switch to TMBad from CppAD
* Checks that length data has the correct dimension in R
* Adds SB to model output
* Adds README files to the C++ code
* Exports logit functions to R
* Adds capabilities in GitHub actions to document

# FIMS 0.4.0

* Allows for CPUE data and landings within a single fleet and changes the
  composition data to be separate from the catch. If there are catches, 
  compositions are based on F, if you have survey compositions those are scaled
  by q, and you can also have composition data without catch or CPUE and those
  match the population.
* Fixes bug in `finalize()` where variables were going out of scope by using a
  shared pointer and RealVector, which allows for estimates tibble to return
  more. Output is integrated between json and TMB using
  `reshape_tmb_estimates()`, `reshape_json_estimates()`, and
  `reshape_json_derived_quantities()`. ParameterVector are now set using
  `resize()` and `purrr::walk()` rather than `methods::new()`.
* Increases code coverage
  * Restructures R tests using `use_testthat_template()`
  * Starts to use Rcpp::Rcerr instead of Rcpp::Rcout
  * Skips parallel and distribution rcpp test
  * Adds `FIMS_dmultinom()` distribution to match TMB
* Fixes bug in `get_number_of_parameters()`.
* Uses static_cast<Type>([-0-9\.]+) to wrap constant variables.
* Fixes cmake_minimum_required to use a range and updates googletest version.
* Adds minimal fims-demo vignette.
* Removes unused estimated_log_M and estimated_log_init_naa from
  inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp.
* Adds gdb to the devcontainer.json file.
* Fixes broken model comparison project links.
* Adds dependabot.yml file to update GitHub actions.
* Combine `initialize_comp()` functions.

# FIMS 0.3.0.1

* Pluralism in cli was incorrect in FIMSFrame.
* Composition modules are initialized using the composition times the
  uncertainty, which was failing for -999 values because -999 * uncertainty
  does not equal -999 and was leading to the likelihood being evaluated.

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
    default is three, to restart the optimizer from the previous run of nlminb;
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
