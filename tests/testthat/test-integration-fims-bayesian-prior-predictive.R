load(test_path("fixtures", "integration_test_data.RData"))

# Set the iteration ID to 1 for accessing specific input/output list
iter_id <- 1

# Extract model input and output data for the specified iteration
om_input <- om_input_list[[iter_id]]
om_output <- om_output_list[[iter_id]]
em_input <- em_input_list[[iter_id]]

test_that("prior predictive check", {
  # Set up fleet and survey without data distributions

  # Extract fishing fleet landings data (observed) and initialize index module
  catch <- em_input[["L.obs"]][["fleet1"]]
  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_index <- methods::new(Index, om_input[["nyr"]])
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) fishing_fleet_index$index_data$set(x - 1, catch[x])
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
  fishing_fleet_selectivity$inflection_point[1]$estimation_type <- "fixed_effects"
  fishing_fleet_selectivity$slope[1]$value <- om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]]
  # turn on estimation of slope
  fishing_fleet_selectivity$slope[1]$estimation_type <- "fixed_effects"

  # Initialize the fishing fleet module
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
  fishing_fleet$SetSelectivityID(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedIndexDataID(fishing_fleet_index$get_id())
  fishing_fleet$SetObservedAgeCompDataID(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetObservedLengthCompDataID(fishing_fleet_length_comp$get_id())

  # Set age-to-length conversion matrix
  # TODO: If an age_to_length_conversion matrix is provided, the code below
  # still executes. Consider adding a check in the Rcpp interface to ensure
  # users provide a vector of inputs.
  fishing_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:(om_input[["nages"]] * om_input[["nlengths"]])) {
    # Set the age-length conversion matrix values
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
  survey_fleet_selectivity$inflection_point[1]$estimation_type <- "fixed_effects"
  survey_fleet_selectivity$slope[1]$value <- om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]
  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimation_type <- "fixed_effects"

  survey_fleet <- methods::new(Fleet)
  survey_fleet$nages$set(om_input[["nages"]])
  survey_fleet$nyears$set(om_input[["nyr"]])
  survey_fleet$nlengths$set(om_input[["nlengths"]])
  survey_fleet$log_q[1]$value <- log(om_output[["survey_q"]][["survey1"]])
  survey_fleet$log_q[1]$estimation_type <- "fixed_effects"
  survey_fleet$SetSelectivityID(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexDataID(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompDataID(survey_fleet_age_comp$get_id())
  survey_fleet$SetObservedLengthCompDataID(survey_fleet_length_comp$get_id())

  survey_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:(om_input[["nages"]] * om_input[["nlengths"]])) {
    survey_fleet$age_to_length_conversion[i]$value <-
      c(t(em_input[["age_to_length_conversion"]]))[i]
  }
  # Turn off estimation for length-at-age
  survey_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  survey_fleet$age_to_length_conversion$set_all_random(FALSE)

  slope_mean <- mean(c(om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]], om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]))
  slope_prior <- methods::new(DnormDistribution)
  slope_prior$expected_values$resize(2)
  slope_prior$expected_values[1]$value <- slope_mean
  slope_prior$expected_values[2]$value <- slope_mean
  slope_prior$log_sd$resize(1)
  slope_prior$log_sd[1]$value <- log(3)
  slope_prior$set_distribution_links("prior", c(fishing_fleet_selectivity$slope$get_id(), survey_fleet_selectivity$slope$get_id()))

  inflection_point_mean <- mean(c(om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]], om_input[["sel_survey"]][["survey1"]][["A50.sel1"]]))
  inflection_point_prior <- methods::new(DnormDistribution)
  inflection_point_prior$expected_values$resize(2)
  inflection_point_prior$expected_values[1]$value <- inflection_point_mean
  inflection_point_prior$expected_values[2]$value <- inflection_point_mean
  inflection_point_prior$log_sd$resize(1)
  inflection_point_prior$log_sd[1]$value <- log(3)
  inflection_point_prior$set_distribution_links("prior", c(fishing_fleet_selectivity$inflection_point$get_id(), survey_fleet_selectivity$inflection_point$get_id()))

  # Add shared selectivity vague prior for fishery and survey

  recruitment <- methods::new(BevertonHoltRecruitment)
  recruitment_process <- new(LogDevsRecruitmentProcess)
  # NOTE: in first set of parameters below (for recruitment),
  # $is_random_effect (default is FALSE) and $estimated (default is FALSE)
  # are defined even if they match the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- log(om_input[["R0"]])
  recruitment$log_rzero[1]$estimation_type <- "fixed_effects"
  # set up logit_steep
  recruitment$logit_steep[1]$value <- -log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2)
  recruitment$logit_steep[1]$estimation_type <- "constant"
  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))
  recruitment$log_devs$resize(om_input[["nyr"]] - 1)
  for (y in 1:(om_input[["nyr"]] - 1)) {
    recruitment$log_devs[y]$value <- om_input[["logR.resid"]][y + 1]
  }
  recruitment$log_devs$set_all_estimable(TRUE)
  recruitment$nyears$set(om_input[["nyr"]])
  recruitment$SetRecruitmentProcessID(recruitment_process$get_id())
  recruitment_distribution <- methods::new(DnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd <- methods::new(ParameterVector, 1)
  recruitment_distribution$log_sd[1]$value <- log(om_input[["logR_sd"]])
  recruitment_distribution$x$resize(om_input[["nyr"]] - 1)
  recruitment_distribution$expected_values$resize(om_input[["nyr"]] - 1)
  for (i in 1:(om_input[["nyr"]] - 1)) {
    recruitment_distribution$x[i]$value <- 0
    recruitment_distribution$expected_values[i]$value <- 0
  }
  recruitment_distribution$set_distribution_links("random_effects", recruitment$log_devs$get_id())

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
  maturity$inflection_point[1]$estimation_type <- "constant"
  maturity$slope[1]$value <- om_input[["slope.mat"]]
  maturity$slope[1]$estimation_type <- "constant"

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


  # Set-up TMB
  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = get_fixed(), re = get_random())
  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE, map = list()
  )

  opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
    control = list(eval.max = 10000, iter.max = 10000, trace = 0)
  )

  names(opt$par) <- names(get_parameter_names(opt$par))
  inflection_point_out <- opt$par[grep(names(opt$par), pattern = "inflection_point")]
  slope_out <- opt$par[grep(names(opt$par), pattern = "slope")]
  for (i in 1:2) {
    expect_equal(unname(inflection_point_out[i]), unname(inflection_point_mean), tolerance = 1e-4)
    expect_equal(unname(slope_out[i]), unname(slope_mean), tolerance = 1e-4)
  }


  # fit <- tmbstan::tmbstan(obj, init =  "best.last.par")
  # postmle <- as.matrix(fit)[, -ncol(as.matrix(fit))]
  # colnames(postmle) <- names(get_parameter_names(obj$par))
  # inflection_point_out <- postmle[,grep(colnames(postmle), pattern = "inflection_point")]
  # slope_out <- postmle[,grep(colnames(postmle), pattern = "slope")]
  # for(i in 1:2){
  #   expect_equal(median(inflection_point_out[,i]), inflection_point_mean, tolerance = .1)
  #   expect_equal(median(slope_out[,i]), slope_mean, tolerance = .1)
  #   expect_equal(var(inflection_point_out[,i]), 9, tolerance = .1)
  #   expect_equal(var(slope_out[,i]), 9, tolerance = .1)
  # }

  clear()
})
