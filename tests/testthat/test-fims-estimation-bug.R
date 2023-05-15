library(FIMS)
library(TMB)

# Find path to FIMS.dll and TMB.dll, then load and unload dlls in test-fims-estimation.R

find_dll_path <- function(package_name) {
  dll_name <- paste(package_name, .Platform$dynlib.ext, sep = "")
  if (dir.exists(file.path(find.package(package_name), "src"))){
    dll_path <- file.path(find.package(package_name), "src", dll_name)
  } else {
    libs_path <- file.path(find.package(package_name), "libs")

    if (.Platform$OS.type == "windows") {
      dll_path <- file.path(libs_path, .Platform$r_arch, dll_name)
    } else {
      dll_path <- file.path(libs_path, dll_name)
    }
  }
}

#remotes::install_github(repo = "Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison")

## Set-up OM (sigmaR = 0.4)
working_dir <- getwd()
message(getwd())
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


unlink(maindir, recursive = T)
message(getwd())

setwd(working_dir)
on.exit(setwd(working_dir), add = TRUE)
# Set-up Rcpp modules and fix parameters to "true"
setup_fims <- function(om_input, om_output, em_input){

  test_env <- new.env(parent = emptyenv())
  test_env$fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Recruitment
  test_env$recruitment <- new(test_env$fims$BevertonHoltRecruitment)
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is taken
  # before the likelihood calculation
  test_env$recruitment$log_sigma_recruit$value <- log(om_input$logR_sd)
  test_env$recruitment$log_rzero$value <- log(om_input$R0)
  test_env$recruitment$log_rzero$is_random_effect <- FALSE
  test_env$recruitment$log_rzero$estimated <- TRUE
  test_env$recruitment$logit_steep$value <- -log(1.0 - om_input$h) + log(om_input$h - 0.2)
  test_env$recruitment$logit_steep$is_random_effect <- FALSE
  test_env$recruitment$logit_steep$estimated <- FALSE
  test_env$recruitment$estimate_deviations <- TRUE
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$deviations <- rep(1, length(om_input$logR.resid))
  test_env$recruitment$deviations <- exp(om_input$logR.resid)

  # Data
  test_env$catch <- em_input$L.obs$fleet1
  test_env$fishing_fleet_index <- new(test_env$fims$Index, length(test_env$catch))
  test_env$fishing_fleet_index$index_data <- test_env$catch
  test_env$fishing_fleet_age_comp <- new(test_env$fims$AgeComp, length(test_env$catch), om_input$nages)
  test_env$fishing_fleet_age_comp$age_comp_data <- c(t(em_input$L.age.obs$fleet1)) * em_input$n.L$fleet1

  test_env$survey_index <-em_input$surveyB.obs$survey1
  test_env$survey_fleet_index <- new(test_env$fims$Index, length(test_env$survey_index))
  test_env$survey_fleet_index$index_data <- test_env$survey_index
  test_env$survey_fleet_age_comp <- new(test_env$fims$AgeComp, length(test_env$survey_index), om_input$nages)
  test_env$survey_fleet_age_comp$age_comp_data <-c(t(em_input$survey.age.obs$survey1)) * em_input$n.survey$survey1

  # Growth
  test_env$ewaa_growth <- new(test_env$fims$EWAAgrowth)
  test_env$ewaa_growth$ages <- om_input$ages
  test_env$ewaa_growth$weights <- om_input$W.mt

  # Maturity
  test_env$maturity <- new(test_env$fims$LogisticMaturity)
  test_env$maturity$median$value <- om_input$A50.mat
  test_env$maturity$median$is_random_effect <- FALSE
  test_env$maturity$median$estimated <- FALSE
  test_env$maturity$slope$value <- om_input$slope
  test_env$maturity$slope$is_random_effect <- FALSE
  test_env$maturity$slope$estimated <- FALSE

  # Fleet
  # Create the fishing fleet
  test_env$fishing_fleet_selectivity <- new(test_env$fims$LogisticSelectivity)
  test_env$fishing_fleet_selectivity$median$value <- om_input$sel_fleet$fleet1$A50.sel1
  test_env$fishing_fleet_selectivity$median$is_random_effect <- FALSE
  test_env$fishing_fleet_selectivity$median$estimated <- TRUE
  test_env$fishing_fleet_selectivity$slope$value <- om_input$sel_fleet$fleet1$slope.sel1
  test_env$fishing_fleet_selectivity$slope$is_random_effect <- FALSE
  test_env$fishing_fleet_selectivity$slope$estimated <- TRUE

  test_env$fishing_fleet <- new(test_env$fims$Fleet)
  test_env$fishing_fleet$nages <- om_input$nages
  test_env$fishing_fleet$nyears <- om_input$nyr
  test_env$fishing_fleet$log_Fmort <- log(om_output$f)
  test_env$fishing_fleet$estimate_F <- TRUE
  test_env$fishing_fleet$random_F <- FALSE
  test_env$fishing_fleet$log_q <- log(1.0)
  test_env$fishing_fleet$estimate_q <- FALSE
  test_env$fishing_fleet$random_q <- FALSE
  test_env$fishing_fleet$log_obs_error$value <- log(sqrt(log(em_input$cv.L$fleet1^2+1)))
  test_env$fishing_fleet$log_obs_error$estimated <- FALSE
  # Need get_id() for setting up observed agecomp and index data?
  test_env$fishing_fleet$SetAgeCompLikelihood(1)
  test_env$fishing_fleet$SetIndexLikelihood(1)
  test_env$fishing_fleet$SetSelectivity(test_env$fishing_fleet_selectivity$get_id())
  test_env$fishing_fleet$SetObservedIndexData(test_env$fishing_fleet_index$get_id())
  test_env$fishing_fleet$SetObservedAgeCompData(test_env$fishing_fleet_age_comp$get_id())

  # Create the survey fleet
  test_env$survey_fleet_selectivity <- new(test_env$fims$LogisticSelectivity)
  test_env$survey_fleet_selectivity$median$value <- om_input$sel_survey$survey1$A50.sel1
  test_env$survey_fleet_selectivity$median$is_random_effect <- FALSE
  test_env$survey_fleet_selectivity$median$estimated <- TRUE
  test_env$survey_fleet_selectivity$slope$value <- om_input$sel_survey$survey1$slope.sel1
  test_env$survey_fleet_selectivity$slope$is_random_effect <- FALSE
  test_env$survey_fleet_selectivity$slope$estimated <- TRUE

  test_env$survey_fleet <- new(test_env$fims$Fleet)
  test_env$survey_fleet$is_survey<-TRUE
  test_env$survey_fleet$nages <- om_input$nages
  test_env$survey_fleet$nyears <- om_input$nyr
  # survey_fleet$log_Fmort <- rep(log(0.0000000000000000000000000001), om_input$nyr) #-Inf?
  test_env$survey_fleet$estimate_F <- FALSE
  test_env$survey_fleet$random_F <- FALSE
  test_env$survey_fleet$log_q <- log(om_output$survey_q$survey1)
  test_env$survey_fleet$estimate_q <- TRUE
  test_env$survey_fleet$random_q <- FALSE
  test_env$survey_fleet$log_obs_error$value <- log(sqrt(log(em_input$cv.survey$survey1^2+1)))
  test_env$survey_fleet$log_obs_error$estimated <- FALSE
  test_env$survey_fleet$SetAgeCompLikelihood(1)
  test_env$survey_fleet$SetIndexLikelihood(1)
  test_env$survey_fleet$SetSelectivity(test_env$survey_fleet_selectivity$get_id())
  test_env$survey_fleet$SetObservedIndexData(test_env$survey_fleet_index$get_id())
  test_env$survey_fleet$SetObservedAgeCompData(test_env$survey_fleet_age_comp$get_id())

  # Population
  test_env$population <- new(test_env$fims$Population)
  # is it a problem these are not Parameters in the Population interface?
  # the Parameter class (from rcpp/rcpp_objects/rcpp_interface_base) cannot handle vectors,
  # do we need a ParameterVector class?
  test_env$population$log_M <- rep(log(om_input$M.age[1]), om_input$nyr*om_input$nages)
  test_env$population$estimate_M <- FALSE
  test_env$population$log_init_naa <- log(om_output$N.age[1, ])
  test_env$population$estimate_init_naa <- TRUE
  test_env$population$nages <- om_input$nages
  test_env$population$ages <- om_input$ages
  test_env$population$nfleets <- sum(om_input$fleet_num, om_input$survey_num)
  test_env$population$nseasons <- 1
  test_env$population$nyears <- om_input$nyr
  test_env$population$prop_female <- om_input$proportion.female[1]
  test_env$population$SetMaturity(test_env$maturity$get_id())
  test_env$population$SetGrowth(test_env$ewaa_growth$get_id())
  test_env$population$SetRecruitment(test_env$recruitment$get_id())

  return(test_env)
}

deterministic_env <- setup_fims(om_input = om_input,
                                om_output = om_output,
                                em_input = em_input)
# Set-up TMB
deterministic_env$fims$CreateTMBModel()
# Create parameter list from Rcpp modules
parameters <- list(p = deterministic_env$fims$get_fixed())
obj <- MakeADFun(data=list(), parameters, DLL="FIMS")

sdr <- TMB::sdreport(obj)
sdr_fixed <- summary(sdr, "fixed")

report <- obj$report()

deterministic_env$fims$clear()
TMB::FreeADFun(obj)

###################################
## Code will cause segfault on Windows
## when lines 195-198 commented out
# dyn.unload(find_dll_path("FIMS"))
# dyn.unload(find_dll_path("TMB"))
# dyn.load(find_dll_path("FIMS"))
# dyn.load(find_dll_path("TMB"))
#################################

nll_env <- setup_fims(om_input = om_input,
                      om_output = om_output,
                      em_input = em_input)
# Set-up TMB
nll_env$fims$CreateTMBModel()
parameters <- list(p = nll_env$fims$get_fixed())

obj <- TMB::MakeADFun(data=list(), parameters, DLL="FIMS")
nll_env$fims$clear()
TMB::FreeADFun(obj)

dyn.unload(find_dll_path("FIMS"))
dyn.unload(find_dll_path("TMB"))
