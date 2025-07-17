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
#' will be optimized using `nlminb`. If `FALSE`, the initial values will be used
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
  # Here the new function initializes the object with length nyr*nages
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
  fishing_fleet$nyears$set(om_input[["nyr"]])
  # Set number of age classes
  fishing_fleet$nages$set(om_input[["nages"]])
  # Set number of length bins
  fishing_fleet$nlengths$set(om_input[["nlengths"]])

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
  survey_fleet$nages$set(om_input[["nages"]])
  survey_fleet$nyears$set(om_input[["nyr"]])
  survey_fleet$nlengths$set(om_input[["nlengths"]])
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
  recruitment$nyears$set(om_input[["nyr"]])

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
  ewaa_growth <- methods::new(EWAAgrowth)
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
  population$nages$set(om_input[["nages"]])
  population$ages$resize(om_input[["nages"]])
  purrr::walk(
    seq_along(om_input[["ages"]]),
    \(x) population$ages$set(x - 1, om_input[["ages"]][x])
  )
  population$nfleets$set(sum(om_input[["fleet_num"]], om_input[["survey_num"]]))
  population$nseasons$set(1)
  population$nyears$set(om_input[["nyr"]])
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
  }

  # Call report using MLE parameter values, or
  # the initial values if optimization is skipped
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
#' will be optimized using `nlminb`. If `FALSE`, the initial values will be used
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

  # Set up default parameters
  fleets <- list(
    fleet1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Landings = "DlnormDistribution",
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    ),
    survey1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Landings = "DlnormDistribution",
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    )
  )

  default_parameters <- data |>
    create_default_parameters(
      fleets = fleets,
      recruitment = list(
        form = "BevertonHoltRecruitment",
        process_distribution = c(log_devs = "DnormDistribution")
      ),
      growth = list(form = "EWAAgrowth"),
      maturity = list(form = "LogisticMaturity")
    )

  parameters <- default_parameters |>
    update_parameters(
      modified_parameters = modified_parameters[[iter_id]]
    )

  # The model will not always run when log_q is very small.
  # We will need to make sure log_q is the true value for deterministic runs but
  # then reset to log(1.0) for estimation runs.
  if (estimation_mode == TRUE) {
    parameters <- parameters |>
      update_parameters(
        modified_parameters = list(
          survey1 = list(
            Fleet.log_q.value = log(1.0)
          )
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

setup_and_run_sp <- function(estimation_mode = TRUE,
                             map = list()) {

  
# #   # path to download the surplus prooduction test data
#   path_sp_data <- test_path("fixtures", "integration_test_data_sp.RData")
#   download.file(
#     "https://github.com/iagomosqueira/simtest_SP/raw/main/data/sims.RData",
#     path_sp_data
#   )
#   # remotes::install_github("flr/FLCore")
#   #load om object
#   load(path_sp_data)

#   # Simulated data and other information (e.g., number of iteration, end year, and start year)
#   args <- list(it = dim(om)[6], ay = 2020, y0 = 1951)
#   tracking <- FLCore::FLQuant(dimnames = list(
#     metric = "conv.est",
#     year = 1951:2020,
#     iter = seq(args$it)
#   ))
#   # OM
#   rom <- FLCore::FLQuants(
#     B = tsb(om),
#     Bstatus = tsb(om) / refpts(om)$Btgt,
#     Bdepletion = tsb(om) / refpts(om)$B0
#   )
#   res <- lapply(
#     setNames(names(rom), nm = c("B", "B/B[MSY]", "B/B[0]")),
#     function(x) FLCore::FLQuants(lapply(list(rom), "[[", x))
#   )
  
#   # Life history parameters from FishLife (https://github.com/James-Thorson-NOAA/FishLife)
#   # More examples can be found at https://github.com/iagomosqueira/simtest_SP/blob/main/data.R
#   # remotes::install_github("James-Thorson-NOAA/FishLife")
#   # remotes::install_github("Henning-Winker/SPMpriors")
#   # remotes::install_github( 'ropensci/rfishbase@fb-21.06', force=TRUE )
# library(FishLife)
# alb <- SPMpriors::flmvn_traits(
#     Genus="Thunnus", 
#     Species="alalunga",
#     h=c(0.6,0.9), 
#     Plot=FALSE
#   )$traits

#   lh <- list(
#     linf = alb[alb$trait=="Loo","mu.stk"],
#     k =  alb[alb$trait=="K","mu.stk"],
#     t0 = -0.5,
#     tm = alb[alb$trait=="tm","mu.stk"],
#     a = 0.00001,
#     b = 3.04,
#     tmax =  ceiling(alb[alb$trait=="tmax","mu.stk"]),
#     s = alb[alb$trait=="h","mu.stk"]
#   )

#   # Load om survey indices
#   om_survey_indices <- lapply(FLCore::FLIndices(A = idx), index)
#   om_survey_indices_se <- rep(0.2, length(FLCore::FLIndices(A = idx)))
#   # Load om landings data
#   om_landings <- FLCore::catch(om)
#   # HANDLE NAs in catch
#   om_landings[is.na(om_landings)] <- 0

#   # Extract args and SET dims
#   # end year: 2020
#   ay <- args$ay
#   # number of iterations: 100
#   it <- args$it
#   # start year:1951
#   y0 <- args$y0
#   # number of years:70
#   ny <- dim(om_landings)[2]

#   # Create ouput
#   empty <- om_landings %=% 0
#   out <- list(
#     ind = FLQuants(lapply(
#       setNames(nm = c("F", "Fstatus", "B", "Bstatus", "Bdepletion")),
#       function(x) propagate(empty, it)
#     )),
#     rps = FLPar(NA, dimnames = list(
#       params = c("FMSY", "BMSY", "MSY", "K", "B0"),
#       iter = seq(it)
#     ), units = c("f", "t", "t", "t", "t")),
#     conv = rep(NA, it)
#   )

#   # Extract the 1st iter landings data for the fishing fleet
#   iter_id <- 1
#   fishing_fleet_landings <- as.data.frame(
#     iter(om_landings, iter_id),
#     drop = TRUE
#   )
#   survey_fleet_index <- model.frame(
#     window(iter(om_survey_indices, iter_id), start = y0),
#     drop = TRUE
#   )
#   se <- survey_fleet_index
#   # Assign indx.se
#   se[, -1] <- as.list(om_survey_indices_se)

#   # JABBA (needs to install JABBA, rjags, and R2jags)
#   # need to run the following in ubuntu before installing rjags:
#   # sudo apt-get update
#   # sudo apt-get install -y jags
#   # install.packages("rjags")
#   # install.packages("R2jags")
#   # remotes::install_github("jabbamodel/JABBA")
#   # TODO: It appears the model is run using default priors, what are these?
  
#   inpput <- JABBA::build_jabba(
#     catch = fishing_fleet_landings,
#     cpue = survey_fleet_index,
#     se = se,
#     assessment = "STK",
#     scenario = "jabba.sa",
#     model.type = "Schaefer",
#     sigma.est = TRUE,
#     fixed.obsE = 0.05,
#     verbose = FALSE
#   )
#   JABBA::mp_jabba(input)

# Simulate data

# library(FishLife)
# library(SPMpriors)
# library(FLRef)
# library(FLife)
# library(patchwork)


# # --- Life history

# # A set of parameters are sampled from FishLife

# alb <- flmvn_traits(Genus="Thunnus", Species="alalunga",
#   h=c(0.6,0.9), Plot=FALSE)$traits

# # Other parameters are specified

# lh <- list(linf = alb[alb$trait=="Loo","mu.stk"],
#   k =  alb[alb$trait=="K","mu.stk"],
#   t0 = -0.5,
#   tm = alb[alb$trait=="tm","mu.stk"],
#   a = 0.00001,
#   b = 3.04,
#   tmax =  ceiling(alb[alb$trait=="tmax","mu.stk"]),
#   s = alb[alb$trait=="h","mu.stk"])

# # Check the corresponding Von Bertalanffy growth curve

# lena <- with(lh, vonbert(linf, k, t0, age=0:tmax))

# # M using Then (), with tmax1 = 22 and tmax2 = 12

# lh$M <- 4.899 * lh$tmax ^- 0.916

# #  Leslie matrix `r`

# bio <- with(lh, jbio(amax=tmax, nsexes=1, Loo=linf, k=k, t0=t0, aW=1000*a, bW=b,
#   mat=c(tm, tm*1.2, 1), M=M, h=s))

# r <- jbleslie(bio)$r


# # --- Generic OM for ALB

# # Life-history parameter set com,pleted by lhPar

# par <- with(lh, lhPar(FLPar(linf=linf, k=k, t0=t0, a=a, b=b, a50=2, s=s, m1=M)))

# # Constructor a basic `FLBRP()` (equilibrium) object.

# eql <- lhEql(par, range = c(min = 0, max = lh$tmax,
#   minfbar = 1, maxfbar = lh$tmax-1, plusgroup=lh$tmax))

# # Adjusts the inbuilt Gislason M to a scaled Lorenzen $M$

# m(eql)[] <- lorenzen(stock.wt(eql), Mref=lh$M, Aref=2)

#  # Specifying a new maturity ogive

# mat(eql) <- newselex(mat(eql), FLPar(S50=lh$tm, mat95=lh$tm * 1.5, Smax=1000,
#   Dcv=0.5,Dmin=0.1))

# # Updates the stock recruitment steepness if biological parameters are modified

# params(eql)

# eql <- updsr(eql, s=lh$s) 

# params(eql)

# # Time horizon can be adjusted with `fbar` range, 1981-2020

# f0 <- 0.01

# fbar(eql) <- FLQuant(f0, dimnames=list(year=1951:2020))

# # Convert FLBRP into FLStock + SRR

# stk <- as(eql,"FLStock")
# units(stk) <- standardUnits(stk)

# srr <- as(as(eql,"predictModel"),"FLSR")

# # Set new selectivity parameters

# selpar <- FLPar(
#   # catch before maturity
#   S50=3, S95=4.2,
#   # Domed-shaped
#   Smax=7, Dcv=0.5, Dmin=0.2)

# # Modify selectivity

# harvest(stk) <-  f0 * newselex(harvest(stk), selpar)

# # Plots weight-at-age, maturity-at-age, natural mortality and selective

# ggplot(FLQuants(stk,"m","catch.sel","mat","catch.wt"))+
#   geom_line(aes(age,data))+
#   facet_wrap(~qname,scale="free")+theme_bw()


# # --- Estimating priors from FLStock and SRR

# #' First, it is straight forward to compute the intrinsic rate of population increase from a Leslie for a specified steepness value. However, this $r$ estimate ignores selectivity and should only be used in the context of a Schaefer model with $F_{MSY} = r/2$.  

# r.leslie <- mean(productivity(stk, s=lh$s)$r)

# # compare
# c(r=r, r.leslie=r.leslie, FMSY=r.leslie / 2)

# #' An alternative is to estimate $r$ and the shape parameter $n$ as function $MSY$, $VB_{MSY}$ and $VB_0$ from an age-structured equilibrium model (ASEM), where $BB$ denotes that the vulnarable (or exploitable) biomass as function of selectivity. 

# brp <- brp(FLBRP(stk, srr))

# r.pella <- asem2spm(brp)
# r.pella

# plotpf(brp, rel=FALSE) + plotpf(brp, rel=TRUE) + plot_layout(guides = "collect")


# # --- Simulating stock dynamics with evolutionary F-trajectories

# # Estimate refpts
# brp <- computeFbrp(stk, srr, proxy="msy", blim=0.3, type="btgt")
# stk <- FLStockR(stk)
# refpts(stk) <- Fbrp(brp)

# ploteq(brp)

# its <- 100 

# # SSB0 for brp
# b0 <- an(refpts(eql)["virgin","ssb"])

# # one-way downhill projection
# control = FLPar(Feq=0.15,Frate=0.05,Fsigma=0.15,SB0=b0,
#   minyear=dims(stk)$minyear+1,
#   maxyear=dims(stk)$maxyear,its=its)

# # Forecasted with random recruitment under an evolving F-trajectory

# set.seed(1234)
# # Random recruitment deviations with sigR = 0.5 and AR1 rho = 0.3
# rec_devs = ar1rlnorm(0.3, 1951:2020, its, 0, 0.5)
# # propagate desired iterations
# stki <- propagate(stk, its)
# # create OM 
# om <- rffwd(stki, srr, control=control, deviances=rec_devs)
# refpts(om) <- refpts(stk)

# plotAdvice(om)

# # Generate LL CPUE, flat-topped selectivity

# cpue.sel <- newselex(harvest(stk),
#   FLPar(S50=3.5, mat95=4.2, Smax=100, Dcv=0.5, Dmin=0.05))

  # idx <- window(bioidx.sim(om,sel=cpue.sel,sigma=0.25,q=0.001),start=1970)
  # FIMS

  # Load surplus production data
  data("data_sp")
  #doesn't work because of age
  #data_4_model <- FIMSFrame(data_sp)
  nyears <- 70 
  survey_index <- data_sp |> dplyr::filter(type == "index") 
  landings <- data_sp |> dplyr::filter(type == "landings")

  clear()
  # create index module
  survey_fleet_index <- methods::new(Index, nyears)
  purrr::walk(
    1:nyears,
    \(x) survey_fleet_index$index_data$set(x - 1, survey_index$value[x])
  )


  # create catch module
  fishing_fleet_landings <- methods::new(Landings, nyears)
  purrr::walk(
    1:nyears,
    \(x) fishing_fleet_landings$landings_data$set(x - 1, landings$value[x])
  )

  # Survey and Fishery Fleet modules
  # Initialize the fishing fleet module
  fishing_fleet <- methods::new(Fleet)
  # Set number of years
  fishing_fleet$nyears$set(nyears)
  fishing_fleet$log_q[1]$value <- log(1.0)
  fishing_fleet$log_q[1]$estimation_type$set("constant")
  fishing_fleet$SetObservedLandingsDataID(fishing_fleet_landings$get_id())
  survey_fleet <- methods::new(Fleet)
  survey_fleet$nyears$set(nyears)
  # Estimate q
  survey_fleet$log_q[1]$value <- 1 #TODO: check initial value
  survey_fleet$log_q[1]$estimation_type$set("fixed_effects")
  survey_fleet$SetObservedIndexDataID(survey_fleet_index$get_id())

# setup distributions for fleet and survey
  # Set up fishery index data using the lognormal
  fishing_fleet_landings_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_landings_distribution$log_sd$resize(nyears)
  for (y in 1:nyears) {
    # Compute lognormal SD from OM coefficient of variation (CV)
    fishing_fleet_landings_distribution$log_sd[y]$value <- log(landings$uncertainty[y])
  }
  fishing_fleet_landings_distribution$log_sd$set_all_estimable(FALSE)
  # Set Data using the IDs from the modules defined above
  fishing_fleet_landings_distribution$set_observed_data(fishing_fleet$GetObservedLandingsDataID())
  fishing_fleet_landings_distribution$set_distribution_links("data", fishing_fleet$log_landings_expected$get_id())
  survey_fleet_index_distribution <- methods::new(DlnormDistribution)

  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution$log_sd$resize(nyears)
  for (y in 1:nyears) {
    survey_fleet_index_distribution$log_sd[y]$value <- log(survey_index$uncertainty[y])
  }
  survey_fleet_index_distribution$log_sd$set_all_estimable(FALSE)
  # Set Data using the IDs from the modules defined above
  survey_fleet_index_distribution$set_observed_data(survey_fleet$GetObservedIndexDataID())
  survey_fleet_index_distribution$set_distribution_links("data", survey_fleet$log_index_expected$get_id())

  # create depletion module
  production <- new(PTDepletion)
  # estimate log r and K
  # TODO: what are good input values for log_r and log_K?
  production$log_r[1]$value <- rnorm(1,0.2, 0.5)  # random draw from prior
  production$log_r[1]$estimation_type$set("fixed_effects")
  production$log_K[1]$value <- rnorm(1, 8*max(landings$value), 1)  # random draw from prior
  production$log_K[1]$estimation_type$set("fixed_effects")
  # Fix to get Schaefer model
  production$log_m[1]$value <- log(1)
  production$log_depletion$resize(nyears)
  for(i in 1:nyears) {
    production$log_depletion[i]$value <- 0
  }
  production$log_depletion$set_all_estimable(TRUE)
  production$nyears$set(nyears)

  production_distribution <- new(DnormDistribution)
  production_distribution$log_sd[1]$value <- log(1)
  production_distribution$set_distribution_links("random_effect", 
    c(production$log_depletion$get_id(), production$log_depletion$get_id()))


  #Setup Priors USING jabba DEFAULTS
  log_r_Prior <- new(DlnormDistribution)
  log_r_Prior$expected_values[1]$value <-0.2
  log_r_Prior$log_sd[1]$value <- log(0.5)
  log_r_Prior$set_distribution_links("prior", production$log_r$get_id())

  log_K_Prior <- new(DlnormDistribution)
  log_K_Prior$expected_values[1]$value <- 8*max(landings$value)
  log_K_Prior$log_sd[1]$value <- log(1.0)
  log_K_Prior$set_distribution_links("prior", production$log_K$get_id())

  # create population module
  population <- new(Population)
  population$nyears$set(nyears)
  population$nages$set(1) # only one age in surplus production
  population$ages$resize(1)
  population$ages$set(0,0) # only one age in surplus production
  # Fix init depletion
  population$log_init_depletion[1]$value <- 1
  population$log_init_depletion[1]$estimation_type$set("fixed_effects")
  population$SetDepletionID(production$get_id())


  #TODO: setup prior on init_depletion even though fixed?

    # Set up catch at age model
  surplus_production <- methods::new(SurplusProduction)
  surplus_production$AddPopulation(population$get_id())
  

  #create TMB Model
  # Set-up TMB
  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(
    p = get_fixed(),
    re = get_random()
  )

  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE, map = list()
  )

  # fit <- tmbstan::tmbstan(obj, init =  "best.last.par")
  # postmle <- as.matrix(fit)[, -ncol(as.matrix(fit))]
  opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
    control = list(eval.max = 10000, iter.max = 10000, trace = 0)
  )
 }