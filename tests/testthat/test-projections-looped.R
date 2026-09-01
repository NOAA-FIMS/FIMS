## Setup ----
# Load necessary data for the integration test
if (!file.exists(testthat::test_path("fixtures", "fit_age_length_comp.RDS"))) {
  prepare_test_data()
}


## Section 1: load in test data for all runs

load(testthat::test_path("fixtures", "integration_test_data.RData"))
map <- list()

iter_id <- 1

om_input <- om_input_list[[iter_id]] # Operating model input for the current iteration
om_output <- om_output_list[[iter_id]] # Operating model output for the current iteration
em_input <- em_input_list[[iter_id]] # Estimation model input for the current iteration

## Section 2: Model run code that should work for each scenario

run_FIMS_projection_scenario <- function(om_input,
                                         om_output,
                                         em_input,
                                         n_projection_years,
                                         projected_catch,
                                         projected_F,
                                         estimate_projected_F,
                                         projected_index,
                                         ssb_ratio_target = NULL) {
  # Clear any previous FIMS settings
  clear()

  # Total years the model covers: data years plus any projection years.
  n_total_years <- om_input[["nyr"]] + n_projection_years

  # Extract fishing fleet catch data (observed) and initialize index module
  catch <- em_input[["L.obs"]][["fleet1"]]

  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_catch <- create_data("catch", n_total_years)
  set_data(fishing_fleet_catch, c(catch, projected_catch))

  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the new function initializes the object with length nyr*n_ages
  fishing_fleet_age_comp <- create_data(
    "age_comp", n_total_years, om_input[["nages"]]
  )

  # Here we add projection period missing age comps for the fishing fleet
  # We take the observed age proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_age_comps <- em_input[["L.age.obs"]][["fleet1"]] * em_input[["n.L"]][["fleet1"]]
  if (n_projection_years > 0) {
    projected_age_comps <- rbind(
      projected_age_comps,
      matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
    )
  }
  set_data(fishing_fleet_age_comp, c(t(projected_age_comps)))

  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- create_data(
    "length_comp", n_total_years, om_input[["nlengths"]]
  )

  # Here we add projection period missing length comps for the fishing fleet
  # We take the observed length proportions and multiply them by the sample size
  # then add two years of missing data -999 for the projection period
  projected_length_comps <- em_input[["L.length.obs"]][["fleet1"]] * em_input[["n.L.lengthcomp"]][["fleet1"]]
  projected_length_comps <- rbind(
    projected_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )
  set_data(fishing_fleet_length_comp, c(t(projected_length_comps)))


  # Fleet
  # Create the fishing fleet
  # turn on estimation of inflection_point and slope
  fishing_fleet_selectivity <- create_selectivity("logistic")
  set_variable_vector(
    fishing_fleet_selectivity, "inflection_point",
    om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]], "fixed_effects"
  )
  set_variable_vector(
    fishing_fleet_selectivity, "slope",
    om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]], "fixed_effects"
  )

  # Initialize the fishing fleet module
  fishing_fleet <- create_fleet()
  set_fleet_constants(
    fishing_fleet, n_total_years, om_input[["nages"]], om_input[["nlengths"]]
  )

  # Log-transform OM fishing mortality for the data years, then the projected
  # values. Whether each projection year is estimated is passed in, so the
  # statuses are assembled alongside the values and set in the same call.
  # NOTE: Is this index correct? It starts at 1 but others start at 0?
  set_variable_vector(
    fishing_fleet, "log_Fmort",
    c(
      log(om_output[["f"]][1:om_input[["nyr"]]]),
      if (n_projection_years > 0) log(projected_F) else numeric(0)
    ),
    c(
      rep("fixed_effects", om_input[["nyr"]]),
      if (n_projection_years > 0) estimate_projected_F else character(0)
    )
  )

  set_variable_vector(fishing_fleet, "log_q", log(1.0), "assumed_known")
  set_fleet_selectivity(fishing_fleet, fishing_fleet_selectivity)
  set_fleet_observed_data(
    fishing_fleet,
    age_comp = fishing_fleet_age_comp,
    length_comp = fishing_fleet_length_comp,
    catch = fishing_fleet_catch
  )

  # Set up fishery index data using the lognormal
  # lognormal observation error transformed on the log scale
  # Compute lognormal SD from OM coefficient of variation (CV)
  fishing_fleet_catch_distribution <- create_distribution("dlnorm")
  set_variable_vector(
    fishing_fleet_catch_distribution, "log_sd",
    rep(log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1))), n_total_years),
    "assumed_known"
  )
  # Set Data using the IDs from the modules defined above
  set_distribution_observed_data(
    fishing_fleet_catch_distribution,
    get_fleet_observed_data_ids(fishing_fleet)[["catch"]]
  )
  set_distribution_links(
    fishing_fleet_catch_distribution, "data",
    get_variable_vector_id(fishing_fleet, "log_catch_expected")
  )

  # Set up fishery age composition data using the multinomial
  fishing_fleet_agecomp_distribution <- create_distribution("dmultinom")
  set_distribution_observed_data(
    fishing_fleet_agecomp_distribution,
    get_fleet_observed_data_ids(fishing_fleet)[["agecomp"]]
  )
  set_distribution_links(
    fishing_fleet_agecomp_distribution, "data",
    get_variable_vector_id(fishing_fleet, "agecomp_proportion")
  )

  # Set up fishery length composition data using the multinomial
  fishing_fleet_lengthcomp_distribution <- create_distribution("dmultinom")
  set_distribution_observed_data(
    fishing_fleet_lengthcomp_distribution,
    get_fleet_observed_data_ids(fishing_fleet)[["lengthcomp"]]
  )
  set_distribution_links(
    fishing_fleet_lengthcomp_distribution, "data",
    get_variable_vector_id(fishing_fleet, "lengthcomp_proportion")
  )
  set_distribution_note(
    fishing_fleet_lengthcomp_distribution,
    "fishing_fleet_lengthcomp_distribution"
  )

  # Set age-to-length conversion matrix, and turn off its estimation
  set_variable_vector(
    fishing_fleet, "age_to_length_conversion",
    c(t(em_input[["age_to_length_conversion"]])), "assumed_known"
  )

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- create_data("index", n_total_years)
  set_data(survey_fleet_index, c(survey_index, projected_index))


  survey_fleet_age_comp <- create_data(
    "age_comp", n_total_years, om_input[["nages"]]
  )

  projected_survey_age_comps <- em_input[["survey.age.obs"]][["survey1"]] * em_input[["n.survey"]][["survey1"]]
  projected_survey_age_comps <- rbind(
    projected_survey_age_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nages"]])
  )
  set_data(survey_fleet_age_comp, c(t(projected_survey_age_comps)))


  survey_fleet_length_comp <- create_data(
    "length_comp", n_total_years, om_input[["nlengths"]]
  )

  projected_survey_length_comps <- em_input[["survey.length.obs"]][["survey1"]] * em_input[["n.survey.lengthcomp"]][["survey1"]]
  projected_survey_length_comps <- rbind(
    projected_survey_length_comps,
    matrix(-999, nrow = n_projection_years, ncol = om_input[["nlengths"]])
  )
  set_data(survey_fleet_length_comp, c(t(projected_survey_length_comps)))

  # Fleet
  # Create the survey fleet
  # turn on estimation of inflection_point and slope
  survey_fleet_selectivity <- create_selectivity("logistic")
  set_variable_vector(
    survey_fleet_selectivity, "inflection_point",
    om_input[["sel_survey"]][["survey1"]][["A50.sel1"]], "fixed_effects"
  )
  set_variable_vector(
    survey_fleet_selectivity, "slope",
    om_input[["sel_survey"]][["survey1"]][["slope.sel1"]], "fixed_effects"
  )

  survey_fleet <- create_fleet()
  set_fleet_constants(
    survey_fleet, n_total_years, om_input[["nages"]], om_input[["nlengths"]]
  )
  # Set very low survey fishing mortality
  set_variable_vector(
    survey_fleet, "log_Fmort", rep(-200, n_total_years), "assumed_known"
  )
  set_variable_vector(
    survey_fleet, "log_q",
    log(om_output[["survey_q"]][["survey1"]]), "fixed_effects"
  )
  set_fleet_selectivity(survey_fleet, survey_fleet_selectivity)
  set_fleet_observed_data(
    survey_fleet,
    age_comp = survey_fleet_age_comp,
    length_comp = survey_fleet_length_comp,
    index = survey_fleet_index
  )

  # Set up survey index data using the lognormal
  # lognormal observation error transformed on the log scale
  # sd = sqrt(log(cv^2 + 1)), sd is log transformed
  survey_fleet_index_distribution <- create_distribution("dlnorm")
  set_variable_vector(
    survey_fleet_index_distribution, "log_sd",
    rep(
      log(sqrt(log(em_input[["cv.survey"]][["survey1"]]^2 + 1))),
      n_total_years
    ),
    "assumed_known"
  )
  # Set Data using the IDs from the modules defined above
  set_distribution_observed_data(
    survey_fleet_index_distribution,
    get_fleet_observed_data_ids(survey_fleet)[["index"]]
  )
  set_distribution_links(
    survey_fleet_index_distribution, "data",
    get_variable_vector_id(survey_fleet, "log_index_expected")
  )

  # Age composition distribution
  survey_fleet_agecomp_distribution <- create_distribution("dmultinom")
  set_distribution_observed_data(
    survey_fleet_agecomp_distribution,
    get_fleet_observed_data_ids(survey_fleet)[["agecomp"]]
  )
  set_distribution_links(
    survey_fleet_agecomp_distribution, "data",
    get_variable_vector_id(survey_fleet, "agecomp_proportion")
  )

  # Length composition distribution
  survey_fleet_lengthcomp_distribution <- create_distribution("dmultinom")
  set_distribution_observed_data(
    survey_fleet_lengthcomp_distribution,
    get_fleet_observed_data_ids(survey_fleet)[["lengthcomp"]]
  )
  set_distribution_links(
    survey_fleet_lengthcomp_distribution, "data",
    get_variable_vector_id(survey_fleet, "lengthcomp_proportion")
  )

  # Set age to length conversion matrix, and turn off its estimation
  # TODO: Check that the dimensions of the matrix of age_to_length_conversion matrix
  #       is rows = length() and columns = length()
  # Transposing the below will have NO impact on the results if the object is
  # already a vector. Additionally, c() ensures that the result is a vector
  # to be consistent but a matrix would be okay.
  # TODO: write a test/documentation to show what order the matrix needs to be
  #       in when passing data to age-length-conversion
  set_variable_vector(
    survey_fleet, "age_to_length_conversion",
    c(t(em_input[["age_to_length_conversion"]])), "assumed_known"
  )

  # Recruitment
  # create new module in the recruitment class (specifically Beverton-Holt,
  # when there are other options, this would be where the option would be chosen)
  recruitment <- create_recruitment("beverton_holt")
  recruitment_process <- create_recruitment("log_devs_process")
  set_recruitment_process(recruitment, recruitment_process)

  # NOTE: the estimation status is given on every call, including where it is
  # "assumed_known". It is an argument of set_variable_vector() rather than a
  # property of the field, which is what lets the same quantity be estimated in
  # one model and assumed known in another.

  # set up log_rzero (equilibrium recruitment)
  set_variable_vector(
    recruitment, "log_rzero", log(om_input[["R0"]]), "fixed_effects"
  )
  # set up logit_steep
  set_variable_vector(
    recruitment, "logit_steep",
    -log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2), "assumed_known"
  )
  set_recruitment_n_years(recruitment, n_total_years)

  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  # alternative setting: recruitment$log_devs <- rep(0, length(om_input$logR.resid))

  # Deviations are estimated over the data years and held at zero, assumed
  # known, over the projection years.
  set_variable_vector(
    recruitment, "log_devs",
    c(
      om_input[["logR.resid"]][2:om_input[["nyr"]]],
      rep(0, n_projection_years)
    ),
    c(
      rep("random_effects", om_input[["nyr"]] - 1),
      rep("assumed_known", n_projection_years)
    )
  )


  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution <- create_distribution("dnorm")
  set_variable_vector(
    recruitment_distribution, "log_sd",
    log(om_input[["logR_sd"]]), "fixed_effects"
  )

  # NOTE: If this doesn't work I would guess that this is the possible source of
  # issues due to the length of x or expected recruitment needing to be the
  # same length as the random effect portion not the whole recruitment vector
  n_devs <- om_input[["nyr"]] - 1 + n_projection_years
  set_variable_vector(
    recruitment_distribution, "observed_values",
    rep(0, n_devs), "assumed_known"
  )
  set_variable_vector(
    recruitment_distribution, "expected_values",
    rep(0, n_devs), "assumed_known"
  )
  set_distribution_links(
    recruitment_distribution, "random_effects",
    get_variable_vector_id(recruitment, "log_devs")
  )

  # Growth
  ewaa_growth <- create_growth("ewaa")
  set_growth_n_years(ewaa_growth, n_total_years)
  set_numeric_vector(ewaa_growth, "ages", om_input[["ages"]])
  set_numeric_vector(ewaa_growth, "weights", om_input[["W.mt"]])


  # Maturity
  maturity <- create_maturity("logistic")
  set_variable_vector(
    maturity, "inflection_point", om_input[["A50.mat"]], "assumed_known"
  )
  set_variable_vector(
    maturity, "slope", om_input[["slope.mat"]], "assumed_known"
  )

  # Population
  population <- create_population()
  set_variable_vector(
    population, "log_M",
    rep(log(om_input[["M.age"]][1]), n_total_years * om_input[["nages"]]),
    "assumed_known"
  )
  set_variable_vector(
    population, "log_init_naa",
    log(om_output[["N.age"]][1, ]), "fixed_effects"
  )
  set_numeric_vector(population, "ages", om_input[["ages"]])

  set_population_constants(population, n_total_years, om_input[["nages"]])
  # The fleet count follows from the fleets linked below, so it is no longer
  # set by hand.
  set_population_processes(
    population,
    maturity = maturity,
    growth = ewaa_growth,
    recruitment = recruitment
  )
  set_population_fleets(population, list(fishing_fleet, survey_fleet))

  if (!is.null(ssb_ratio_target)) {
    # Setup log_f_multiplier to allow F_mort values in the projection period
    # to be scaled to achieve the target spawning biomass ratio. log_Fmort will
    # be assumed known in years where log_f_multiplier is estimated and
    # log_f_multiplier will be assumed known in the years where log_Fmort is
    # estimated to avoid confounding of the estimates.
    #
    # log_f_multiplier will have no impact on results when fixed at 0. It is
    # assumed known over the data years and estimated over the projection years.
    set_variable_vector(
      population, "log_f_multiplier",
      rep(0.0, n_total_years),
      c(
        rep("assumed_known", om_input[["nyr"]]),
        rep("random_effects", n_projection_years)
      )
    )
    if (n_projection_years > 0) {

      F_mult_distribution <- create_distribution("dnorm")

      # log_f_multiplier likelihood is setup with an expected mean target
      # to force the values to be close to equal. This setup is needed because
      # the currently likelihood pointer structure requires likelihood functions
      # to apply to an entire parameter vector rather than specific years. This
      # will be replaced in future refactoring that is currently in progress to
      # allow subvector pointers for likelihood components and to incorporate
      # mapping off parameters through TMB. The convoluted process below of
      # setting up various log_sd values was to maintain assumed_known values without
      # having the estimator get stuck fitting the mean rather than the
      # spawning biomass ratio target.

      set_distribution_fixed_mean(F_mult_distribution, 1)
      # set_distribution_fixed_mean(F_mult_distribution, -0.6931472)
      set_variable_vector(
        F_mult_distribution, "expected_mean", 1, "fixed_effects"
      )

      # log_sd is built up in the same overlapping passes the loops used: the
      # data years, then all projection years, then the last 30, then the last
      # 5. Later passes overwrite earlier ones.
      f_mult_log_sd <- numeric(n_total_years)
      f_mult_log_sd[1:om_input[["nyr"]]] <- 200
      f_mult_log_sd[(om_input[["nyr"]] + 1):n_total_years] <- -0
      f_mult_log_sd[
        (om_input[["nyr"]] + max(1, n_projection_years - 30)):n_total_years
      ] <- -5
      f_mult_log_sd[
        (om_input[["nyr"]] + max(1, n_projection_years - 5)):n_total_years
      ] <- -5

      set_variable_vector(
        F_mult_distribution, "observed_values",
        rep(0, n_total_years), "assumed_known"
      )
      set_variable_vector(
        F_mult_distribution, "expected_values",
        rep(0, n_total_years), "assumed_known"
      )
      set_variable_vector(
        F_mult_distribution, "log_sd", f_mult_log_sd, "assumed_known"
      )
      set_distribution_links(
        F_mult_distribution, "random_effects",
        get_variable_vector_id(population, "log_f_multiplier")
      )
    }

    # Setup projection prior target
    # Similar issues occur here as with log_f_multiplier in that a prior has to
    # be set for all spawning biomass ratios rather than for just a single year.
    # This is being improved/replaced in ongoing development.
    # The ratio is targeted in every year; log_sd is loosened everywhere except
    # the last few projection years, built up in the same overlapping passes the
    # loops used.
    n_ssb <- n_total_years + 1
    ssb_log_sd <- numeric(n_ssb)
    ssb_log_sd[1:(om_input[["nyr"]] + 1)] <- 200
    if (n_projection_years > 0) {
      ssb_log_sd[(om_input[["nyr"]] + 2):n_ssb] <- 200
      ssb_log_sd[
        (om_input[["nyr"]] + max(2, n_projection_years - 5)):n_ssb
      ] <- -5
    }

    SSB_ratio_prior <- create_distribution("dnorm")
    set_variable_vector(
      SSB_ratio_prior, "observed_values",
      rep(ssb_ratio_target, n_ssb), "assumed_known"
    )
    set_variable_vector(
      SSB_ratio_prior, "expected_values",
      rep(ssb_ratio_target, n_ssb), "assumed_known"
    )
    set_variable_vector(
      SSB_ratio_prior, "log_sd", ssb_log_sd, "assumed_known"
    )
    set_distribution_links(
      SSB_ratio_prior, "prior",
      get_variable_vector_id(population, "spawning_biomass_ratio")
    )
  }
  # Set up catch at age model
  caa <- create_fishery_model("catch_at_age")
  set_model_populations(caa, list(population))

  # Set-up TMB
  CreateTMBModel()
  # Create parameter list from Rcpp modules
  parameters <- list(
    p = get_fixed(),
    re = get_random()
  )
  obj <- TMB::MakeADFun(
    data = list(), parameters, DLL = "FIMS",
    silent = TRUE, map = map, random = "re"
  )

  # Optimization with nlminb

  opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
    control = list(eval.max = 10000, iter.max = 10000, trace = 0)
  )
  FIMS::set_fixed(opt$par)
  fims_finalized <- get_output(caa)

  # Call report using MLE parameter values, or
  # the initial values if optimization is skipped
  report <- obj[["report"]](obj[["env"]][["last.par.best"]])


  sdr <- TMB::sdreport(obj)
  sdr_report <- summary(sdr, "report")
  sdr_fixed <- summary(sdr, "fixed")
  row.names(sdr_fixed) <- names(FIMS:::get_parameter_names(sdr_fixed[, 1]))

  clear()

  # Output results for scenario comparison
  outputs <- list()
  outputs[[1]] <- sdr_report
  outputs[[2]] <- sdr_fixed
  outputs[[3]] <- report
  outputs[[4]] <- fims_finalized

  return(outputs)
}

## Section 3: Setup comparison scenario runs

## Run FIMS with no projection years as a control run ##
n_projection_years <- 0
projected_catch <- rep(-999, n_projection_years)
projected_F <- rep(om_output[["f"]][om_input$nyr], n_projection_years)
estimate_projected_F <- rep("assumed_known", n_projection_years)
projected_index <- rep(-999, n_projection_years)

no_projection_outputs <- run_FIMS_projection_scenario(om_input,
  om_output,
  em_input,
  n_projection_years,
  projected_catch,
  projected_F,
  estimate_projected_F,
  projected_index,
  ssb_ratio_target = NULL
)

sdr_fixed_no_project <- no_projection_outputs[[2]]
sdr_report_no_project <- no_projection_outputs[[1]]

## Run FIMS with 5 projection years and no catch ##
n_projection_years <- 5
projected_catch <- rep(-999, n_projection_years)
projected_F <- rep(om_output[["f"]][om_input$nyr], n_projection_years)
estimate_projected_F <- rep("assumed_known", n_projection_years)
projected_index <- rep(-999, n_projection_years)

projection_outputs <- run_FIMS_projection_scenario(om_input,
  om_output,
  em_input,
  n_projection_years,
  projected_catch,
  projected_F,
  estimate_projected_F,
  projected_index,
  ssb_ratio_target = NULL
)

sdr_fixed_5_year_project <- projection_outputs[[2]]
sdr_report_5_year_project <- projection_outputs[[1]]

## Run FIMS with 5 projection years and low future catch ##
n_projection_years <- 5
projected_catch <- rep(em_input[["L.obs"]][["fleet1"]][om_input$nyr] * 0.5, n_projection_years)
projected_F <- rep(om_output[["f"]][om_input$nyr], n_projection_years)
estimate_projected_F <- rep("fixed_effects", n_projection_years)
projected_index <- rep(-999, n_projection_years)

low_catch_projection_outputs <- run_FIMS_projection_scenario(om_input,
  om_output,
  em_input,
  n_projection_years,
  projected_catch,
  projected_F,
  estimate_projected_F,
  projected_index,
  ssb_ratio_target = NULL
)

sdr_fixed_5_year_project_catch_low <- low_catch_projection_outputs[[2]]
sdr_report_5_year_project_catch_low <- low_catch_projection_outputs[[1]]
## Run FIMS with 5 projection years and high future catch ##
n_projection_years <- 5
projected_catch <- rep(em_input[["L.obs"]][["fleet1"]][om_input$nyr] * 2, n_projection_years)
projected_F <- rep(om_output[["f"]][om_input$nyr], n_projection_years)
estimate_projected_F <- rep("fixed_effects", n_projection_years)
projected_index <- rep(-999, n_projection_years)

high_catch_projection_outputs <- run_FIMS_projection_scenario(om_input,
  om_output,
  em_input,
  n_projection_years,
  projected_catch,
  projected_F,
  estimate_projected_F,
  projected_index,
  ssb_ratio_target = NULL
)

sdr_fixed_5_year_project_catch_high <- high_catch_projection_outputs[[2]]
sdr_report_5_year_project_catch_high <- high_catch_projection_outputs[[1]]
## Run FIMS with 10 projection years and an SSB ratio target ##
n_projection_years <- 10
projected_catch <- rep(-999, n_projection_years)
projected_F <- rep(om_output[["f"]][om_input$nyr], n_projection_years)
estimate_projected_F <- rep("assumed_known", n_projection_years)
projected_index <- rep(-999, n_projection_years)
ssb_ratio_target <- 0.4

ssb_ratio_target_projection_outputs <- run_FIMS_projection_scenario(
  om_input,
  om_output,
  em_input,
  n_projection_years,
  projected_catch,
  projected_F,
  estimate_projected_F,
  projected_index,
  ssb_ratio_target
)

sdr_fixed_10_year_project_SSB_target <- ssb_ratio_target_projection_outputs[[2]]
sdr_report_10_year_project_SSB_target <- ssb_ratio_target_projection_outputs[[1]]
## Section 3: Compare scenario results

# important_row_names <- c("spawning_biomass","spawning_biomass_ratio","log_devs","log_Fmort")

# sdr_report_5_year_project[which((abs(sdr_report_5_year_project[row.names(sdr_report_5_year_project)=="log_devs","Estimate"]-sdr_report_no_project[row.names(sdr_report_no_project)=="log_devs","Estimate"])/abs(sdr_report_no_project[row.names(sdr_report_no_project)=="log_devs","Estimate"]))>0.01),]
# Compare fixed parameter estimates between control and fixed F runs
# Results are identical as expected

estimation_error <- max(((abs(sdr_fixed_no_project[, "Estimate"] - sdr_fixed_5_year_project[, "Estimate"]) / abs(sdr_fixed_no_project[, "Estimate"])) / abs(sdr_fixed_no_project[, "Std. Error"])))

sd_error <- max(abs(sdr_fixed_no_project[, "Std. Error"] - sdr_fixed_5_year_project[, "Std. Error"]) / abs(sdr_fixed_no_project[, "Std. Error"]))

# rec_devs <- sdr_report_5_year_project[row.names(sdr_report_5_year_project) == "log_devs", ][30:34, ]


test_that("projections with no data achieve same estimates and no projection model run", {
  #' @description Test that rec devs were fixed at zero in projection.
  # expect_equal(sum(rec_devs[, "Estimate"]), 0)

  #' @description Test that the maximum parameter estimate difference between a projection run and no projection run is less than 1 standard error.
  expect_lt(estimation_error, 1)

  #' @description Test that the maximum parameter standard deviation estimate difference between a projection run and no projection run is less than 10% of base estimate.
  expect_lt(sd_error, 0.1)
})

# Compare fixed parameter estimates between control and estimated F runs with reasonable low catch targets
# Results are very similar <1% maximum difference
# Not sure why there is any difference? Maybe there is an assumption of symmetry
# between the F pars that back propagates the impact of Future F's to past F's???
# or this is just the result of rounding error in the final gradient convergence thresholds??
estimation_error <- max(abs(sdr_fixed_5_year_project_catch_low[-c(33:37), "Estimate"] - sdr_fixed_no_project[, "Estimate"]) / abs(sdr_fixed_no_project[, "Estimate"]))

sd_error <- max(abs(sdr_fixed_5_year_project_catch_low[-c(33:37), "Std. Error"] - sdr_fixed_no_project[, "Std. Error"]) / abs(sdr_fixed_no_project[, "Std. Error"]))

# rec_devs <- sdr_report_5_year_project_catch_low[row.names(sdr_report_5_year_project_catch_low) == "log_devs", ][30:34, ]

test_that("projections with low catch data achieve same estimates and no projection model run", {
  #' @description Test that rec devs were fixed at zero in projection.
  # expect_equal(sum(rec_devs[, "Estimate"]), 0)

  #' @description Test that the maximum parameter estimate difference between a low catch projection run and no projection run is less than 10%.
  expect_lt(estimation_error, 0.1)

  #' @description Test that the maximum parameter standard deviation estimate difference between a low catch projection run and no projection run is less than 10%.
  expect_lt(sd_error, 0.1)
})

# Compare fixed parameter estimates between control and estimated F runs with high catch targets (overfishing)
# Results are not all similar something is happening with the Fmort results in particular
# This is somewhat expected as the model is being forced to make the future projected catches obtainable.
# This interaction means we would need to restrain or detect scenarios where this interaction is occurring.
# We need to test if this is just a catch issue or if it effects reference point projections targeting SPR.
estimation_error <- max(abs(sdr_fixed_5_year_project_catch_high[-c(33:37), "Estimate"] - sdr_fixed_no_project[, "Estimate"]) / abs(sdr_fixed_no_project[, "Estimate"]))

sd_error <- max(abs(sdr_fixed_5_year_project_catch_high[-c(33:37), "Std. Error"] - sdr_fixed_no_project[, "Std. Error"]) / abs(sdr_fixed_no_project[, "Std. Error"]))

test_that("projections with high catch data achieve same estimates and no projection model run", {
  #' @description Test that the maximum parameter estimate difference between a low catch projection run and no projection run is less than 10%.
  expect_gt(estimation_error, 0.7)

  #' @description Test that the maximum parameter standard deviation estimate difference between a low catch projection run and no projection run is less than 10%.
  expect_gt(sd_error, 0.7)
})
# Compare fixed parameter estimates between control and estimated F runs with high catch targets (overfishing)
# But super high log_sd = 5 on the projection catches so the model isn't forced to fit the targets
# Results are almost identical 1.426e-5 max proportion error
# In this case the model seems to estimate the most catch it can without changing
# the estimation period fits which is likely our best case scenario vs fixed catch
# projections such as SS that can go wild when the catches are too high.
# This appears to achieve close to a constant F projection which is interesting.

# TODO: Find the code to run these comparisons in another old projections branch
# max((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
# min((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Estimate"] - sdr_fixed_no_project[,"Estimate"])/ sdr_fixed_no_project[,"Estimate"])
#
# max((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])
# min((sdr_fixed_5_year_project_catch_high_cv_high[-c(33:37),"Std. Error"] - sdr_fixed_no_project[,"Std. Error"])/ sdr_fixed_no_project[,"Std. Error"])


# Comparison of SSB target results

estimation_error <- max(abs(sdr_fixed_10_year_project_SSB_target[-c(50), "Estimate"] - sdr_fixed_no_project[, "Estimate"]) / abs(sdr_fixed_no_project[, "Estimate"]))

sd_error <- max(abs(sdr_fixed_10_year_project_SSB_target[-c(50), "Std. Error"] - sdr_fixed_no_project[, "Std. Error"]) / abs(sdr_fixed_no_project[, "Std. Error"]))

ssb_ratio_estimation_error <- max(abs(sdr_report_10_year_project_SSB_target[rownames(sdr_report_10_year_project_SSB_target) == "spawning_biomass_ratio", "Estimate"][1:31] -
  sdr_report_no_project[rownames(sdr_report_no_project) == "spawning_biomass_ratio", "Estimate"][1:31]) / abs(sdr_report_no_project[rownames(sdr_report_no_project) == "spawning_biomass_ratio", "Estimate"][1:31]))


ssb_ratio_target_error <- abs(sdr_report_10_year_project_SSB_target[rownames(sdr_report_10_year_project_SSB_target) == "spawning_biomass_ratio", "Estimate"][length(sdr_report_10_year_project_SSB_target[rownames(sdr_report_10_year_project_SSB_target) == "spawning_biomass_ratio", "Estimate"])] - ssb_ratio_target) / ssb_ratio_target

test_that("projections with spawning biomass ratio target achieve same estimates and no projection model run", {
  #' @description Test that the maximum parameter estimate difference between a low catch projection run and no projection run is less than 10%.
  expect_lt(estimation_error, 0.5)

  #' @description Test that the maximum parameter standard deviation estimate difference between a low catch projection run and no projection run is less than 10%.
  expect_lt(sd_error, 0.5)

  #' @description Test that the maximum parameter estimate difference between a low catch projection run and no projection run is less than 10%.
  expect_lt(ssb_ratio_estimation_error, 1.1)

  #' @description Test that the maximum parameter standard deviation estimate difference between a low catch projection run and no projection run is less than 10%.
  expect_lt(ssb_ratio_target_error, 0.1)
})
