dll_name <- paste("FIMS", .Platform$dynlib.ext, sep = "")
if (dir.exists(file.path(find.package("FIMS"), "src"))){
  dll_path <- file.path(find.package("FIMS"), "src", dll_name)
} else {
  libs_path <- file.path(find.package("FIMS"), "libs")

  if (.Platform$OS.type == "windows") {
    dll_path <- file.path(libs_path, .Platform$r_arch, dll_name)
  } else {
    dll_path <- file.path(libs_path, dll_name)
  }
}
install.packages("remotes")
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
setup_fims <- function(om_input, om_output, em_input){

  test_env <- new.env(parent = emptyenv())
  test_env$fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Recruitment
  test_env$recruitment <- new(test_env$fims$BevertonHoltRecruitment)
  # log_sigma_recruit is NOT logged. It needs to enter the model logged b/c the exp() is taken before the likelihood calculation
  test_env$recruitment$log_sigma_recruit$value <- log(om_input$logR_sd)
  test_env$recruitment$log_rzero$value <- log(om_input$R0)
  # recruitment$log_rzero$value <- 12
  test_env$recruitment$log_rzero$is_random_effect <- FALSE
  test_env$recruitment$log_rzero$estimated <- TRUE
  test_env$recruitment$logit_steep$value <- -log(1.0 - om_input$h) + log(om_input$h - 0.2)
  test_env$recruitment$logit_steep$is_random_effect <- FALSE
  test_env$recruitment$logit_steep$estimated <- FALSE
  test_env$recruitment$logit_steep$min <- 0.2
  test_env$recruitment$logit_steep$max <- 1.0
  test_env$recruitment$estimate_deviations <- TRUE
  #recruit deviations should enter the model in normal space. The log is taken in the likelihood calculations
  # recruitment$deviations <- rep(1, length(om_input$logR.resid))
  test_env$recruitment$deviations <- exp(om_input$logR.resid)

  # Data
  test_env$catch <- em_input$L.obs$fleet1
  test_env$fishing_fleet_index <- new(test_env$fims$Index, length(test_env$catch))
  test_env$fishing_fleet_index$index_data <- test_env$catch
  test_env$fishing_fleet_age_comp <- new(test_env$fims$AgeComp, length(test_env$catch), om_input$nages)
  test_env$fishing_fleet_age_comp$age_comp_data <- c(t(em_input$L.age.obs$fleet1)) * 200


  test_env$survey_index <-em_input$surveyB.obs$survey1
  test_env$survey_fleet_index <- new(test_env$fims$Index, length(test_env$survey_index))
  test_env$survey_fleet_index$index_data <- test_env$survey_index
  test_env$survey_fleet_age_comp <- new(test_env$fims$AgeComp, length(test_env$survey_index), om_input$nages)
  test_env$survey_fleet_age_comp$age_comp_data <-c(t(em_input$survey.age.obs$survey1)) *200


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
  # the Parameter class (from rcpp/rcpp_objects/rcpp_interface_base) cannot handle vectors, do we need a ParameterVector class?
  test_env$population$log_M <- rep(log(om_input$M.age[1]), om_input$nyr*om_input$nages)
  test_env$population$estimate_M <- FALSE
  test_env$population$log_init_naa <- log(om_output$N.age[1, ])
  test_env$population$estimate_init_naa <- TRUE
  test_env$population$nages <- om_input$nages
  test_env$population$ages <- om_input$ages * 1.0
  test_env$population$nfleets <- sum(om_input$fleet_num, om_input$survey_num)
  test_env$population$nseasons <- 1
  test_env$population$nyears <- om_input$nyr
  test_env$population$prop_female <- om_input$proportion.female[1]
  test_env$population$SetMaturity(test_env$maturity$get_id())
  test_env$population$SetGrowth(test_env$ewaa_growth$get_id())
  test_env$population$SetRecruitment(test_env$recruitment$get_id())

  return(test_env)
}

test_that("deterministic test of fims", {
  dyn.load(dll_path)
  test_env <- setup_fims(om_input = om_input,
                         om_output = om_output,
                         em_input = em_input)
  # Set-up TMB
  test_env$fims$CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = test_env$fims$get_fixed())
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

  # # Biomass
  # for (i in 1:length(om_output$biomass.mt)){
  #   expect_lte(abs(report$biomass[i] - om_output$biomass.mt[i])/om_output$biomass.mt[i], 0.001)
  # }
  #
  # # Spawning biomass
  # for (i in 1:length(om_output$SSB)){
  #   expect_lte(abs(report$ssb[i] - om_output$SSB[i])/om_output$SSB[i], 0.001)
  # }
  #
  # # Recruitment
  # fims_naa <- matrix(report$naa[1:(om_input$nyr*om_input$nages)],
  #                    nrow = om_input$nyr, byrow = TRUE)
  #
  # for (i in 1:length(om_output$N.age[,1])){
  #   expect_lte(abs(fims_naa[i,1] - om_output$N.age[i,1])/
  #                om_output$N.age[i,1], 0.001)
  # }
  #
  # expect_equal(fims_naa[,1],
  #                 report$recruitment[2:length(report$recruitment)])
  #
  # for (i in 1:length(om_output$N.age[,1])){
  #   expect_lte(abs(report$recruitment[i+1] - om_output$N.age[i,1])/
  #                om_output$N.age[i,1], 0.001)
  # }
  #
  # # recruitment deviations (fixed at initial "true" values)
  # expect_equal(log(report$rec_dev), om_input$logR.resid)
  #
  # # Expected catch
  # # Should we compare expected catch from FIMS with "true" catch from OM without observation error
  # # or with "true" catch observations with observation error
  # fims_object <- report$expected_index[,1]
  # for (i in 1:length(om_output$L.mt$fleet1)){
  #   expect_lte(abs(fims_object[i] - om_output$L.mt$fleet1[i])/om_output$L.mt$fleet1[i], 0.001)
  # }
  # for (i in 1:length(em_input$L.obs$fleet1)){
  #   expect_lte(abs(fims_object[i] - em_input$L.obs$fleet1[i])/em_input$L.obs$fleet1[i], 0.1)
  # }
  #
  # # Expected catch number at age
  # for (i in 1:length(c(t(om_output$L.age$fleet1)))){
  #   expect_lte(abs(report$cnaa[i,1] - c(t(om_output$L.age$fleet1))[i])/
  #                c(t(om_output$L.age$fleet1))[i], 0.001)
  # }
  #
  # # Expected catch number at age in proportion
  # fims_cnaa <- matrix(report$cnaa[1:(om_input$nyr*om_input$nages), 1],
  #                     nrow = om_input$nyr, byrow = TRUE)
  # fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)
  # for (i in 1:length(c(t(em_input$L.age.obs$fleet1)))){
  #   print(i)
  #   expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input$L.age.obs$fleet1))[i])/
  #                c(t(em_input$L.age.obs$fleet1))[i], 0.1)
  # }
  #
  # # Expected survey index
  # fims_object <- report$expected_index[,2]
  # for (i in 1:length(om_output$survey_index_biomass$survey1)){
  #   expect_lte(abs(fims_object[i] - om_output$survey_index_biomass$survey1[i])/
  #                om_output$survey_index_biomass$survey1[i], 0.001)
  # }
  # for (i in 1:length(em_input$surveyB.obs$survey1)){
  #   expect_lte(abs(fims_object[i] - em_input$surveyB.obs$survey1[i])/
  #                em_input$surveyB.obs$survey1[i], 0.2)
  # }
  #
  # # Expected survey number at age
  # for (i in 1:length(c(t(om_output$survey_age_comp$survey1)))){
  #   expect_lte(abs(report$cnaa[i,2] - c(t(om_output$survey_age_comp$survey1))[i])/
  #                c(t(om_output$survey_age_comp$survey1))[i], 0.001)
  # }
  #
  # # Expected catch number at age in proportion
  # fims_cnaa <- matrix(report$cnaa[1:(om_input$nyr*om_input$nages), 2],
  #                     nrow = om_input$nyr, byrow = TRUE)
  # fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)
  #
  # for (i in 1:length(c(t(em_input$survey.age.obs)))){
  #   expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input$L.age.obs$fleet1))[i])/
  #                c(t(em_input$L.age.obs$fleet1))[i], 0.1)
  # }


  test_env$fims$clear()

})

test_that("nll test of fims", {
  dyn.load(dll_path)
  test_env <- setup_fims(om_input = om_input,
                         om_output = om_output,
                         em_input = em_input)
  # Set-up TMB
  test_env$fims$CreateTMBModel()
  parameters <- list(p = test_env$fims$get_fixed())
  par_list <- 1:length(parameters[[1]])
  par_list[2:length(par_list)] <- NA
  map <- list(p=factor(par_list))
  obj <- TMB::MakeADFun(data=list(), parameters, DLL="FIMS", map = map)

  sdr <- TMB::sdreport(obj)
  sdr_fixed <- summary(sdr, "fixed")

  # log(R0)
  fims_logR0 <- sdr_fixed[1, "Estimate"]
  expect_lte(abs(fims_logR0 - log(om_input$R0))/log(om_input$R0), 0.0001)

  report <- obj$report()
  obj <- TMB::MakeADFun(data=list(), parameters, DLL="FIMS", map = map)
  jnll <- obj$fn()

  #recruitment likelihood
  rec_nll <- -sum(dnorm(log(test_env$recruitment$deviations), rep(0, om_input$nyr),
                        om_input$logR_sd, TRUE))

  #catch and survey index expected likelihoods
  index_nll_fleet <- -sum(dnorm(log(test_env$catch),
                                log(om_output$L.mt$fleet1),
                                sqrt(log(em_input$cv.L$fleet1^2+1)), TRUE))
  index_nll_survey <- -sum(dnorm(log(test_env$survey_index),
                                 log(om_output$survey_index_biomass$survey1),
                                 sqrt(log(em_input$cv.survey$survey1^2+1)), TRUE))
  index_nll <- index_nll_fleet + index_nll_survey
  # age comp likelihoods
  fishing_acomp_observed <- em_input$L.age.obs$fleet1
  fishing_acomp_expected <- om_output$L.age$fleet1/rowSums(om_output$L.age$fleet1)
  survey_acomp_observed <- em_input$survey.age.obs$survey1
  survey_acomp_expected <- om_output$survey_age_comp$survey1/rowSums(om_output$survey_age_comp$survey1)
  age_comp_nll_fleet <- age_comp_nll_survey <- 0
  for(y in 1:30){
    age_comp_nll_fleet <- age_comp_nll_fleet -
      dmultinom(fishing_acomp_observed[y,]*em_input$n.L$fleet1, em_input$n.L$fleet1,
                fishing_acomp_expected[y,],TRUE)

    age_comp_nll_survey <- age_comp_nll_survey -
      dmultinom(survey_acomp_observed[y,]*em_input$n.survey$survey1, em_input$n.survey$survey1,
                survey_acomp_expected[y,],TRUE)
  }
  age_comp_nll <- age_comp_nll_fleet + age_comp_nll_survey
  expected_jnll <- rec_nll + index_nll + age_comp_nll

  expect_equal(jnll, expected_jnll, tolerance = 1)

  test_env$fims$clear()
})

test_that("estimation test of fims", {
  dyn.load(dll_path)
  test_env <- setup_fims(om_input = om_input,
                         om_output = om_output,
                         em_input = em_input)

  # Set-up TMB
  test_env$fims$CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(p = test_env$fims$get_fixed())
  obj <- TMB::MakeADFun(data=list(), parameters, DLL="FIMS")

  opt<- with(obj,optim(par, fn, gr, method = "BFGS",
                       control = list(maxit=1000000, reltol = 1e-15)))

  report <- obj$report()

  # Numbers at age
  for (i in 1:length(c(t(om_output$N.age)))){
    expect_lte(abs(report$naa[i] - c(t(om_output$N.age))[i])/c(t(om_output$N.age))[i], 0.25)
  }

  # Biomass
  for (i in 1:length(om_output$biomass.mt)){
    expect_lte(abs(report$biomass[i] - om_output$biomass.mt[i])/om_output$biomass.mt[i], 0.15)
  }

  # Spawning biomass
  for (i in 1:length(om_output$SSB)){
    expect_lte(abs(report$ssb[i] - om_output$SSB[i])/om_output$SSB[i], 0.15)
  }

  # Recruitment
  fims_naa <- matrix(report$naa[1:(om_input$nyr*om_input$nages)],
                     nrow = om_input$nyr, byrow = TRUE)

  for (i in 1:length(om_output$N.age[,1])){
    expect_lte(abs(fims_naa[i,1] - om_output$N.age[i,1])/
                 om_output$N.age[i,1], 0.25)
  }

  # expect_equal(fims_naa[,1],
  #              report$recruitment[2:length(report$recruitment)])

  # for (i in 1:length(om_output$N.age[,1])){
  #   expect_lte(abs(report$recruitment[i+1] - om_output$N.age[i,1])/
  #                om_output$N.age[i,1], 0.25)
  # }

  # recruitment deviations
  for (i in 1:length(om_input$logR.resid)){
    expect_lte(abs(log(report$rec_dev[i]) - om_input$logR.resid[i])/
                 om_input$logR.resid[i], 1) # 1
  }

  # Expected catch
  # Should we compare expected catch from FIMS with "true" catch from OM without observation error
  # or with "true" catch observations with observation error
  fims_object <- report$expected_index[,1]
  for (i in 1:length(om_output$L.mt$fleet1)){
    expect_lte(abs(fims_object[i] - om_output$L.mt$fleet1[i])/om_output$L.mt$fleet1[i], 0.01)
  }
  for (i in 1:length(em_input$L.obs$fleet1)){
    expect_lte(abs(fims_object[i] - em_input$L.obs$fleet1[i])/em_input$L.obs$fleet1[i], 0.025)
  }

  # Expected catch number at age
  for (i in 1:length(c(t(om_output$L.age$fleet1)))){
    expect_lte(abs(report$cnaa[i,1] - c(t(om_output$L.age$fleet1))[i])/
                 c(t(om_output$L.age$fleet1))[i], 0.25)
  }

  # Expected catch number at age in proportion
  fims_cnaa <- matrix(report$cnaa[1:(om_input$nyr*om_input$nages), 1],
                      nrow = om_input$nyr, byrow = TRUE)
  fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)
  for (i in 1:length(c(t(em_input$L.age.obs$fleet1)))){

    if (c(t(em_input$L.age.obs$fleet1))[i] == 0) {
      expect_lte(abs(c(t(fims_cnaa_proportion))[i] - (c(t(em_input$L.age.obs$fleet1))[i]+0.001))/
                   (c(t(em_input$L.age.obs$fleet1))[i]+0.001), 18)
    } else {
      expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input$L.age.obs$fleet1))[i])/
                   c(t(em_input$L.age.obs$fleet1))[i], 3) # Inf when i = 299; landings was 0.
    }
  }


  # Expected survey index
  fims_object <- report$expected_index[,2]
  for (i in 1:length(om_output$survey_index_biomass$survey1)){
    expect_lte(abs(fims_object[i] - om_output$survey_index_biomass$survey1[i])/
                 om_output$survey_index_biomass$survey1[i], 0.05)
  }
  for (i in 1:length(em_input$surveyB.obs$survey1)){
    expect_lte(abs(fims_object[i] - em_input$surveyB.obs$survey1[i])/
                 em_input$surveyB.obs$survey1[i], 0.25)
  }

  # Expected survey number at age
  # for (i in 1:length(c(t(om_output$survey_age_comp$survey1)))){
  #   expect_lte(abs(report$cnaa[i,2] - c(t(om_output$survey_age_comp$survey1))[i])/
  #                c(t(om_output$survey_age_comp$survey1))[i], 0.001)
  # }

  # Expected catch number at age in proportion
  fims_cnaa <- matrix(report$cnaa[1:(om_input$nyr*om_input$nages), 2],
                      nrow = om_input$nyr, byrow = TRUE)
  fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)

  for (i in 1:length(c(t(em_input$survey.age.obs)))){
    expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input$L.age.obs$fleet1))[i])/
                 c(t(em_input$L.age.obs$fleet1))[i], 0.15)
  }


  test_env$fims$clear()
})
