 load(test_path("fixtures", "integration_test_data.RData"))
 map <- list()

 iter_id <- 1

 om_input <- om_input_list[[iter_id]] # Operating model input for the current iteration
  om_output <- om_output_list[[iter_id]] # Operating model output for the current iteration
  em_input <- em_input_list[[iter_id]] # Estimation model input for the current iteration

  ## Run FIMS with no projection years as a control run ##

  n_projection_years <- 0
  projected_landings <- rep(-999,n_projection_years)
  projected_F <- rep(om_output[["f"]][om_input$nyr],n_projection_years)
  estim_projected_F <- rep("constant",n_projection_years)
  projected_index <- rep(-999,n_projection_years)

  # Clear any previous FIMS settings
  clear()

  # Extract fishing fleet landings data (observed) and initialize index module
  landings <- em_input[["L.obs"]][["fleet1"]]

  # set fishing fleet landings data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery landings
  fishing_fleet_landings <- methods::new(Landings, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) fishing_fleet_landings$landings_data$set(x - 1, landings[x])
  )
  # set projection years
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) fishing_fleet_landings$landings_data$set(x - 1, projected_landings[x-om_input[["nyr"]]])
    )
  }
  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*nages
  fishing_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  # Here we add projection period missing age comps for the fishing fleet
  # We take the observed age proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_age_comps <- em_input[["L.age.obs"]][["fleet1"]]* em_input[["n.L"]][["fleet1"]]
  if(n_projection_years>0){
    projected_age_comps <- rbind(
      projected_age_comps,
      matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
    )
  }
  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) fishing_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_age_comps)))[x]
    )
  )


  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  # Here we add projection period missing length comps for the fishing fleet
  # We take the observed length proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_length_comps <- em_input[["L.length.obs"]][["fleet1"]]* em_input[["n.L.lengthcomp"]][["fleet1"]]
  projected_length_comps <- rbind(
    projected_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) fishing_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_length_comps)))[x]
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
  fishing_fleet$nyears$set((om_input[["nyr"]]+n_projection_years))
  # Set number of age classes
  fishing_fleet$nages$set(om_input[["nages"]])
  # Set number of length bins
  fishing_fleet$nlengths$set(om_input[["nlengths"]])

  fishing_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    # NOTE: Is this index y correct? It starts at 1 but others start at 0?
    fishing_fleet$log_Fmort[y]$value <- (log(om_output[["f"]][y]))
    fishing_fleet$log_Fmort[y]$estimation_type$set("fixed_effects")
  }

  if(n_projection_years>0){
    for (y in (om_input$nyr+1):(om_input$nyr+n_projection_years)) {
      # Log-transform OM fishing mortality
      # NOTE: Is this index y correct? It starts at 1 but others start at 0?
      fishing_fleet$log_Fmort[y]$value <- (log(projected_F[y-om_input$nyr]))
      fishing_fleet$log_Fmort[y]$estimation_type$set(estim_projected_F[y-om_input$nyr])
    }
  }

  fishing_fleet$log_q[1]$value <- (log(1.0))
  fishing_fleet$log_q[1]$estimation_type$set("constant")
  fishing_fleet$SetSelectivityID(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedLandingsDataID(fishing_fleet_landings$get_id())
  fishing_fleet$SetObservedAgeCompDataID(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetObservedLengthCompDataID(fishing_fleet_length_comp$get_id())

  # Set up fishery index data using the lognormal
  fishing_fleet_landings_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_landings_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input[["nyr"]] + n_projection_years)) {
    # Compute lognormal SD from OM coefficient of variation (CV)
    fishing_fleet_landings_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1))))
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
  fishing_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    fishing_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }

  # Turn off estimation for length-at-age
  fishing_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  fishing_fleet$age_to_length_conversion$set_all_random(FALSE) # I don't think this is needed, both set estimable and set random set to "constant" if false

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- methods::new(Index, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) survey_fleet_index$index_data$set(x - 1, survey_index[x])
  )
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) survey_fleet_index$index_data$set(x - 1, projected_index[x-om_input[["nyr"]]])
    )
  }

  survey_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  projected_survey_age_comps <- em_input[["survey.age.obs"]][["survey1"]]* em_input[["n.survey"]][["survey1"]]
  projected_survey_age_comps <- rbind(
    projected_survey_age_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) survey_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_survey_age_comps)))[x]
    )
  )

  survey_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  projected_survey_length_comps <- em_input[["survey.length.obs"]][["survey1"]]* em_input[["n.survey.lengthcomp"]][["survey1"]]
  projected_survey_length_comps <- rbind(
    projected_survey_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) survey_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_survey_length_comps)))[x]
    )
  )
  # Fleet
  # Create the survey fleet
  survey_fleet_selectivity <- methods::new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point[1]$value <- (om_input[["sel_survey"]][["survey1"]][["A50.sel1"]])

  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  survey_fleet_selectivity$slope[1]$value <- (om_input[["sel_survey"]][["survey1"]][["slope.sel1"]])

  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimation_type$set("fixed_effects")

  survey_fleet <- methods::new(Fleet)
  survey_fleet$nages$set(om_input[["nages"]])
  survey_fleet$nyears$set(om_input[["nyr"]] + n_projection_years)
  survey_fleet$nlengths$set(om_input[["nlengths"]])
  survey_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    # Set very low survey fishing mortality
    survey_fleet$log_Fmort[y]$value <- (-200)
  }
  survey_fleet$log_Fmort$set_all_estimable(FALSE)
  survey_fleet$log_q[1]$value <- (log(om_output[["survey_q"]][["survey1"]]))
  survey_fleet$log_q[1]$estimation_type$set("fixed_effects")
  survey_fleet$SetSelectivityID(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexDataID(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompDataID(survey_fleet_age_comp$get_id())
  survey_fleet$SetObservedLengthCompDataID(survey_fleet_length_comp$get_id())

  # Set up survey index data using the lognormal
  survey_fleet_index_distribution <- methods::new(DlnormDistribution)

  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    survey_fleet_index_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.survey"]][["survey1"]]^2 + 1))))
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
  # TODO: Fix code below to not use 1:x and instead use seq_along() where this
  #       doesn't currently break because we are only testing models with both
  #       age and length data but it would break for only age data.
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    # Transposing the below will have NO impact on the results if the object is
    # already a vector. Additionally, c() ensures that the result is a vector
    # to be consistent but a matrix would be okay.
    # TODO: write a test/documentation to show what order the matrix needs to be
    #       in when passing data to age-length-conversion
    survey_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }
  # Turn off estimation for length-at-age
  survey_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  survey_fleet$age_to_length_conversion$set_all_random(FALSE)

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- methods::new(BevertonHoltRecruitment)
  recruitment_process <- new(LogDevsRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(recruitment_process$get_id())

  # NOTE: in first set of parameters below (for recruitment),
  # $estimation_type (default is "constant")
  # is defined even if it matches the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- (log(om_input[["R0"]]))
  recruitment$log_rzero[1]$estimation_type$set("fixed_effects")
  # set up logit_steep
  recruitment$logit_steep[1]$value <- (-log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2))
  recruitment$logit_steep[1]$estimation_type$set("constant")
  recruitment$nyears$set(om_input[["nyr"]] + n_projection_years)

  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))

  #adding two years for projections
  recruitment$log_devs$resize(om_input[["nyr"]] - 1 + n_projection_years)

  #set values for estimated years
  for (y in 1:(om_input[["nyr"]] - 1)) {
    recruitment$log_devs[y]$value <- (om_input[["logR.resid"]][y + 1])
    recruitment$log_devs[y]$estimation_type$set("random_effects")
  }

  if(n_projection_years>0){
    #set projection years to zero
    for(y in (om_input[["nyr"]]):(om_input[["nyr"]] - 1 + n_projection_years)){
      recruitment$log_devs[y]$value <- (0)
      recruitment$log_devs[y]$estimation_type$set("constant")
    }
  }





  recruitment_distribution <- methods::new(DnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd$resize(1)
  recruitment_distribution$log_sd[1]$value <- (log(om_input[["logR_sd"]]))

  #NOTE: If this doesn't work I would guess that this is the possible source of
  # issues due to the length of x or expected recruitment needing to be the
  # same length as the random effect portion not the whole recruitment vector
  recruitment_distribution$x$resize(om_input[["nyr"]] - 1)
  recruitment_distribution$expected_values$resize(om_input[["nyr"]] - 1)
  for (i in 1:(om_input[["nyr"]] - 1)) {
    recruitment_distribution$x[i]$value <- (0)
    recruitment_distribution$expected_values[i]$value <- (0)
  }

  recruitment_distribution$log_sd[1]$estimation_type$set("fixed_effects")
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
  maturity$inflection_point[1]$value <- (om_input[["A50.mat"]])
  maturity$inflection_point[1]$estimation_type$set("constant")
  maturity$slope[1]$value <- (om_input[["slope.mat"]])
  maturity$slope[1]$estimation_type$set("constant")

  # Population
  population <- methods::new(Population)
  population$log_M$resize((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])
  for (i in 1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])) {
    population$log_M[i]$value <- (log(om_input[["M.age"]][1]))
  }
  population$log_M$set_all_estimable(FALSE)
  population$log_init_naa$resize(om_input[["nages"]])
  for (i in 1:om_input$nages) {
    population$log_init_naa[i]$value <- (log(om_output[["N.age"]][1, i]))
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
  population$nyears$set(om_input[["nyr"]] + n_projection_years)
  population$SetRecruitmentID(recruitment$get_id())
  population$SetGrowthID(ewaa_growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$AddFleet(fishing_fleet$get_id())
  population$AddFleet(survey_fleet$get_id())

  # Set up catch at age model
  # caa <- methods::new(CatchAtAge)
  # caa$AddPopulation(population$get_id())

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

  opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
                       control = list(eval.max = 10000, iter.max = 10000, trace = 0)
  )


  # Call report using MLE parameter values, or
  # the initial values if optimization is skipped
  report <- obj[["report"]](obj[["env"]][["last.par.best"]])


  sdr <- TMB::sdreport(obj)
  sdr_report <- summary(sdr, "report")
  sdr_fixed <- summary(sdr, "fixed")
  row.names(sdr_fixed) <- names(FIMS:::get_parameter_names(sdr_fixed[, 1]))

  clear()

  #Save control results for comparison with projection results

  sdr_report_no_project <- sdr_report
  sdr_fixed_no_project <- sdr_fixed

  ## Setup model to produce 5 projection years with no
  ## catch, index, or composition data and recruitment deviations
  ## and F in projection years set as constant values

  n_projection_years <- 5
  projected_landings <- rep(-999,n_projection_years)
  projected_F <- rep(om_output[["f"]][om_input$nyr],n_projection_years)
  estim_projected_F <- rep("constant",n_projection_years)
  projected_index <- rep(-999,n_projection_years)

  # Clear any previous FIMS settings
  clear()

  # Extract fishing fleet landings data (observed) and initialize index module
  landings <- em_input[["L.obs"]][["fleet1"]]

  # set fishing fleet landings data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery landings
  fishing_fleet_landings <- methods::new(Landings, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) fishing_fleet_landings$landings_data$set(x - 1, landings[x])
  )
  # set projection years
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) fishing_fleet_landings$landings_data$set(x - 1, projected_landings[x-om_input[["nyr"]]])
    )
  }
  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*nages
  fishing_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  # Here we add projection period missing age comps for the fishing fleet
  # We take the observed age proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_age_comps <- em_input[["L.age.obs"]][["fleet1"]]* em_input[["n.L"]][["fleet1"]]
  if(n_projection_years>0){
    projected_age_comps <- rbind(
      projected_age_comps,
      matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
    )
  }
  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) fishing_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_age_comps)))[x]
    )
  )


  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  # Here we add projection period missing length comps for the fishing fleet
  # We take the observed length proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_length_comps <- em_input[["L.length.obs"]][["fleet1"]]* em_input[["n.L.lengthcomp"]][["fleet1"]]
  projected_length_comps <- rbind(
    projected_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) fishing_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_length_comps)))[x]
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
  fishing_fleet$nyears$set((om_input[["nyr"]]+n_projection_years))
  # Set number of age classes
  fishing_fleet$nages$set(om_input[["nages"]])
  # Set number of length bins
  fishing_fleet$nlengths$set(om_input[["nlengths"]])

  fishing_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    # NOTE: Is this index y correct? It starts at 1 but others start at 0?
    fishing_fleet$log_Fmort[y]$value <- (log(om_output[["f"]][y]))
    fishing_fleet$log_Fmort[y]$estimation_type$set("fixed_effects")
  }

  if(n_projection_years>0){
    for (y in (om_input$nyr+1):(om_input$nyr+n_projection_years)) {
      # Log-transform OM fishing mortality
      # NOTE: Is this index y correct? It starts at 1 but others start at 0?
      fishing_fleet$log_Fmort[y]$value <- (log(projected_F[y-om_input$nyr]))
      fishing_fleet$log_Fmort[y]$estimation_type$set(estim_projected_F[y-om_input$nyr])
    }
  }

  fishing_fleet$log_q[1]$value <- (log(1.0))
  fishing_fleet$log_q[1]$estimation_type$set("constant")
  fishing_fleet$SetSelectivityID(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedLandingsDataID(fishing_fleet_landings$get_id())
  fishing_fleet$SetObservedAgeCompDataID(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetObservedLengthCompDataID(fishing_fleet_length_comp$get_id())

  # Set up fishery index data using the lognormal
  fishing_fleet_landings_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_landings_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input[["nyr"]] + n_projection_years)) {
    # Compute lognormal SD from OM coefficient of variation (CV)
    fishing_fleet_landings_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1))))
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
  fishing_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    fishing_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }

  # Turn off estimation for length-at-age
  fishing_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  fishing_fleet$age_to_length_conversion$set_all_random(FALSE) # I don't think this is needed, both set estimable and set random set to "constant" if false

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- methods::new(Index, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) survey_fleet_index$index_data$set(x - 1, survey_index[x])
  )
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) survey_fleet_index$index_data$set(x - 1, projected_index[x-om_input[["nyr"]]])
    )
  }

  survey_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  projected_survey_age_comps <- em_input[["survey.age.obs"]][["survey1"]]* em_input[["n.survey"]][["survey1"]]
  projected_survey_age_comps <- rbind(
    projected_survey_age_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) survey_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_survey_age_comps)))[x]
    )
  )

  survey_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  projected_survey_length_comps <- em_input[["survey.length.obs"]][["survey1"]]* em_input[["n.survey.lengthcomp"]][["survey1"]]
  projected_survey_length_comps <- rbind(
    projected_survey_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) survey_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_survey_length_comps)))[x]
    )
  )
  # Fleet
  # Create the survey fleet
  survey_fleet_selectivity <- methods::new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point[1]$value <- (om_input[["sel_survey"]][["survey1"]][["A50.sel1"]])

  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  survey_fleet_selectivity$slope[1]$value <- (om_input[["sel_survey"]][["survey1"]][["slope.sel1"]])

  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimation_type$set("fixed_effects")

  survey_fleet <- methods::new(Fleet)
  survey_fleet$nages$set(om_input[["nages"]])
  survey_fleet$nyears$set(om_input[["nyr"]] + n_projection_years)
  survey_fleet$nlengths$set(om_input[["nlengths"]])
  survey_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    # Set very low survey fishing mortality
    survey_fleet$log_Fmort[y]$value <- (-200)
  }
  survey_fleet$log_Fmort$set_all_estimable(FALSE)
  survey_fleet$log_q[1]$value <- (log(om_output[["survey_q"]][["survey1"]]))
  survey_fleet$log_q[1]$estimation_type$set("fixed_effects")
  survey_fleet$SetSelectivityID(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexDataID(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompDataID(survey_fleet_age_comp$get_id())
  survey_fleet$SetObservedLengthCompDataID(survey_fleet_length_comp$get_id())

  # Set up survey index data using the lognormal
  survey_fleet_index_distribution <- methods::new(DlnormDistribution)

  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    survey_fleet_index_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.survey"]][["survey1"]]^2 + 1))))
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
  # TODO: Fix code below to not use 1:x and instead use seq_along() where this
  #       doesn't currently break because we are only testing models with both
  #       age and length data but it would break for only age data.
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    # Transposing the below will have NO impact on the results if the object is
    # already a vector. Additionally, c() ensures that the result is a vector
    # to be consistent but a matrix would be okay.
    # TODO: write a test/documentation to show what order the matrix needs to be
    #       in when passing data to age-length-conversion
    survey_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }
  # Turn off estimation for length-at-age
  survey_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  survey_fleet$age_to_length_conversion$set_all_random(FALSE)

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- methods::new(BevertonHoltRecruitment)
  recruitment_process <- new(LogDevsRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(recruitment_process$get_id())

  # NOTE: in first set of parameters below (for recruitment),
  # $estimation_type (default is "constant")
  # is defined even if it matches the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- (log(om_input[["R0"]]))
  recruitment$log_rzero[1]$estimation_type$set("fixed_effects")
  # set up logit_steep
  recruitment$logit_steep[1]$value <- (-log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2))
  recruitment$logit_steep[1]$estimation_type$set("constant")
  recruitment$nyears$set(om_input[["nyr"]] + n_projection_years)

  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))

#adding two years for projections
recruitment$log_devs$resize(om_input[["nyr"]] - 1 + n_projection_years)

#set values for estimated years
for (y in 1:(om_input[["nyr"]] - 1)) {
  recruitment$log_devs[y]$value <- (om_input[["logR.resid"]][y + 1])
  recruitment$log_devs[y]$estimation_type$set("random_effects")
}

if(n_projection_years>0){
  #set projection years to zero
  for(y in (om_input[["nyr"]]):(om_input[["nyr"]] - 1 + n_projection_years)){
    recruitment$log_devs[y]$value <- (0)
    recruitment$log_devs[y]$estimation_type$set("constant")
  }

}




  recruitment_distribution <- methods::new(DnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd$resize(1)
  recruitment_distribution$log_sd[1]$value <- (log(om_input[["logR_sd"]]))

  #NOTE: If this doesn't work I would guess that this is the possible source of
  # issues due to the length of x or expected recruitment needing to be the
  # same length as the random effect portion not the whole recruitment vector
  recruitment_distribution$x$resize(om_input[["nyr"]] - 1)
  recruitment_distribution$expected_values$resize(om_input[["nyr"]] - 1)
  for (i in 1:(om_input[["nyr"]] - 1)) {
    recruitment_distribution$x[i]$value <- (0)
    recruitment_distribution$expected_values[i]$value <- (0)
  }

  recruitment_distribution$log_sd[1]$estimation_type$set("fixed_effects")
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
  maturity$inflection_point[1]$value <- (om_input[["A50.mat"]])
  maturity$inflection_point[1]$estimation_type$set("constant")
  maturity$slope[1]$value <- (om_input[["slope.mat"]])
  maturity$slope[1]$estimation_type$set("constant")

  # Population
  population <- methods::new(Population)
  population$log_M$resize((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])
  for (i in 1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])) {
    population$log_M[i]$value <- (log(om_input[["M.age"]][1]))
  }
  population$log_M$set_all_estimable(FALSE)
  population$log_init_naa$resize(om_input[["nages"]])
  for (i in 1:om_input$nages) {
    population$log_init_naa[i]$value <- (log(om_output[["N.age"]][1, i]))
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
  population$nyears$set(om_input[["nyr"]] + n_projection_years)
  population$SetRecruitmentID(recruitment$get_id())
  population$SetGrowthID(ewaa_growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$AddFleet(fishing_fleet$get_id())
  population$AddFleet(survey_fleet$get_id())

  # Set up catch at age model
  # caa <- methods::new(CatchAtAge)
  # caa$AddPopulation(population$get_id())

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

    opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
      control = list(eval.max = 10000, iter.max = 10000, trace = 0)
    )


  # Call report using MLE parameter values, or
  # the initial values if optimization is skipped
  report <- obj[["report"]](obj[["env"]][["last.par.best"]])


  sdr <- TMB::sdreport(obj)
  sdr_report <- summary(sdr, "report")
  sdr_fixed <- summary(sdr, "fixed")
  row.names(sdr_fixed) <- names(FIMS:::get_parameter_names(sdr_fixed[, 1]))

  clear()

  #Save results for comparison with control and other projections

  sdr_report_5_year_project <- sdr_report
  sdr_fixed_5_year_project <- sdr_fixed

  ## Setup model to produce 5 projection years with no
  ## index, or composition data and recruitment deviations
  ## in projection years set as constant values. Catch values
  ## for projection years are provided and F is estimated.

  n_projection_years <- 5
  projected_landings <- rep(landings[om_input$nyr]*0.5,n_projection_years)
  projected_F <- rep(om_output[["f"]][om_input$nyr],n_projection_years)
  estim_projected_F <- rep("fixed_effects",n_projection_years)
  projected_index <- rep(-999,n_projection_years)

  # Clear any previous FIMS settings
  clear()

  # Extract fishing fleet landings data (observed) and initialize index module
  landings <- em_input[["L.obs"]][["fleet1"]]

  # set fishing fleet landings data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery landings
  fishing_fleet_landings <- methods::new(Landings, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) fishing_fleet_landings$landings_data$set(x - 1, landings[x])
  )
  # set projection years
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) fishing_fleet_landings$landings_data$set(x - 1, projected_landings[x-om_input[["nyr"]]])
    )
  }
  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*nages
  fishing_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  # Here we add projection period missing age comps for the fishing fleet
  # We take the observed age proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_age_comps <- em_input[["L.age.obs"]][["fleet1"]]* em_input[["n.L"]][["fleet1"]]
  if(n_projection_years>0){
    projected_age_comps <- rbind(
      projected_age_comps,
      matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
    )
  }
  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) fishing_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_age_comps)))[x]
    )
  )


  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  # Here we add projection period missing length comps for the fishing fleet
  # We take the observed length proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_length_comps <- em_input[["L.length.obs"]][["fleet1"]]* em_input[["n.L.lengthcomp"]][["fleet1"]]
  projected_length_comps <- rbind(
    projected_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) fishing_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_length_comps)))[x]
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
  fishing_fleet$nyears$set((om_input[["nyr"]]+n_projection_years))
  # Set number of age classes
  fishing_fleet$nages$set(om_input[["nages"]])
  # Set number of length bins
  fishing_fleet$nlengths$set(om_input[["nlengths"]])

  fishing_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    # NOTE: Is this index y correct? It starts at 1 but others start at 0?
    fishing_fleet$log_Fmort[y]$value <- (log(om_output[["f"]][y]))
    fishing_fleet$log_Fmort[y]$estimation_type$set("fixed_effects")
  }

  if(n_projection_years>0){
    for (y in (om_input$nyr+1):(om_input$nyr+n_projection_years)) {
      # Log-transform OM fishing mortality
      # NOTE: Is this index y correct? It starts at 1 but others start at 0?
      fishing_fleet$log_Fmort[y]$value <- (log(projected_F[y-om_input$nyr]))
      fishing_fleet$log_Fmort[y]$estimation_type$set(estim_projected_F[y-om_input$nyr])
    }
  }

  fishing_fleet$log_q[1]$value <- (log(1.0))
  fishing_fleet$log_q[1]$estimation_type$set("constant")
  fishing_fleet$SetSelectivityID(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedLandingsDataID(fishing_fleet_landings$get_id())
  fishing_fleet$SetObservedAgeCompDataID(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetObservedLengthCompDataID(fishing_fleet_length_comp$get_id())

  # Set up fishery index data using the lognormal
  fishing_fleet_landings_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_landings_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input[["nyr"]] + n_projection_years)) {
    # Compute lognormal SD from OM coefficient of variation (CV)
    fishing_fleet_landings_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1))))
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
  fishing_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    fishing_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }

  # Turn off estimation for length-at-age
  fishing_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  fishing_fleet$age_to_length_conversion$set_all_random(FALSE) # I don't think this is needed, both set estimable and set random set to "constant" if false

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- methods::new(Index, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) survey_fleet_index$index_data$set(x - 1, survey_index[x])
  )
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) survey_fleet_index$index_data$set(x - 1, projected_index[x-om_input[["nyr"]]])
    )
  }

  survey_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  projected_survey_age_comps <- em_input[["survey.age.obs"]][["survey1"]]* em_input[["n.survey"]][["survey1"]]
  projected_survey_age_comps <- rbind(
    projected_survey_age_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) survey_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_survey_age_comps)))[x]
    )
  )

  survey_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  projected_survey_length_comps <- em_input[["survey.length.obs"]][["survey1"]]* em_input[["n.survey.lengthcomp"]][["survey1"]]
  projected_survey_length_comps <- rbind(
    projected_survey_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) survey_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_survey_length_comps)))[x]
    )
  )
  # Fleet
  # Create the survey fleet
  survey_fleet_selectivity <- methods::new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point[1]$value <- (om_input[["sel_survey"]][["survey1"]][["A50.sel1"]])

  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  survey_fleet_selectivity$slope[1]$value <- (om_input[["sel_survey"]][["survey1"]][["slope.sel1"]])

  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimation_type$set("fixed_effects")

  survey_fleet <- methods::new(Fleet)
  survey_fleet$nages$set(om_input[["nages"]])
  survey_fleet$nyears$set(om_input[["nyr"]] + n_projection_years)
  survey_fleet$nlengths$set(om_input[["nlengths"]])
  survey_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    # Set very low survey fishing mortality
    survey_fleet$log_Fmort[y]$value <- (-200)
  }
  survey_fleet$log_Fmort$set_all_estimable(FALSE)
  survey_fleet$log_q[1]$value <- (log(om_output[["survey_q"]][["survey1"]]))
  survey_fleet$log_q[1]$estimation_type$set("fixed_effects")
  survey_fleet$SetSelectivityID(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexDataID(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompDataID(survey_fleet_age_comp$get_id())
  survey_fleet$SetObservedLengthCompDataID(survey_fleet_length_comp$get_id())

  # Set up survey index data using the lognormal
  survey_fleet_index_distribution <- methods::new(DlnormDistribution)

  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    survey_fleet_index_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.survey"]][["survey1"]]^2 + 1))))
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
  # TODO: Fix code below to not use 1:x and instead use seq_along() where this
  #       doesn't currently break because we are only testing models with both
  #       age and length data but it would break for only age data.
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    # Transposing the below will have NO impact on the results if the object is
    # already a vector. Additionally, c() ensures that the result is a vector
    # to be consistent but a matrix would be okay.
    # TODO: write a test/documentation to show what order the matrix needs to be
    #       in when passing data to age-length-conversion
    survey_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }
  # Turn off estimation for length-at-age
  survey_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  survey_fleet$age_to_length_conversion$set_all_random(FALSE)

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- methods::new(BevertonHoltRecruitment)
  recruitment_process <- new(LogDevsRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(recruitment_process$get_id())

  # NOTE: in first set of parameters below (for recruitment),
  # $estimation_type (default is "constant")
  # is defined even if it matches the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- (log(om_input[["R0"]]))
  recruitment$log_rzero[1]$estimation_type$set("fixed_effects")
  # set up logit_steep
  recruitment$logit_steep[1]$value <- (-log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2))
  recruitment$logit_steep[1]$estimation_type$set("constant")
  recruitment$nyears$set(om_input[["nyr"]] + n_projection_years)

  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))

  #adding two years for projections
  recruitment$log_devs$resize(om_input[["nyr"]] - 1 + n_projection_years)

  #set values for estimated years
  for (y in 1:(om_input[["nyr"]] - 1)) {
    recruitment$log_devs[y]$value <- (om_input[["logR.resid"]][y + 1])
    recruitment$log_devs[y]$estimation_type$set("random_effects")
  }

  if(n_projection_years>0){
    #set projection years to zero
    for(y in (om_input[["nyr"]]):(om_input[["nyr"]] - 1 + n_projection_years)){
      recruitment$log_devs[y]$value <- (0)
      recruitment$log_devs[y]$estimation_type$set("constant")
    }

  }




  recruitment_distribution <- methods::new(DnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd$resize(1)
  recruitment_distribution$log_sd[1]$value <- (log(om_input[["logR_sd"]]))

  #NOTE: If this doesn't work I would guess that this is the possible source of
  # issues due to the length of x or expected recruitment needing to be the
  # same length as the random effect portion not the whole recruitment vector
  recruitment_distribution$x$resize(om_input[["nyr"]] - 1)
  recruitment_distribution$expected_values$resize(om_input[["nyr"]] - 1)
  for (i in 1:(om_input[["nyr"]] - 1)) {
    recruitment_distribution$x[i]$value <- (0)
    recruitment_distribution$expected_values[i]$value <- (0)
  }

  recruitment_distribution$log_sd[1]$estimation_type$set("fixed_effects")
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
  maturity$inflection_point[1]$value <- (om_input[["A50.mat"]])
  maturity$inflection_point[1]$estimation_type$set("constant")
  maturity$slope[1]$value <- (om_input[["slope.mat"]])
  maturity$slope[1]$estimation_type$set("constant")

  # Population
  population <- methods::new(Population)
  population$log_M$resize((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])
  for (i in 1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])) {
    population$log_M[i]$value <- (log(om_input[["M.age"]][1]))
  }
  population$log_M$set_all_estimable(FALSE)
  population$log_init_naa$resize(om_input[["nages"]])
  for (i in 1:om_input$nages) {
    population$log_init_naa[i]$value <- (log(om_output[["N.age"]][1, i]))
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
  population$nyears$set(om_input[["nyr"]] + n_projection_years)
  population$SetRecruitmentID(recruitment$get_id())
  population$SetGrowthID(ewaa_growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$AddFleet(fishing_fleet$get_id())
  population$AddFleet(survey_fleet$get_id())

  # Set up catch at age model
  # caa <- methods::new(CatchAtAge)
  # caa$AddPopulation(population$get_id())

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

  opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
                       control = list(eval.max = 10000, iter.max = 10000, trace = 0)
  )


  # Call report using MLE parameter values, or
  # the initial values if optimization is skipped
  report <- obj[["report"]](obj[["env"]][["last.par.best"]])


  sdr <- TMB::sdreport(obj)
  sdr_report <- summary(sdr, "report")
  sdr_fixed <- summary(sdr, "fixed")
  row.names(sdr_fixed) <- names(FIMS:::get_parameter_names(sdr_fixed[, 1]))

  clear()

  #Save results for comparison

  sdr_report_5_year_project_catch_low <- sdr_report
  sdr_fixed_5_year_project_catch_low <- sdr_fixed


  ## Setup model to produce 5 projection years with no
  ## index, or composition data and recruitment deviations
  ## in projection years set as constant values. Catch values
  ## for projection years are provided and F is estimated.

  n_projection_years <- 5
  projected_landings <- rep(landings[om_input$nyr]*2,n_projection_years)
  projected_F <- rep(om_output[["f"]][om_input$nyr],n_projection_years)
  estim_projected_F <- rep("fixed_effects",n_projection_years)
  projected_index <- rep(-999,n_projection_years)

  # Clear any previous FIMS settings
  clear()

  # Extract fishing fleet landings data (observed) and initialize index module
  landings <- em_input[["L.obs"]][["fleet1"]]

  # set fishing fleet landings data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery landings
  fishing_fleet_landings <- methods::new(Landings, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) fishing_fleet_landings$landings_data$set(x - 1, landings[x])
  )
  # set projection years
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) fishing_fleet_landings$landings_data$set(x - 1, projected_landings[x-om_input[["nyr"]]])
    )
  }
  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*nages
  fishing_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  # Here we add projection period missing age comps for the fishing fleet
  # We take the observed age proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_age_comps <- em_input[["L.age.obs"]][["fleet1"]]* em_input[["n.L"]][["fleet1"]]
  if(n_projection_years>0){
    projected_age_comps <- rbind(
      projected_age_comps,
      matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
    )
  }
  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) fishing_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_age_comps)))[x]
    )
  )


  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  # Here we add projection period missing length comps for the fishing fleet
  # We take the observed length proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_length_comps <- em_input[["L.length.obs"]][["fleet1"]]* em_input[["n.L.lengthcomp"]][["fleet1"]]
  projected_length_comps <- rbind(
    projected_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) fishing_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_length_comps)))[x]
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
  fishing_fleet$nyears$set((om_input[["nyr"]]+n_projection_years))
  # Set number of age classes
  fishing_fleet$nages$set(om_input[["nages"]])
  # Set number of length bins
  fishing_fleet$nlengths$set(om_input[["nlengths"]])

  fishing_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    # NOTE: Is this index y correct? It starts at 1 but others start at 0?
    fishing_fleet$log_Fmort[y]$value <- (log(om_output[["f"]][y]))
    fishing_fleet$log_Fmort[y]$estimation_type$set("fixed_effects")
  }

  if(n_projection_years>0){
    for (y in (om_input$nyr+1):(om_input$nyr+n_projection_years)) {
      # Log-transform OM fishing mortality
      # NOTE: Is this index y correct? It starts at 1 but others start at 0?
      fishing_fleet$log_Fmort[y]$value <- (log(projected_F[y-om_input$nyr]))
      fishing_fleet$log_Fmort[y]$estimation_type$set(estim_projected_F[y-om_input$nyr])
    }
  }

  fishing_fleet$log_q[1]$value <- (log(1.0))
  fishing_fleet$log_q[1]$estimation_type$set("constant")
  fishing_fleet$SetSelectivityID(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedLandingsDataID(fishing_fleet_landings$get_id())
  fishing_fleet$SetObservedAgeCompDataID(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetObservedLengthCompDataID(fishing_fleet_length_comp$get_id())

  # Set up fishery index data using the lognormal
  fishing_fleet_landings_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_landings_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input[["nyr"]] + n_projection_years)) {
    # Compute lognormal SD from OM coefficient of variation (CV)
    fishing_fleet_landings_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1))))
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
  fishing_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    fishing_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }

  # Turn off estimation for length-at-age
  fishing_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  fishing_fleet$age_to_length_conversion$set_all_random(FALSE) # I don't think this is needed, both set estimable and set random set to "constant" if false

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- methods::new(Index, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) survey_fleet_index$index_data$set(x - 1, survey_index[x])
  )
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) survey_fleet_index$index_data$set(x - 1, projected_index[x-om_input[["nyr"]]])
    )
  }

  survey_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  projected_survey_age_comps <- em_input[["survey.age.obs"]][["survey1"]]* em_input[["n.survey"]][["survey1"]]
  projected_survey_age_comps <- rbind(
    projected_survey_age_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) survey_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_survey_age_comps)))[x]
    )
  )

  survey_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  projected_survey_length_comps <- em_input[["survey.length.obs"]][["survey1"]]* em_input[["n.survey.lengthcomp"]][["survey1"]]
  projected_survey_length_comps <- rbind(
    projected_survey_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) survey_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_survey_length_comps)))[x]
    )
  )
  # Fleet
  # Create the survey fleet
  survey_fleet_selectivity <- methods::new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point[1]$value <- (om_input[["sel_survey"]][["survey1"]][["A50.sel1"]])

  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  survey_fleet_selectivity$slope[1]$value <- (om_input[["sel_survey"]][["survey1"]][["slope.sel1"]])

  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimation_type$set("fixed_effects")

  survey_fleet <- methods::new(Fleet)
  survey_fleet$nages$set(om_input[["nages"]])
  survey_fleet$nyears$set(om_input[["nyr"]] + n_projection_years)
  survey_fleet$nlengths$set(om_input[["nlengths"]])
  survey_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    # Set very low survey fishing mortality
    survey_fleet$log_Fmort[y]$value <- (-200)
  }
  survey_fleet$log_Fmort$set_all_estimable(FALSE)
  survey_fleet$log_q[1]$value <- (log(om_output[["survey_q"]][["survey1"]]))
  survey_fleet$log_q[1]$estimation_type$set("fixed_effects")
  survey_fleet$SetSelectivityID(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexDataID(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompDataID(survey_fleet_age_comp$get_id())
  survey_fleet$SetObservedLengthCompDataID(survey_fleet_length_comp$get_id())

  # Set up survey index data using the lognormal
  survey_fleet_index_distribution <- methods::new(DlnormDistribution)

  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    survey_fleet_index_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.survey"]][["survey1"]]^2 + 1))))
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
  # TODO: Fix code below to not use 1:x and instead use seq_along() where this
  #       doesn't currently break because we are only testing models with both
  #       age and length data but it would break for only age data.
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    # Transposing the below will have NO impact on the results if the object is
    # already a vector. Additionally, c() ensures that the result is a vector
    # to be consistent but a matrix would be okay.
    # TODO: write a test/documentation to show what order the matrix needs to be
    #       in when passing data to age-length-conversion
    survey_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }
  # Turn off estimation for length-at-age
  survey_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  survey_fleet$age_to_length_conversion$set_all_random(FALSE)

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- methods::new(BevertonHoltRecruitment)
  recruitment_process <- new(LogDevsRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(recruitment_process$get_id())

  # NOTE: in first set of parameters below (for recruitment),
  # $estimation_type (default is "constant")
  # is defined even if it matches the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- (log(om_input[["R0"]]))
  recruitment$log_rzero[1]$estimation_type$set("fixed_effects")
  # set up logit_steep
  recruitment$logit_steep[1]$value <- (-log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2))
  recruitment$logit_steep[1]$estimation_type$set("constant")
  recruitment$nyears$set(om_input[["nyr"]] + n_projection_years)

  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))

  #adding two years for projections
  recruitment$log_devs$resize(om_input[["nyr"]] - 1 + n_projection_years)

  #set values for estimated years
  for (y in 1:(om_input[["nyr"]] - 1)) {
    recruitment$log_devs[y]$value <- (om_input[["logR.resid"]][y + 1])
    recruitment$log_devs[y]$estimation_type$set("random_effects")
  }

  if(n_projection_years>0){
    #set projection years to zero
    for(y in (om_input[["nyr"]]):(om_input[["nyr"]] - 1 + n_projection_years)){
      recruitment$log_devs[y]$value <- (0)
      recruitment$log_devs[y]$estimation_type$set("constant")
    }

  }




  recruitment_distribution <- methods::new(DnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd$resize(1)
  recruitment_distribution$log_sd[1]$value <- (log(om_input[["logR_sd"]]))

  #NOTE: If this doesn't work I would guess that this is the possible source of
  # issues due to the length of x or expected recruitment needing to be the
  # same length as the random effect portion not the whole recruitment vector
  recruitment_distribution$x$resize(om_input[["nyr"]] - 1)
  recruitment_distribution$expected_values$resize(om_input[["nyr"]] - 1)
  for (i in 1:(om_input[["nyr"]] - 1)) {
    recruitment_distribution$x[i]$value <- (0)
    recruitment_distribution$expected_values[i]$value <- (0)
  }

  recruitment_distribution$log_sd[1]$estimation_type$set("fixed_effects")
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
  maturity$inflection_point[1]$value <- (om_input[["A50.mat"]])
  maturity$inflection_point[1]$estimation_type$set("constant")
  maturity$slope[1]$value <- (om_input[["slope.mat"]])
  maturity$slope[1]$estimation_type$set("constant")

  # Population
  population <- methods::new(Population)
  population$log_M$resize((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])
  for (i in 1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])) {
    population$log_M[i]$value <- (log(om_input[["M.age"]][1]))
  }
  population$log_M$set_all_estimable(FALSE)
  population$log_init_naa$resize(om_input[["nages"]])
  for (i in 1:om_input$nages) {
    population$log_init_naa[i]$value <- (log(om_output[["N.age"]][1, i]))
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
  population$nyears$set(om_input[["nyr"]] + n_projection_years)
  population$SetRecruitmentID(recruitment$get_id())
  population$SetGrowthID(ewaa_growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$AddFleet(fishing_fleet$get_id())
  population$AddFleet(survey_fleet$get_id())

  # Set up catch at age model
  # caa <- methods::new(CatchAtAge)
  # caa$AddPopulation(population$get_id())

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

  opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
                       control = list(eval.max = 10000, iter.max = 10000, trace = 0)
  )


  # Call report using MLE parameter values, or
  # the initial values if optimization is skipped
  report <- obj[["report"]](obj[["env"]][["last.par.best"]])


  sdr <- TMB::sdreport(obj)
  sdr_report <- summary(sdr, "report")
  sdr_fixed <- summary(sdr, "fixed")
  row.names(sdr_fixed) <- names(FIMS:::get_parameter_names(sdr_fixed[, 1]))

  clear()

  #Save results for comparison

  sdr_report_5_year_project_catch_high <- sdr_report
  sdr_fixed_5_year_project_catch_high <- sdr_fixed

  ## Setup model to produce 5 projection years with no
  ## index, composition, or catch data and recruitment deviations
  ## in projection years set as constant values. SSB priors
  ## are provided with very high log_sd=20 in all but terminal
  ## projection year when log_sd=0 and F is estimated.

  n_projection_years <- 10
  projected_landings <- rep(-999,n_projection_years)
  projected_F <- rep(om_output[["f"]][om_input$nyr],n_projection_years)
  estim_projected_F <- rep("fixed_effects",n_projection_years)
  projected_index <- rep(-999,n_projection_years)

  # Clear any previous FIMS settings
  clear()

  # Extract fishing fleet landings data (observed) and initialize index module
  landings <- em_input[["L.obs"]][["fleet1"]]

  # set fishing fleet landings data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery landings
  fishing_fleet_landings <- methods::new(Landings, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) fishing_fleet_landings$landings_data$set(x - 1, landings[x])
  )
  # set projection years
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) fishing_fleet_landings$landings_data$set(x - 1, projected_landings[x-om_input[["nyr"]]])
    )
  }
  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*nages
  fishing_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  # Here we add projection period missing age comps for the fishing fleet
  # We take the observed age proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_age_comps <- em_input[["L.age.obs"]][["fleet1"]]* em_input[["n.L"]][["fleet1"]]
  if(n_projection_years>0){
    projected_age_comps <- rbind(
      projected_age_comps,
      matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
    )
  }
  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) fishing_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_age_comps)))[x]
    )
  )


  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  # Here we add projection period missing length comps for the fishing fleet
  # We take the observed length proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_length_comps <- em_input[["L.length.obs"]][["fleet1"]]* em_input[["n.L.lengthcomp"]][["fleet1"]]
  projected_length_comps <- rbind(
    projected_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) fishing_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_length_comps)))[x]
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
  fishing_fleet$nyears$set((om_input[["nyr"]]+n_projection_years))
  # Set number of age classes
  fishing_fleet$nages$set(om_input[["nages"]])
  # Set number of length bins
  fishing_fleet$nlengths$set(om_input[["nlengths"]])

  fishing_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    # NOTE: Is this index y correct? It starts at 1 but others start at 0?
    fishing_fleet$log_Fmort[y]$value <- (log(om_output[["f"]][y]))
    fishing_fleet$log_Fmort[y]$estimation_type$set("fixed_effects")
  }

  if(n_projection_years>0){
    for (y in (om_input$nyr+1):(om_input$nyr+n_projection_years)) {
      # Log-transform OM fishing mortality
      # NOTE: Is this index y correct? It starts at 1 but others start at 0?
      fishing_fleet$log_Fmort[y]$value <- (log(projected_F[y-om_input$nyr]))
      fishing_fleet$log_Fmort[y]$estimation_type$set(estim_projected_F[y-om_input$nyr])
    }
  }

  fishing_fleet$log_q[1]$value <- (log(1.0))
  fishing_fleet$log_q[1]$estimation_type$set("constant")
  fishing_fleet$SetSelectivityID(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedLandingsDataID(fishing_fleet_landings$get_id())
  fishing_fleet$SetObservedAgeCompDataID(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetObservedLengthCompDataID(fishing_fleet_length_comp$get_id())

  # Set up fishery index data using the lognormal
  fishing_fleet_landings_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_landings_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input[["nyr"]])) {
    # Compute lognormal SD from OM coefficient of variation (CV)
    fishing_fleet_landings_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1))))
  }
  if(n_projection_years>0){
    for (y in (om_input[["nyr"]] + 1):(om_input[["nyr"]] + n_projection_years)) {
      # Compute lognormal SD from OM coefficient of variation (CV)
      fishing_fleet_landings_distribution$log_sd[y]$value <- 5
    }

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
  fishing_fleet$age_to_length_conversion$resize(om_input[["nages"]] * om_input[["nlengths"]])
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    fishing_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }

  # Turn off estimation for length-at-age
  fishing_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  fishing_fleet$age_to_length_conversion$set_all_random(FALSE) # I don't think this is needed, both set estimable and set random set to "constant" if false

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- methods::new(Index, om_input[["nyr"]] + n_projection_years)
  purrr::walk(
    1:om_input[["nyr"]],
    \(x) survey_fleet_index$index_data$set(x - 1, survey_index[x])
  )
  if(n_projection_years>0){
    purrr::walk(
      (om_input[["nyr"]]+1):(om_input[["nyr"]]+n_projection_years),
      \(x) survey_fleet_index$index_data$set(x - 1, projected_index[x-om_input[["nyr"]]])
    )
  }

  survey_fleet_age_comp <- methods::new(AgeComp, (om_input[["nyr"]] + n_projection_years), om_input[["nages"]])

  projected_survey_age_comps <- em_input[["survey.age.obs"]][["survey1"]]* em_input[["n.survey"]][["survey1"]]
  projected_survey_age_comps <- rbind(
    projected_survey_age_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]]),
    \(x) survey_fleet_age_comp$age_comp_data$set(
      x - 1,
      (c(t(projected_survey_age_comps)))[x]
    )
  )

  survey_fleet_length_comp <- methods::new(LengthComp, (om_input[["nyr"]] + n_projection_years), om_input[["nlengths"]])

  projected_survey_length_comps <- em_input[["survey.length.obs"]][["survey1"]]* em_input[["n.survey.lengthcomp"]][["survey1"]]
  projected_survey_length_comps <- rbind(
    projected_survey_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )

  purrr::walk(
    1:((om_input[["nyr"]] + n_projection_years) * om_input[["nlengths"]]),
    \(x) survey_fleet_length_comp$length_comp_data$set(
      x - 1,
      (c(t(projected_survey_length_comps)))[x]
    )
  )
  # Fleet
  # Create the survey fleet
  survey_fleet_selectivity <- methods::new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point[1]$value <- (om_input[["sel_survey"]][["survey1"]][["A50.sel1"]])

  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
  survey_fleet_selectivity$slope[1]$value <- (om_input[["sel_survey"]][["survey1"]][["slope.sel1"]])

  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimation_type$set("fixed_effects")

  survey_fleet <- methods::new(Fleet)
  survey_fleet$nages$set(om_input[["nages"]])
  survey_fleet$nyears$set(om_input[["nyr"]] + n_projection_years)
  survey_fleet$nlengths$set(om_input[["nlengths"]])
  survey_fleet$log_Fmort$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    # Set very low survey fishing mortality
    survey_fleet$log_Fmort[y]$value <- (-200)
  }
  survey_fleet$log_Fmort$set_all_estimable(FALSE)
  survey_fleet$log_q[1]$value <- (log(om_output[["survey_q"]][["survey1"]]))
  survey_fleet$log_q[1]$estimation_type$set("fixed_effects")
  survey_fleet$SetSelectivityID(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexDataID(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompDataID(survey_fleet_age_comp$get_id())
  survey_fleet$SetObservedLengthCompDataID(survey_fleet_length_comp$get_id())

  # Set up survey index data using the lognormal
  survey_fleet_index_distribution <- methods::new(DlnormDistribution)

  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution$log_sd$resize(om_input[["nyr"]] + n_projection_years)
  for (y in 1:(om_input$nyr + n_projection_years)) {
    survey_fleet_index_distribution$log_sd[y]$value <- (log(sqrt(log(em_input[["cv.survey"]][["survey1"]]^2 + 1))))
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
  # TODO: Fix code below to not use 1:x and instead use seq_along() where this
  #       doesn't currently break because we are only testing models with both
  #       age and length data but it would break for only age data.
  for (i in 1:length(em_input[["age_to_length_conversion"]])) {
    # Transposing the below will have NO impact on the results if the object is
    # already a vector. Additionally, c() ensures that the result is a vector
    # to be consistent but a matrix would be okay.
    # TODO: write a test/documentation to show what order the matrix needs to be
    #       in when passing data to age-length-conversion
    survey_fleet$age_to_length_conversion[i]$value <- (c(t(em_input[["age_to_length_conversion"]]))[i])
  }
  # Turn off estimation for length-at-age
  survey_fleet$age_to_length_conversion$set_all_estimable(FALSE)
  survey_fleet$age_to_length_conversion$set_all_random(FALSE)

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- methods::new(BevertonHoltRecruitment)
  recruitment_process <- new(LogDevsRecruitmentProcess)
  recruitment$SetRecruitmentProcessID(recruitment_process$get_id())

  # NOTE: in first set of parameters below (for recruitment),
  # $estimation_type (default is "constant")
  # is defined even if it matches the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- (log(om_input[["R0"]]))
  recruitment$log_rzero[1]$estimation_type$set("fixed_effects")
  # set up logit_steep
  recruitment$logit_steep[1]$value <- (-log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2))
  recruitment$logit_steep[1]$estimation_type$set("constant")
  recruitment$nyears$set(om_input[["nyr"]] + n_projection_years)

  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))

  #adding two years for projections
  recruitment$log_devs$resize(om_input[["nyr"]] - 1 + n_projection_years)

  #set values for estimated years
  for (y in 1:(om_input[["nyr"]] - 1)) {
    recruitment$log_devs[y]$value <- (om_input[["logR.resid"]][y + 1])
    recruitment$log_devs[y]$estimation_type$set("random_effects")
  }

  if(n_projection_years>0){
    #set projection years to zero
    for(y in (om_input[["nyr"]]):(om_input[["nyr"]] - 1 + n_projection_years)){
      recruitment$log_devs[y]$value <- (0)
      recruitment$log_devs[y]$estimation_type$set("constant")
    }

  }




  recruitment_distribution <- methods::new(DnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd$resize(1)
  recruitment_distribution$log_sd[1]$value <- (log(om_input[["logR_sd"]]))

  #NOTE: If this doesn't work I would guess that this is the possible source of
  # issues due to the length of x or expected recruitment needing to be the
  # same length as the random effect portion not the whole recruitment vector
  recruitment_distribution$x$resize(om_input[["nyr"]] - 1)
  recruitment_distribution$expected_values$resize(om_input[["nyr"]] - 1)
  for (i in 1:(om_input[["nyr"]] - 1)) {
    recruitment_distribution$x[i]$value <-recruitment$log_devs[i]$value
    recruitment_distribution$expected_values[i]$value <- (0)
  }

  recruitment_distribution$log_sd[1]$estimation_type$set("fixed_effects")
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
  maturity$inflection_point[1]$value <- (om_input[["A50.mat"]])
  maturity$inflection_point[1]$estimation_type$set("constant")
  maturity$slope[1]$value <- (om_input[["slope.mat"]])
  maturity$slope[1]$estimation_type$set("constant")

  # Population
  population <- methods::new(Population)
  population$log_M$resize((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])
  for (i in 1:((om_input[["nyr"]] + n_projection_years) * om_input[["nages"]])) {
    population$log_M[i]$value <- (log(om_input[["M.age"]][1]))
  }
  population$log_M$set_all_estimable(FALSE)
  population$log_init_naa$resize(om_input[["nages"]])
  for (i in 1:om_input$nages) {
    population$log_init_naa[i]$value <- (log(om_output[["N.age"]][1, i]))
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
  population$nyears$set(om_input[["nyr"]] + n_projection_years)
  population$SetRecruitmentID(recruitment$get_id())
  population$SetGrowthID(ewaa_growth$get_id())
  population$SetMaturityID(maturity$get_id())
  population$AddFleet(fishing_fleet$get_id())
  population$AddFleet(survey_fleet$get_id())

  #Setup projection prior target
  SSB_prior <- methods::new(DnormDistribution)
  SSB_prior$expected_values$resize((om_input[["nyr"]] + n_projection_years + 1))
  SSB_prior$x$resize((om_input[["nyr"]] + n_projection_years + 1))
  SSB_prior$log_sd$resize((om_input[["nyr"]] + n_projection_years + 1))
  for(y in 1:(om_input[["nyr"]] + 1)){
    SSB_prior$x[y]$value <- 3000
    SSB_prior$expected_values[y]$value <- 3000
    SSB_prior$log_sd[y]$value <- 20
  }
  if(n_projection_years>0){
    for(y in (om_input[["nyr"]] + 2):(om_input[["nyr"]] + 1 + n_projection_years)){
      SSB_prior$x[y]$value <- 3000
      SSB_prior$expected_values[y]$value <- 3000
      SSB_prior$log_sd[y]$value <- -1
    }
  }
  SSB_prior$set_distribution_links("prior", population$spawning_biomass$get_id())

  # Set up catch at age model
  # caa <- methods::new(CatchAtAge)
  # caa$AddPopulation(population$get_id())

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

  opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
                       control = list(eval.max = 10000, iter.max = 10000, trace = 0)
  )


  # Call report using MLE parameter values, or
  # the initial values if optimization is skipped
  report <- obj[["report"]](obj[["env"]][["last.par.best"]])


  sdr <- TMB::sdreport(obj)
  sdr_report <- summary(sdr, "report")
  sdr_fixed <- summary(sdr, "fixed")
  row.names(sdr_fixed) <- names(FIMS:::get_parameter_names(sdr_fixed[, 1]))

  clear()

  #Save results for comparison

  sdr_report_5_year_project_SSB_target <- sdr_report
  sdr_fixed_5_year_project_SSB_target <- sdr_fixed
  #Compare results across model runs

  # #Compare fixed parameter estimates between control and fixed F runs
  # #Results are identical as expected
  # max(sdr_fixed_no_project[,"Estimate"]-sdr_fixed_5_year_project[,"Estimate"])
  # min(sdr_fixed_no_project[,"Estimate"]-sdr_fixed_5_year_project[,"Estimate"])
  #
  # max(sdr_fixed_no_project[,"Std. Error"]-sdr_fixed_5_year_project[,"Std. Error"])
  # max(sdr_fixed_no_project[,"Std. Error"]-sdr_fixed_5_year_project[,"Std. Error"])
  #
  # #Compare fixed parameter estimates between control and estimated F runs with reasonable low catch targets
  # #Results are very similar <1% maximum difference
  # #Not sure why there is any difference? Maybe there is an assumption of symmetry
  # #between the F pars that back propagates the impact of Future F's to past F's???
  # #or this is just the result of rounding error in the final gradient convergence thresholds??
  # max((sdr_fixed_5_year_project_catch_low[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
  # min((sdr_fixed_5_year_project_catch_low[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
  #
  # max((sdr_fixed_5_year_project_catch_low[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])
  # min((sdr_fixed_5_year_project_catch_low[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])
  #
  # #Compare fixed parameter estimates between control and estimated F runs with high catch targets (overfishing)
  # #Results are not all similar something is happening with the Fmort results in particular
  # #This is somewhat expected as the model is being forced to make the future projected catches obtainable.
  # #This interaction means we would need to restrain or detect scenarios where this interaction is occuring.
  # #We need to test if this is just a catch issue or if it effects reference point projections targeting SPR.
  # max((sdr_fixed_5_year_project_catch_high[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
  # min((sdr_fixed_5_year_project_catch_high[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
  #
  # max((sdr_fixed_5_year_project_catch_high[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])
  # min((sdr_fixed_5_year_project_catch_high[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])
  #
  # #Compare fixed parameter estimates between control and estimated F runs with high catch targets (overfishing)
  # #But super high log_sd = 5 on the projection catches so the model isn't forced to fit the targets
  # #Results are almost identical 1.426e-5 max proportion error
  # #In this case the model seems to estimate the most catch it can without changing
  # #the estimation period fits which is likely our best case scenario vs fixed catch
  # #projections such as SS that can go wild when the catches are too high.
  # #This appears to achieve close to a constant F projection which is interesting.
  # max((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
  # min((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
  #
  # max((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])
  # min((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])
  #
  #
  # #Comparison of SSB target results
  # max((sdr_fixed_5_year_project_SSB_target[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
  # min((sdr_fixed_5_year_project_SSB_target[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
  #
  # max((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])
  # min((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])
  #
  # sdr_report_5_year_project_SSB_target[rownames(sdr_report_5_year_project_SSB_target)=="SSB","Estimate"]
  #
  # sdr_report_no_project[rownames(sdr_report_no_project)=="SSB","Estimate"]
  #
