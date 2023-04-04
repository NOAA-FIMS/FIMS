## The following packages need to be updated prior to debugging:
# install.packages("Matrix")
# install.packages("TMB", type = "source")
# use updated version of pkgbuild with correct compilation flags for debugging:
# remotes::install_github(repo = "Andrea-Havron-NOAA/pkgbuild")

devtools::load_all()

library(FIMS)
data(package = "FIMS")

# remove from test_that wrapper for debugging
# test_that("deterministic test of fims", {

## Install required packages
required_pkg <- c("remotes", "devtools", "here")
pkg_to_install <- required_pkg[!(required_pkg %in%
  installed.packages()[, "Package"])]
if (length(pkg_to_install)) install.packages(pkg_to_install)

remotes::install_github(repo = "Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison")
library(ASSAMC)

## Set-up OM (sigmaR = 0.4)
maindir <- file.path(here::here(), "tests", "testthat", "_snaps")
model_input <- save_initial_input()
FIMS_C0 <- save_initial_input(
  base_case = TRUE,
  input_list = model_input,
  maindir = maindir,
  om_sim_num = 1,
  keep_sim_num = 1,
  figure_number = 1,
  seed_num = 9924,
  case_name = "FIMS_C0"
)

run_om(input_list = FIMS_C0)

## Set-up Rcpp modules and fix parameters to "true"
fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
# fims$clear()

# Recruitment
recruitment <- new(fims$BevertonHoltRecruitment)
recruitment$rzero$value <- om_input$R0
recruitment$rzero$is_random_effect <- FALSE
recruitment$rzero$estimated <- TRUE
recruitment$steep$value <- om_input$h
recruitment$steep$is_random_effect <- FALSE
recruitment$steep$estimated <- TRUE

recruitment_nll <- new(fims$RecruitmentNLL)
recruitment_nll$do_bias_correction <- FALSE
recruitment_nll$estimate_recruit_deviations <- TRUE
recruitment_nll$log_sigma_recruit$value <- om_input$logR_sd
recruitment_nll$log_sigma_recruit$is_random_effect <- FALSE
recruitment_nll$log_sigma_recruit$estimated <- TRUE
recruitment_nll$recruitment_bias_adj <- rep(0.0, om_input$nyr)
recruitment_nll$recruitment_devs <- om_input$logR.resid

# Growth
ewaa_growth <- new(fims$EWAAgrowth)
age_frame <- FIMSFrameAge(data_mile1)
ewaa_growth$ages <- m_ages(age_frame)
ewaa_growth$weights <- m_weightatage(age_frame)

# Maturity
maturity <- new(fims$LogisticMaturity)
maturity$median$value <- om_input$A50.mat
maturity$median$is_random_effect <- FALSE
maturity$median$estimated <- TRUE
maturity$slope$value <- om_input$slope
maturity$slope$is_random_effect <- FALSE
maturity$slope$estimated <- TRUE

# Fleet
# Create the fishing fleet
fishing_fleet_selectivity <- new(fims$LogisticSelectivity)
fishing_fleet_selectivity$median$value <- om_input$sel_fleet$fleet1$A50.sel1
fishing_fleet_selectivity$median$is_random_effect <- FALSE
fishing_fleet_selectivity$median$estimated <- TRUE
fishing_fleet_selectivity$slope$value <- om_input$sel_fleet$fleet1$slope.sel1
fishing_fleet_selectivity$slope$is_random_effect <- FALSE
fishing_fleet_selectivity$slope$estimated <- TRUE

fishing_fleet <- new(fims$Fleet)
# Need get_id() for setting up observed agecomp and index data?
fishing_fleet$nages <- om_input$nages
fishing_fleet$nyears <- om_input$nyr
fishing_fleet$log_Fmort <- log(om_output$f)
fishing_fleet$estimate_F <- TRUE
fishing_fleet$random_F <- FALSE
fishing_fleet$log_q <- rep(log(1.0), om_input$nyr)
fishing_fleet$estimate_q <- TRUE
fishing_fleet$random_q <- FALSE
fishing_fleet$SetAgeCompLikelihood(1)
fishing_fleet$SetIndexLikelihood(1)
fishing_fleet$SetObservedAgeCompData(1, as.matrix(c(t(em_input$L.age.obs$fleet1)))) 
fishing_fleet$SetObservedIndexData(1, em_input$L.obs$fleet1)
fishing_fleet$SetSelectivity(fishing_fleet_selectivity$get_id()) 

# Create the survey fleet
survey_fleet_selectivity <- new(fims$LogisticSelectivity)
survey_fleet_selectivity$median$value <- om_input$sel_survey$survey1$A50.sel1
survey_fleet_selectivity$median$is_random_effect <- FALSE
survey_fleet_selectivity$median$estimated <- TRUE
survey_fleet_selectivity$slope$value <- om_input$sel_survey$survey1$slope.sel1
survey_fleet_selectivity$slope$is_random_effect <- FALSE
survey_fleet_selectivity$slope$estimated <- TRUE

survey_fleet <- new(fims$Fleet)
survey_fleet$nages <- om_input$nages
survey_fleet$nyears <- om_input$nyr
survey_fleet$log_Fmort <- rep(log(0.0), om_input$nyr) #-Inf?
survey_fleet$estimate_F <- TRUE
survey_fleet$random_F <- FALSE
survey_fleet$log_q <- rep(log(om_output$survey_q$survey1), om_input$nyr)
survey_fleet$estimate_q <- TRUE
survey_fleet$random_q <- FALSE
survey_fleet$SetAgeCompLikelihood(1)
survey_fleet$SetIndexLikelihood(1)
survey_fleet$SetObservedAgeCompData(2, as.matrix(c(t(em_input$survey.age.obs$survey1)))) 
survey_fleet$SetObservedIndexData(2, em_input$survey.obs$survey1) 
survey_fleet$SetSelectivity(survey_fleet_selectivity$get_id()) 

# Population
population <- new(fims$Population)
# is it a problem these are not Parameters in the Population interface?
# the Parameter class (from rcpp/rcpp_objects/rcpp_interface_base) cannot handle vectors, do we need a ParameterVector class?
population$log_M <- rep(log(om_input$M.age), om_input$nyr)
population$log_init_naa <- log(om_output$N.age[1, ])
population$nages <- om_input$nages
population$ages <- om_input$ages * 1.0
population$nfleets <- sum(om_input$fleet_num, om_input$survey_num)
population$nseasons <- 1
population$nyears <- om_input$nyr
population$prop_female <- om_input$proportion.female[1]
population$SetMaturity(maturity$get_id())
population$SetGrowth(ewaa_growth$get_id())
population$SetRecruitment(recruitment$get_id())

## Set-up TMB
fims$CreateTMBModel()
# # Create parameter list from Rcpp modules
parameters <- list(p = fims$get_fixed())
# #crashes at population line below with year = 1 and index_ya=12
# 364             this->recruitment->evaluate(this->spawning_biomass[year - 1], phi0) *
obj <- MakeADFun(data=list(), parameters, DLL="FIMS")
# message("success!")
#report <- obj$report()

# Test
# TO DO:
# - extract TMB output
# - use for loop to check individual values from a vector
# - use absolute relative error later
# - set up tolerance values later

# # Numbers at age
# expect_equal(report, c(t(om_output$N.age)))
# # Biomass
# expect_equal(report, om_output$biomass.mt)
# # Spawning biomass
# expect_equal(report, om_output$SSB)
# # Expected catch
# expect_equal(report, om_output$L.mt)
# # Expected index
# expect_equal(report, om_output$survey_index_biomass)
# })