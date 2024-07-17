# load stuff
library(dplyr)
library(reshape2)
library(FIMS)
library(TMB)
library(ggplot2)

#-------------------------------------------------------------------------------
# run petrale model from case-studies
# (requires working directory to be "case-studies" repo)
setwd("content")
knitr::purl(
  input = "NWFSC-petrale.qmd", 
  output = "NWFSC-petrale.R"
)

source("NWFSC-petrale.R")
sdr <- TMB::sdreport(obj)

####Include parnames depending on which parameters have estimation turned on
#Order that these are in will depend on the order they are set in the code above
parname <- 999

#Find fleet names; is there a way to extract fleet names from FIMS objects?
#Here just hard code as fleet1 and survey1 as written above
#Also hardcoded is 1:nyears. Will need to change if years are in actual year values
#Find object where there is a vector of years



####Fishing fleet selectivity
if(fish_selex$inflection_point$estimated) parname <- c(parname,
                                                       "fleet1-sel-inf_poi")
if(fish_selex$slope$estimated) parname <- c(parname, "fleet1-sel-slo")

####Fishing fleet
if(fish_fleet$estimate_F) parname <- c(parname, paste0("fleet1-F_mor-year", 1:nyears))
# if(fish_fleet$estimate_q)

####Survey fleet selectivity
if(survey_selex$inflection_point$estimated) parname <- c(parname, "survey1-sel-inf_poi")
if(survey_selex$slope$estimated) parname <- c(parname, "survey1-sel-slo" )

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







