## The following packages need to be updated prior to debugging:
# install.packages("Matrix")
# install.packages("TMB", type = "source")
# use updated version of pkgbuild with correct compilation flags for debugging:
# remotes::install_github(repo = "Andrea-Havron-NOAA/pkgbuild")


library(FIMS)
library(jsonlite)


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


# Recruitment
recruitment <- new(BevertonHoltRecruitment)
# logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is taken
# before the likelihood calculation
recruitment$log_sigma_recruit$value <- log(om_input$logR_sd)
recruitment$log_rzero$value <- log(om_input$R0 + runif(1, min = 0, max = 1000000))
recruitment$log_rzero$is_random_effect <- FALSE
recruitment$log_rzero$estimated <- TRUE
recruitment$logit_steep$value <- -log(1.0 - om_input$h) + log(om_input$h - 0.2)
recruitment$logit_steep$is_random_effect <- FALSE
recruitment$logit_steep$estimated <- FALSE
recruitment$estimate_log_devs <- TRUE
# recruit deviations should enter the model in normal space.
# The log is taken in the likelihood calculations
# alternative setting: recruitment$deviations <- rep(1, length(om_input$logR.resid))
recruitment$log_devs <- (om_input$logR.resid)

# Data
catch <- em_input$L.obs$fleet1
fishing_fleet_index <- new(Index, length(catch))
fishing_fleet_index$index_data <- catch
fishing_fleet_age_comp <- new(AgeComp, length(catch), om_input$nages)
fishing_fleet_age_comp$age_comp_data <- c(t(em_input$L.age.obs$fleet1)) * em_input$n.L$fleet1

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
fishing_fleet_selectivity$inflection_point$estimated <- TRUE
fishing_fleet_selectivity$slope$value <- om_input$sel_fleet$fleet1$slope.sel1
fishing_fleet_selectivity$slope$is_random_effect <- FALSE
fishing_fleet_selectivity$slope$estimated <- TRUE

fishing_fleet <- new(Fleet)
fishing_fleet$nages <- om_input$nages
fishing_fleet$nyears <- om_input$nyr
fishing_fleet$log_Fmort <- log(om_output$f)
fishing_fleet$estimate_F <- TRUE
fishing_fleet$random_F <- FALSE
fishing_fleet$log_q$value <- log(1.0)

fishing_fleet$log_obs_error <- log(sqrt(log(em_input$cv.L$fleet1^2 + 1)))

# Need get_id() for setting up observed agecomp and index data?
fishing_fleet$SetAgeCompLikelihood(1)
fishing_fleet$SetIndexLikelihood(1)
fishing_fleet$SetSelectivity(fishing_fleet_selectivity$get_id())
fishing_fleet$SetObservedIndexData(fishing_fleet_index$get_id())
fishing_fleet$SetObservedAgeCompData(fishing_fleet_age_comp$get_id())

# Create the survey fleet
survey_fleet_selectivity <- new(LogisticSelectivity)
survey_fleet_selectivity$inflection_point$value <- om_input$sel_survey$survey1$A50.sel1
survey_fleet_selectivity$inflection_point$is_random_effect <- FALSE
survey_fleet_selectivity$inflection_point$estimated <- TRUE
survey_fleet_selectivity$slope$value <- om_input$sel_survey$survey1$slope.sel1
survey_fleet_selectivity$slope$is_random_effect <- FALSE
survey_fleet_selectivity$slope$estimated <- TRUE

survey_fleet <- new(Fleet)
survey_fleet$is_survey <- TRUE
survey_fleet$nages <- om_input$nages
survey_fleet$nyears <- om_input$nyr
# survey_fleet$log_Fmort <- rep(log(0.0000000000000000000000000001), om_input$nyr) #-Inf?
survey_fleet$estimate_F <- FALSE
survey_fleet$random_F <- FALSE
survey_fleet$log_q$value <- log(om_output$survey_q$survey1)

survey_fleet$log_obs_error <- log(sqrt(log(em_input$cv.survey$survey1^2 + 1)))

survey_fleet$SetAgeCompLikelihood(1)
survey_fleet$SetIndexLikelihood(1)
survey_fleet$SetSelectivity(survey_fleet_selectivity$get_id())
survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
survey_fleet$SetObservedAgeCompData(survey_fleet_age_comp$get_id())

# Population
population <- new(Population)
# is it a problem these are not Parameters in the Population interface?
# the Parameter class (from rcpp/rcpp_objects/rcpp_interface_base) cannot handle vectors,
# do we need a ParameterVector class?
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



PrepareFIMS<-function(){
    ## Set-up TMB
    CreateTMBModel()

    parameters <- list(p = get_fixed())
    obj <- TMB::MakeADFun(data = list(), parameters, DLL = "FIMS")
    SetFIMSFunctions(obj$fn, obj$gr)
    return(obj)
}
obj<-PrepareFIMS()

opt<-optim(obj$par, obj$fn, obj$gr,
  method = "BFGS",
  control = list(maxit = 1000000, reltol = 1e-15)
)

cat(obj$p)
opt
Finalize(opt$p)


obj$fn(opt$p)
opt

Finalize(opt$p)
write(ToJSON(), file = "fims_output.json")
cat(ToJSON())
fromJSON(ToJSON())


