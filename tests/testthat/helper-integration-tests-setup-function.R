# Helper file for FIMS R tests----
# This file contains multiple functions that are used to set up and run
# FIMS models with or without wrapper functions. The functions are sourced by
# devtools::load_all().

# FIMS dmultinorm function ----
#' FIMS dmultinom()
#' This function matches the dmultinom() function in TMB and differs from R
#' by NOT rounding obs to the nearest integer. The function is evaluated in
#' log space and returns the log probability mass function.
#'
#' @param x A vector of length K of numeric values.
#' @param p A numeric non-negative vector of length K, specifying the probability
#' for the K classes; must sum 1.
#'
#' @return The log of the probability mass function for the multinomial.
FIMS_dmultinom <- function(x, p) {
  xp1 <- x + 1
  log_pmf <- lgamma(sum(x) + 1) - sum(lgamma(xp1)) + sum(x * log(p))
  return(log_pmf)
}


# FIMS helper function to run FIMS model without wrappers ----
#' Set up and run FIMS model without using wrapper functions
#'
#' This function sets up and runs the FIMS for a given iteration.
#' It configures the model with the OM inputs and outputs (see simulated data from
#' tests/testthat/fixtures/simulate-integration-test-data.R),
#' and runs the optimization process.
#' It then generates and returns the results including parameter estimates, model
#' reports, and standard deviation reports.
#'
#' @param iter_id An integer specifying the iteration ID to use for loading
#' the OM data.
#' @param om_input_list A list of OM inputs, where each element
#' corresponds to a different iteration.
#' @param om_output_list A list of OM outputs, where each element
#' corresponds to a different iteration.
#' @param em_input_list A list of EM inputs, where each element
#' corresponds to a different iteration.
#' @param estimation_mode A logical value indicating whether to perform
#' optimization (`TRUE`) or skip it (`FALSE`). If `TRUE`, the model parameters
#' will be optimized using `nlminb`. If `FALSE`, the input values will be used
#' for the report.
#' @param random_effects A logical value indicating whether to include random
#' effects in the model (`TRUE`) or skip it (`FALSE`). If `TRUE`, random effects
#' will be included on recruitment in the model.
#' @param map A list used to specify mapping for the `MakeADFun` function from
#' the TMB package.
#'
#' @return A list containing the following elements:
#' \itemize{
#'   \item{parameters: A list of parameters for the TMB model.}
#'   \item{obj: The TMB model object created by `TMB::MakeADFun`.}
#'   \item{opt: The result of the optimization process, if `estimation_mode`
#'   is `TRUE`. `NULL` if `estimation_mode` is `FALSE`.}
#'   \item{report: The model report obtained from the TMB model.}
#'   \item{sdr_report: Summary of the standard deviation report for the
#'   model parameters.}
#'   \item{sdr_fixed: Summary of the standard deviation report for the
#'   fixed parameters.}
#' }
#' @examples
#' results <- setup_and_run_FIMS_without_wrappers(
#'   iter_id = 1,
#'   om_input_list = om_input_list,
#'   om_output_list = om_output_list,
#'   em_input_list = em_input_list,
#'   estimation_mode = TRUE,
#'   random_effects = NULL
#' )
setup_and_run_FIMS_without_wrappers <- function(iter_id,
                                                om_input_list,
                                                om_output_list,
                                                em_input_list,
                                                estimation_mode = TRUE,
                                                random_effects = NULL,
                                                map = list()) {
  # Load operating model data for the current iteration
  om_input <- om_input_list[[iter_id]] # Operating model input for the current iteration
  om_output <- om_output_list[[iter_id]] # Operating model output for the current iteration
  em_input <- em_input_list[[iter_id]] # Estimation model input for the current iteration

  # Clear any previous FIMS settings
  clear()

  # Extract fishing fleet catch data (observed) and initialize index module
  catch <- em_input[["L.obs"]][["fleet1"]]

  # set fishing fleet catch data, need to set dimensions of data index
  # currently FIMS only has a fleet module that takes index for both survey index and fishery catch
  fishing_fleet_catch <- create_data("catch", om_input[["nyr"]])
  set_data(
    fishing_fleet_catch, catch)

  # set fishing fleet age comp data, need to set dimensions of age comps
  # Here the create function initializes the object with length nyr*n_ages
  fishing_fleet_age_comp <- create_data("age_comp", om_input[["nyr"]], om_input[["nages"]])

  # Here we fill in the values for the object with the observed age comps for fleet one
  # we multiply these proportions by the sample size for likelihood weighting
  set_data(
    fishing_fleet_age_comp,
    c(t(em_input[["L.age.obs"]][["fleet1"]])) * em_input[["n.L"]][["fleet1"]]
  )

  # set fishing fleet length comp data, need to set dimensions of length comps
  fishing_fleet_length_comp <- create_data(
    "length_comp", om_input[["nyr"]], om_input[["nlengths"]]
  )
  set_data(
    fishing_fleet_length_comp,
    c(t(em_input[["L.length.obs"]][["fleet1"]])) *
      em_input[["n.L.lengthcomp"]][["fleet1"]]
  )

  # Fleet
  # Create the fishing fleet
  # turn on estimation of inflection_point and slope
  fishing_fleet_selectivity <- create_selectivity("Logistic")
  set_variable_vector(
    fishing_fleet_selectivity, "inflection_point",
    om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]], "fixed_effects"
  )
  set_variable_vector(
    fishing_fleet_selectivity, "slope",
    om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]], "fixed_effects"
  )

  # Initialize the fishing fleet module and set its dimensions
  fishing_fleet <- create_fleet()
  set_fleet_constants(
    fishing_fleet,
    om_input[["nyr"]], om_input[["nages"]], om_input[["nlengths"]]
  )

  # Log-transform OM fishing mortality
  set_variable_vector(
    fishing_fleet, "log_Fmort",
    log(om_output[["f"]][1:om_input[["nyr"]]]), "fixed_effects"
  )
  set_variable_vector(
    fishing_fleet, "log_q", log(1.0), "assumed_known"
  )
  set_fleet_selectivity(fishing_fleet, fishing_fleet_selectivity)
  # A slot left out means the fleet has no data of that kind.
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
    rep(
      log(sqrt(log(em_input[["cv.L"]][["fleet1"]]^2 + 1))),
      om_input[["nyr"]]
    ),
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
    get_fleet_observed_data_ids(fishing_fleet)[["age_comp"]]
  )
  set_distribution_links(
    fishing_fleet_agecomp_distribution, "data",
    get_variable_vector_id(fishing_fleet, "agecomp_proportion")
  )

  # Set up fishery length composition data using the multinomial
  fishing_fleet_lengthcomp_distribution <- create_distribution("dmultinom")
  set_distribution_observed_data(
    fishing_fleet_lengthcomp_distribution,
    get_fleet_observed_data_ids(fishing_fleet)[["length_comp"]]
  )
  set_distribution_links(
    fishing_fleet_lengthcomp_distribution, "data",
    get_variable_vector_id(fishing_fleet, "lengthcomp_proportion"
    )
  )
  set_distribution_note(
    fishing_fleet_lengthcomp_distribution,
    "fishing_fleet_lengthcomp_distribution"
  )

  # Set age-to-length conversion matrix, and turn off its estimation
  # TODO: If an age_to_length_conversion matrix is provided, the code below
  # still executes. Consider adding a check in the Rcpp interface to ensure
  # users provide a vector of inputs.
  set_variable_vector(
    fishing_fleet, "age_to_length_conversion",
    c(t(em_input[["age_to_length_conversion"]])), "assumed_known"
  )

  # Repeat similar setup for the survey fleet (e.g., index, age comp, and length comp)
  # This includes initializing logistic selectivity, observed data modules, and distribution links.
  survey_index <- em_input[["surveyB.obs"]][["survey1"]]
  survey_fleet_index <- create_data("index", om_input[["nyr"]])
  set_data(
    survey_fleet_index, survey_index
  )

  survey_fleet_age_comp <- create_data("age_comp", om_input[["nyr"]], om_input[["nages"]])
  set_data(
    survey_fleet_age_comp,
    c(t(em_input[["survey.age.obs"]][["survey1"]])) *
      em_input[["n.survey"]][["survey1"]]
  )

  survey_fleet_length_comp <- create_data(
    "length_comp", om_input[["nyr"]], om_input[["nlengths"]]
  )
  set_data(
    survey_fleet_length_comp,
    c(t(em_input[["survey.length.obs"]][["survey1"]])) *
      em_input[["n.survey.lengthcomp"]][["survey1"]]
  )

  # Fleet
  # Create the survey fleet
  # turn on estimation of inflection_point and slope
  survey_fleet_selectivity <- create_selectivity("Logistic")
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
    survey_fleet,
    om_input[["nyr"]], om_input[["nages"]], om_input[["nlengths"]]
  )
  # Set very low survey fishing mortality
  set_variable_vector(
    survey_fleet, "log_Fmort",
    rep(-200, om_input[["nyr"]]), "assumed_known"
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
      om_input[["nyr"]]
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
    get_fleet_observed_data_ids(survey_fleet)[["age_comp"]]
  )
  set_distribution_links(
    survey_fleet_agecomp_distribution, "data",
    get_variable_vector_id(survey_fleet, "agecomp_proportion")
  )

  # Length composition distribution
  survey_fleet_lengthcomp_distribution <- create_distribution("dmultinom")
  set_distribution_observed_data(
    survey_fleet_lengthcomp_distribution,
    get_fleet_observed_data_ids(survey_fleet)[["length_comp"]]
  )
  set_distribution_links(
    survey_fleet_lengthcomp_distribution, "data",
    get_variable_vector_id(survey_fleet, "lengthcomp_proportion"
    )
  )

  # Set age to length conversion matrix, and turn off estimation for
  # length-at-age.
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
  recruitment <- create_recruitment("BevertonHolt")
  if (is.null(random_effects) || random_effects[["recruitment"]] == "log_devs") {
    recruitment_process <- create_recruitment("log_devs")
  } else {
    recruitment_process <- create_recruitment("log_r")
  }
  set_recruitment_process(recruitment, recruitment_process)

  # NOTE: the estimation status is given on every call, including where it is
  # "assumed_known". It is an argument of set_variable_vector() rather than a
  # property of the field, which is what lets the same quantity be estimated in
  # one model and assumed known in another.

  # set up log_rzero (equilibrium recruitment)
  set_variable_vector(
    recruitment, "log_rzero",
    log(om_input[["R0"]]), "fixed_effects"
  )
  # set up logit_steep
  set_variable_vector(
    recruitment, "logit_steep",
    -log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2), "assumed_known"
  )
  set_recruitment_n_years(recruitment, om_input[["nyr"]])

  # turn on estimation of deviations
  # recruit deviations should enter the model in normal space.
  # The log is taken in the likelihood calculations
  if (is.null(random_effects) || random_effects[["recruitment"]] == "log_devs") {
    # The status is applied in the same call that sets the values, so it is
    # resolved here rather than in a second pass. When random_effects is NULL
    # the deviations keep the default status, matching the previous behavior.
    log_devs_status <- if (
      "recruitment" %in% names(random_effects) &&
        random_effects[["recruitment"]] == "log_devs"
    ) {
      "random_effects"
    } else {
      "assumed_known"
    }
    set_variable_vector(
    recruitment, "log_devs",
      om_input[["logR.resid"]][2:om_input[["nyr"]]], log_devs_status
    )
  }

  if ("recruitment" %in% names(random_effects)) {
    if (random_effects[["recruitment"]] == "log_r") {
      recruits_true <- matrix(c(t(om_output[["N.age"]])),
        om_input[["nyr"]], om_input[["nages"]],
        byrow = TRUE
      )[, 1]
      log_r_values <- if (!estimation_mode) {
        recruits_true[2:om_input[["nyr"]]]
      } else {
        rep(0, om_input[["nyr"]] - 1)
      }
      set_variable_vector(
    recruitment, "log_r",
        log_r_values, "random_effects"
      )
    }
  }

  if ("selectivity" %in% names(random_effects)) {
    # The previous version branched on random_effects[["selectivity"]] and set
    # fields named log_devs, log_sel, and inflection_point$slope on the
    # selectivity modules. None of those exist on LogisticSelectivityInterface,
    # which carries only inflection_point and slope, so those branches could
    # never have run. No test passes a "selectivity" entry. Rather than
    # translate code that cannot work, this reports the gap.
    cli::cli_abort(
      "Random effects on selectivity are not implemented in the interface."
    )
  }

  # set up logR_sd using the normal log_sd parameter
  # logR_sd is NOT logged. It needs to enter the model logged b/c the exp() is
  # taken before the likelihood calculation
  recruitment_distribution <- create_distribution("dnorm")
  set_variable_vector(
    recruitment_distribution, "log_sd",
    log(om_input[["logR_sd"]]), "assumed_known"
  )
  set_variable_vector(
    recruitment_distribution, "observed_values",
    rep(0, om_input[["nyr"]] - 1), "assumed_known"
  )
  set_variable_vector(
    recruitment_distribution, "expected_values",
    rep(0, om_input[["nyr"]] - 1), "assumed_known"
  )

  if ("recruitment" %in% names(random_effects)) {
    if (random_effects[["recruitment"]] == "log_devs") {
      set_variable_vector(
    recruitment_distribution, "log_sd",
        log(om_input[["logR_sd"]]), "fixed_effects"
      )
      set_distribution_links(
    recruitment_distribution, "random_effects",
        get_variable_vector_id(recruitment, "log_devs")
      )
    }
    if (random_effects[["recruitment"]] == "log_r") {
      set_variable_vector(
    recruitment_distribution, "log_sd",
        log(1), "fixed_effects"
      )
      set_distribution_links(
    recruitment_distribution, "random_effects",
        c(
          get_variable_vector_id(recruitment, "log_r"),
          get_variable_vector_id(recruitment, "log_expected_recruitment"
          )
        )
      )
    }
  }

  if (is.null(random_effects)) {
    set_distribution_links(
    recruitment_distribution, "random_effects",
      get_variable_vector_id(recruitment, "log_devs")
    )
  }

  # Growth
  ewaa_growth <- create_growth("EWAA")
  set_growth_n_years(ewaa_growth, om_input[["nyr"]])
  set_numeric_vector(
    ewaa_growth, "ages", om_input[["ages"]]
  )
  set_numeric_vector(
    ewaa_growth, "weights", c(t(om_input[["W.mt"]]))
  )

  # Maturity
  maturity <- create_maturity("Logistic")
  set_variable_vector(
    maturity, "inflection_point",
    om_input[["A50.mat"]], "assumed_known"
  )
  set_variable_vector(
    maturity, "slope",
    om_input[["slope.mat"]], "assumed_known"
  )

  # Population
  population <- create_population()
  set_variable_vector(
    population, "log_M",
    rep(
      log(om_input[["M.age"]][1]),
      om_input[["nyr"]] * om_input[["nages"]]
    ),
    "assumed_known"
  )
  set_variable_vector(
    population, "log_init_naa",
    log(om_output[["N.age"]][1, ]), "fixed_effects"
  )
  set_population_constants(population, om_input[["nyr"]], om_input[["nages"]])
  set_numeric_vector(
    population, "ages", om_input[["ages"]]
  )
  # The fleet count is no longer set here: it follows from the fleets linked
  # below, so the two cannot disagree.
  set_population_processes(
    population,
    maturity = maturity,
    growth = ewaa_growth,
    recruitment = recruitment
  )
  set_population_fleets(population, list(fishing_fleet, survey_fleet))

  # Set up catch at age model
  caa <- create_fishery_model("CatchAtAge")
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
  opt <- NULL
  if (estimation_mode == TRUE) {
    opt <- stats::nlminb(obj[["par"]], obj[["fn"]], obj[["gr"]],
      control = list(eval.max = 10000, iter.max = 10000, trace = 0)
    )
    FIMS::set_fixed(opt$par)
    # Defaults to every module registered since the last clear(), which is
    # what the model was built from.
    fims_finalized <- get_output(caa)

    sdr <- TMB::sdreport(obj)
    sdr_report <- summary(sdr, "report")
    sdr_fixed <- summary(sdr, "fixed")
    sdr_random <- summary(sdr, "random")
  } else {
    sdr <- list()
    sdr_report <- list()
    sdr_fixed <- matrix(obj[["env"]]$parList()[["p"]], ncol = 1, dimnames = list(NULL, "Estimate"))
    sdr_random <- matrix(obj[["env"]]$parList()[["re"]], ncol = 1, dimnames = list(NULL, "Estimate"))
  }
  row.names(sdr_fixed) <- names(FIMS:::get_parameter_names(sdr_fixed[, 1]))
  row.names(sdr_random) <- names(FIMS:::get_random_names(sdr_random[, 1]))

  # Call report using MLE parameter values, or
  # the input values if optimization is skipped
  report <- obj[["report"]](obj[["env"]][["last.par.best"]])


  clear()

  # Return the results as a list
  return(list(
    parameters = parameters,
    obj = obj,
    opt = opt,
    report = report,
    sdr_report = sdr_report,
    sdr_fixed = sdr_fixed,
    sdr_random = sdr_random,
    sdr = sdr
  ))
}


# FIMS helper function to run FIMS model with wrappers ----
#' Set Up and Run FIMS Model using wrapper functions
#'
#' This function sets up and runs the FIMS for a given iteration.
#' It configures the model with the OM inputs and outputs (see simulated data from
#' tests/testthat/fixtures/simulate-integration-test-data.R),
#' and runs the optimization process.
#' It then generates and returns the results including parameter estimates, model
#' reports, and standard deviation reports.
#'
#' @param iter_id An integer specifying the iteration ID to use for loading
#' the OM data.
#' @param om_input_list A list of OM inputs, where each element
#' corresponds to a different iteration.
#' @param om_output_list A list of OM outputs, where each element
#' corresponds to a different iteration.
#' @param em_input_list A list of EM inputs, where each element
#' corresponds to a different iteration.
#' @param estimation_mode A logical value indicating whether to perform
#' optimization (`TRUE`) or skip it (`FALSE`). If `TRUE`, the model parameters
#' will be optimized using `nlminb`. If `FALSE`, the input values will be used
#' for the report.
#' @param random_effects A logical value indicating whether to include random
#' effects in the model (`TRUE`) or skip it (`FALSE`). If `TRUE`, random effects
#' will be included on recruitment in the model.
#' @param map A list used to specify mapping for the `MakeADFun` function from
#' the TMB package.
#'
#' @return A list containing the following elements:
#' \itemize{
#'   \item{parameters: A list of parameters for the TMB model.}
#'   \item{obj: The TMB model object created by `TMB::MakeADFun`.}
#'   \item{opt: The result of the optimization process, if `estimation_mode`
#'   is `TRUE`. `NULL` if `estimation_mode` is `FALSE`.}
#'   \item{report: The model report obtained from the TMB model.}
#'   \item{sdr_report: Summary of the standard deviation report for the
#'   model parameters.}
#'   \item{sdr_fixed: Summary of the standard deviation report for the
#'   fixed parameters.}
#' }
#' @examples
#' results <- setup_and_run_FIMS_with_wrappers(
#'   iter_id = 1,
#'   om_input_list = om_input_list,
#'   om_output_list = om_output_list,
#'   em_input_list = em_input_list,
#'   estimation_mode = TRUE
#' )
setup_and_run_FIMS_with_wrappers <- function(iter_id,
                                             om_input_list,
                                             om_output_list,
                                             em_input_list,
                                             estimation_mode = TRUE,
                                             random_effects = FALSE,
                                             modified_parameters,
                                             map = list()) {
  # Load operating model data for the current iteration
  om_input <- om_input_list[[iter_id]]
  om_output <- om_output_list[[iter_id]]
  em_input <- em_input_list[[iter_id]]

  # Clear any previous FIMS settings
  clear()

  data <- FIMS::FIMSFrame(data_big)
  if (tibble::is_tibble(modified_parameters)) {
    parameters <- modified_parameters
  } else {
    parameters <- modified_parameters[[iter_id]]
  }

  # The model will not always run when log_q is very small.
  # We will need to make sure log_q is the true value for deterministic runs but
  # then reset to log(1.0) for estimation runs.
  if (estimation_mode == TRUE) {
    parameters <- parameters |>
      dplyr::mutate(
        value = dplyr::if_else(
          fleet == "survey1" & label == "log_q",
          log(1.0),
          value
        )
      )
  }

  parameter_list <- initialize_fims(
    parameters = parameters,
    data = data
  )

  fit <- fit_fims(input = parameter_list, optimize = estimation_mode)

  clear()
  # Return the results as a list
  return(fit)
}
