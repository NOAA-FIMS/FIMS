load(test_path("fixtures", "integration_test_data.RData"))

test_that("deterministic test of fims", {
  iter_id <- 1

  result <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = FALSE
  )

  # Set up TMB's computational graph
  obj <- result$obj

  # Calculate standard errors
  sdr <- TMB::sdreport(obj)
  sdr_fixed <- result$sdr_fixed

  # Call report using deterministic parameter values
  # obj$report() requires parameter list to avoid errors
  report <- obj$report(obj$par)

  # Compare log(R0) to true value
  fims_logR0 <- sdr_fixed[1, "Estimate"]
  expect_gt(fims_logR0, 0.0)
  expect_equal(fims_logR0, log(om_input_list[[iter_id]]$R0))

  # Compare numbers at age to true value
  for (i in 1:length(c(t(om_output_list[[iter_id]]$N.age)))) {
    expect_equal(report$naa[[1]][i], c(t(om_output_list[[iter_id]]$N.age))[i])
  }

  # Compare biomass to true value
  for (i in 1:length(om_output_list[[iter_id]]$biomass.mt)) {
    expect_equal(report$biomass[[1]][i], om_output_list[[iter_id]]$biomass.mt[i])
  }

  # Compare spawning biomass to true value
  for (i in 1:length(om_output_list[[iter_id]]$SSB)) {
    expect_equal(report$ssb[[1]][i], om_output_list[[iter_id]]$SSB[i])
  }

  # Compare recruitment to true value
  fims_naa <- matrix(report$naa[[1]][1:(om_input_list[[iter_id]]$nyr * om_input_list[[iter_id]]$nages)],
    nrow = om_input_list[[iter_id]]$nyr, byrow = TRUE
  )

  # loop over years to compare recruitment by year
  for (i in 1:om_input_list[[iter_id]]$nyr) {
    expect_equal(fims_naa[i, 1], om_output_list[[iter_id]]$N.age[i, 1])
  }

  # confirm that recruitment matches the numbers in the first age
  # by comparing to fims_naa (what's reported from FIMS)
  expect_equal(
    fims_naa[1:om_input_list[[iter_id]]$nyr, 1],
    report$recruitment[[1]][1:om_input_list[[iter_id]]$nyr]
  )

  # confirm that recruitment matches the numbers in the first age
  # by comparing to the true values from the OM
  for (i in 1:om_input_list[[iter_id]]$nyr) {
    expect_equal(report$recruitment[[1]][i], om_output_list[[iter_id]]$N.age[i, 1])
  }

  # recruitment log_devs (fixed at initial "true" values)
  # the initial value of om_input$logR.resid is dropped from the model
  expect_equal(report$log_recruit_dev[[1]], om_input_list[[iter_id]]$logR.resid[-1])

  # F (fixed at initial "true" values)
  expect_equal(report$F_mort[[1]], om_output_list[[iter_id]]$f)

  # Expected catch
  fims_index <- report$exp_index
  for (i in 1:length(om_output_list[[iter_id]]$L.mt$fleet1)) {
    expect_equal(fims_index[[1]][i], om_output_list[[iter_id]]$L.mt$fleet1[i])
  }

  # Expect small relative error for deterministic test
  fims_object_are <- rep(0, length(em_input_list[[iter_id]]$L.obs$fleet1))
  for (i in 1:length(em_input_list[[iter_id]]$L.obs$fleet1)) {
    fims_object_are[i] <- abs(fims_index[[1]][i] - em_input_list[[iter_id]]$L.obs$fleet1[i]) / em_input_list[[iter_id]]$L.obs$fleet1[i]
  }

  # Expect 95% of relative error to be within 2*cv
  expect_lte(sum(fims_object_are > om_input_list[[iter_id]]$cv.L$fleet1 * 2.0), length(em_input_list[[iter_id]]$L.obs$fleet1) * 0.05)

  # Compare expected catch number at age to true values
  for (i in 1:length(c(t(om_output_list[[iter_id]]$L.age$fleet1)))) {
    expect_equal(report$cnaa[[1]][i], c(t(om_output_list[[iter_id]]$L.age$fleet1))[i])
  }

  # Expected catch number at age in proportion
  # QUESTION: Isn't this redundant with the non-proportion test above?
  fims_cnaa <- matrix(report$cnaa[[1]][1:(om_input_list[[iter_id]]$nyr * om_input_list[[iter_id]]$nages)],
    nrow = om_input_list[[iter_id]]$nyr, byrow = TRUE
  )
  fims_cnaa_proportion <- fims_cnaa / rowSums(fims_cnaa)
  om_cnaa_proportion <- om_output_list[[iter_id]]$L.age$fleet1 / rowSums(om_output_list[[iter_id]]$L.age$fleet1)

  for (i in 1:length(c(t(om_cnaa_proportion)))) {
    expect_equal(c(t(fims_cnaa_proportion))[i], c(t(om_cnaa_proportion))[i])
  }

  # Expected survey index.
  # Using [[2]] because the survey is the 2nd fleet.
  cwaa <- matrix(report$cwaa[[2]][1:(om_input_list[[iter_id]]$nyr * om_input_list[[iter_id]]$nages)],
    nrow = om_input_list[[iter_id]]$nyr, byrow = TRUE
  )
  expect_equal(fims_index[[2]], apply(cwaa, 1, sum) * om_output_list[[iter_id]]$survey_q$survey1)

  for (i in 1:length(om_output_list[[iter_id]]$survey_index_biomass$survey1)) {
    expect_equal(fims_index[[2]][i], om_output_list[[iter_id]]$survey_index_biomass$survey1[i])
  }

  fims_object_are <- rep(0, length(em_input_list[[iter_id]]$surveyB.obs$survey1))
  for (i in 1:length(em_input_list[[iter_id]]$survey.obs$survey1)) {
    fims_object_are[i] <- abs(fims_index[[2]][i] - em_input_list[[iter_id]]$surveyB.obs$survey1[i]) / em_input_list[[iter_id]]$surveyB.obs$survey1[i]
  }
  # Expect 95% of relative error to be within 2*cv
  expect_lte(
    sum(fims_object_are > om_input_list[[iter_id]]$cv.survey$survey1 * 2.0),
    length(em_input_list[[iter_id]]$surveyB.obs$survey1) * 0.05
  )

  # Expected catch number at age in proportion
  fims_cnaa <- matrix(report$cnaa[[2]][1:(om_input_list[[iter_id]]$nyr * om_input_list[[iter_id]]$nages)],
    nrow = om_input_list[[iter_id]]$nyr, byrow = TRUE
  )

  for (i in 1:length(c(t(om_output_list[[iter_id]]$survey_age_comp$survey1)))) {
    expect_equal(report$cnaa[[2]][i], c(t(om_output_list[[iter_id]]$survey_age_comp$survey1))[i])
  }

  fims_cnaa_proportion <- fims_cnaa / rowSums(fims_cnaa)
  om_cnaa_proportion <- om_output_list[[iter_id]]$survey_age_comp$survey1 / rowSums(om_output_list[[iter_id]]$survey_age_comp$survey1)

  for (i in 1:length(c(t(om_cnaa_proportion)))) {
    expect_equal(c(t(fims_cnaa_proportion))[i], c(t(om_cnaa_proportion))[i])
  }
})

test_that("estimation test of fims using fit_fims()", {
  # Initialize the iteration identifier and run FIMS with the 1st set of OM values
  iter_id <- 1
  result <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = TRUE
  )

  # Compare FIMS results with model comparison project OM values
  validate_fims(
    report = result$report,
    sdr = TMB::sdreport(result$obj),
    sdr_report = result$sdr_report,
    om_input = om_input_list[[iter_id]],
    om_output = om_output_list[[iter_id]],
    em_input = em_input_list[[iter_id]],
    use_fimsfit = TRUE
  )
})
