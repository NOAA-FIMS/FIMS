clear()
clear_logs()

## define the dimensions and global variables
years <- 1970:2023
nyears <- length(years)
nseasons <- 1
nages <- 10
ages <- 1:nages

## load AFSC GOA pollock data
pkfitfinal <- readRDS(url("https://github.com/NOAA-FIMS/case-studies/raw/main/content/data_files/pkfitfinal.RDS"))
pkfit0 <- readRDS(url("https://github.com/NOAA-FIMS/case-studies/raw/main/content/data_files/pkfit0.RDS"))
pkinput0 <- readRDS(url("https://github.com/NOAA-FIMS/case-studies/raw/main/content/data_files/pkinput0.RDS"))
pkinput <- readRDS(url("https://github.com/NOAA-FIMS/case-studies/raw/main/content/data_files/pkinput.RDS"))
parfinal <- pkfitfinal$obj$env$parList()

fimsdat <- pkdat0 <- pkinput0$dat

## build a FIMS and PK data set that match
## need to fill missing years with -999 so it's ignored in FIMS
ind2 <- 0 * pkfit0$rep$Eindxsurv2 - 999
ind2[which(years %in% fimsdat$srvyrs2)] <- fimsdat$indxsurv2
CV2 <- rep(1, length = nyears) # actually SE in log space
CV2[which(years %in% fimsdat$srvyrs2)] <- fimsdat$indxsurv_log_sd2
paa2 <- pkfit0$rep$Esrvp2 * 0 - 999
paa2[which(years %in% fimsdat$srv_acyrs2), ] <- fimsdat$srvp2
Npaa2 <- rep(1, nyears)
Npaa2[which(years %in% fimsdat$srv_acyrs2)] <- fimsdat$multN_srv2

ind3 <- 0 * pkfit0$rep$Eindxsurv3 - 999
ind3[which(years %in% fimsdat$srvyrs3)] <- fimsdat$indxsurv3
CV3 <- rep(1, length = nyears) # actually SE in log space
CV3[which(years %in% fimsdat$srvyrs3)] <- fimsdat$indxsurv_log_sd3
paa3 <- pkfit0$rep$Esrvp3 * 0 - 999
paa3[which(years %in% fimsdat$srv_acyrs3), ] <- fimsdat$srvp3
Npaa3 <- rep(1, nyears)
Npaa3[which(years %in% fimsdat$srv_acyrs3)] <- fimsdat$multN_srv3

ind6 <- 0 * pkfit0$rep$Eindxsurv6 - 999
ind6[which(years %in% fimsdat$srvyrs6)] <- fimsdat$indxsurv6
CV6 <- rep(1, length = nyears) # actually SE in log space
CV6[which(years %in% fimsdat$srvyrs6)] <- fimsdat$indxsurv_log_sd6
paa6 <- pkfit0$rep$Esrvp6 * 0 - 999
paa6[which(years %in% fimsdat$srv_acyrs6), ] <- fimsdat$srvp6
Npaa6 <- rep(1, nyears)
Npaa6[which(years %in% fimsdat$srv_acyrs6)] <- fimsdat$multN_srv6

## repeat with fish catch at age, using expected in missing years
caa <- pkfit0$rep$Ecatp * 0 - 999
caa[which(years %in% fimsdat$fshyrs), ] <- fimsdat$catp
Ncaa <- rep(1, nyears)
Ncaa[which(years %in% fimsdat$fshyrs)] <- fimsdat$multN_fsh

## put into fims friendly form
res <- data.frame(
  type = character(),
  name = character(),
  age = integer(),
  datestart = character(),
  dateend = character(),
  value = double(),
  unit = character(),
  uncertainty = double()
)
landings <- data.frame(
  type = "landings",
  name = "fleet1",
  age = NA,
  datestart = paste0(seq(fimsdat$styr, fimsdat$endyr), "-01-01"),
  dateend = paste0(seq(fimsdat$styr, fimsdat$endyr), "-12-31"),
  value = as.numeric(fimsdat$cattot) * 1e3,
  unit = "mt",
  uncertainty = fimsdat$cattot_log_sd[1]
)
index2 <- data.frame(
  type = "index",
  name = "survey2",
  age = NA,
  datestart = paste0(seq(fimsdat$styr, fimsdat$endyr), "-01-01"),
  dateend = paste0(seq(fimsdat$styr, fimsdat$endyr), "-12-31"),
  value = ifelse(ind2 > 0, ind2 * 1e9, ind2),
  unit = "",
  uncertainty = CV2
)
index3 <- data.frame(
  type = "index",
  name = "survey3",
  age = NA,
  datestart = paste0(seq(fimsdat$styr, fimsdat$endyr), "-01-01"),
  dateend = paste0(seq(fimsdat$styr, fimsdat$endyr), "-12-31"),
  value = ifelse(ind3 > 0, ind3 * 1e9, ind3),
  unit = "",
  uncertainty = CV3
)
index6 <- data.frame(
  type = "index",
  name = "survey6",
  age = NA,
  datestart = paste0(seq(fimsdat$styr, fimsdat$endyr), "-01-01"),
  dateend = paste0(seq(fimsdat$styr, fimsdat$endyr), "-12-31"),
  value = ifelse(ind6 > 0, ind6 * 1e9, ind6),
  unit = "",
  uncertainty = CV6
)
## these have -999 for missing data years
catchage <- data.frame(
  type = "age",
  name = "fleet1",
  age = rep(seq(1, nages), nyears),
  datestart = rep(paste0(
    seq(fimsdat$styr, fimsdat$endyr), "-01-01"
  ), each = nages),
  dateend = rep(paste0(
    seq(fimsdat$styr, fimsdat$endyr), "-12-31"
  ), each = nages),
  value = as.numeric(t(caa)),
  unit = "",
  uncertainty = rep(Ncaa, each = nages)
)
indexage2 <- data.frame(
  type = "age",
  name = "survey2",
  age = rep(seq(1, nages), nyears),
  datestart = rep(paste0(
    seq(fimsdat$styr, fimsdat$endyr), "-01-01"
  ), each = nages),
  dateend = rep(paste0(
    seq(fimsdat$styr, fimsdat$endyr), "-12-31"
  ), each = nages),
  value = as.numeric(t(paa2)),
  unit = "",
  uncertainty = rep(Npaa2, each = nages)
)
indexage3 <- data.frame(
  type = "age",
  name = "survey3",
  age = rep(seq(1, nages), nyears),
  datestart = rep(paste0(
    seq(fimsdat$styr, fimsdat$endyr), "-01-01"
  ), each = nages),
  dateend = rep(paste0(
    seq(fimsdat$styr, fimsdat$endyr), "-12-31"
  ), each = nages),
  value = as.numeric(t(paa3)),
  unit = "",
  uncertainty = rep(Npaa3, each = nages)
)
indexage6 <- data.frame(
  type = "age",
  name = "survey6",
  age = rep(seq(1, nages), nyears),
  datestart = rep(paste0(
    seq(fimsdat$styr, fimsdat$endyr), "-01-01"
  ), each = nages),
  dateend = rep(paste0(
    seq(fimsdat$styr, fimsdat$endyr), "-12-31"
  ), each = nages),
  value = as.numeric(t(paa6)),
  unit = "",
  uncertainty = rep(Npaa6, each = nages)
)
indexage <- rbind(indexage2, indexage3, indexage6)
index <- rbind(index2, index3, index6)
## indexage=indexage2
## index=index2
res <- rbind(res, landings, index, catchage, indexage)

age_frame <- FIMS::FIMSFrame(res)
fishery_catch <- FIMS::m_landings(age_frame)
fishery_agecomp <- FIMS::m_agecomp(age_frame, "fleet1")
survey_index2 <- FIMS::m_index(age_frame, "survey2")
survey_agecomp2 <- FIMS::m_agecomp(age_frame, "survey2")
survey_index3 <- FIMS::m_index(age_frame, "survey3")
survey_agecomp3 <- FIMS::m_agecomp(age_frame, "survey3")
survey_index6 <- FIMS::m_index(age_frame, "survey6")
survey_agecomp6 <- FIMS::m_agecomp(age_frame, "survey6")
# need to think about how to deal with multiple fleets - only using 1 fleeet for now
fish_index <- methods::new(Index, nyears)
fish_age_comp <- methods::new(AgeComp, nyears, nages)
fish_index$index_data <- fishery_catch
fish_age_comp$age_comp_data <-
  fishery_agecomp * catchage$uncertainty#rep(Ncaa, each=nages)

## set up FIMS data objects
clear()
clear_logs()
estimate_fish_selex <- TRUE
estimate_survey_selex <- TRUE
estimate_q2 <- TRUE
estimate_q3 <- TRUE
estimate_q6 <- TRUE
estimate_F <- TRUE
estimate_recdevs <- TRUE

### set up fishery
## fleet selectivity: converted from time-varying ascending
## slope/intercept to constant double-logistic
## methods::show(DoubleLogisticSelectivity)
fish_selex <- methods::new(DoubleLogisticSelectivity)
fish_selex$inflection_point_asc$value <- parfinal$inf1_fsh_mean
fish_selex$inflection_point_asc$is_random_effect <- FALSE
fish_selex$inflection_point_asc$estimated <- estimate_fish_selex
fish_selex$inflection_point_desc$value <- parfinal$inf2_fsh_mean
fish_selex$inflection_point_desc$is_random_effect <- FALSE
fish_selex$inflection_point_desc$estimated <- estimate_fish_selex
fish_selex$slope_asc$value <- exp(parfinal$log_slp1_fsh_mean)
fish_selex$slope_asc$is_random_effect <- FALSE
fish_selex$slope_asc$estimated <- estimate_fish_selex
fish_selex$slope_desc$value <- exp(parfinal$log_slp2_fsh_mean)
fish_selex$slope_desc$is_random_effect <- FALSE
fish_selex$slope_desc$estimated <- estimate_fish_selex
## create fleet object
fish_fleet <- methods::new(Fleet)
fish_fleet$nages <- nages
fish_fleet$nyears <- nyears
fish_fleet$log_Fmort <- log(pkfitfinal$rep$F)
fish_fleet$estimate_F <- estimate_F
fish_fleet$random_F <- FALSE
fish_fleet$log_q <- 0 # why is this length two in Chris' case study?
fish_fleet$estimate_q <- FALSE
fish_fleet$random_q <- FALSE
fish_fleet$log_obs_error <- log(landings$uncertainty)
## fish_fleet$log_obs_error$estimated <- FALSE
# Next two lines not currently used by FIMS
fish_fleet$SetAgeCompLikelihood(1)
fish_fleet$SetIndexLikelihood(1)
# Set Index, AgeComp, and Selectivity using the IDs from the modules defined above
fish_fleet$SetObservedIndexData(fish_index$get_id())
fish_fleet$SetObservedAgeCompData(fish_age_comp$get_id())
fish_fleet$SetSelectivity(fish_selex$get_id())


## Setup survey 2
survey_fleet_index <- methods::new(Index, nyears)
survey_age_comp <- methods::new(AgeComp, nyears, nages)
survey_fleet_index$index_data <- survey_index2
survey_age_comp$age_comp_data <-
  survey_agecomp2 * indexage2$uncertainty
## survey selectivity: ascending logistic
## methods::show(DoubleLogisticSelectivity)
survey_selex <- methods::new(DoubleLogisticSelectivity)
survey_selex$inflection_point_asc$value <- parfinal$inf1_srv2
survey_selex$inflection_point_asc$is_random_effect <- FALSE
survey_selex$inflection_point_asc$estimated <- estimate_survey_selex
survey_selex$slope_asc$value <- exp(parfinal$log_slp1_srv2)
survey_selex$slope_asc$is_random_effect <- FALSE
survey_selex$slope_asc$estimated <- estimate_survey_selex
## not estimated to make it ascending only, fix at input values
survey_selex$inflection_point_desc$value <- parfinal$inf2_srv2
survey_selex$inflection_point_desc$is_random_effect <- FALSE
survey_selex$inflection_point_desc$estimated <- FALSE
survey_selex$slope_desc$value <- exp(parfinal$log_slp2_srv2)
survey_selex$slope_desc$is_random_effect <- FALSE
survey_selex$slope_desc$estimated <- FALSE
survey_fleet <- methods::new(Fleet)
survey_fleet$is_survey <- TRUE
survey_fleet$nages <- nages
survey_fleet$nyears <- nyears
survey_fleet$estimate_F <- FALSE
survey_fleet$random_F <- FALSE
survey_fleet$log_q <- parfinal$log_q2_mean
survey_fleet$estimate_q <- estimate_q2
survey_fleet$random_q <- FALSE
# sd = sqrt(log(cv^2 + 1)), sd is log transformed
survey_fleet$log_obs_error <- log(index2$uncertainty)
## survey_fleet$log_obs_error$estimated <- FALSE
survey_fleet$SetAgeCompLikelihood(1)
survey_fleet$SetIndexLikelihood(1)
survey_fleet$SetSelectivity(survey_selex$get_id())
survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
survey_fleet$SetObservedAgeCompData(survey_age_comp$get_id())

## Setup survey 3
survey_fleet_index <- methods::new(Index, nyears)
survey_age_comp <- methods::new(AgeComp, nyears, nages)
survey_fleet_index$index_data <- survey_index3
survey_age_comp$age_comp_data <-
  survey_agecomp3 * indexage3$uncertainty
## survey selectivity: ascending logistic
## methods::show(LogisticSelectivity)
survey_selex <- methods::new(LogisticSelectivity)
survey_selex$inflection_point$value <- parfinal$inf1_srv3
survey_selex$inflection_point$is_random_effect <- FALSE
survey_selex$inflection_point$estimated <- estimate_survey_selex
survey_selex$slope$value <- exp(parfinal$log_slp1_srv3)
survey_selex$slope$is_random_effect <- FALSE
survey_selex$slope$estimated <- estimate_survey_selex
survey_fleet <- methods::new(Fleet)
survey_fleet$is_survey <- TRUE
survey_fleet$nages <- nages
survey_fleet$nyears <- nyears
survey_fleet$estimate_F <- FALSE
survey_fleet$random_F <- FALSE
survey_fleet$log_q <- parfinal$log_q3_mean
survey_fleet$estimate_q <- estimate_q3
survey_fleet$random_q <- FALSE
# sd = sqrt(log(cv^2 + 1)), sd is log transformed
survey_fleet$log_obs_error <- log(index3$uncertainty)
## survey_fleet$log_obs_error$estimated <- FALSE
survey_fleet$SetAgeCompLikelihood(2)
survey_fleet$SetIndexLikelihood(2)
survey_fleet$SetSelectivity(survey_selex$get_id())
survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
survey_fleet$SetObservedAgeCompData(survey_age_comp$get_id())

## Setup survey 6
survey_fleet_index <- methods::new(Index, nyears)
survey_age_comp <- methods::new(AgeComp, nyears, nages)
survey_fleet_index$index_data <- survey_index6
survey_age_comp$age_comp_data <-
  survey_agecomp6 * indexage6$uncertainty
## survey selectivity: ascending logistic
## methods::show(DoubleLogisticSelectivity)
survey_selex <- methods::new(DoubleLogisticSelectivity)
survey_selex$inflection_point_asc$value <- parfinal$inf1_srv6
survey_selex$inflection_point_asc$is_random_effect <- FALSE
survey_selex$inflection_point_asc$estimated <- FALSE
survey_selex$slope_asc$value <- exp(parfinal$log_slp1_srv6)
survey_selex$slope_asc$is_random_effect <- FALSE
survey_selex$slope_asc$estimated <- FALSE
## not estimated to make it ascending only, fix at input values
survey_selex$inflection_point_desc$value <- parfinal$inf2_srv6
survey_selex$inflection_point_desc$is_random_effect <- FALSE
survey_selex$inflection_point_desc$estimated <-
  estimate_survey_selex
survey_selex$slope_desc$value <- exp(parfinal$log_slp2_srv6)
survey_selex$slope_desc$is_random_effect <- FALSE
survey_selex$slope_desc$estimated <- estimate_survey_selex
survey_fleet <- methods::new(Fleet)
survey_fleet$is_survey <- TRUE
survey_fleet$nages <- nages
survey_fleet$nyears <- nyears
survey_fleet$estimate_F <- FALSE
survey_fleet$random_F <- FALSE
survey_fleet$log_q <- parfinal$log_q6
survey_fleet$estimate_q <- estimate_q6
survey_fleet$random_q <- FALSE
# sd = sqrt(log(cv^2 + 1)), sd is log transformed
survey_fleet$log_obs_error <- log(index6$uncertainty)
## survey_fleet$log_obs_error$estimated <- FALSE
survey_fleet$SetAgeCompLikelihood(3)
survey_fleet$SetIndexLikelihood(3)
survey_fleet$SetSelectivity(survey_selex$get_id())
survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
survey_fleet$SetObservedAgeCompData(survey_age_comp$get_id())

# Population module
# recruitment
recruitment <- methods::new(BevertonHoltRecruitment)
## methods::show(BevertonHoltRecruitment)
recruitment$log_sigma_recruit$value <- log(parfinal$sigmaR)
recruitment$log_rzero$value <- parfinal$mean_log_recruit + log(1e9)
recruitment$log_rzero$is_random_effect <- FALSE
recruitment$log_rzero$estimated <- TRUE
## note: do not set steepness exactly equal to 1, use 0.99 instead in ASAP run
recruitment$logit_steep$value <-
  -log(1.0 - .99999) + log(.99999 - 0.2)
recruitment$logit_steep$is_random_effect <- FALSE
recruitment$logit_steep$estimated <- FALSE
recruitment$estimate_log_devs <- estimate_recdevs
recruitment$log_devs <-  parfinal$dev_log_recruit[-1]

## growth  -- assumes single WAA vector for everything, based on
## Srv1 above
waa <- pkinput$dat$wt_srv1
ewaa_growth <- methods::new(EWAAgrowth)
ewaa_growth$ages <- ages
# NOTE: FIMS currently cannot use matrix of WAA, so have to ensure constant WAA over time in ASAP file for now
ewaa_growth$weights <- waa[1, ]
## NOTE: FIMS assumes SSB calculated at the start of the year, so
## need to adjust ASAP to do so as well for now, need to make
## timing of SSB calculation part of FIMS later
## maturity
## NOTE: for now tricking FIMS into thinking age 0 is age 1, so need to adjust A50 for maturity because FIMS calculations use ages 0-5+ instead of 1-6
maturity <- new(LogisticMaturity)
maturity$inflection_point$value <- 4.5
maturity$inflection_point$is_random_effect <- FALSE
maturity$inflection_point$estimated <- FALSE
maturity$slope$value <- 1.5
maturity$slope$is_random_effect <- FALSE
maturity$slope$estimated <- FALSE

# population
population <- new(Population)
population$log_M <-
  log(as.numeric(t(matrix(
    rep(pkfitfinal$rep$M, each = nyears), nrow = nyears
  ))))
population$estimate_M <- FALSE
population$log_init_naa <-
  c(log(pkfitfinal$rep$recruit[1]), log(pkfitfinal$rep$initN)) + log(1e9)
population$estimate_init_naa <-
  FALSE # TRUE , NOTE: fixing at ASAP estimates to test SSB calculations
population$nages <- nages
population$ages <- ages
population$nfleets <- 2 # 1 fleet and 1 survey
population$nseasons <- nseasons
population$nyears <- nyears
## population$prop_female <- 1.0 # ASAP assumption
population$SetMaturity(maturity$get_id())
population$SetGrowth(ewaa_growth$get_id())
population$SetRecruitment(recruitment$get_id())

## make FIMS model
success <- CreateTMBModel()
parameters <- list(p = get_fixed())
obj <- TMB::MakeADFun(data = list(), parameters, DLL = "FIMS", silent = TRUE)
rep1 <- obj$report() # FIMS initial values
## try fitting the model
opt <- TMBhelper::fit_tmb(obj, getsd=FALSE, newtonsteps=0, control=list(trace=100))
# opt <- with(obj, nlminb(start=par, objective=fn, gradient=gr))
gr <- max(abs(obj$gr())) # from Cole, can use TMBhelper::fit_tmb to get val to <1e-10
rep2 <- obj$report(obj$env$last.par.best) ## FIMS after estimation

goa_pollock_env <- rlang::env(!!!as.list(.GlobalEnv))
saveRDS(goa_pollock_env, file = test_path("fixtures", "goa_pollock_env.RData"))

clear()
