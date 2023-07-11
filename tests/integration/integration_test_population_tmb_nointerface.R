path <- "tests/integration/"
om_output <- om_input <- NULL
c_case <- 0
i_iter <- 1

github_dir <- paste0("https://github.com/Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison/raw/master/FIMS_integration_test_data/FIMS_C", c_case, "/output/OM/")
Rdata_file <- paste0("OM", i_iter, ".RData") # e.g. OM1.Rdata
# this loads the file directly from github
# (which was easier to figure out than downloading the Rdata first)
load(url(paste0(github_dir, Rdata_file)))


library(TMB)
#dyn.unload(dynlib(paste0(path, "integration_test_population_tmb_nointerface")))
# compile(paste0(path, "integration_test_population_tmb_nointerface.cpp"),
#         flags = "-DTMB_MODEL")
#compile in debugging mode (flags set for Windows machine)
compile(paste0(path, "integration_test_population_tmb_nointerface.cpp"),
        flags = "-DTMB_MODEL -O1 -g", DLLFLAGS = "")
dyn.load(dynlib(paste0(path, "integration_test_population_tmb_nointerface")))

#Need code chunk below to run with data:
# data(package = "FIMS")
# age_frame <- FIMSFrameAge(data_mile1)
# fims_frame <- FIMSFrame(data_mile1)

naa = c(993947.488, 811707.7933, 661434.4148, 537804.7782,
        436664.0013, 354303.3502, 287396.9718, 233100.2412, 189054.0219,
        153328.4354, 124353.2448, 533681.2692)

Dat <- list(
  fleet_num = 1,
  survey_num = 1,
  yr = om_input$nyr,
  ages = om_input$ages-1,
  W_kg = om_input$W.kg,
  ln_M = log(om_input$M),
  init_naa = naa
)

Par <- list(
  fleet_sel_A50 = om_input$sel_fleet$fleet1$A50.sel,
  fleet_sel_slope = om_input$sel_fleet$fleet1$slope.sel,
  surv_sel_A50 = om_input$sel_survey$survey1$A50.sel,
  surv_sel_slope = om_input$sel_survey$survey1$slope.sel,
  log_Fmort = as.matrix(log(om_input$f)),
  R0 = om_input$R0,
  h = om_input$h,
  logR_sd = om_input$logR_sd,
  A50_mat = om_input$A50.mat,
  slope_mat = om_input$slope.mat
)
#crashes Rstudio - next step: comment out population
obj <- MakeADFun(Dat, Par, DLL = "integration_test_population_tmb_nointerface")
rep <- obj$report()
rep$pop_naa
