#' Set Up and Run FIMS Model without using wrapper functions
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
#' @param map A list used to specify mapping for the `MakeADFun` function from
#' the TMB package.
#'
#' @return A list containing the following elements:
#' \itemize{
#'   \item{parameters:} A list of parameters for the TMB model.
#'   \item{obj:} The TMB model object created by `TMB::MakeADFun`.
#'   \item{opt:} The result of the optimization process, if `estimation_mode`
#'   is `TRUE`. `NULL` if `estimation_mode` is `FALSE`.
#'   \item{report:} The model report obtained from the TMB model.
#'   \item{sdr_report:} Summary of the standard deviation report for the
#'   model parameters.
#'   \item{sdr_fixed:} Summary of the standard deviation report for the
#'   fixed parameters.
#' }
#' @examples
#' results <- setup_and_run_FIMS_without_wrappers(
#'   iter_id = 1,
#'   om_input_list = om_input_list,
#'   om_output_list = om_output_list,
#'   em_input_list = em_input_list,
#'   estimation_mode = TRUE
#' )
setup_and_run_FIMS_without_wrappers <- function(iter_id,
                               om_input_list,
                               om_output_list,
                               em_input_list,
                               estimation_mode = TRUE,
                               map = list()) {

  # Load operating model data
  om_input <- om_input_list[[iter_id]]
  om_output <- om_output_list[[iter_id]]
  em_input <- em_input_list[[iter_id]]

  # Clear any previous FIMS settings
  clear()

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- new(BevertonHoltRecruitment)

  # NOTE: in first set of parameters below (for recruitment),
  # $is_random_effect (default is FALSE) and $estimated (default is FALSE)
  # are defined even if they match the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero$value <- log(om_input$R0)
  recruitment$log_rzero$is_random_effect <- FALSE
  recruitment$log_rzero$estimated <- TRUE
  # set up logit_steep
  recruitment$logit_steep$value <- -log(1.0 - om_input$h) + log(om_input$h - 0.2)
  recruitment$logit_steep$is_random_effect <- FALSE
  recruitment$logit_steep$estimated <- FALSE
  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))
  recruitment$log_devs <- methods::new(ParameterVector, om_input$logR.resid[-1], om_input$nyr-1)

  recruitment_distribution <- new(TMBDnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd <- new(ParameterVector, 1)
  recruitment_distribution$log_sd[1]$value <- log(om_input$logR_sd)
  recruitment_distribution$log_sd[1]$estimated <- FALSE
  recruitment_distribution$x <- new(ParameterVector, om_input$nyr)
  recruitment_distribution$expected_values <- new(ParameterVector, om_input$nyr)
  for (i in 1:om_input$nyr) {
    recruitment_distribution$x[i]$value <- 0
    recruitment_distribution$expected_values[i]$value <- 0
  }
  recruitment_distribution$set_distribution_links("random_effects", recruitment$log_devs$get_id())
  recruitment$estimate_log_devs <- TRUE

  # Data
  catch <- em_input$L.obs$fleet1
  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_index <- new(Index, om_input$nyr)
  fishing_fleet_index$index_data <- catch
  # set fishing fleet age comp data, need to set dimensions of age comps
  fishing_fleet_age_comp <- new(AgeComp, om_input$nyr, om_input$nages)
  fishing_fleet_age_comp$age_comp_data <- c(t(em_input$L.age.obs$fleet1)) * em_input$n.L$fleet1

  # repeat for surveys
  survey_index <- em_input$surveyB.obs$survey1
  survey_fleet_index <- new(Index, om_input$nyr)
  survey_fleet_index$index_data <- survey_index
  survey_fleet_age_comp <- new(AgeComp, om_input$nyr, om_input$nages)
  survey_fleet_age_comp$age_comp_data <- c(t(em_input$survey.age.obs$survey1)) * em_input$n.survey$survey1

  # Growth
  ewaa_growth <- new(EWAAgrowth)
  ewaa_growth$ages <- om_input$ages
  ewaa_growth$weights <- om_input$W.mt

  # Maturity
  maturity <- new(LogisticMaturity)
  maturity$inflection_point$value <- om_input$A50.mat
  maturity$inflection_point$is_random_effect <- FALSE
  maturity$inflection_point$estimated <- FALSE
  maturity$slope$value <- om_input$slope
  maturity$slope$is_random_effect <- FALSE
  maturity$slope$estimated <- FALSE

  # Fleet
  # Create the fishing fleet
  fishing_fleet_selectivity <- new(LogisticSelectivity)
  fishing_fleet_selectivity$inflection_point$value <- om_input$sel_fleet$fleet1$A50.sel1
  fishing_fleet_selectivity$inflection_point$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  fishing_fleet_selectivity$inflection_point$estimated <- TRUE
  fishing_fleet_selectivity$slope$value <- om_input$sel_fleet$fleet1$slope.sel1
  # turn on estimation of slope
  fishing_fleet_selectivity$slope$is_random_effect <- FALSE
  fishing_fleet_selectivity$slope$estimated <- TRUE

  fishing_fleet <- new(Fleet)
  fishing_fleet$nages <- om_input$nages
  fishing_fleet$nyears <- om_input$nyr
  fishing_fleet$log_Fmort <- methods::new(ParameterVector, log(om_output$f), om_input$nyr)
  fishing_fleet$log_Fmort$set_all_estimable(TRUE)
  fishing_fleet$log_q <- log(1.0)
  fishing_fleet$estimate_q <- FALSE
  fishing_fleet$random_q <- FALSE
  fishing_fleet$SetSelectivity(fishing_fleet_selectivity$get_id())

  # Set up fishery index data using the lognormal
  fishing_fleet_index_distribution <- methods::new(TMBDlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_index_distribution$log_logsd <- new(ParameterVector, om_input$nyr)
  for (y in 1:om_input$nyr) {
    fishing_fleet_index_distribution$log_logsd[y]$value <- log(sqrt(log(em_input$cv.L$fleet1^2 + 1)))
  }
  fishing_fleet_index_distribution$log_logsd$set_all_estimable(FALSE)
  # Set Data using the IDs from the modules defined above
  fishing_fleet_index_distribution$set_observed_data(fishing_fleet_index$get_id())
  fishing_fleet_index_distribution$set_distribution_links("data", fishing_fleet$log_expected_index$get_id())

  # Set up fishery age composition data using the multinomial
  fishing_fleet_agecomp_distribution <- methods::new(TMBDmultinomDistribution)
  fishing_fleet_agecomp_distribution$set_observed_data(fishing_fleet_age_comp$get_id())
  fishing_fleet_agecomp_distribution$set_distribution_links("data", fishing_fleet$proportion_catch_numbers_at_age$get_id())

  # Create the survey fleet
  survey_fleet_selectivity <- new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point$value <- om_input$sel_survey$survey1$A50.sel1
  survey_fleet_selectivity$inflection_point$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point$estimated <- TRUE
  survey_fleet_selectivity$slope$value <- om_input$sel_survey$survey1$slope.sel1
  survey_fleet_selectivity$slope$is_random_effect <- FALSE
  # turn on estimation of slope
  survey_fleet_selectivity$slope$estimated <- TRUE

  survey_fleet <- new(Fleet)
  survey_fleet$is_survey <- TRUE
  survey_fleet$nages <- om_input$nages
  survey_fleet$nyears <- om_input$nyr
  survey_fleet$log_q <- log(om_output$survey_q$survey1)
  survey_fleet$estimate_q <- TRUE
  survey_fleet$random_q <- FALSE
  survey_fleet$SetSelectivity(survey_fleet_selectivity$get_id())

  # Set up survey index data using the lognormal
  survey_fleet_index_distribution <- methods::new(TMBDlnormDistribution)
  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution$log_logsd <- new(ParameterVector, om_input$nyr)
  for (y in 1:om_input$nyr) {
    survey_fleet_index_distribution$log_logsd[y]$value <- log(sqrt(log(em_input$cv.survey$survey1^2 + 1)))
  }
  survey_fleet_index_distribution$log_logsd$set_all_estimable(FALSE)
  # Set Data using the IDs from the modules defined above
  survey_fleet_index_distribution$set_observed_data(survey_fleet_index$get_id())
  survey_fleet_index_distribution$set_distribution_links("data", survey_fleet$log_expected_index$get_id())

  # Age composition data

  survey_fleet_agecomp_distribution <- methods::new(TMBDmultinomDistribution)
  survey_fleet_agecomp_distribution$set_observed_data(survey_fleet_age_comp$get_id())
  survey_fleet_agecomp_distribution$set_distribution_links("data", survey_fleet$proportion_catch_numbers_at_age$get_id())


  # Population
  population <- new(Population)
  population$log_M <- methods::new(ParameterVector, rep(log(om_input$M.age[1]), om_input$nyr * om_input$nages), om_input$nyr * om_input$nages)
  population$log_M$set_all_estimable(FALSE)
  population$log_init_naa <- methods::new(ParameterVector, log(om_output$N.age[1, ]), om_input$nages)
  population$log_init_naa$set_all_estimable(TRUE)
  population$nages <- om_input$nages
  population$ages <- om_input$ages
  population$nfleets <- sum(om_input$fleet_num, om_input$survey_num)
  population$nseasons <- 1
  population$nyears <- om_input$nyr
  population$SetMaturity(maturity$get_id())
  population$SetGrowth(ewaa_growth$get_id())
  population$SetRecruitment(recruitment$get_id())

  # Set-up TMB
  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = get_fixed())
  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE, map = map
  )

  # Optimization with nlminb
  opt <- NULL
  if (estimation_mode == TRUE) {
    opt <- stats::nlminb(obj$par, obj$fn, obj$gr,
                         control = list(eval.max = 800, iter.max = 800)
    )
  }
  # Call report using MLE parameter values, or
  # the initial values if optimization is skipped
  report <- obj$report(obj$env$last.par.best)

  sdr <- TMB::sdreport(obj)
  sdr_report <- summary(sdr, "report")
  sdr_fixed <- summary(sdr, "fixed")

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
#' @param map A list used to specify mapping for the `MakeADFun` function from
#' the TMB package.
#'
#' @return A list containing the following elements:
#' \itemize{
#'   \item{parameters:} A list of parameters for the TMB model.
#'   \item{obj:} The TMB model object created by `TMB::MakeADFun`.
#'   \item{opt:} The result of the optimization process, if `estimation_mode`
#'   is `TRUE`. `NULL` if `estimation_mode` is `FALSE`.
#'   \item{report:} The model report obtained from the TMB model.
#'   \item{sdr_report:} Summary of the standard deviation report for the
#'   model parameters.
#'   \item{sdr_fixed:} Summary of the standard deviation report for the
#'   fixed parameters.
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
                                                map = list()) {
  # Load operating model data for the current iteration
  om_input <- om_input_list[[iter_id]]
  om_output <- om_output_list[[iter_id]]
  em_input <- em_input_list[[iter_id]]

  # Clear any previous FIMS settings
  clear()

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- new(BevertonHoltRecruitment)

  # NOTE: in first set of parameters below (for recruitment),
  # $is_random_effect (default is FALSE) and $estimated (default is FALSE)
  # are defined even if they match the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up logR_sd
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment$log_sigma_recruit$value <- log(om_input$logR_sd)
  recruitment$log_sigma_recruit$is_random_effect <- FALSE
  recruitment$log_sigma_recruit$estimated <- FALSE
  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero$value <- log(om_input$R0)
  recruitment$log_rzero$is_random_effect <- FALSE
  recruitment$log_rzero$estimated <- TRUE
  # set up logit_steep
  recruitment$logit_steep$value <- -log(1.0 - om_input$h) + log(om_input$h - 0.2)
  recruitment$logit_steep$is_random_effect <- FALSE
  recruitment$logit_steep$estimated <- FALSE
  # turn on estimation of deviations
  recruitment$estimate_log_devs <- TRUE
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))
  recruitment$log_devs <- om_input$logR.resid[-1]

  # Data
  catch <- em_input$L.obs$fleet1
  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_index <- new(Index, length(catch))
  fishing_fleet_index$index_data <- catch
  # set fishing fleet age comp data, need to set dimensions of age comps
  fishing_fleet_age_comp <- new(AgeComp, length(catch), om_input$nages)
  fishing_fleet_age_comp$age_comp_data <- c(t(em_input$L.age.obs$fleet1)) * em_input$n.L$fleet1

  # repeat for surveys
  survey_index <- em_input$surveyB.obs$survey1
  survey_fleet_index <- new(Index, length(survey_index))
  survey_fleet_index$index_data <- survey_index
  survey_fleet_age_comp <- new(AgeComp, length(survey_index), om_input$nages)
  survey_fleet_age_comp$age_comp_data <- c(t(em_input$survey.age.obs$survey1)) * em_input$n.survey$survey1

  # Growth
  ewaa_growth <- new(EWAAgrowth)
  ewaa_growth$ages <- om_input$ages
  ewaa_growth$weights <- om_input$W.mt

  # Maturity
  maturity <- new(LogisticMaturity)
  maturity$inflection_point$value <- om_input$A50.mat
  maturity$inflection_point$is_random_effect <- FALSE
  maturity$inflection_point$estimated <- FALSE
  maturity$slope$value <- om_input$slope
  maturity$slope$is_random_effect <- FALSE
  maturity$slope$estimated <- FALSE

  # Fleet
  # Create the fishing fleet
  fishing_fleet_selectivity <- new(LogisticSelectivity)
  fishing_fleet_selectivity$inflection_point$value <- om_input$sel_fleet$fleet1$A50.sel1
  fishing_fleet_selectivity$inflection_point$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  fishing_fleet_selectivity$inflection_point$estimated <- TRUE
  fishing_fleet_selectivity$slope$value <- om_input$sel_fleet$fleet1$slope.sel1
  # turn on estimation of slope
  fishing_fleet_selectivity$slope$is_random_effect <- FALSE
  fishing_fleet_selectivity$slope$estimated <- TRUE

  fishing_fleet <- new(Fleet)
  fishing_fleet$nages <- om_input$nages
  fishing_fleet$nyears <- om_input$nyr
  fishing_fleet$log_Fmort <- log(om_output$f)
  fishing_fleet$estimate_F <- TRUE
  fishing_fleet$random_F <- FALSE
  fishing_fleet$log_q <- log(1.0)
  fishing_fleet$estimate_q <- FALSE
  fishing_fleet$random_q <- FALSE
  fishing_fleet$log_obs_error <- rep(log(sqrt(log(em_input$cv.L$fleet1^2 + 1))), om_input$nyr)
  fishing_fleet$estimate_obs_error <- FALSE
  # Modules are linked together using module IDs
  # Each module has a get_id() function that returns the unique ID for that module
  # Each fleet uses the module IDs to link up the correct module to the correct fleet
  # Note: Likelihoods not yet set up as a stand-alone modules, so no get_id()
  fishing_fleet$SetAgeCompLikelihood(1)
  fishing_fleet$SetIndexLikelihood(1)
  fishing_fleet$SetSelectivity(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedIndexData(fishing_fleet_index$get_id())
  fishing_fleet$SetObservedAgeCompData(fishing_fleet_age_comp$get_id())

  # Create the survey fleet
  survey_fleet_selectivity <- new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point$value <- om_input$sel_survey$survey1$A50.sel1
  survey_fleet_selectivity$inflection_point$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point$estimated <- TRUE
  survey_fleet_selectivity$slope$value <- om_input$sel_survey$survey1$slope.sel1
  survey_fleet_selectivity$slope$is_random_effect <- FALSE
  # turn on estimation of slope
  survey_fleet_selectivity$slope$estimated <- TRUE

  survey_fleet <- new(Fleet)
  survey_fleet$is_survey <- TRUE
  survey_fleet$nages <- om_input$nages
  survey_fleet$nyears <- om_input$nyr
  survey_fleet$estimate_F <- FALSE
  survey_fleet$random_F <- FALSE
  survey_fleet$log_q <- log(om_output$survey_q$survey1)
  survey_fleet$estimate_q <- TRUE
  survey_fleet$random_q <- FALSE
  survey_fleet$log_obs_error <- rep(log(sqrt(log(em_input$cv.survey$survey1^2 + 1))), om_input$nyr)
  survey_fleet$estimate_obs_error <- FALSE
  survey_fleet$SetAgeCompLikelihood(1)
  survey_fleet$SetIndexLikelihood(1)
  survey_fleet$SetSelectivity(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompData(survey_fleet_age_comp$get_id())

  # Population
  population <- new(Population)
  population$log_M <- rep(log(om_input$M.age[1]), om_input$nyr * om_input$nages)
  population$estimate_M <- FALSE
  population$log_init_naa <- log(om_output$N.age[1, ])
  population$estimate_init_naa <- TRUE
  population$nages <- om_input$nages
  population$ages <- om_input$ages
  population$nfleets <- sum(om_input$fleet_num, om_input$survey_num)
  population$nseasons <- 1
  population$nyears <- om_input$nyr
  population$SetMaturity(maturity$get_id())
  population$SetGrowth(ewaa_growth$get_id())
  population$SetRecruitment(recruitment$get_id())

  # Set-up TMB
  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = get_fixed())
  input <- list()
  input$parameters <- parameters
  input$version = "Model Comparison Project example"
  fit <- fit_fims(input)

  clear()
  # Return the results as a list
  return(list(
    parameters = fit$input$parameters,
    obj = fit$obj,
    opt = fit$opt,
    report = fit$rep,
    sdr_report = fit$sd
  ))
}

