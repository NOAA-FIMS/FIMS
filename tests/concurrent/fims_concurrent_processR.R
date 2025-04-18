library(FIMS)
library(minimizR)
library(processR)


# get the Rcpp module
p <- Rcpp::Module(module = "processR", PACKAGE = "processR")



## Set-up OM (sigmaR = 0.4)
working_dir <- getwd()

maindir <- tempdir()
model_input <- ASSAMC::save_initial_input()
FIMS_C0_estimation <- ASSAMC::save_initial_input(
  base_case = TRUE,
  input_list = model_input,
  maindir = maindir,
  om_sim_num = 1,
  keep_sim_num = 1,
  figure_number = 1,
  seed_num = 9924,
  case_name = "FIMS_C0_estimation"
)

ASSAMC::run_om(input_list = FIMS_C0_estimation)

on.exit(unlink(maindir, recursive = T), add = TRUE)

setwd(working_dir)



NUMBER_OF_MODEL_RUNS <- 100


# Fix and change R0 randomly to get different output for each
# model run.
init_fims <- function(i) {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")


  # Extract fishing fleet landings data (observed) and initialize index module
  catch <- c(t(em_input[["L.obs"]][["fleet1"]]))
  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_index <- methods::new(Index, om_input[["nyr"]])
  fishing_fleet_index$index_data$fromR(catch)
  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*nages
  fishing_fleet_age_comp <- methods::new(AgeComp, om_input[["nyr"]], om_input[["nages"]])
  # Here we fill in the values for the object with the observed age comps for fleet one
  # we multiply these proportions by the sample size for likelihood weighting
  fleet_age_comp <- c(t(em_input[["L.age.obs"]][["fleet1"]]))
  fishing_fleet_age_comp$age_comp_data$fromR(fleet_age_comp)

  # Fleet
  # Create the fishing fleet
  fishing_fleet_selectivity <- methods::new(LogisticSelectivity)
  fishing_fleet_selectivity$inflection_point[1]$value <- om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]]
  fishing_fleet_selectivity$inflection_point[1]$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  fishing_fleet_selectivity$inflection_point[1]$estimated <- TRUE
  fishing_fleet_selectivity$slope[1]$value <- om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]]
  # turn on estimation of slope
  fishing_fleet_selectivity$slope[1]$is_random_effect <- FALSE
  fishing_fleet_selectivity$slope[1]$estimated <- TRUE

  # Initialize the fishing fleet module
  fishing_fleet <- methods::new(Fleet)
  # Set number of years
  # fishing_fleet$nyears<-10
  # om_input[["nyr"]]



  # typeof(fishing_fleet$nyears)

  fishing_fleet$nyears$set(as.integer(om_input[["nyr"]]))

  # Set number of age classes
  fishing_fleet$nages$set(om_input[["nages"]])
  # Set number of length bins
  # fishing_fleet$nlengths$set(om_input[["nlengths"]])


  fishing_fleet$log_Fmort$resize(om_input[["nyr"]])
  for (y in 1:om_input$nyr) {
    # Log-transform OM fishing mortality
    fishing_fleet$log_Fmort[y]$value <- log(om_output[["f"]][y])
  }
  fishing_fleet$log_Fmort$set_all_estimable(TRUE)
  fishing_fleet$log_q[1]$value <- log(1.0)
  fishing_fleet$estimate_q <- FALSE
  fishing_fleet$random_q <- FALSE
  fishing_fleet$SetSelectivity(fishing_fleet_selectivity$get_id())
  fishing_fleet$SetObservedIndexData(fishing_fleet_index$get_id())
  fishing_fleet$SetObservedAgeCompData(fishing_fleet_age_comp$get_id())
  # fishing_fleet$SetObservedLengthCompData(fishing_fleet_length_comp$get_id())

  # Set up fishery index data using the lognormal
  fishing_fleet_index_distribution <- methods::new(DlnormDistribution)
  # lognormal observation error transformed on the log scale
  fishing_fleet_index_distribution$log_sd$resize(om_input[["nyr"]])
  for (y in 1:om_input[["nyr"]]) {
    # Compute lognormal SD from OM coefficient of variation (CV)
    fishing_fleet_index_distribution$log_sd[y]$value <- log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1)))
  }
  fishing_fleet_index_distribution$log_sd$set_all_estimable(FALSE)
  # Set Data using the IDs from the modules defined above
  fishing_fleet_index_distribution$set_observed_data(fishing_fleet$GetObservedIndexDataID())
  fishing_fleet_index_distribution$set_distribution_links("data", fishing_fleet$log_expected_index$get_id())

  # Set up fishery age composition data using the multinomial
  fishing_fleet_agecomp_distribution <- methods::new(DmultinomDistribution)
  fishing_fleet_agecomp_distribution$dims$resize(2)
  fishing_fleet_agecomp_distribution$dims$set(0, om_input[["nyr"]])
  fishing_fleet_agecomp_distribution$dims$set(1, om_input[["nages"]])

  fishing_fleet_agecomp_distribution$set_observed_data(fishing_fleet$GetObservedAgeCompDataID())
  fishing_fleet_agecomp_distribution$set_distribution_links("data", fishing_fleet$proportion_catch_numbers_at_age$get_id())

  # Set up fishery length composition data using the multinomial
  # fishing_fleet_lengthcomp_distribution <- methods::new(DmultinomDistribution)
  # fishing_fleet_lengthcomp_distribution$set_observed_data(fishing_fleet$GetObservedLengthCompDataID())
  # fishing_fleet_lengthcomp_distribution$set_distribution_links("data", fishing_fleet$proportion_catch_numbers_at_length$get_id())

  # Set age-to-length conversion matrix
  # TODO: If an age_to_length_conversion matrix is provided, the code below
  # still executes. Consider adding a check in the Rcpp interface to ensure
  # users provide a vector of inputs.
  # fishing_fleet$age_length_conversion_matrix <- methods::new(
  #  ParameterVector,
  #  c(t(em_input[["age_to_length_conversion"]])),
  #  om_input[["nages"]] * om_input[["nlengths"]]
  # )
  # Turn off estimation for length-at-age
  # fishing_fleet$age_length_conversion_matrix$set_all_estimable(FALSE)
  # fishing_fleet$age_length_conversion_matrix$set_all_random(FALSE)

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- c(t(em_input[["surveyB.obs"]][["survey1"]]))
  survey_fleet_index <- methods::new(Index, om_input[["nyr"]])
  survey_fleet_index$index_data$fromR(survey_index)

  survey_age_comp <- c(t(em_input[["survey.age.obs"]][["survey1"]]))
  survey_fleet_age_comp <- methods::new(AgeComp, om_input[["nyr"]], om_input[["nages"]])
  survey_fleet_age_comp$age_comp_data$fromR(survey_age_comp)
  # survey_lengthcomp <- em_input[["survey.length.obs"]][["survey1"]]
  # survey_fleet_length_comp <- methods::new(LengthComp, om_input[["nyr"]], om_input[["nlengths"]])
  # survey_fleet_length_comp$length_comp_data <- new(RealVector,
  # 					       c(t(survey_lengthcomp)) * em_input[["n.survey.lengthcomp"]][["survey1"]],
  # 					       length(c(t(survey_lengthcomp))))
  # Fleet
  # Create the survey fleet
  survey_fleet_selectivity <- methods::new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point[1]$value <- om_input[["sel_survey"]][["survey1"]][["A50.sel1"]]
  survey_fleet_selectivity$inflection_point[1]$is_random_effect <- FALSE
  # turn on estimation of inflection_point
  survey_fleet_selectivity$inflection_point[1]$estimated <- TRUE
  survey_fleet_selectivity$slope[1]$value <- om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]
  survey_fleet_selectivity$slope[1]$is_random_effect <- FALSE
  # turn on estimation of slope
  survey_fleet_selectivity$slope[1]$estimated <- TRUE

  survey_fleet <- methods::new(Fleet)
  survey_fleet$is_survey <- TRUE
  survey_fleet$nages$set(om_input[["nages"]])
  survey_fleet$nyears$set(om_input[["nyr"]])
  # survey_fleet$nlengths$set(om_input[["nlengths"]])
  survey_fleet$log_q[1]$value <- log(om_output[["survey_q"]][["survey1"]])
  survey_fleet$log_q[1]$estimated <- TRUE
  survey_fleet$estimate_q <- TRUE
  survey_fleet$random_q <- FALSE
  survey_fleet$SetSelectivity(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompData(survey_fleet_age_comp$get_id())
  # survey_fleet$SetObservedLengthCompData(survey_fleet_length_comp$get_id())

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
  survey_fleet_index_distribution$set_distribution_links("data", survey_fleet$log_expected_index$get_id())

  # Age composition distribution
  survey_fleet_agecomp_distribution <- methods::new(DmultinomDistribution)
  survey_fleet_agecomp_distribution$dims$resize(2)
  survey_fleet_agecomp_distribution$dims$set(0, om_input[["nyr"]])
  survey_fleet_agecomp_distribution$dims$set(1, om_input[["nages"]])
  survey_fleet_agecomp_distribution$set_observed_data(survey_fleet$GetObservedAgeCompDataID())
  survey_fleet_agecomp_distribution$set_distribution_links("data", survey_fleet$proportion_catch_numbers_at_age$get_id())

  # Length composition distribution
  # survey_fleet_lengthcomp_distribution <- methods::new(DmultinomDistribution)
  # survey_fleet_lengthcomp_distribution$set_observed_data(survey_fleet$GetObservedLengthCompDataID())
  # survey_fleet_lengthcomp_distribution$set_distribution_links("data", survey_fleet$proportion_catch_numbers_at_length$get_id()) # Set age to length conversion matrix
  # survey_fleet$age_length_conversion_matrix <- methods::new(
  #  ParameterVector,
  #  c(t(em_input[["age_to_length_conversion"]])),
  #  om_input[["nages"]] * om_input[["nlengths"]]
  # )
  # Turn off estimation for length-at-age
  survey_fleet$age_length_conversion_matrix$set_all_estimable(FALSE)
  survey_fleet$age_length_conversion_matrix$set_all_random(FALSE)

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- methods::new(BevertonHoltRecruitment)

  # NOTE: in first set of parameters below (for recruitment),
  # $is_random_effect (default is FALSE) and $estimated (default is FALSE)
  # are defined even if they match the defaults in order to provide an example
  # of how that is done. Other sections of the code below leave defaults in
  # place as appropriate.

  # set up log_rzero (equilibrium recruitment)
  recruitment$log_rzero[1]$value <- log(om_input[["R0"]])
  recruitment$log_rzero[1]$is_random_effect <- FALSE
  recruitment$log_rzero[1]$estimated <- TRUE
  # set up logit_steep
  recruitment$logit_steep[1]$value <- -log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2)
  recruitment$logit_steep[1]$is_random_effect <- FALSE
  recruitment$logit_steep[1]$estimated <- FALSE
  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))
  recruitment$log_devs$resize(om_input[["nyr"]] - 1)
  for (y in 1:(om_input[["nyr"]] - 1)) {
    recruitment$log_devs[y]$value <- om_input[["logR.resid"]][y + 1]
  }
  recruitment_distribution <- methods::new(DnormDistribution)
  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution$log_sd <- methods::new(ParameterVector, 1)
  recruitment_distribution$log_sd[1]$value <- log(om_input[["logR_sd"]])
  recruitment_distribution$log_sd[1]$estimated <- FALSE
  recruitment_distribution$x$resize(om_input[["nyr"]] - 1)
  recruitment_distribution$expected_values$resize(om_input[["nyr"]] - 1)
  for (i in 1:(om_input[["nyr"]] - 1)) {
    recruitment_distribution$x[i]$value <- 0
    recruitment_distribution$expected_values[i]$value <- 0
  }
  recruitment_distribution$set_distribution_links("random_effects", recruitment$log_devs$get_id())
  recruitment$estimate_log_devs <- TRUE

  # Growth
  ewaa_growth <- methods::new(EWAAgrowth)

  ewaa_growth$ages$resize(om_input[["nages"]])
  for (i in 0:(om_input[["nages"]] - 1)) {
    ewaa_growth$ages$set(i, om_input[["ages"]][[i + 1]])
  }

  ewaa_growth$weights$resize(om_input[["nages"]])
  for (i in 0:(om_input[["nages"]] - 1)) {
    ewaa_growth$weights$set(i, om_input[["W.mt"]][[i + 1]])
  }


  # Maturity
  maturity <- methods::new(LogisticMaturity)
  maturity$inflection_point[1]$value <- om_input[["A50.mat"]]
  maturity$inflection_point[1]$is_random_effect <- FALSE
  maturity$inflection_point[1]$estimated <- FALSE
  maturity$slope[1]$value <- om_input[["slope.mat"]]
  maturity$slope[1]$is_random_effect <- FALSE
  maturity$slope[1]$estimated <- FALSE

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
  for (i in 0:(om_input[["nages"]] - 1)) {
    population$ages$set(i, om_input[["ages"]][[i + 1]])
  }

  population$nyears$set(om_input[["nyr"]])
  population$nfleets$set(sum(om_input[["fleet_num"]], om_input[["survey_num"]]))
  population$nseasons$set(1)



  population$SetRecruitment(recruitment$get_id())
  population$SetGrowth(ewaa_growth$get_id())
  population$SetMaturity(maturity$get_id())
  ## Set-up TMB
  CreateTMBModel()
  parameters <- list(p = get_fixed())
  obj <- TMB::MakeADFun(data = list(), parameters, DLL = "FIMS")

  clear()
  return(obj)
}




run_fims <- function() {
  results <<- list()
  library(FIMS)
  library(minimizR)
  minimizer <- 2


  id <- processR.rank
  index <- 1
  for (i in begin[id]:end[id]) {
    print(paste(id, paste(i, "me be here")))
    obj <- init_fims(i)
    if (minimizer == 1) {
      results[[index]] <<- optim(obj$par, obj$fn, obj$gr,
        method = "BFGS",
        control = list(maxit = 1000000, reltol = 1e-15)
      )
    } else {
      results[[index]] <<- minimizR(obj$par, obj$fn, obj$gr, control = list(tolerance = 1e-4, verbose = FALSE))
    }
    # write(x = i, file = paste0(paste("id",processR.rank)),".txt")
    index <- index + 1
  }
  # return(results)
}







id <- 0 # mpi.comm.rank(comm = 0)
ns <- processR::HardwareConcurrency()

nsims <- NUMBER_OF_MODEL_RUNS

begin <- rep(0, ns)
end <- rep(0, ns)

# create scenario segments
if (id == 0) {
  segments <- nsims / ns
  print(paste("segments ", segments))
  for (i in 1:ns) {
    if (i < ns) {
      begin[i] <- as.integer((i - 1) * segments + 1)
      end[i] <- as.integer(i * segments)
    } else {
      begin[i] <- as.integer((i - 1) * segments + 1)
      end[i] <- nsims
    }
  }
  print(begin)
  print(end)
}



start <- Sys.time()

# create a pool of child processes
pool <- list()

for (i in 1:processR::HardwareConcurrency()) {
  # create a new child process
  pool[[i]] <- new(p$Process)

  # start the process. pass the function, environment, and rank
  pool[[i]]$start(run_fims, environment(), i)
}

# iterate of the children and capture information
for (i in 1:processR::HardwareConcurrency()) {
  # wait for the child to complete
  pool[[i]]$wait()

  # get child out stream
  # message<-pool[[i]]$get_message()

  # access the childs environment
  env <- as.environment(pool[[i]]$get_environment())
  # show minimizer results
  # print(env[["results"]])
}

end_ <- Sys.time()



runtime <- (end_ - start)
print(paste0(paste0(paste0(NUMBER_OF_MODEL_RUNS, " model runs completed in "), runtime), " seconds."))

print(begin)
print(end)

line <- paste("processR runtime ", runtime)
write(line, file = "time.txt", append = TRUE)
