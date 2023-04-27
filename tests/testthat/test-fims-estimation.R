# The following packages need to be updated prior to debugging:
# install.packages("Matrix")
# install.packages("TMB", type = "source")
# use updated version of pkgbuild with correct compilation flags for debugging:
# remotes::install_github(repo = "Andrea-Havron-NOAA/pkgbuild")

devtools::load_all()

library(FIMS)
library(testthat)
data(package = "FIMS")

# remove from test_that wrapper for debugging
# test_that("deterministic test of fims", {

# Install required packages
required_pkg <- c("remotes", "devtools", "here")
pkg_to_install <- required_pkg[!(required_pkg %in%
  installed.packages()[, "Package"])]
if (length(pkg_to_install)) install.packages(pkg_to_install)

remotes::install_github(repo = "Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison")
library(ASSAMC)

## Set-up OM (sigmaR = 0.4)
maindir <- file.path(here::here(), "tests", "testthat", "_snaps")
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

# Set-up Rcpp modules and fix parameters to "true"
fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

# Recruitment
recruitment <- new(fims$BevertonHoltRecruitment)
# log_sigma_recruit is NOT logged. It needs to enter the model logged b/c the exp() is taken before the likelihood calculation
recruitment$log_sigma_recruit$value <- log(om_input$logR_sd)
recruitment$log_rzero$value <- log(om_input$R0)
# recruitment$log_rzero$value <- 12
recruitment$log_rzero$is_random_effect <- FALSE
recruitment$log_rzero$estimated <- TRUE
recruitment$logit_steep$value <- -log(1.0 - om_input$h) + log(om_input$h - 0.2)
recruitment$logit_steep$is_random_effect <- FALSE
recruitment$logit_steep$estimated <- FALSE
recruitment$logit_steep$min <- 0.2
recruitment$logit_steep$max <- 1.0
recruitment$estimate_deviations <- TRUE
#recruit deviations should enter the model in normal space. The log is taken in the likelihood calculations
# recruitment$deviations <- rep(1, length(om_input$logR.resid))
recruitment$deviations <- exp(om_input$logR.resid)

# Data
catch <- em_input$L.obs$fleet1
fishing_fleet_index <- new(fims$Index, length(catch))
fishing_fleet_index$index_data <- catch
fishing_fleet_age_comp <- new(fims$AgeComp, length(catch), om_input$nages)
fishing_fleet_age_comp$age_comp_data <- c(t(em_input$L.age.obs$fleet1)) * 200


survey_index <-em_input$surveyB.obs$survey1
survey_fleet_index <- new(fims$Index, length(survey_index))
survey_fleet_index$index_data <- survey_index
survey_fleet_age_comp <- new(fims$AgeComp, length(survey_index), om_input$nages)
survey_fleet_age_comp$age_comp_data <-c(t(em_input$survey.age.obs$survey1)) *200


# Growth
ewaa_growth <- new(fims$EWAAgrowth)
ewaa_growth$ages <- om_input$ages
ewaa_growth$weights <- om_input$W.mt

# Maturity
maturity <- new(fims$LogisticMaturity)
maturity$median$value <- om_input$A50.mat
maturity$median$is_random_effect <- FALSE
maturity$median$estimated <- FALSE
maturity$slope$value <- om_input$slope
maturity$slope$is_random_effect <- FALSE
maturity$slope$estimated <- FALSE

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
fishing_fleet$nages <- om_input$nages
fishing_fleet$nyears <- om_input$nyr
fishing_fleet$log_Fmort <- log(om_output$f)
fishing_fleet$estimate_F <- TRUE
fishing_fleet$random_F <- FALSE
fishing_fleet$log_q <- log(1.0)
fishing_fleet$estimate_q <- FALSE
fishing_fleet$random_q <- FALSE
fishing_fleet$log_obs_error$value <- log(sqrt(log(em_input$cv.L$fleet1^2+1)))
fishing_fleet$log_obs_error$estimated <- FALSE
# Need get_id() for setting up observed agecomp and index data?
fishing_fleet$SetAgeCompLikelihood(1)
fishing_fleet$SetIndexLikelihood(1)
fishing_fleet$SetSelectivity(fishing_fleet_selectivity$get_id())
fishing_fleet$SetObservedIndexData(fishing_fleet_index$get_id())
fishing_fleet$SetObservedAgeCompData(fishing_fleet_age_comp$get_id())

# Create the survey fleet
survey_fleet_selectivity <- new(fims$LogisticSelectivity)
survey_fleet_selectivity$median$value <- om_input$sel_survey$survey1$A50.sel1
survey_fleet_selectivity$median$is_random_effect <- FALSE
survey_fleet_selectivity$median$estimated <- TRUE
survey_fleet_selectivity$slope$value <- om_input$sel_survey$survey1$slope.sel1
survey_fleet_selectivity$slope$is_random_effect <- FALSE
survey_fleet_selectivity$slope$estimated <- TRUE

survey_fleet <- new(fims$Fleet)
survey_fleet$is_survey<-TRUE
survey_fleet$nages <- om_input$nages
survey_fleet$nyears <- om_input$nyr
# survey_fleet$log_Fmort <- rep(log(0.0000000000000000000000000001), om_input$nyr) #-Inf?
survey_fleet$estimate_F <- FALSE
survey_fleet$random_F <- FALSE
survey_fleet$log_q <- log(om_output$survey_q$survey1)
survey_fleet$estimate_q <- TRUE
survey_fleet$random_q <- FALSE
survey_fleet$log_obs_error$value <- log(sqrt(log(em_input$cv.survey$survey1^2+1)))
survey_fleet$log_obs_error$estimated <- FALSE
survey_fleet$SetAgeCompLikelihood(1)
survey_fleet$SetIndexLikelihood(1)
survey_fleet$SetSelectivity(survey_fleet_selectivity$get_id())
survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
survey_fleet$SetObservedAgeCompData(survey_fleet_age_comp$get_id())

# Population
population <- new(fims$Population)
# is it a problem these are not Parameters in the Population interface?
# the Parameter class (from rcpp/rcpp_objects/rcpp_interface_base) cannot handle vectors, do we need a ParameterVector class?
population$log_M <- rep(log(om_input$M.age[1]), om_input$nyr*om_input$nages)
population$estimate_M <- FALSE
population$log_init_naa <- log(om_output$N.age[1, ])
population$estimate_init_naa <- TRUE
population$nages <- om_input$nages
population$ages <- om_input$ages * 1.0
population$nfleets <- sum(om_input$fleet_num, om_input$survey_num)
population$nseasons <- 1
population$nyears <- om_input$nyr
population$prop_female <- om_input$proportion.female[1]
population$SetMaturity(maturity$get_id())
population$SetGrowth(ewaa_growth$get_id())
population$SetRecruitment(recruitment$get_id())

test_that("deterministic test of fims", {
  # Set-up TMB
  fims$CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = fims$get_fixed())
  # par_list <- 1:65
  # par_list[c(32:65)] <- NA
  # map <- list(p=factor(par_list))
  par_list <- 1:length(parameters[[1]])
  par_list[2:length(par_list)] <- NA
  # par_list <- rep(NA, length(parameters[[1]]))
  map <- list(p=factor(par_list))
  obj <- MakeADFun(data=list(), parameters, DLL="FIMS", map = map)

  sdr <- TMB::sdreport(obj)
  sdr_fixed <- summary(sdr, "fixed")

  # log(R0)
  fims_logR0 <- sdr_fixed[1, "Estimate"]
  expect_lte(abs(fims_logR0 - log(om_input$R0))/log(om_input$R0), 0.0001)

  report <- obj$report()

  # Numbers at age
  for (i in 1:length(c(t(om_output$N.age)))){
    expect_lte(abs(report$naa[i] - c(t(om_output$N.age))[i])/c(t(om_output$N.age))[i], 0.001)
  }

  # Biomass
  for (i in 1:length(om_output$biomass.mt)){
    expect_lte(abs(report$biomass[i] - om_output$biomass.mt[i])/om_output$biomass.mt[i], 0.001)
  }

  # Spawning biomass
  for (i in 1:length(om_output$SSB)){
    expect_lte(abs(report$ssb[i] - om_output$SSB[i])/om_output$SSB[i], 0.001)
  }

  # Recruitment
  fims_naa <- matrix(report$naa[1:(om_input$nyr*om_input$nages)],
                     nrow = om_input$nyr, byrow = TRUE)

  for (i in 1:length(om_output$N.age[,1])){
    expect_lte(abs(fims_naa[i,1] - om_output$N.age[i,1])/
                 om_output$N.age[i,1], 0.001)
  }

  expect_equal(fims_naa[,1],
                  report$recruitment[2:length(report$recruitment)])

  for (i in 1:length(om_output$N.age[,1])){
    expect_lte(abs(report$recruitment[i+1] - om_output$N.age[i,1])/
                 om_output$N.age[i,1], 0.001)
  }

  # recruitment deviations (fixed at initial "true" values)
  expect_equal(log(report$rec_dev), om_input$logR.resid)

  # Expected catch
  # Should we compare expected catch from FIMS with "true" catch from OM without observation error
  # or with "true" catch observations with observation error
  fims_object <- report$expected_index[,1]
  for (i in 1:length(om_output$L.mt$fleet1)){
    expect_lte(abs(fims_object[i] - om_output$L.mt$fleet1[i])/om_output$L.mt$fleet1[i], 0.001)
  }
  for (i in 1:length(em_input$L.obs$fleet1)){
    expect_lte(abs(fims_object[i] - em_input$L.obs$fleet1[i])/em_input$L.obs$fleet1[i], 0.1)
  }

  # Expected catch number at age
  for (i in 1:length(c(t(om_output$L.age$fleet1)))){
    expect_lte(abs(report$cnaa[i,1] - c(t(om_output$L.age$fleet1))[i])/
                 c(t(om_output$L.age$fleet1))[i], 0.001)
  }

  # Expected catch number at age in proportion
  fims_cnaa <- matrix(report$cnaa[1:(om_input$nyr*om_input$nages), 1],
                      nrow = om_input$nyr, byrow = TRUE)
  fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)
  for (i in 1:length(c(t(em_input$L.age.obs$fleet1)))){
    print(i)
    expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input$L.age.obs$fleet1))[i])/
                 c(t(em_input$L.age.obs$fleet1))[i], 0.1)
  }

  # Expected survey index
  fims_object <- report$expected_index[,2]
  for (i in 1:length(om_output$survey_index_biomass$survey1)){
    expect_lte(abs(fims_object[i] - om_output$survey_index_biomass$survey1[i])/
                 om_output$survey_index_biomass$survey1[i], 0.001)
  }
  for (i in 1:length(em_input$surveyB.obs$survey1)){
    expect_lte(abs(fims_object[i] - em_input$surveyB.obs$survey1[i])/
                 em_input$surveyB.obs$survey1[i], 0.2)
  }

  # Expected survey number at age
  for (i in 1:length(c(t(om_output$survey_age_comp$survey1)))){
    expect_lte(abs(report$cnaa[i,2] - c(t(om_output$survey_age_comp$survey1))[i])/
                 c(t(om_output$survey_age_comp$survey1))[i], 0.001)
  }

  # Expected catch number at age in proportion
  fims_cnaa <- matrix(report$cnaa[1:(om_input$nyr*om_input$nages), 2],
                      nrow = om_input$nyr, byrow = TRUE)
  fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)

  for (i in 1:length(c(t(em_input$survey.age.obs)))){
    expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input$L.age.obs$fleet1))[i])/
                 c(t(em_input$L.age.obs$fleet1))[i], 0.1)
  }


  fims$clear()
  dll_path <- here::here("src", "FIMS.dll")
  dyn.unload(dll_path)
  dyn.load(dll_path)

})

test_that("nll test of fims", {
  # Set-up TMB
  fims$CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = fims$get_fixed())
  # par_list <- 1:65
  # par_list[c(32:65)] <- NA
  # map <- list(p=factor(par_list))
  par_list <- 1:length(parameters[[1]])
  par_list[2:length(par_list)] <- NA
  # par_list <- rep(NA, length(parameters[[1]]))
  map <- list(p=factor(par_list))
  obj <- MakeADFun(data=list(), parameters, DLL="FIMS", map = map)

  sdr <- TMB::sdreport(obj)
  sdr_fixed <- summary(sdr, "fixed")

  # log(R0)
  fims_logR0 <- sdr_fixed[1, "Estimate"]
  expect_lte(abs(fims_logR0 - log(om_input$R0))/log(om_input$R0), 0.0001)

  report <- obj$report()
  obj <- MakeADFun(data=list(), parameters, DLL="FIMS", map = map)
  obj$gr(obj$par)
  p <- fims$get_fixed()
  jnll <- obj$fn()
  jnll
  report_deterministic <- obj$report()

  ## Test deterministic
  #recruitment likelihood
  rec_nll <- -sum(dnorm(log(rep(1, om_input$nyr)), rep(0, om_input$nyr),
                        om_input$logR_sd, TRUE))
  #correct values for index_nll_fleet and index_nll_survey?
  index_nll_fleet <- -sum(dnorm(log(catch),
                                log(om_output$L.mt$fleet1),
                                sqrt(log(em_input$cv.L$fleet1^2+1)), TRUE))
  index_nll_survey <- -sum(dnorm(log(survey_index),
                                 log(om_output$survey_index_biomass$survey1),
                                 sqrt(log(em_input$cv.survey$survey1^2+1)), TRUE))
  index_nll <- index_nll_fleet + index_nll_survey

  #correct values for observed and expected?
  fishing_acomp_observed <- em_input$L.age.obs$fleet1
  fishing_acomp_expected <- em_input$L.age.obs$fleet1
  survey_acomp_observed <- em_input$survey.age.obs$survey1
  survey_acomp_expected <- em_input$survey.age.obs$survey1
  #or should expected be: t(apply(om_output$survey_age_comp$survey1, 1 ,function(x) x/sum(x)))?
  age_comp_nll_fleet <- age_comp_nll_survey <- 0
  for(y in 1:30){
    age_comp_nll_fleet <- age_comp_nll_fleet -
      dmultinom(fishing_acomp_observed[y,]*200, 200,
                fishing_acomp_expected[y,],TRUE)

    age_comp_nll_survey <- age_comp_nll_survey -
      dmultinom(survey_acomp_observed[y,]*200, 200,
                survey_acomp_expected[y,],TRUE)
  }
  age_comp_nll <- age_comp_nll_fleet + age_comp_nll_survey
  expected_jnll <- rec_nll + index_nll + age_comp_nll
  expect_equal(jnll, expected_jnll)
})

test_that("estimation test of fims", {
  # Set-up TMB
  fims$CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = fims$get_fixed())
  obj <- MakeADFun(data=list(), parameters, DLL="FIMS")

  opt<- with(obj,optim(par, fn, gr, method = "BFGS",
                       control = list(maxit=1000000, reltol = 1e-15)))


  sdr <- TMB::sdreport(obj)
  sdr_fixed <- summary(sdr, "fixed")

  # log(R0)
  fims_logR0 <- sdr_fixed[1, "Estimate"]
  expect_lte(abs(fims_logR0 - log(om_input$R0))/log(om_input$R0), 0.01)

  report <- obj$report()

  # Numbers at age
  for (i in 1:length(c(t(om_output$N.age)))){
    expect_lte(abs(report$naa[i] - c(t(om_output$N.age))[i])/c(t(om_output$N.age))[i], 0.1) # 0.25
  }

  # Biomass
  for (i in 1:length(om_output$biomass.mt)){
    expect_lte(abs(report$biomass[i] - om_output$biomass.mt[i])/om_output$biomass.mt[i], 0.1)
  }

  # Spawning biomass
  for (i in 1:length(om_output$SSB)){
    expect_lte(abs(report$ssb[i] - om_output$SSB[i])/om_output$SSB[i], 0.1)
  }

  # Recruitment
  fims_naa <- matrix(report$naa[1:(om_input$nyr*om_input$nages)],
                     nrow = om_input$nyr, byrow = TRUE)

  for (i in 1:length(om_output$N.age[,1])){
    expect_lte(abs(fims_naa[i,1] - om_output$N.age[i,1])/
                 om_output$N.age[i,1], 0.1) # 0.2
  }

  expect_equal(fims_naa[,1],
               report$recruitment[2:length(report$recruitment)])

  for (i in 1:length(om_output$N.age[,1])){
    expect_lte(abs(report$recruitment[i+1] - om_output$N.age[i,1])/
                 om_output$N.age[i,1], 0.1) #0.2
  }

  # recruitment deviations
  for (i in 1:length(om_input$logR.resid)){
    expect_lte(abs(log(report$rec_dev[i]) - om_input$logR.resid[i])/
                 om_input$logR.resid[i], 0.1) # 1
  }


  # Expected catch
  # Should we compare expected catch from FIMS with "true" catch from OM without observation error
  # or with "true" catch observations with observation error
  fims_object <- report$expected_index[,1]
  for (i in 1:length(om_output$L.mt$fleet1)){
    expect_lte(abs(fims_object[i] - om_output$L.mt$fleet1[i])/om_output$L.mt$fleet1[i], 0.01)
  }
  for (i in 1:length(em_input$L.obs$fleet1)){
    expect_lte(abs(fims_object[i] - em_input$L.obs$fleet1[i])/em_input$L.obs$fleet1[i], 0.001)
  }

  # Expected catch number at age
  for (i in 1:length(c(t(om_output$L.age$fleet1)))){
    expect_lte(abs(report$cnaa[i,1] - c(t(om_output$L.age$fleet1))[i])/
                 c(t(om_output$L.age$fleet1))[i], 0.1) # 0.25
  }

  # Expected catch number at age in proportion
  fims_cnaa <- matrix(report$cnaa[1:(om_input$nyr*om_input$nages), 1],
                      nrow = om_input$nyr, byrow = TRUE)
  fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)
  for (i in 1:length(c(t(em_input$L.age.obs$fleet1)))){
    expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input$L.age.obs$fleet1))[i])/
                 c(t(em_input$L.age.obs$fleet1))[i], 0.1) # Inf when i = 299; landings was 0.
  }

  # Expected survey index
  fims_object <- report$expected_index[,2]
  for (i in 1:length(om_output$survey_index_biomass$survey1)){
    expect_lte(abs(fims_object[i] - om_output$survey_index_biomass$survey1[i])/
                 om_output$survey_index_biomass$survey1[i], 0.1)
  }
  for (i in 1:length(em_input$surveyB.obs$survey1)){
    expect_lte(abs(fims_object[i] - em_input$surveyB.obs$survey1[i])/
                 em_input$surveyB.obs$survey1[i], 0.1) # 0.25
  }

  # Expected survey number at age
  for (i in 1:length(c(t(om_output$survey_age_comp$survey1)))){
    expect_lte(abs(report$cnaa[i,2] - c(t(om_output$survey_age_comp$survey1))[i])/
                 c(t(om_output$survey_age_comp$survey1))[i], 0.001)
  }

  # Expected catch number at age in proportion
  fims_cnaa <- matrix(report$cnaa[1:(om_input$nyr*om_input$nages), 2],
                      nrow = om_input$nyr, byrow = TRUE)
  fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)

  for (i in 1:length(c(t(em_input$survey.age.obs)))){
    expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input$L.age.obs$fleet1))[i])/
                 c(t(em_input$L.age.obs$fleet1))[i], 0.1) # 0.15
  }


  fims$clear()
  dll_path <- here::here("src", "FIMS.dll")
  dyn.unload(dll_path)
  dyn.load(dll_path)

})

## Test deterministic
#recruitment likelihood
rec_nll <- -sum(dnorm(log(recruitment$deviations), rep(0, om_input$nyr),
                      om_input$logR_sd, TRUE))
#catch and survey index expected likelihoods
index_nll_fleet <- -sum(dnorm(log(catch),
                        log(om_output$L.mt$fleet1),
                        em_input$cv.L$fleet1, TRUE))
index_nll_survey <- -sum(dnorm(log(survey_index),
                            log(om_output$survey_index_biomass$survey1),
                            sqrt(log(em_input$cv.survey$survey1^2+1)), TRUE))
index_nll <- index_nll_fleet + index_nll_survey
#age comp likelihoods
fishing_acomp_observed <- em_input$L.age.obs$fleet1
fishing_acomp_expected <- t(apply(om_output$L.age$fleet1, 1 ,function(x) x/sum(x)))
survey_acomp_observed <- em_input$survey.age.obs$survey1
survey_acomp_expected <- t(apply(om_output$survey_age_comp$survey1, 1 ,function(x) x/sum(x)))
age_comp_nll_fleet <- age_comp_nll_survey <- 0
for(y in 1:30){
  age_comp_nll_fleet <- age_comp_nll_fleet -
    dmultinom(fishing_acomp_observed[y,]*200, 200,
              fishing_acomp_expected[y,],TRUE)



# obj$par gradient at zero indicates detached parameters
#try just estimating F then build up
#for loop for
opt<- with(obj,optim(par, fn, gr, method = "BFGS", control = list(maxit=1000000, reltol = 1e-15)))
#opt <- with(obj, nlminb(par, fn, gr,
#control = list(iter.max=100000,eval.max=200000, rel.tol = 1e-15)))
print(opt)

opt$par
obj$gr(opt$par)
#q()
sdr <- TMB::sdreport(obj)
#q()
#opt$par
#opt$objective
summary(sdr, "fixed")
summary(sdr, "report")
message("success!")
report <- obj$report()

# Test
# TO DO:
# - extract TMB output
# - use for loop to check individual values from a vector
# - use absolute relative error later
# - set up tolerance values later

## Numbers at age
for (i in 1:length(c(t(om_output$N.age)))){
  expect_lte(abs(report$naa[i] - c(t(om_output$N.age))[i])/c(t(om_output$N.age))[i], 0.1)
}
cbind(report$naa[1:360], c(t(om_output$N.age)))

# Biomass
for (i in 1:length(om_output$biomass.mt)){
  expect_lte(abs(report$biomass[i] - om_output$biomass.mt[i])/om_output$biomass.mt[i], 0.05)
}
cbind(report$biomass[1:30], om_output$biomass.mt)
plot(report$biomass[1:30], type="l")
lines(om_output$biomass.mt)

# Spawning biomass
for (i in 1:length(om_output$SSB)){
  expect_lte(abs(report$ssb[i] - om_output$SSB[i])/om_output$SSB[i], 0.05)
}
cbind(report$ssb[1:30], om_output$SSB)

plot(report$ssb[1:30], type="l")
lines(om_output$SSB)

# Expected catch
fims_object <- report$expected_catch[seq(1, length(report$expected_catch), 2)]
for (i in 1:length(om_output$L.mt$fleet1)){
  expect_lte(abs(fims_object[i] - om_output$L.mt$fleet1[i])/om_output$L.mt$fleet1[i], 0.05)
}
cbind(fims_object[1:30], om_output$L.mt$fleet1)
plot(om_output$L.mt$fleet1, type="l")
lines(fims_object[1:30], col="red")
# # Expected index (need to add survey index to FIMS)
# fims_object <- report$expected_index[seq(2, length(report$expected_index), 2)]
# for (i in 1:length(om_output$survey_index_biomass$survey1)){
#   expect_lte(abs(fims_object[i] - om_output$survey_index_biomass$survey1[i])/om_output$survey_index_biomass$survey1[i], 0.05)
# }

# recruitment deviations
expect_equal(log(report$rec_dev), om_input$logR.resid, tolerance = 0.001)
cbind(log(report$rec_dev)[1:30], om_input$logR.resid)
# libs_path <- system.file("libs", package = "FIMS")
# dll_name <- paste("FIMS", .Platform$dynlib.ext, sep = "")
# if (.Platform$OS.type == "windows") {
#   dll_path <- file.path(libs_path, .Platform$r_arch, dll_name)
# } else {
#   dll_path <- file.path(libs_path, dll_name)
# }

