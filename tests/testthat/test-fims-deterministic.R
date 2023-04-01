library(FIMS)
data(package = "FIMS")

test_that("deterministic test of fims", {

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
recruitment_nll$log_sigma_recruit$value <- log(0.4)
recruitment_nll$recruitment_devs <- om_input$logR.resid
recruitment_nll$estimate_recruit_deviations <- TRUE

# Growth
growth <- new(fims$EWAAgrowth)
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

# Natural mortality
#create new population module and set log_M directly
population <- new(fims$Population)

  population$log_M <- log(om_input$M.age)
  population$log_init_naa <- log(om_output$N.age[1,])

# Create the population:
population <- new(fims$Population)
population$nages <- om_input$nages
population$nfleets <- 2
population$nseasons <- 1
population$nyears <- om_input$nyr
population$prop_female <- om_input$proportion.female[1]

# Fleet
# Create the fleet
# Fleet selectivity settings
fleet_selectivity <- new(fims$LogisticSelectivity)
  fleet_selectivity$median$value <- 10.0
  fleet_selectivity$median$min <- 8.0
  fleet_selectivity$median$max <- 12.0
  fleet_selectivity$median$is_random_effect <- FALSE
  fleet_selectivity$median$estimated <- TRUE
  fleet_selectivity$slope$value <- 0.2
  fleet_selectivity$slope$is_random_effect <- FALSE
  fleet_selectivity$slope$estimated <- TRUE

fleet <- new(fims$Fleet)
fleet$SetSelectivity(fleet_selectivity$get_id())

fleet$nages <- om_input$nages
#fleet$nseasons <- 1
fleet$nyears <- om_input$nyr
fleet$log_q <- 0
#fleet$log_Fmort$value <- log(om_output$f)
fleet$log_Fmort <- log(om_output$f)
#not working: fleet$log_Fmort$estimated <- TRUE

survey_selectivity <- new(fims$LogisticSelectivity)
  survey_selectivity$median$value <- 10.0
  survey_selectivity$median$min <- 8.0
  survey_selectivity$median$max <- 12.0
  survey_selectivity$median$is_random_effect <- FALSE
  survey_selectivity$median$estimated <- TRUE
  survey_selectivity$slope$value <- 0.2
  survey_selectivity$slope$is_random_effect <- FALSE
  survey_selectivity$slope$estimated <- TRUE

survey <- new(fims$Fleet)
survey$log_q <- log(0.00003)
survey$log_Fmort <- log(om_output$f)
survey$nages <- om_input$nages
survey$nyears <- om_input$nyr
survey$estimate_F <- TRUE
survey$estimate_q <- TRUE
survey$random_q <- FALSE
survey$random_F <- FALSE
survey$SetSelectivity(survey_selectivity$get_id())

fims$CreateTMBModel()
## Set-up TMB
# Create parameter list from Rcpp modules
parameters <- list(p = fims$get_fixed())
obj <- MakeADFun(data=list(), parameters, DLL="FIMS")
report <- obj$report()

# numbers at age
#test output: expect_equal(?om_output$naa, report$naa) #need to add REPORT_F to population for any output to compare



})
