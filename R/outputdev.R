#-------------------------------------------------------------------------------

#----------------Code copied using this 
# knitr::purl(input = "C:/Users/peter.kuriyama/SynologyDrive/Research/noaa/FIMS/FIMS/vignettes/fims-demo.Rmd",
#             output = 'C:/Users/peter.kuriyama/SynologyDrive/Research/noaa/FIMS/FIMS/vignettes/fims-demo.R',
#             documentation = 0)
#Can modify if vignette example changes in the future

#-------------------------------------------------------------------------------
#----------------
knitr::opts_chunk$set(echo = TRUE)
library(dplyr)
library(reshape2)
# automatically loads fims Rcpp module
library(FIMS)
library(TMB)

# clear memory
clear()

nyears <- 30 # the number of years which we have data for.
nseasons <- 1 # the number of seasons in each year. FIMS currently defaults to 1
ages <- c(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) # age vector.
nages <- 12 # the number of age groups.

# use FIMS data frame
# data(package = "FIMS")
fims_frame <- FIMSFrame(data_mile1)

str(fims_frame)
fims_frame@data %>%
  dplyr::filter(type == "landings") %>%
  head()
fims_frame@data %>%
  dplyr::filter(type == "index") %>%
  head()

# age_frame <- FIMS::FIMSFrameAge(data_mile1) #Is deprecated
age_frame <- FIMS::FIMSFrame(data_mile1)


# fishery data
# This does the same thing as the one line below only
# using fims_frame rather than age_frame
# fishery_catch <- fims_frame@data %>%
#   dplyr::filter(type == "landings") %>%
#   .$value

fishery_catch <- FIMS::m_landings(age_frame)

# This does the same thing as the two lines below only
# using fims_frame rather than age_frame
# fishery_agecomp <- fims_frame@data %>%
#   dplyr::filter(type == "age") %>%
#   dplyr::filter(name == "fleet1") %>%
#   .$value


fishery_agecomp <- FIMS::m_agecomp(age_frame, "fleet1")

# survey data
survey_index <- FIMS::m_index(age_frame, "survey1")

# survey agecomp not set up in fimsframe yet
survey_agecomp <- FIMS::m_agecomp(age_frame, "survey1")

show(Index)
show(AgeComp)

# fleet index data
fishing_fleet_index <- methods::new(Index, nyears)
# fleet age composition data
fishing_fleet_age_comp <- methods::new(AgeComp, nyears, nages)
fishing_fleet_index$index_data <- fishery_catch
# Effective sampling size is 200
fishing_fleet_age_comp$age_comp_data <- fishery_agecomp * 200

methods::show(LogisticSelectivity)
fishing_fleet_selectivity <- methods::new(LogisticSelectivity)
fishing_fleet_selectivity$inflection_point$value <- 2.0
fishing_fleet_selectivity$inflection_point$is_random_effect <- FALSE
fishing_fleet_selectivity$inflection_point$estimated <- TRUE
fishing_fleet_selectivity$slope$value <- 1.0
fishing_fleet_selectivity$slope$is_random_effect <- FALSE
fishing_fleet_selectivity$slope$estimated <- TRUE

show(Fleet)

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

# fleet index data
survey_fleet_index <- methods::new(Index, nyears)
# survey age composition data
survey_fleet_age_comp <- methods::new(AgeComp, nyears, nages)
survey_fleet_index$index_data <- survey_index
# Effective sampling size is 200
survey_fleet_age_comp$age_comp_data <- survey_agecomp * 200

survey_fleet_selectivity <- new(LogisticSelectivity)
survey_fleet_selectivity$inflection_point$value <- 1.5
survey_fleet_selectivity$inflection_point$is_random_effect <- FALSE
survey_fleet_selectivity$inflection_point$estimated <- TRUE
survey_fleet_selectivity$slope$value <- 2.0
survey_fleet_selectivity$slope$is_random_effect <- FALSE
survey_fleet_selectivity$slope$estimated <- TRUE

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

# Recruitment
recruitment <- methods::new(BevertonHoltRecruitment)
methods::show(BevertonHoltRecruitment)

recruitment$log_sigma_recruit$value <- log(0.4)
recruitment$log_rzero$value <- log(1e+06)
recruitment$log_rzero$is_random_effect <- FALSE
recruitment$log_rzero$estimated <- TRUE
recruitment$logit_steep$value <- -log(1.0 - 0.75) + log(0.75 - 0.2)
recruitment$logit_steep$is_random_effect <- FALSE
recruitment$logit_steep$estimated <- FALSE

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

# Growth
ewaa_growth <- methods::new(EWAAgrowth)
ewaa_growth$ages <- ages
ewaa_growth$weights <- c(
  0.0005306555, 0.0011963283, 0.0020582654,
  0.0030349873, 0.0040552124, 0.0050646975,
  0.0060262262, 0.0069169206, 0.0077248909,
  0.0084461128, 0.0090818532, 0.0096366950
)

# Maturity
maturity <- new(LogisticMaturity)
maturity$inflection_point$value <- 2.25
maturity$inflection_point$is_random_effect <- FALSE
maturity$inflection_point$estimated <- FALSE
maturity$slope$value <- 3
maturity$slope$is_random_effect <- FALSE
maturity$slope$estimated <- FALSE

# Population
population <- new(Population)
population$log_M <- rep(log(0.2), nyears * nages)
population$estimate_M <- FALSE
population$log_init_naa <- log(c(
  993947.5, 811707.8, 661434.4,
  537804.8, 436664.0, 354303.4,
  287397.0, 233100.2, 189054.0,
  153328.4, 124353.2, 533681.3
))
population$estimate_init_naa <- TRUE
population$nages <- nages
population$ages <- ages
population$nfleets <- 2 # 1 fleet and 1 survey
population$nseasons <- nseasons
population$nyears <- nyears

population$SetMaturity(maturity$get_id())
population$SetGrowth(ewaa_growth$get_id())
population$SetRecruitment(recruitment$get_id())

sucess <- CreateTMBModel()

parameters <- list(p = get_fixed())
# obj <- MakeADFun(data = list(), parameters, DLL = "FIMS", silent = TRUE)

obj <- MakeADFun(data = list(), 
                 parameters, DLL = "FIMS", silent = FALSE)

opt <- nlminb(obj$par, obj$fn, obj$gr,
              control = list(eval.max = 800, iter.max = 800)
) # , method = "BFGS",
#   control = list(maxit=1000000, reltol = 1e-15))

sdr <- TMB::sdreport(obj)

sdr_fixed <- summary(sdr, "fixed")
report <- obj$report()



#-------------------------------------------------------------------------------
#Start of parameter output dataframe development
#-------------------------------------------------------------------------------

#get parameter vector and information
#estimable parameters are added to list
#fixed parameter list 
#derived values
#call some finalzied function, can acceses, save recruitment$loGR

parameters <- list(p = get_fixed())

####Include parnames depending on which parameters have estimation turned on
#Order that these are in will depend on the order they are set in the code above
parname <- 999

#Find fleet names; is there a way to extract fleet names from FIMS objects?
#Here just hard code as fleet1 and survey1 as written above
#Also hardcoded is 1:nyears. Will need to change if years are in actual year values
#Find object where there is a vector of years



####Fishing fleet selectivity
if(fishing_fleet_selectivity$inflection_point$estimated) parname <- c(parname,
                                                       "fleet1-sel-inf_poi")
if(fishing_fleet_selectivity$slope$estimated) parname <- c(parname, "fleet1-sel-slo")

####Fishing fleet
if(fishing_fleet$estimate_F) parname <- c(parname, paste0("fleet1-F_mor-year", 1:nyears))
# if(fish_fleet$estimate_q)

####Survey fleet selectivity
if(survey_fleet_selectivity$inflection_point$estimated) parname <- c(parname, "survey1-sel-inf_poi")
if(survey_fleet_selectivity$slope$estimated) parname <- c(parname, "survey1-sel-slo" )

####Survey fleet
if(survey_fleet$estimate_F) parname <- c(parname, "survey1-F_mor")
if(survey_fleet$estimate_q) parname <- c(parname, "survey1-ln_Q")
if(survey_fleet$estimate_obs_error) parname <- c(parname, "survey1-obs_err")

####Recruitment
if(recruitment$log_sigma_recruit$estimated) parname <- c(parname, "ln_sig_rec")
if(recruitment$log_rzero$estimated) parname <- c(parname, "ln_rzero")
if(recruitment$logit_steep$estimated) parname <- c(parname, "logit_h")

if(recruitment$estimate_log_devs) parname <- c(parname, paste0("ln_rec_dev-year", 1:nyears))

####Maturity
if(maturity$inflection_point$estimated) parname <- c(parname, "mat-inf_poi")
if(maturity$slope$estimated) parname <- c(parname, "mat-slo")

####Population parameters
if(population$estimate_M) parname <- c(parname, paste0("ln_M_", 
                                                       crossing(1:nyears, ages) %>% 
                                                         mutate(ya = paste(1:nyears, ages)) %>% pull(ya)))
if(population$estimate_init_naa) parname <- c(parname, paste0("ln_ini_naa-age", ages))

parname <- parname[-1]

#Estimated parameters, include starting values
est_pars <- tibble(parname = parname, starting_value = parameters$p)
est_pars$estimate <- sdr$par.fixed
est_pars$sd <- sqrt(diag(sdr$cov.fixed))
est_pars$gradient <- sdr$gradient.fixed[1, ]
est_pars$estimated <- TRUE


#--------------------------------------
#Pull out fixed parameters by module
#Not currently a good way to do this, so will do this manually below

#Find class of diferent objects

objclasses <- sapply(mget(ls()), FUN = function(xx){ 
  out <- class(xx)[1]
  return(out)
})

rcppobj <- objclasses[grep("Rcpp", objclasses)]

#Pull out objects that contain model settings (i.e. not data)
rcppobj <- rcppobj[grep("Selectivity|Maturity|Recruitment|Population|Fleet|EWAA", rcppobj)]

#Apply statement to pull things out of each Rcpp object
fixedpars <- lapply(1:length(rcppobj), FUN = function(xx){
  
  temp <- rcppobj[xx]
  print(xx)
  #Empirical weight at age class
  if(length(grep("EWAA", temp)) > 0){
    ewaa <- get(names(temp))
    
    out <- tibble(parname = paste0("ewaa-age", ewaa$ages),
                  starting_value = ewaa$weights, estimate = NA,
                  sd = NA, gradient = NA, estimated = FALSE)
    return(out)
  }
  
  #Fleet
  if(length(grep("Fleet", temp)) > 0){
    fleet <- get(names(temp))
    
    F_mor <- tibble(parname = paste0("ln_F_mor-year", 1:fleet$nyears),
                    #If fleet is a survey fleet, no log_Fmort specified
                    starting_value = ifelse(length(fleet$log_Fmort) != 0, fleet$log_Fmort, NA), 
                    estimate = NA,
                    sd = NA, gradient = NA, estimated = fleet$estimate_F)
    
    obs_err <- tibble(parname = paste0("ln_obs_err-year", 1:fleet$nyears),
                    starting_value = fleet$log_obs_error, estimate = NA,
                    sd = NA, gradient = NA, estimated = fleet$estimate_obs_error)
    
    qvals <- tibble(parname = paste0("ln_q-year", 1:fleet$nyears),
                    starting_value = fleet$log_q, estimate = NA,
                    sd = NA, gradient = NA, estimated = fleet$estimate_q)
    
    out <- rbind(F_mor, obs_err, qvals)
    out$parname <- paste0(names(temp), "_", out$parname)
    
    return(out)
  }
  
  #Selectivities
  if(length(grep("Selectivity", temp)) > 0){
    sel <- get(names(temp)) 
    
    #The fields will depend on the parameterization of selectivity
    infpoi <- tibble(parname = paste0(names(temp),"-sel-inf_poi"), 
                      starting_value = sel$inflection_point$value,
                      estimate = NA, sd = NA, gradient = NA, 
                      estimated = sel$inflection_point$estimated)
    
    slope <- tibble(parname = paste0(names(temp), "-sel-slo"), 
                     starting_value = sel$slope$value,
                     estimate = NA, sd = NA, gradient = NA, 
                     estimated = sel$slope$estimated)
    #Need to somehow identify the fleet these are associated with
    out <- rbind(infpoi, slope)  
    return(out)
  }
  
  #Maturity
  if(length(grep("Maturity", temp)) > 0){
    maturity <- get(names(temp)) 
    
    
    #The fields will depend on the parameterization of selectivity
    infpoi <- tibble(parname = "inf_poi", 
                     starting_value = maturity$inflection_point$value,
                     estimate = NA, sd = NA, gradient = NA, 
                     estimated = maturity$inflection_point$estimated)
    
    slope <- tibble(parname = "slo", 
                    starting_value = maturity$slope$value,
                    estimate = NA, sd = NA, gradient = NA, 
                    estimated = maturity$slope$estimated)
    #Need to somehow identify the fleet these are associated with
    out <- rbind(infpoi, slope)
    out$parname <- paste0("mat-", out$parname)
    return(out)
  }
  
  
  #Population
  if(length(grep("Population", temp)) > 0){
    pop <- get(names(temp))
    
    init_naa <- tibble(parname = paste0("ln_ini_naa-age", pop$ages),
                       starting_value = pop$log_init_naa,
                       estimate = NA, sd = NA, gradient = NA, 
                       estimated = pop$estimate_init_naa)
    
    
    #Will have to modify this depending on the M parameterization
    age_year <- tidyr::crossing(age = pop$ages, year = 1:pop$nyears) %>%
      dplyr::mutate(parname = paste0("age", age, "-year", year))
    
    M <- tibble(parname = paste0("ln_M-", age_year$parname),
                       starting_value = pop$log_M,
                       estimate = NA, sd = NA, gradient = NA, 
                       estimated = pop$estimate_M)
    
    #Proportion female
    prop_female <- tibble(parname = "pro_fem", 
                          starting_value = pop$proportion_female,
                          estimate = NA, sd = NA, gradient = NA, 
                          estimated = pop$estimate_prop_female)
    
    out <- rbind(init_naa, M, prop_female)  
    out$parname <- paste0('pop-', out$parname)
    return(out)
  }
  
  #Recruitment
  if(length(grep("Recruitment", temp)) > 0){
    rec <- get(names(temp))
    
    recdevs <- tibble(parname = paste0("ln_rec_dev-year", 1:length(rec$log_devs)),
                      starting_value = rec$log_devs,
                      estimate = NA, sd = NA, gradient = NA,
                      estimated = rec$estimate_log_devs)
    
    rzero <- tibble(parname = 'ln_r0', 
                    starting_value = rec$log_rzero$value,
                    estimate = NA, sd = NA, gradient = NA,
                    estimated = rec$log_rzero$estimated)
    
    sigmar <- tibble(parname = "ln_sig_rec", 
                     starting_value = rec$log_sigma_recruit$value,
                     estimate = NA, sd = NA, gradient = NA,
                     estimated = rec$log_sigma_recruit$estimated)
    
    steep <- tibble(parname = "logit_h",
                    starting_value = rec$logit_steep$value,
                    estimate = NA, sd = NA, gradient = NA,
                    estimated = rec$logit_steep$estimated)
    
    out <- rbind(recdevs, rzero, sigmar, steep)
    
    return(out)
  }
  
})

#Collapse fixedpars
fixedpars <- do.call(rbind, fixedpars)

fixedpars <- fixedpars %>% dplyr::filter(estimated == FALSE)

all_pars <- rbind(fixedpars, est_pars)

unique(all_pars$parname)







