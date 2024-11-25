load(test_path("fixtures", "integration_test_data.RData"))

fleets <- list(
  fleet1 = list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Index = "TMBDlnormDistribution",
      AgeComp = "TMBDmultinomDistribution"
    )
  ),
  survey1 = list(
    selectivity = list(form = "LogisticSelectivity"),
    data_distribution = c(
      Index = "TMBDlnormDistribution",
      AgeComp = "TMBDmultinomDistribution"
    )
  )
)

data("data_mile1")
data <- FIMS::FIMSFrame(data_mile1)
default_parameters <- data_mile1 |>
  FIMS::FIMSFrame() |>
  create_default_parameters(
    fleets = fleets,
    recruitment = list(
      form = "BevertonHoltRecruitment",
      process_distribution = c(log_devs = "TMBDnormDistribution")
    ),
    growth = list(form = "EWAAgrowth"),
    maturity = list(form = "LogisticMaturity")
  )

modified_parameters <- list(
  fleet1 = list(
    Fleet.log_Fmort.value = log(om_output_list[[1]]$f)
  ),
  survey1 = list(
    LogisticSelectivity.inflection_point.value = 1.5,
    LogisticSelectivity.slope.value = 2,
    Fleet.log_q.value = log(om_output_list[[1]]$survey_q$survey1)
  ),
  recruitment = list(
    BevertonHoltRecruitment.log_rzero.value = log(om_input_list[[1]]$R0),
    BevertonHoltRecruitment.log_devs.value = om_input_list[[1]]$logR.resid[-1],
    BevertonHoltRecruitment.log_devs.estimated = FALSE,
    TMBDnormDistribution.log_sd.value = om_input_list[[1]]$logR_sd
  ),
  maturity = list(
    LogisticMaturity.inflection_point.value = om_input_list[[1]]$A50.mat,
    LogisticMaturity.inflection_point.estimated = FALSE,
    LogisticMaturity.slope.value = om_input_list[[1]]$slope.mat,
    LogisticMaturity.slope.estimated = FALSE
  ),
  population = list(
    Population.log_init_naa.value = log(om_output_list[[1]]$N.age[1, ])
  )
)

parameters <- default_parameters |>
  update_parameters(
    modified_parameters = modified_parameters
  )

test_that("deterministic test of fims", {
  iter_id <- 1

  result <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = FALSE
  )

  # Call report using deterministic parameter values
  # obj$report() requires parameter list to avoid errors
  report <- result@report

  # Compare log(R0) to true value
  fims_logR0 <- as.numeric(result@obj$par[36])
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

test_that("nll test of fims", {
  iter_id <- 1

  result <- setup_and_run_FIMS_with_wrappers(
    iter_id = iter_id,
    om_input_list = om_input_list,
    om_output_list = om_output_list,
    em_input_list = em_input_list,
    estimation_mode = FALSE
  )

  # Set up TMB's computational graph
  obj <- result@obj
  report <- result@report

  # Calculate standard errors
  # sdr <- result@sdreport
  # sdr_fixed <- result$sdr_fixed

  # log(R0)
  fims_logR0 <- as.numeric(result@obj$par[36])
  expect_equal(fims_logR0, log(om_input_list[[iter_id]]$R0))

  # recruitment likelihood
  # log_devs is of length nyr-1
  rec_nll <- -sum(dnorm(
    om_input_list[[iter_id]]$logR.resid[-1], rep(0, om_input_list[[iter_id]]$nyr - 1),
    om_input_list[[iter_id]]$logR_sd, TRUE
  ))

  # catch and survey index expected likelihoods
  index_nll_fleet <- -sum(dlnorm(
    em_input_list[[iter_id]]$L.obs$fleet1,
    log(om_output_list[[iter_id]]$L.mt$fleet1),
    sqrt(log(em_input_list[[iter_id]]$cv.L$fleet1^2 + 1)), TRUE
  ))
  index_nll_survey <- -sum(dlnorm(
    em_input_list[[iter_id]]$surveyB.obs$survey1,
    log(om_output_list[[iter_id]]$survey_index_biomass$survey1),
    sqrt(log(em_input_list[[iter_id]]$cv.survey$survey1^2 + 1)), TRUE
  ))
  index_nll <- index_nll_fleet + index_nll_survey
  # age comp likelihoods
  fishing_acomp_observed <- em_input_list[[iter_id]]$L.age.obs$fleet1
  fishing_acomp_expected <- om_output_list[[iter_id]]$L.age$fleet1 / rowSums(om_output_list[[iter_id]]$L.age$fleet1)
  survey_acomp_observed <- em_input_list[[iter_id]]$survey.age.obs$survey1
  survey_acomp_expected <- om_output_list[[iter_id]]$survey_age_comp$survey1 / rowSums(om_output_list[[iter_id]]$survey_age_comp$survey1)
  age_comp_nll_fleet <- age_comp_nll_survey <- 0
  for (y in 1:om_input_list[[iter_id]]$nyr) {
    age_comp_nll_fleet <- age_comp_nll_fleet -
      dmultinom(
        fishing_acomp_observed[y, ] * em_input_list[[iter_id]]$n.L$fleet1, em_input_list[[iter_id]]$n.L$fleet1,
        fishing_acomp_expected[y, ], TRUE
      )

    age_comp_nll_survey <- age_comp_nll_survey -
      dmultinom(
        survey_acomp_observed[y, ] * em_input_list[[iter_id]]$n.survey$survey1, em_input_list[[iter_id]]$n.survey$survey1,
        survey_acomp_expected[y, ], TRUE
      )
  }
  age_comp_nll <- age_comp_nll_fleet + age_comp_nll_survey
  expected_jnll <- rec_nll + index_nll + age_comp_nll
  jnll <- report$jnll

  expect_equal(report$nll_components[1], rec_nll)
  expect_equal(report$nll_components[2], index_nll_fleet)
  expect_equal(report$nll_components[3], age_comp_nll_fleet)
  expect_equal(report$nll_components[4], index_nll_survey)
  expect_equal(report$nll_components[5], age_comp_nll_survey)
  expect_equal(jnll, expected_jnll)
})

test_that("estimation test of fims using wrapper functions", {
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
    report = result@report,
    sdr = result@estimates,
    sdr_report = result@estimates,
    om_input = om_input_list[[iter_id]],
    om_output = om_output_list[[iter_id]],
    em_input = em_input_list[[iter_id]],
    use_fimsfit = TRUE
  )
})

test_that("estimation test of fims using high-level wrappers", {
  # Load operating model data for the current iteration
  iter_id <- 1
  om_input <- om_input_list[[iter_id]]
  om_output <- om_output_list[[iter_id]]
  em_input <- em_input_list[[iter_id]]

  # Clear any previous FIMS settings
  clear()
  parameter_list <- initialize_fims(
    parameters = parameters,
    data = data
  )
  input <- list()
  input$parameters <- parameter_list
  input$version <- "Model Comparison Project example"
  fit <- fit_fims(input, optimize = TRUE)

  clear()

  validate_fims(
    report = fit@report,
    sdr = fit@estimates,
    sdr_report = fit@estimates,
    om_input = om_input_list[[iter_id]],
    om_output = om_output_list[[iter_id]],
    em_input = em_input_list[[iter_id]],
    use_fimsfit = TRUE
  )
})
