library(FIMS)
library(minimizR)
library(snowfall)


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



NUMBER_OF_MODEL_RUNS<-100


# Fix and change R0 randomly to get different output for each
# model run.
init_fims<-function(i){
    fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
    
    # Recruitment
    recruitment <- new(fims$BevertonHoltRecruitment)
    # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is taken
    # before the likelihood calculation
    recruitment$log_sigma_recruit$value <- log(om_input$logR_sd)
    recruitment$log_rzero$value <- log(om_input$R0)# + runif(1,min=0, max=1000))
    recruitment$log_rzero$is_random_effect <- FALSE
    recruitment$log_rzero$estimated <- TRUE
    recruitment$logit_steep$value <- -log(1.0 - om_input$h) + log(om_input$h - 0.2)
    recruitment$logit_steep$is_random_effect <- FALSE
    recruitment$logit_steep$estimated <- FALSE
    recruitment$estimate_deviations <- TRUE
    # recruit deviations should enter the model in normal space.
    # The log is taken in the likelihood calculations
    # alternative setting: recruitment$deviations <- rep(1, length(om_input$logR.resid))
    recruitment$deviations <- exp(om_input$logR.resid)
    
    # Data
    catch <- em_input$L.obs$fleet1
    fishing_fleet_index <- new(fims$Index, length(catch))
    fishing_fleet_index$index_data <- catch
    fishing_fleet_age_comp <- new(fims$AgeComp, length(catch), om_input$nages)
    fishing_fleet_age_comp$age_comp_data <- c(t(em_input$L.age.obs$fleet1)) * em_input$n.L$fleet1
    
    survey_index <- em_input$surveyB.obs$survey1
    survey_fleet_index <- new(fims$Index, length(survey_index))
    survey_fleet_index$index_data <- survey_index
    survey_fleet_age_comp <- new(fims$AgeComp, length(survey_index), om_input$nages)
    survey_fleet_age_comp$age_comp_data <- c(t(em_input$survey.age.obs$survey1)) * em_input$n.survey$survey1
    
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
    fishing_fleet$log_obs_error$value <- log(sqrt(log(em_input$cv.L$fleet1^2 + 1)))
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
    survey_fleet$is_survey <- TRUE
    survey_fleet$nages <- om_input$nages
    survey_fleet$nyears <- om_input$nyr
    # survey_fleet$log_Fmort <- rep(log(0.0000000000000000000000000001), om_input$nyr) #-Inf?
    survey_fleet$estimate_F <- FALSE
    survey_fleet$random_F <- FALSE
    survey_fleet$log_q <- log(om_output$survey_q$survey1)
    survey_fleet$estimate_q <- TRUE
    survey_fleet$random_q <- FALSE
    survey_fleet$log_obs_error$value <- log(sqrt(log(em_input$cv.survey$survey1^2 + 1)))
    survey_fleet$log_obs_error$estimated <- FALSE
    survey_fleet$SetAgeCompLikelihood(1)
    survey_fleet$SetIndexLikelihood(1)
    survey_fleet$SetSelectivity(survey_fleet_selectivity$get_id())
    survey_fleet$SetObservedIndexData(survey_fleet_index$get_id())
    survey_fleet$SetObservedAgeCompData(survey_fleet_age_comp$get_id())
    
    # Population
    population <- new(fims$Population)
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
    
    ## Set-up TMB
    fims$CreateTMBModel()
    parameters <- list(p = fims$get_fixed())
    obj<-TMB::MakeADFun(data = list(), parameters, DLL = "FIMS")
    
    fims$clear()
    return(obj)
}




run_fims<-function(id,begin,end){
    library(FIMS)
    library(minimizR)
    minimizer<-2
    results<-list()
    
    
    index<-1
    for(i in begin[id]:end[id]){
        obj<-init_fims(i)
        if(minimizer == 1){
            results[[index]] <-  optim(obj$par, obj$fn, obj$gr,
            method = "BFGS",
            control = list(maxit = 1000000, reltol = 1e-15)
            )
        }else{
            results[[index]]<-minimizR(obj$par, obj$fn, obj$gr, control = list(tolerance = 1e-8))
        }
        
        index = index+1
    }
    return(results)
}







id <- 0
ns <- 11

nsims <- NUMBER_OF_MODEL_RUNS

begin <- rep(0, ns)
end <- rep(0, ns)

#create scenario segments
if (id == 0) {
    segments <- nsims / ns
    print(paste("segments ", segments))
    for (i in 1:ns) {
        if (i < ns) {
            begin[i] <- as.integer((i - 1) * segments+1)
            end[i] <- as.integer(i * segments)
        } else{
            begin[i] <- as.integer((i - 1) * segments+1)
            end[i] <- nsims
        }
    }
    print(begin)
    print(end)
}

completed<-list()

sfInit(parallel=TRUE, cpus=ns)
sfExport( "run_fims", "init_fims", "om_input","om_output", "em_input")
start<-Sys.time()
append(sfLapply(1:ns, run_fims, begin, end), completed)
end<-Sys.time()
runtime<-end-start
print(paste0(paste0(paste0(NUMBER_OF_MODEL_RUNS," model runs completed in "),runtime)," seconds."))

line=paste("Snowfall runtime ", runtime)    
write(line,file="time.txt",append=TRUE)
