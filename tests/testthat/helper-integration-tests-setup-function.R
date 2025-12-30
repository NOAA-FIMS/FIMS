# Helper file for FIMS R tests----
# This file contains multiple functions that are used to set up and run
# FIMS models with or without wrapper functions. The functions are sourced by
# devtools::load_all().

# FIMS dmultinorm function ----
#' FIMS dmultinom()
#' This function matches the dmultinom() function in TMB and differs from R
#' by NOT rounding obs to the nearest integer. The function is evaluated in
#' log space and returns the log probability mass function.
#'
#' @param x A vector of length K of numeric values.
#' @param p A numeric non-negative vector of length K, specifying the probability
#' for the K classes; must sum 1.
#'
#' @return The log of the probability mass function for the multinomial.
FIMS_dmultinom <- function(x, p) {
  xp1 <- x + 1
  log_pmf <- lgamma(sum(x) + 1) - sum(lgamma(xp1)) + sum(x * log(p))
  return(log_pmf)
}


# FIMS helper function to run FIMS model without wrappers ----
#' Set up and run FIMS model without using wrapper functions
#'
#' This function sets up and runs the FIMS for a given iteration.
#' It configures the model with the OM inputs and outputs (see simulated data from
#' tests/testthat/fixtures/simulate-integration-test-data.R),
#' and runs the optimization process.
#' It then generates and returns the results including parameter estimates, model
#' reports, and standard deviation reports.
#'
#' @param iter_id An integer specifying the iteration ID to use for loading
#' the OM data.
#' @param om_input_list A list of OM inputs, where each element
#' corresponds to a different iteration.
#' @param om_output_list A list of OM outputs, where each element
#' corresponds to a different iteration.
#' @param em_input_list A list of EM inputs, where each element
#' corresponds to a different iteration.
#' @param estimation_mode A logical value indicating whether to perform
#' optimization (`TRUE`) or skip it (`FALSE`). If `TRUE`, the model parameters
#' will be optimized using `nlminb`. If `FALSE`, the input values will be used
#' for the report.
#' @param random_effects A logical value indicating whether to include random
#' effects in the model (`TRUE`) or skip it (`FALSE`). If `TRUE`, random effects
#' will be included on recruitment in the model.
#' @param map A list used to specify mapping for the `MakeADFun` function from
#' the TMB package.
#'
#' @return A list containing the following elements:
#' \itemize{
#'   \item{parameters: A list of parameters for the TMB model.}
#'   \item{obj: The TMB model object created by `TMB::MakeADFun`.}
#'   \item{opt: The result of the optimization process, if `estimation_mode`
#'   is `TRUE`. `NULL` if `estimation_mode` is `FALSE`.}
#'   \item{report: The model report obtained from the TMB model.}
#'   \item{sdr_report: Summary of the standard deviation report for the
#'   model parameters.}
#'   \item{sdr_fixed: Summary of the standard deviation report for the
#'   fixed parameters.}
#' }
#' @examples
#' results <- setup_and_run_FIMS_without_wrappers(
#'   iter_id = 1,
#'   om_input_list = om_input_list,
#'   om_output_list = om_output_list,
#'   em_input_list = em_input_list,
#'   estimation_mode = TRUE,
#'   random_effects = NULL
#' )
setup_and_run_FIMS_without_wrappers <- function(iter_id,
                                                om_input_list,
                                                om_output_list,
                                                em_input_list,
                                                estimation_mode = TRUE,
                                                random_effects = NULL,
                                                map = list()) {
  # Load operating model data for the current iteration
  om_input <- om_input_list[[iter_id]] # Operating model input for the current iteration
  om_output <- om_output_list[[iter_id]] # Operating model output for the current iteration
  em_input <- em_input_list[[iter_id]] # Estimation model input for the current iteration

  # Clear any previous FIMS settings
  clear()

  # Extract fishing fleet landings data (observed) and initialize index module
  landings <- em_input[["L.obs"]][["fleet1"]]

  # set fishing fleet landings data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery landings
  fishing_fleet_landings <- methods::new(Landings, om_input[["nyr"]])
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) fishing_fleet_landings$landings_data$set(x - 1, landings[x])
  )

  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*n_ages
  fishing_fleet_age_comp <- methods::new(AgeComp, om_input[["nyr"]], om_input[["nages"]])

  # Here we fill in the values for the object with the observed age comps for fleet one
  # we multiply these proportions by the sample size for likelihood weighting
  purrr::walk(
    1:(om_input[["nyr"]] * om_input[["nages"]]),
    \(x) fishing_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(em_input[["L.age.obs"]][["fleet1"]])) * em_input[["n.L"]][["fleet1"]])[x]
    )
  )

  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- methods::new(LengthComp, om_input[["nyr"]], om_input[["nlengths"]])
  purrr::walk(
    1:(om_input[["nyr"]] * om_input[["nlengths"]]),
    \(x) fishing_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(em_input[["L.length.obs"]][["fleet1"]])) * em_input[["n.L.lengthcomp"]][["fleet1"]])[x]
    )
  )
  # Fleet
  # Create the fishing fleet
  fishing_fleet_selectivity <- methods::new(LogisticSelectivity)
  fishing_fleet_selectivity$inflection_point[1]$value <- om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]]

  # turn on estimation of inflection_point
  fishing_fleet_selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  fishing_fleet_selectivity$slope[1]$value <- om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]]

  # turn on estimation of slope
  fishing_fleet_selectivity$slope[1]$estimation_type$set("fixed_effects")

  # Initialize the fishing fleet module
  fishing_fleet <- methods::new(Fleet)
  # Set number of years
  fishing_fleet$n_years$set(om_input[["nyr"]])
  # Set number of age classes
  fishing_fleet$n_ages$set(om_input[["nages"]])
  # Set number of length bins
  fishing_fleet$n_lengths$set(om_input[["nlengths"]])

  fishing_fleet$log_Fmort$resize(om_input[["nyr"]])
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    fishing_fleet$log_Fmort[y]$value <- log(om_output[["f"]][y])
  }
  fishing_fleet$log_Fmort$set_all_estimable(TRUE)
  fishing_fleet$log_q[1]$value <- log(1.0)
  fishing_fleet$log_q[1]$estimation_type$set("constant")
  fishing_fleet$SetSelectivityID(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedLandingsDataID(fishing_fleet_landings$get_id())
  fishing_fleet$SetObservedAgeCompDataID(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetObservedLengthCompDataID(fishing_fleet_length_comp$get_id())

  # Set up fishery index data using the lognormal
  fishing_fleet_landings_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_landings_distribution$log_sd$resize(om_input[["nyr"]])
  for (y in 1:om_input[["nyr"]]) {
    # Compute lognormal SD from OM coefficient of variation (CV)
    fishing_fleet_landings_distribution$log_sd[y]$value <- log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1)))
  }
  fishing_fleet_landings_distribution$log_sd$set_all_estimable(FALSE)
  # Set Data using the IDs from the modules defined above
  fishing_fleet_landings_distribution$set_observed_data(fishing_fleet$GetObservedLandingsDataID())
  fishing_fleet_landings_distribution$set_distribution_links("data", fishing_fleet$log_landings_expected$get_id())

  # Set up fishery age composition data using the multinomial
  fishing_fleet_agecomp_distribution <- methods::new(DmultinomDistribution)
  fishing_fleet_agecomp_distribution$set_observed_data(fishing_fleet$GetObservedAgeCompDataID())
  fishing_fleet_agecomp_distribution$set_distribution_links("data", fishing_fleet$agecomp_proportion$get_id())

  # Set up fishery length composition data using the multinomial
  fishing_fleet_lengthcomp_distribution <- methods::new(DmultinomDistribution)
  fishing_fleet_lengthcomp_distribution$set_observed_data(fishing_fleet$GetObservedLengthCompDataID())
  fishing_fleet_lengthcomp_distribution$set_distribution_links("data", fishing_fleet$lengthcomp_proportion$get_id())
  fishing_fleet_lengthcomp_distribution$set_note("fishing_fleet_lengthcomp_distribution")
  # Set age-to-length conversion matrix
  # TODO: If an age_to_length_conversion matrix is provided, the code below
  # still executes. Consider adding a check in the Rcpp interface to ensure
  # users provide a vector of inputs.
  fishing_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    fishing_fleet$age_to_length_conversion[i]$value <- c(t(em_input[["age_to_length_conversion"]]))[i]
  }

  # Turn off estimation for length-at-age
  fishing_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  fishing_fleet$age_to_length_conversion$set_all_random(FALSE)

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- methods::new(Index, om_input[["nyr"]])
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) survey_fleet_index$index_data$set(x - 1, survey_index[x])
  )

  survey_fleet_age_comp <- methods::new(AgeComp, om_input[["nyr"]], om_input[["nages"]])
  purrr::walk(
    1:(om_input[["nyr"]] * om_input[["nages"]]),
    \(x) survey_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(em_input[["survey.age.obs"]][["survey1"]])) * em_input[["n.survey"]][["survey1"]])[x]
    )
  )

  survey_lengthcomp <- em_input[["survey.length.obs"]][["survey1"]]
  survey_fleet_length_comp <- methods::new(LengthComp, om_input[["nyr"]], om_input[["nlengths"]])
  purrr::walk(
    1:(om_input[["nyr"]] * om_input[["nlengths"]]),
    \(x) survey_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(survey_lengthcomp)) * em_input[["n.survey.lengthcomp"]][["survey1"]])[x]
    )
  )
  # Fleet
  # Create the survey fleet
  survey_fleet_selectivity <- methods::new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point[1]$value <- om_input[["sel_survey"]][["survey1"]][["A50.sel1"]]

  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  survey_fleet_selectivity$slope[1]$value <- om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]

  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimation_type$set("fixed_effects")

  survey_fleet <- methods::new(Fleet)
  survey_fleet$n_ages$set(om_input[["nages"]])
  survey_fleet$n_years$set(om_input[["nyr"]])
  survey_fleet$n_lengths$set(om_input[["nlengths"]])
  survey_fleet$log_Fmort$resize(om_input[["nyr"]])
  for (y in 1:om_input$nyr) {
    # Set very low survey fishing mortality
    survey_fleet$log_Fmort[y]$value <- -200
  }
  survey_fleet$log_Fmort$set_all_estimable(FALSE)
  survey_fleet$log_q[1]$value <- log(om_output[["survey_q"]][["survey1"]])
  survey_fleet$log_q[1]$estimation_type$set("fixed_effects")
  survey_fleet$SetSelectivityID(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexDataID(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompDataID(survey_fleet_age_comp$get_id())
  survey_fleet$SetObservedLengthCompDataID(survey_fleet_length_comp$get_id())

  # Set up survey index data using the lognormal
  survey_fleet_index_distribution <- methods::new(DlnormDistribution)

  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution$log_sd$resize(om_input[["nyr"]])
  for (y in 1:om_input$nyr) {
    survey_fleet_index_distribution$log_sd[y]$value <- log(sqrt(log(em_input[["cv.survey"]][["survey1"]]^2 + 1)))
  }
  survey_fleet_index_distribution$log_sd$set_all_estimable(FALSE)
  # Set Data using the IDs from the modules defined above
  survey_fleet_index_distribution$set_observed_data(survey_fleet$GetObservedIndexDataID())
  survey_fleet_index_distribution$set_distribution_links("data", survey_fleet$log_index_expected$get_id())

  # Age composition distribution
  survey_fleet_agecomp_distribution <- methods::new(DmultinomDistribution)
  survey_fleet_agecomp_distribution$set_observed_data(survey_fleet$GetObservedAgeCompDataID())
  survey_fleet_agecomp_distribution$set_distribution_links("data", survey_fleet$agecomp_proportion$get_id())

  # Length composition distribution
  survey_fleet_lengthcomp_distribution <- methods::new(DmultinomDistribution)
  survey_fleet_lengthcomp_distribution$set_observed_data(survey_fleet$GetObservedLengthCompDataID())
  survey_fleet_lengthcomp_distribution$set_distribution_links("data", survey_fleet$lengthcomp_proportion$get_id()) # Set age to length conversion matrix
  survey_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  # TODO: Check that the dimensions of the matrix of age_to_length_conversion matrix
  #       is rows = length() and columns = length()
  for (i in seq_along(em_input[["age_to_length_conversion"]])) {
    # Transposing the below will have NO impact on the results if the object is
    # already a vector. Additionally, c() ensures that the result is a vector
    # to be consistent but a matrix would be okay.
    # TODO: write a test/documentation to show what order the matrix needs to be
    #       in when passing data to age-length-conversion
    survey_fleet$age_to_length_conversion[i]$value <- c(t(em_input[["age_to_length_conversion"]]))[i]
  }
  # Turn off estimation for length-at-age
  survey_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  survey_fleet$age_to_length_conversion$set_all_random(FALSE)

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- methods::new(BevertonHoltRecruitment)
  if (is.null(random_effects) || random_effects[["recruitment"]] == "log_devs") {
    recruitment_process <- new(LogDevsRecruitmentProcess)
  } else {
    recruitment_process <- new(LogRRecruitmentProcess)
  }
  recruitment$SetRecruitmentProcessID(recruitment_process$get_id())

  # NOTE: in first set of parameters below (for recruitment),
  # $estimation_type (default is "constant")
  # is defined even if it matches the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- log(om_input[["R0"]])
  recruitment$log_rzero[1]$estimation_type$set("fixed_effects")
  # set up logit_steep
  recruitment$logit_steep[1]$value <- -log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2)
  recruitment$logit_steep[1]$estimation_type$set("constant")
  recruitment$n_years$set(om_input[["nyr"]])

  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))


  if (is.null(random_effects) || random_effects[["recruitment"]] == "log_devs") {
    recruitment$log_devs$resize(om_input[["nyr"]] - 1)
    for (y in 1:(om_input[["nyr"]] - 1)) {
      recruitment$log_devs[y]$value <- om_input[["logR.resid"]][y + 1]
    }
  }
  if ("recruitment" %in% names(random_effects)) {
    if (random_effects[["recruitment"]] == "log_devs") {
      recruitment$log_devs$set_all_random(TRUE)
    }
    if (random_effects[["recruitment"]] == "log_r") {
      recruitment$log_r$resize(om_input[["nyr"]] - 1)
      for (y in 1:(om_input[["nyr"]] - 1)) {
        recruitment$log_r[y]$value <- 1
      }
      recruitment$log_r$set_all_random(TRUE)
    }
  }
  if (is.null(random_effects)) {
    # TODO: integration tests fail after setting recruitment log_devs all estimable.
    # We need to debug the issue, then uncomment the line below.
    # recruitment$log_devs$set_all_estimable(TRUE)
  }

  if ("selectivity" %in% names(random_effects)) {
    if (random_effects[["selectivity"]] == "log_devs") {
      fishing_fleet_selectivity$log_devs$set_all_random(TRUE)
      survey_fleet_selectivity$log_devs$set_all_random(TRUE)
    }
    if (random_effects[["selectivity"]] == "log_sel") {
      fishing_fleet_selectivity$log_sel$set_all_random(TRUE)
      survey_fleet_selectivity$log_sel$set_all_random(TRUE)
    }
    if (random_effects[["selectivity"]] == "pars") {
      fishing_fleet_selectivity$inflection_point$estimation_type$set("random_effects")
      fishing_fleet_selectivity$inflection_point$slope <- "random_effects"
      survey_fleet_selectivity$inflection_point$estimation_type$set("random_effects")
      survey_fleet_selectivity$inflection_point$slope <- "random_effects"
    }
  }
  recruitment_distribution <- methods::new(DnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd$resize(1)
  recruitment_distribution$log_sd[1]$value <- log(om_input[["logR_sd"]])
  recruitment_distribution$x$resize(om_input[["nyr"]] - 1)
  recruitment_distribution$expected_values$resize(om_input[["nyr"]] - 1)
  for (i in 1:(om_input[["nyr"]] - 1)) {
    recruitment_distribution$x[i]$value <- 0
    recruitment_distribution$expected_values[i]$value <- 0
  }
  if ("recruitment" %in% names(random_effects)) {
    if (random_effects[["recruitment"]] == "log_devs") {
      recruitment_distribution$log_sd[1]$estimation_type$set("fixed_effects")
      recruitment_distribution$set_distribution_links("random_effects", recruitment$log_devs$get_id())
    }
    if (random_effects[["recruitment"]] == "log_r") {
      recruitment_distribution$log_sd[1]$value <- log(1)
      recruitment_distribution$log_sd[1]$estimation_type$set("fixed_effects")
      recruitment_distribution$set_distribution_links("random_effects", c(recruitment$log_r$get_id(), recruitment$log_expected_recruitment$get_id()))
    }
  }

  if (is.null(random_effects)) {
    recruitment_distribution$set_distribution_links("random_effects", recruitment$log_devs$get_id())
  }

  # Growth
  ewaa_growth <- methods::new(EWAAGrowth)
  ewaa_growth$ages$resize(om_input[["nages"]])
  purrr::walk(
    seq_along(om_input[["ages"]]),
    \(x) ewaa_growth$ages$set(x - 1, om_input[["ages"]][x])
  )
  ewaa_growth$weights$resize(om_input[["nages"]])
  purrr::walk(
    seq_along(om_input[["W.mt"]]),
    \(x) ewaa_growth$weights$set(x - 1, om_input[["W.mt"]][x])
  )

  # Maturity
  maturity <- methods::new(LogisticMaturity)
  maturity$inflection_point[1]$value <- om_input[["A50.mat"]]
  maturity$inflection_point[1]$estimation_type$set("constant")
  maturity$slope[1]$value <- om_input[["slope.mat"]]
  maturity$slope[1]$estimation_type$set("constant")

  # Population
  population <- methods::new(Population)
  population$log_M$resize(om_input[["nyr"]] * om_input[["nages"]])
  for (i in 1:(om_input[["nyr"]] * om_input[["nages"]])) {
    population$log_M[i]$value <- log(om_input[["M.age"]][1])
  }
  population$log_M$set_all_estimable(FALSE)
  population$log_init_naa$resize(om_input[["nages"]])
  for (i in 1:om_input$nages) {
    population$log_init_naa[i]$value <- log(om_output[["N.age"]][1, i])
  }
  population$log_init_naa$set_all_estimable(TRUE)
  population$n_ages$set(om_input[["nages"]])
  population$ages$resize(om_input[["nages"]])
  purrr::walk(
    seq_along(om_input[["ages"]]),
    \(x) population$ages$set(x - 1, om_input[["ages"]][x])
  )
  population$n_fleets$set(sum(om_input[["fleet_num"]], om_input[["survey_num"]]))
  population$n_years$set(om_input[["nyr"]])
  population$SetRecruitmentID(recruitment$get_id())
  population$SetGrowthID(ewaa_growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$AddFleet(fishing_fleet$get_id())
  population$AddFleet(survey_fleet$get_id())

  # Set up catch at age model
  caa <- methods::new(CatchAtAge)
  caa$AddPopulation(population$get_id())

  # Set-up TMB
  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(
    p = get_fixed(),
    re = get_random()
  )
  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE, map = map, random = "re"
  )

  # Optimization with nlminb
  opt <- NULL
  if (estimation_mode == TRUE) {
    opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
      control = list(eval.max = 10000, iter.max = 10000, trace = 0)
    )
    FIMS::set_fixed(opt$par)
    fims_finalized <- caa$get_output(do_sd_report = estimation_mode)
  }

  # Call report using MLE parameter values, or
  # the input values if optimization is skipped
  report <- obj[["report"]](obj[["env"]][["last.par.best"]])


  sdr <- TMB::sdreport(obj)
  sdr_report <- summary(sdr, "report")
  sdr_fixed <- summary(sdr, "fixed")
  row.names(sdr_fixed) <- names(FIMS:::get_parameter_names(sdr_fixed[, 1]))

  clear()

  # Return the results as a list
  return(list(
    parameters = parameters,
    obj = obj,
    opt = opt,
    report = report,
    sdr_report = sdr_report,
    sdr_fixed = sdr_fixed,
    sdr = sdr
  ))
}

# FIMS helper function to run FIMS model with wrappers ----
#' Set Up and Run FIMS Model using wrapper functions
#'
#' This function sets up and runs the FIMS for a given iteration.
#' It configures the model with the OM inputs and outputs (see simulated data from
#' tests/testthat/fixtures/simulate-integration-test-data.R),
#' and runs the optimization process.
#' It then generates and returns the results including parameter estimates, model
#' reports, and standard deviation reports.
#'
#' @param iter_id An integer specifying the iteration ID to use for loading
#' the OM data.
#' @param om_input_list A list of OM inputs, where each element
#' corresponds to a different iteration.
#' @param om_output_list A list of OM outputs, where each element
#' corresponds to a different iteration.
#' @param em_input_list A list of EM inputs, where each element
#' corresponds to a different iteration.
#' @param estimation_mode A logical value indicating whether to perform
#' optimization (`TRUE`) or skip it (`FALSE`). If `TRUE`, the model parameters
#' will be optimized using `nlminb`. If `FALSE`, the input values will be used
#' for the report.
#' @param random_effects A logical value indicating whether to include random
#' effects in the model (`TRUE`) or skip it (`FALSE`). If `TRUE`, random effects
#' will be included on recruitment in the model.
#' @param map A list used to specify mapping for the `MakeADFun` function from
#' the TMB package.
#'
#' @return A list containing the following elements:
#' \itemize{
#'   \item{parameters: A list of parameters for the TMB model.}
#'   \item{obj: The TMB model object created by `TMB::MakeADFun`.}
#'   \item{opt: The result of the optimization process, if `estimation_mode`
#'   is `TRUE`. `NULL` if `estimation_mode` is `FALSE`.}
#'   \item{report: The model report obtained from the TMB model.}
#'   \item{sdr_report: Summary of the standard deviation report for the
#'   model parameters.}
#'   \item{sdr_fixed: Summary of the standard deviation report for the
#'   fixed parameters.}
#' }
#' @examples
#' results <- setup_and_run_FIMS_with_wrappers(
#'   iter_id = 1,
#'   om_input_list = om_input_list,
#'   om_output_list = om_output_list,
#'   em_input_list = em_input_list,
#'   estimation_mode = TRUE
#' )
setup_and_run_FIMS_with_wrappers <- function(iter_id,
                                             om_input_list,
                                             om_output_list,
                                             em_input_list,
                                             estimation_mode = TRUE,
                                             random_effects = FALSE,
                                             modified_parameters,
                                             map = list()) {
  # Load operating model data for the current iteration
  om_input <- om_input_list[[iter_id]]
  om_output <- om_output_list[[iter_id]]
  em_input <- em_input_list[[iter_id]]

  # Clear any previous FIMS settings
  clear()

  data <- FIMS::FIMSFrame(data1)
  parameters <- modified_parameters

  # The model will not always run when log_q is very small.
  # We will need to make sure log_q is the true value for deterministic runs but
  # then reset to log(1.0) for estimation runs.
  if (estimation_mode == TRUE) {
    parameters <- parameters |>
      dplyr::mutate(
        value = dplyr::if_else(
          fleet_name == "survey1" & label == "log_q",
          log(1.0),
          value
        )
      )
  }

  parameter_list <- initialize_fims(
    parameters = parameters,
    data = data
  )

  fit <- fit_fims(input = parameter_list, optimize = estimation_mode)

  clear()
  # Return the results as a list
  return(fit)
}
