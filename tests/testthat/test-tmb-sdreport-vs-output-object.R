test_that("TMB sdreport and output object show same point estimates", {
  # use knitr::purl(file.path(getwd(), "vignettes", "fims-demo.Rmd")) to convert
  # fims-demo.Rmd to fims-demo.R and paste the R code into this test
 
  ## ----fims1, warning=FALSE, message=FALSE--------------------------------------------------------------------------------------------------------
  # clear memory
  clear()


  ## ----fims-dims----------------------------------------------------------------------------------------------------------------------------------
  nyears <- 30 # the number of years which we have data for.
  nseasons <- 1 # the number of seasons in each year. FIMS currently defaults to 1
  ages <- c(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) # age vector.
  nages <- 12 # the number of age groups.


  ## ----fimsframe----------------------------------------------------------------------------------------------------------------------------------
  # use FIMS data frame
  data(package = "FIMS")
  fims_frame <- FIMSFrame(data_mile1)


  ## ----ageframe-----------------------------------------------------------------------------------------------------------------------------------
  fims_frame@data |>
    dplyr::filter(type == "landings") |>
    utils::head()
  fims_frame@data |>
    dplyr::filter(type == "index") |>
    utils::head()


  ## ----data---------------------------------------------------------------------------------------------------------------------------------------
  # fishery data
  fishery_catch <- FIMS::m_landings(fims_frame)
  fishery_agecomp <- FIMS::m_agecomp(fims_frame, "fleet1")

  # survey data
  survey_index <- FIMS::m_index(fims_frame, "survey1")

  # survey agecomp not set up in fimsframe yet
  survey_agecomp <- FIMS::m_agecomp(fims_frame, "survey1")

  ## ----fleet-set-data-----------------------------------------------------------------------------------------------------------------------------
  # fleet index data
  fishing_fleet_index <- methods::new(Index, nyears)
  # fleet age composition data
  fishing_fleet_age_comp <- methods::new(AgeComp, nyears, nages)
  fishing_fleet_index$index_data <- fishery_catch # unit: mt
  # Effective sampling size is 200
  fishing_fleet_age_comp$age_comp_data <- fishery_agecomp * 200 # unit: number at age; proportion at age also works


  ## ----fleet_selectivity--------------------------------------------------------------------------------------------------------------------------
  methods::show(LogisticSelectivity)
  fishing_fleet_selectivity <- methods::new(LogisticSelectivity)
  fishing_fleet_selectivity$inflection_point$value <- 2.0
  fishing_fleet_selectivity$inflection_point$is_random_effect <- FALSE
  fishing_fleet_selectivity$inflection_point$estimated <- TRUE
  fishing_fleet_selectivity$slope$value <- 1.0
  fishing_fleet_selectivity$slope$is_random_effect <- FALSE
  fishing_fleet_selectivity$slope$estimated <- TRUE

  ## ----fleet--------------------------------------------------------------------------------------------------------------------------------------
  # Create fleet module
  fishing_fleet <- methods::new(Fleet)
  # Set nyears and nages
  fishing_fleet$nages <- nages
  fishing_fleet$nyears <- nyears
  # Set values for log_Fmort
  fishing_fleet$log_Fmort <- log(c(
    0.009459165, 0.02728886, 0.04506364,
    0.06101782, 0.04860075, 0.08742055,
    0.0884472, 0.1866079, 0.109009, 0.1327043,
    0.1506155, 0.161243, 0.1166402, 0.1693461,
    0.1801919, 0.1612405, 0.3145732, 0.2572476,
    0.2548873, 0.2514621, 0.3491014, 0.2541077,
    0.4184781, 0.3457212, 0.3436855, 0.3141712,
    0.3080268, 0.4317453, 0.3280309, 0.4996754
  ))
  # Turn on estimation for F
  fishing_fleet$estimate_F <- TRUE
  fishing_fleet$random_F <- FALSE
  # Set value for log_q
  fishing_fleet$log_q <- log(1.0)
  fishing_fleet$estimate_q <- FALSE
  fishing_fleet$random_q <- FALSE
  fishing_fleet$log_obs_error <- rep(log(sqrt(log(0.01^2 + 1))), nyears)
  fishing_fleet$estimate_obs_error <- FALSE
  # Next two lines not currently used by FIMS
  fishing_fleet$SetAgeCompLikelihood(1)
  fishing_fleet$SetIndexLikelihood(1)
  # Set Index, AgeComp, and Selectivity using the IDs from the modules defined above
  fishing_fleet$SetObservedIndexData(fishing_fleet_index$get_id())
  fishing_fleet$SetObservedAgeCompData(fishing_fleet_age_comp$get_id())
  fishing_fleet$SetSelectivity(fishing_fleet_selectivity$get_id())


  ## ----survey-set-data----------------------------------------------------------------------------------------------------------------------------
  # fleet index data
  survey_fleet_index <- methods::new(Index, nyears)
  # survey age composition data
  survey_fleet_age_comp <- methods::new(AgeComp, nyears, nages)
  survey_fleet_index$index_data <- survey_index # unit: mt; it's possible to use other units as long as the index is assumed to be proportional to biomass
  # Effective sampling size is 200
  survey_fleet_age_comp$age_comp_data <- survey_agecomp * 200 # unit: number at age; proportion at age also works


  ## ----survey-selectivity-------------------------------------------------------------------------------------------------------------------------
  survey_fleet_selectivity <- new(LogisticSelectivity)
  survey_fleet_selectivity$inflection_point$value <- 1.5
  survey_fleet_selectivity$inflection_point$is_random_effect <- FALSE
  survey_fleet_selectivity$inflection_point$estimated <- TRUE
  survey_fleet_selectivity$slope$value <- 2.0
  survey_fleet_selectivity$slope$is_random_effect <- FALSE
  survey_fleet_selectivity$slope$estimated <- TRUE


  ## ----survey-------------------------------------------------------------------------------------------------------------------------------------
  survey_fleet <- methods::new(Fleet)
  survey_fleet$is_survey <- TRUE
  survey_fleet$nages <- nages
  survey_fleet$nyears <- nyears
  survey_fleet$estimate_F <- FALSE
  survey_fleet$random_F <- FALSE
  survey_fleet$log_q <- log(3.315143e-07)
  survey_fleet$estimate_q <- TRUE
  survey_fleet$random_q <- FALSE
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet$log_obs_error <- rep(log(sqrt(log(0.2^2 + 1))), nyears)
  survey_fleet$estimate_obs_error <- FALSE
  survey_fleet$SetAgeCompLikelihood(1)
  survey_fleet$SetIndexLikelihood(1)
  survey_fleet$SetSelectivity(survey_fleet_selectivity$get_id())
  survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
  survey_fleet$SetObservedAgeCompData(survey_fleet_age_comp$get_id())


  ## ----recruitment--------------------------------------------------------------------------------------------------------------------------------
  # Recruitment
  recruitment <- methods::new(BevertonHoltRecruitment)
  
  ## ----set-up-recruitment-------------------------------------------------------------------------------------------------------------------------
  recruitment$log_sigma_recruit$value <- log(0.4)
  recruitment$log_rzero$value <- log(1e+06) # unit: log(number)
  recruitment$log_rzero$is_random_effect <- FALSE
  recruitment$log_rzero$estimated <- TRUE
  recruitment$logit_steep$value <- -log(1.0 - 0.75) + log(0.75 - 0.2)
  recruitment$logit_steep$is_random_effect <- FALSE
  recruitment$logit_steep$estimated <- FALSE


  ## ----rec-devs-----------------------------------------------------------------------------------------------------------------------------------
  recruitment$estimate_log_devs <- FALSE
  recruitment$log_devs <- c(
    0.08904850, 0.43787763, -0.13299042, -0.43251973,
    0.64861200, 0.50640852, -0.06958319, 0.30246260,
    -0.08257384, 0.20740372, 0.15289604, -0.21709207,
    -0.13320626, 0.11225374, -0.10650836, 0.26877132,
    0.24094126, -0.54480751, -0.23680557, -0.58483386,
    0.30122785, 0.21930545, -0.22281699, -0.51358369,
    0.15740234, -0.53988240, -0.19556523, 0.20094360,
    0.37248740, -0.07163145
  )


  ## ----growth-------------------------------------------------------------------------------------------------------------------------------------
  # Growth
  ewaa_growth <- methods::new(EWAAgrowth)
  ewaa_growth$ages <- ages
  ewaa_growth$weights <- c(
    0.0005306555, 0.0011963283, 0.0020582654,
    0.0030349873, 0.0040552124, 0.0050646975,
    0.0060262262, 0.0069169206, 0.0077248909,
    0.0084461128, 0.0090818532, 0.0096366950
  ) # unit: mt


  ## ----maturity-----------------------------------------------------------------------------------------------------------------------------------
  # Maturity
  maturity <- new(LogisticMaturity)
  maturity$inflection_point$value <- 2.25
  maturity$inflection_point$is_random_effect <- FALSE
  maturity$inflection_point$estimated <- FALSE
  maturity$slope$value <- 3
  maturity$slope$is_random_effect <- FALSE
  maturity$slope$estimated <- FALSE


  ## ----population---------------------------------------------------------------------------------------------------------------------------------
  # Population
  population <- new(Population)
  population$log_M <- rep(log(0.2), nyears * nages)
  population$estimate_M <- FALSE
  population$log_init_naa <- log(c(
    993947.5, 811707.8, 661434.4,
    537804.8, 436664.0, 354303.4,
    287397.0, 233100.2, 189054.0,
    153328.4, 124353.2, 533681.3
  )) # unit: in number
  population$estimate_init_naa <- TRUE
  population$nages <- nages
  population$ages <- ages
  population$nfleets <- 2 # 1 fleet and 1 survey
  population$nseasons <- nseasons
  population$nyears <- nyears


  ## ----set-pop-modules----------------------------------------------------------------------------------------------------------------------------
  population$SetMaturity(maturity$get_id())
  population$SetGrowth(ewaa_growth$get_id())
  population$SetRecruitment(recruitment$get_id())


  ## ----model--------------------------------------------------------------------------------------------------------------------------------------
  sucess <- CreateTMBModel()
  parameters <- list(p = get_fixed())
  obj <- MakeADFun(data = list(), parameters, DLL = "FIMS", silent = TRUE)


  ## ----fit_model----------------------------------------------------------------------------------------------------------------------------------
  opt <- nlminb(obj$par, obj$fn, obj$gr,
                control = list(eval.max = 800, iter.max = 800)
  )

  ## ----tmb_report---------------------------------------------------------------------------------------------------------------------------------
  sdr <- TMB::sdreport(obj)
  sdr_fixed <- summary(sdr, "fixed")
  report <- obj$report(obj$env$last.par.best)
  
  # Check biomass
  for (i in seq_along(report$biomass[[1]])) {
    expect_equal(report$biomass[[1]][i], unname(sdr$value[names(sdr$value) %in% "Biomass"])[i])
  }

  # Check spawning biomass
  for (i in seq_along(report$ssb[[1]])) {
    expect_equal(report$ssb[[1]][i], unname(sdr$value[names(sdr$value) %in% "SSB"])[i])
  }
  
  # Check NAA
  for (i in seq_along(report$naa[[1]])) {
    expect_equal(report$naa[[1]][i], unname(sdr$value[names(sdr$value) %in% "NAA"])[i])
  }

  # Check LogRecDev
  for (i in seq_along(report$log_recruit_dev[[1]])) {
    expect_equal(report$log_recruit_dev[[1]][i], unname(sdr$value[names(sdr$value) %in% "LogRecDev"])[i])
  }

  # Check FMort
  for (i in seq_along(report$F_mort[[1]])) {
    expect_equal(report$F_mort[[1]][i], unname(sdr$value[names(sdr$value) %in% "FMort"])[i])
  }
  
  # Check ExpectedIndex
  for (i in seq_along(unlist(report$exp_index))) {
    expect_equal(unlist(report$exp_index)[i], unname(sdr$value[names(sdr$value) %in% "ExpectedIndex"])[i])
  }

  # Check CNAA
  for (i in seq_along(unlist(report$cnaa))) {
    expect_equal(unlist(report$cnaa)[i], unname(sdr$value[names(sdr$value) %in% "CNAA"])[i])
  }

  ## ----clear--------------------------------------------------------------------------------------------------------------------------------------
  clear()


})
