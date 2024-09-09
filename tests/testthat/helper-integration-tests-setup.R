#' Set Up and Run FIMS Model
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
#' results <- setup_and_run_FIMS(
#'   iter_id = 1,
#'   om_input_list = om_input_list,
#'   om_output_list = om_output_list,
#'   em_input_list = em_input_list,
#'   estimation_mode = TRUE
#' )
setup_and_run_FIMS <- function(iter_id,
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
  recruitment$log_rzero[1]$value <- log(om_input$R0)
  recruitment$log_rzero[1]$is_random_effect <- FALSE
  recruitment$log_rzero[1]$estimated <- TRUE
  # set up logit_steep
  recruitment$logit_steep[1]$value <- -log(1.0 - om_input$h) + log(om_input$h - 0.2)
  recruitment$logit_steep[1]$is_random_effect <- FALSE
  recruitment$logit_steep[1]$estimated <- FALSE
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
  maturity$inflection_point[1]$value <- om_input$A50.mat
  maturity$inflection_point[1]$is_random_effect <- FALSE
  maturity$inflection_point[1]$estimated <- FALSE
  maturity$slope[1]$value <- om_input$slope
  maturity$slope[1]$is_random_effect <- FALSE
  maturity$slope[1]$estimated <- FALSE

  # Fleet
  # Create the fishing fleet
  fishing_fleet_selectivity <- new(LogisticSelectivity)
  fishing_fleet_selectivity$inflection_point[1]$value <- om_input$sel_fleet$fleet1$A50.sel1
  fishing_fleet_selectivity$inflection_point[1]$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  fishing_fleet_selectivity$inflection_point[1]$estimated <- TRUE
  fishing_fleet_selectivity$slope[1]$value <- om_input$sel_fleet$fleet1$slope.sel1
  # turn on estimation of slope
  fishing_fleet_selectivity$slope[1]$is_random_effect <- FALSE
  fishing_fleet_selectivity$slope[1]$estimated <- TRUE

  fishing_fleet <- new(Fleet)
  fishing_fleet$nages <- om_input$nages
  fishing_fleet$nyears <- om_input$nyr
  fishing_fleet$log_Fmort <- methods::new(ParameterVector, log(om_output$f), om_input$nyr)
  fishing_fleet$log_Fmort$set_all_estimable(TRUE)
  fishing_fleet$log_q[1]$value <- log(1.0)
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
  survey_fleet_selectivity$inflection_point[1]$value <- om_input$sel_survey$survey1$A50.sel1
  survey_fleet_selectivity$inflection_point[1]$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimated <- TRUE
  survey_fleet_selectivity$slope[1]$value <- om_input$sel_survey$survey1$slope.sel1
  survey_fleet_selectivity$slope[1]$is_random_effect <- FALSE
  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimated <- TRUE

  survey_fleet <- new(Fleet)
  survey_fleet$is_survey <- TRUE
  survey_fleet$nages <- om_input$nages
  survey_fleet$nyears <- om_input$nyr
  survey_fleet$log_q[1]$value <- log(om_output$survey_q$survey1)
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

#' Validate FIMS Model Output
#'
#' This function validates the output from the FIMS
#' against the known OM values.
#' It performs various checks to ensure that the estimates provided by the FIMS
#' are within acceptable tolerance compared to the operating model values.
#'
#' @param report A list containing the results of the TMB model report. This
#' includes the estimated recruitment numbers and other relevant metrics.
#' @param sdr A list containing the standard deviation report from the TMB model.
#' @param sdr_report A matrix containing the summary of the standard deviation report.
#' @param om_input A list containing the operating model inputs, such as years,
#' ages, and other parameters.
#' @param om_output A list containing the operating model outputs, including metrics
#' such as numbers at age, biomass, spawning biomass, fishing mortality, and survey indices.
#' @param em_input A list containing the estimation model inputs, including observed
#' catches, survey indices, and other relevant data.
#'
#' @return None. The function uses `testthat` functions to perform validations.
#' It ensures that the output is within the expected range of error based on
#' standard deviations provided.
#'
#' @examples
#' # Assume `result` is a list of outputs obtained from running `setup_and_run_FIMS()`.
#' # The `result` list contains components such as `report`, `sdr_report`, and `obj`.
#'
#' validate_fims(
#'   report = result$report,
#'   sdr = TMB::sdreport(result$obj),
#'   sdr_report = result$sdr_report,
#'   om_input = om_input_list[[1]],
#'   om_output = om_output_list[[1]],
#'   em_input = em_input_list[[1]]
#' )
validate_fims <- function(
    report,
    sdr,
    sdr_report,
    om_input,
    om_output,
    em_input) {
  # Numbers at age
  # Estimates and SE for NAA
  sdr_naa <- sdr_report[which(rownames(sdr_report) == "NAA"), ]
  naa_are <- rep(0, length(c(t(om_output$N.age))))
  for (i in 1:length(c(t(om_output$N.age)))) {
    naa_are[i] <- abs(sdr_naa[i, 1] - c(t(om_output$N.age))[i])
  }
  # Expect 95% of absolute error to be within 2*SE of NAA
  expect_lte(
    sum(naa_are > qnorm(.975) * sdr_naa[1:length(c(t(om_output$N.age))), 2]),
    0.05 * length(c(t(om_output$N.age)))
  )

  # Biomass
  sdr_biomass <- sdr_report[which(rownames(sdr_report) == "Biomass"), ]
  biomass_are <- rep(0, length(om_output$biomass.mt))
  for (i in 1:length(om_output$biomass.mt)) {
    biomass_are[i] <- abs(sdr_biomass[i, 1] - om_output$biomass.mt[i]) # / om_output$biomass.mt[i]
    # expect_lte(biomass_are[i], 0.15)
  }
  expect_lte(
    sum(biomass_are > qnorm(.975) * sdr_biomass[1:length(om_output$biomass.mt), 2]),
    0.05 * length(om_output$biomass.mt)
  )

  # Spawning biomass
  sdr_sb <- sdr_report[which(rownames(sdr_report) == "SSB"), ]
  sb_are <- rep(0, length(om_output$SSB))
  for (i in 1:length(om_output$SSB)) {
    sb_are[i] <- abs(sdr_sb[i, 1] - om_output$SSB[i]) # / om_output$SSB[i]
    # expect_lte(sb_are[i], 0.15)
  }
  expect_lte(
    sum(sb_are > qnorm(.975) * sdr_sb[1:length(om_output$SSB), 2]),
    0.05 * length(om_output$SSB)
  )

  # Recruitment
  fims_naa <- matrix(report$naa[[1]][1:(om_input$nyr * om_input$nages)],
    nrow = om_input$nyr, byrow = TRUE
  )
  sdr_naa1_vec <- sdr_report[which(rownames(sdr_report) == "NAA"), 2]
  sdr_naa1 <- sdr_naa1_vec[seq(1, om_input$nyr * om_input$nages, by = om_input$nages)]
  fims_naa1_are <- rep(0, om_input$nyr)
  for (i in 1:om_input$nyr) {
    fims_naa1_are[i] <- abs(fims_naa[i, 1] - om_output$N.age[i, 1]) # /
    # om_output$N.age[i, 1]
    # expect_lte(fims_naa1_are[i], 0.25)
  }
  expect_lte(
    sum(fims_naa1_are > qnorm(.975) * sdr_naa1[1:length(om_output$SSB)]),
    0.05 * length(om_output$SSB)
  )

  expect_equal(
    fims_naa[, 1],
    report$recruitment[[1]][1:om_input$nyr]
  )

  # recruitment log deviations
  # the initial value of om_input$logR.resid is dropped from the model
  sdr_rdev <- sdr_report[which(rownames(sdr_report) == "LogRecDev"), ]
  rdev_are <- rep(0, length(om_input$logR.resid) - 1)

  for (i in 1:(length(report$log_recruit_dev[[1]]) - 1)) {
    rdev_are[i] <- abs(report$log_recruit_dev[[1]][i] - om_input$logR.resid[i + 1]) # /
    #   exp(om_input$logR.resid[i])
    # expect_lte(rdev_are[i], 1) # 1
  }
  expect_lte(
    sum(rdev_are > qnorm(.975) * sdr_rdev[1:length(om_input$logR.resid) - 1, 2]),
    0.05 * length(om_input$logR.resid)
  )

  # F (needs to be updated when std.error is available)
  sdr_F <- sdr_report[which(rownames(sdr_report) == "FMort"), ]
  f_are <- rep(0, length(om_output$f))
  for (i in 1:length(om_output$f)) {
    f_are[i] <- abs(sdr_F[i, 1] - om_output$f[i])
  }
  # Expect 95% of absolute error to be within 2*SE of Fmort
  expect_lte(
    sum(f_are > qnorm(.975) * sdr_F[1:length(om_output$f), 2]),
    0.05 * length(om_output$f)
  )

  # Expected fishery catch and survey index
  fims_index <- sdr_report[which(rownames(sdr_report) == "ExpectedIndex"), ]
  fims_catch <- fims_index[1:om_input$nyr, ]
  fims_survey <- fims_index[(om_input$nyr + 1):(om_input$nyr * 2), ]

  # Expected fishery catch - om_output
  catch_are <- rep(0, length(om_output$L.mt$fleet1))
  for (i in 1:length(om_output$L.mt$fleet1)) {
    catch_are[i] <- abs(fims_catch[i, 1] - om_output$L.mt$fleet1[i])
  }
  # Expect 95% of absolute error to be within 2*SE of fishery catch
  expect_lte(
    sum(catch_are > qnorm(.975) * fims_catch[, 2]),
    0.05 * length(om_output$L.mt$fleet1)
  )

  # Expected fishery catch - em_input
  catch_are <- rep(0, length(em_input$L.obs$fleet1))
  for (i in 1:length(em_input$L.obs$fleet1)) {
    catch_are[i] <- abs(fims_catch[i, 1] - em_input$L.obs$fleet1[i])
  }
  # Expect 95% of absolute error to be within 2*SE of fishery catch
  expect_lte(
    sum(catch_are > qnorm(.975) * fims_catch[, 2]),
    0.05 * length(em_input$L.obs$fleet1)
  )


  # Expected fishery catch number at age
  sdr_cnaa <- sdr_report[which(rownames(sdr_report) == "CNAA"), ]
  cnaa_are <- rep(0, length(c(t(om_output$L.age$fleet1))))
  for (i in 1:length(c(t(om_output$L.age$fleet1)))) {
    cnaa_are[i] <- abs(sdr_cnaa[i, 1] - c(t(om_output$L.age$fleet1))[i])
  }
  # Expect 95% of absolute error to be within 2*SE of CNAA
  expect_lte(
    sum(cnaa_are > qnorm(.975) * sdr_cnaa[, 2]),
    0.05 * length(c(t(om_output$L.age$fleet1)))
  )

  # Expected survey index - om_output
  index_are <- rep(0, length(om_output$survey_index_biomass$survey1))
  for (i in 1:length(om_output$survey_index_biomass$survey1)) {
    index_are[i] <- abs(fims_survey[i, 1] - om_output$survey_index_biomass$survey1[i])
  }
  # Expect 95% of absolute error to be within 2*SE of survey index
  expect_lte(
    sum(index_are > qnorm(.975) * fims_survey[, 2]),
    0.05 * length(om_output$survey_index_biomass$survey1)
  )

  # Expected survey index - em_input
  index_are <- rep(0, length(em_input$surveyB.obs$survey1))
  for (i in 1:length(em_input$surveyB.obs$survey1)) {
    index_are[i] <- abs(fims_survey[i, 1] - em_input$surveyB.obs$survey1[i])
  }
  # # Expect 95% of absolute error to be within 2*SE of survey index
  # expect_lte(
  #   sum(index_are > qnorm(.975) * fims_survey[, 2]),
  #   0.05 * length(em_input$surveyB.obs$survey1)
  # )

  for (i in 1:length(em_input$surveyB.obs$survey1)) {
    expect_lte(abs(fims_survey[i, 1] - em_input$surveyB.obs$survey1[i]) /
      em_input$surveyB.obs$survey1[i], 0.25)
  }

  # Expected survey number at age
  # for (i in 1:length(c(t(om_output$survey_age_comp$survey1)))){
  #   expect_lte(abs(report$cnaa[i,2] - c(t(om_output$survey_age_comp$survey1))[i])/
  #                c(t(om_output$survey_age_comp$survey1))[i], 0.001)
  # }

  # Expected catch number at age in proportion
  # fims_cnaa <- matrix(report$cnaa[1:(om_input$nyr*om_input$nages), 2],
  #                     nrow = om_input$nyr, byrow = TRUE)
  # fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)
  #
  # for (i in 1:length(c(t(em_input$survey.age.obs)))){
  #   expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input$L.age.obs$fleet1))[i])/
  #                c(t(em_input$L.age.obs$fleet1))[i], 0.15)
  # }
}
