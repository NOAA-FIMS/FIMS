run_fims_age <- function(data = data_mile1) {
  
  # Ensure data is a FIMSFrameAge
  age_frame <- FIMSFrameAge(data)
  # Set the parameters to be the model-comparison project
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  # Set up survey 
 # Survey index values and observation errors
survey_index <- new(fims$IndexData)
survey_index$values <- age_frame@
survey_index$error <- rep(em_input$cv.survey$survey1, times = om_input$nyr)
# Survey composition
survey_comp <- new(fims$AgeCompData)
survey_comp$values <- as.vector(t(em_input$survey.age.obs$survey1))
survey_comp$sample_size <- rep(em_input$n.survey$survey1, times = om_input$nyr)
# Likelihood component settings
survey_index_comp_nll <- new(fims$Lognormal)
survey_index_comp_nll$use_bias_correction <- FALSE
survey_age_comp_nll <- new(fims$Multinomial)
# Survey selectivity settings
survey_selectivity <- new(fims$LogisticSelectivity)
survey_selectivity$a50$value <- om_input$sel_survey$survey1$A50.sel
survey_selectivity$a50$estimated <- TRUE
survey_selectivity$a50$phase <- 2
survey_selectivity$a50$min <- 0
survey_selectivity$a50$max <- max(om_input$ages)
survey_selectivity$slope$value <- 1 / om_input$sel_survey$survey1$slope.sel
survey_selectivity$slope$estimated <- TRUE
survey_selectivity$slope$phase <- 2
survey_selectivity$slope$min <- 0.0001
survey_selectivity$slope$max <- 5
# Create the survey
survey <- new(fims$Survey)
survey$AddIndexData(survey_index$id, "undifferentiated")
survey$AddAgeCompData(survey_comp$id, "undifferentiated")
survey$SetIndexNllComponent(survey_index_comp_nll$id)
survey$SetAgeCompNllComponent(survey_age_comp_nll$id)
survey$AddSelectivity(survey_selectivity$id, 1, area1$id)
# Catchability settings
survey$q$value <- em_input$survey_q$survey1
survey$q$min <- 0
survey$q$max <- 10
survey$q$estimated <- TRUE
survey$q$phase <- 1

  # Run TMB
  
  return(TRUE)
}