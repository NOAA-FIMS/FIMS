library(FIMS)

# Native .Call equivalent of tmb_Rcpp.R.
#
# Goal: mirror the Rcpp initialization as closely as possible. The only
# intended difference is the interface used to create/register FIMS objects.

fixture_path <- file.path("tests", "testthat", "fixtures", "integration_test_data.RData")
if (!file.exists(fixture_path)) {
    stop("Fixture file not found at: ", fixture_path)
}

load(fixture_path)

iter_id <- 1
random_effects <- NULL

run_fims <- function() {
    om_input <- om_input_list[[iter_id]]
    om_output <- om_output_list[[iter_id]]
    em_input <- em_input_list[[iter_id]]

    nyr <- as.integer(om_input[["nyr"]])
    nages <- as.integer(om_input[["nages"]])
    nlengths <- as.integer(om_input[["nlengths"]])
    ages <- as.numeric(om_input[["ages"]])

    # -----------------------------------------------------------------------------
    # Clear Information -- equivalent to clear()
    # -----------------------------------------------------------------------------
    .Call("fims_call_information_clear", PACKAGE = "FIMS")

    # -----------------------------------------------------------------------------
    # Observed data values -- exactly the values used by tmb_Rcpp.R
    # -----------------------------------------------------------------------------
    fishing_landings <- as.numeric(em_input[["L.obs"]][["fleet1"]])
    fishing_age_comp <-
        as.numeric(c(t(em_input[["L.age.obs"]][["fleet1"]]))) *
            as.numeric(em_input[["n.L"]][["fleet1"]])
    fishing_length_comp <-
        as.numeric(c(t(em_input[["L.length.obs"]][["fleet1"]]))) *
            as.numeric(em_input[["n.L.lengthcomp"]][["fleet1"]])

    survey_index <- as.numeric(em_input[["surveyB.obs"]][["survey1"]])
    survey_age_comp <-
        as.numeric(c(t(em_input[["survey.age.obs"]][["survey1"]]))) *
            as.numeric(em_input[["n.survey"]][["survey1"]])
    survey_length_comp <-
        as.numeric(c(t(em_input[["survey.length.obs"]][["survey1"]]))) *
            as.numeric(em_input[["n.survey.lengthcomp"]][["survey1"]])

    age_to_length_conversion <-
        as.numeric(c(t(em_input[["age_to_length_conversion"]])))

    # -----------------------------------------------------------------------------
    # Fishing selectivity -- Rcpp: both parameters are fixed effects
    # -----------------------------------------------------------------------------
    fishing_selectivity_id <- selectivity_logistic_create(
        inflection_point = as.numeric(
            om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]]
        ),
        slope = as.numeric(
            om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]]
        ),
        inflection_point_estimation_type = "fixed_effects",
        slope_estimation_type = "fixed_effects"
    )

    # -----------------------------------------------------------------------------
    # Fishing fleet
    # Rcpp:
    #   log_Fmort: nyr fixed effects
    #   log_q:     scalar constant
    # -----------------------------------------------------------------------------
    fishing_fleet_id <- fleet_create(
        log_fmort = log(as.numeric(om_output[["f"]])),
        log_q = log(1.0),
        selectivity_id = fishing_selectivity_id,
        age_to_length_conversion = age_to_length_conversion,
        log_fmort_estimation_type = rep("fixed_effects", nyr),
        log_q_estimation_type = "constant"
    )

    # -----------------------------------------------------------------------------
    # Survey selectivity -- Rcpp: both parameters are fixed effects
    # -----------------------------------------------------------------------------
    survey_selectivity_id <- selectivity_logistic_create(
        inflection_point = as.numeric(
            om_input[["sel_survey"]][["survey1"]][["A50.sel1"]]
        ),
        slope = as.numeric(
            om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]
        ),
        inflection_point_estimation_type = "fixed_effects",
        slope_estimation_type = "fixed_effects"
    )

    # -----------------------------------------------------------------------------
    # Survey fleet
    # Rcpp:
    #   log_Fmort: nyr constants at -200
    #   log_q:     ONE scalar fixed effect (do not expand to nyr parameters)
    # -----------------------------------------------------------------------------
    survey_fleet_id <- fleet_create(
        log_fmort = rep(-200, nyr),
        log_q = log(as.numeric(om_output[["survey_q"]][["survey1"]])),
        selectivity_id = survey_selectivity_id,
        age_to_length_conversion = age_to_length_conversion,
        log_fmort_estimation_type = "constant",
        log_q_estimation_type = "fixed_effects"
    )

    # -----------------------------------------------------------------------------
    # Recruitment -- mirror tmb_Rcpp.R literally
    # -----------------------------------------------------------------------------
    logit_steep <-
        -log(1.0 - as.numeric(om_input[["h"]])) +
        log(as.numeric(om_input[["h"]]) - 0.2)

    recruitment_id <- recruitment_beverton_holt_create(
        logit_steep = logit_steep,
        log_rzero = log(as.numeric(om_input[["R0"]])),
        log_devs = as.numeric(om_input[["logR.resid"]][-1]),
        logit_steep_estimation_type = "constant",
        log_rzero_estimation_type = "fixed_effects"
    )

    # NOTE: tmb_Rcpp.R leaves log_devs at their default estimation type when
    # random_effects <- NULL because the fixed-effect assignment is nested inside
    # `if ("recruitment" %in% names(random_effects))`. Do not promote log_devs to
    # fixed effects here unless the Rcpp implementation is changed as well.

    # -----------------------------------------------------------------------------
    # Growth -- EWAA
    # -----------------------------------------------------------------------------
    growth_id <- growth_ewaa_create(
        ages = ages,
        weights = as.numeric(c(t(om_input[["W.mt"]]))),
        n_years = nyr
    )

    # -----------------------------------------------------------------------------
    # Maturity -- Rcpp: both parameters constant
    # -----------------------------------------------------------------------------
    maturity_id <- maturity_logistic_create(
        inflection_point = as.numeric(om_input[["A50.mat"]]),
        slope = as.numeric(om_input[["slope.mat"]]),
        inflection_point_estimation_type = "constant",
        slope_estimation_type = "constant"
    )

    # -----------------------------------------------------------------------------
    # Population
    # Rcpp:
    #   log_M:        nyr*nages constants
    #   log_init_naa: nages fixed effects
    #   f multiplier: left at its default; native representation is explicit
    #                 constant log(1) by year.
    # -----------------------------------------------------------------------------
    population_id <- population_create(
        log_m = rep(
            log(as.numeric(om_input[["M.age"]][1])),
            nyr * nages
        ),
        log_f_multiplier = rep(log(1.0), nyr),
        log_init_naa = log(as.numeric(om_output[["N.age"]][1, ])),
        log_m_estimation_type = "constant",
        log_f_multiplier_estimation_type = "constant",
        log_init_naa_estimation_type = rep("fixed_effects", nages),
        maturity_id = maturity_id,
        growth_id = growth_id,
        recruitment_id = recruitment_id,
        fleet_ids = c(fishing_fleet_id, survey_fleet_id)
    )

    # -----------------------------------------------------------------------------
    # Register the native model container.
    #
    # This native call must happen before likelihood construction because
    # fims_call_build_default_likelihood() attaches data/distributions to the
    # registered model. It is therefore not lifecycle-equivalent to the final
    # Rcpp CreateTMBModel() call, despite the similar name.
    # -----------------------------------------------------------------------------
    create_model_result <- .Call(
        "fims_call_create_model",
        PACKAGE = "FIMS"
    )

    # -----------------------------------------------------------------------------
    # Data + distributions + links
    #
    # Numerically, this builder should mirror the objects and links created in
    # tmb_Rcpp.R. In particular, verify that it applies exactly these transforms:
    #   landings/index: log_sd = log(sqrt(log(cv^2 + 1)))
    #   recruitment:    log_sd = log(logR_sd)
    # -----------------------------------------------------------------------------
    likelihood_result <- .Call(
        "fims_call_build_default_likelihood",
        as.integer(fishing_fleet_id),
        as.integer(survey_fleet_id),
        fishing_landings,
        as.numeric(em_input[["cv.L"]][["fleet1"]]),
        fishing_age_comp,
        fishing_length_comp,
        survey_index,
        as.numeric(em_input[["cv.survey"]][["survey1"]]),
        survey_age_comp,
        survey_length_comp,
        as.numeric(om_input[["logR_sd"]]),
        nyr,
        nages,
        nlengths,
        PACKAGE = "FIMS"
    )

    # -----------------------------------------------------------------------------
    # Parameter extraction -- equivalent to get_fixed()/get_random()
    # -----------------------------------------------------------------------------
    fixed_parameters <- as.numeric(
        .Call("fims_call_information_get_fixed", PACKAGE = "FIMS")
    )
    random_parameters <- as.numeric(
        .Call("fims_call_information_get_random", PACKAGE = "FIMS")
    )

    parameter_names <- tryCatch(
        as.character(
            .Call("fims_call_information_get_parameter_names", PACKAGE = "FIMS")
        ),
        error = function(e) character(0)
    )

    parameters <- list(
        p = fixed_parameters,
        re = random_parameters
    )

    # -----------------------------------------------------------------------------
    # TMB objective -- same MakeADFun call as tmb_Rcpp.R
    # -----------------------------------------------------------------------------
    obj <- TMB::MakeADFun(
        data = list(),
        parameters = parameters,
        DLL = "FIMS",
        silent = TRUE
    )

    objective_at_start <- as.numeric(obj[["fn"]](as.numeric(obj[["par"]])))

    # -----------------------------------------------------------------------------
    # Initial gradient diagnostics
    # -----------------------------------------------------------------------------
    par_start <- as.numeric(obj[["par"]])
    grad_start <- as.numeric(obj[["gr"]](par_start))

    initial_report <- tryCatch(
        obj[["report"]](par_start),
        error = function(e) NULL
    )

    # -----------------------------------------------------------------------------
    # Optimize using the same nlminb setup as tmb_Rcpp.R
    # -----------------------------------------------------------------------------
    opt <- stats::nlminb(
        start = par_start,
        objective = obj[["fn"]],
        gradient = obj[["gr"]],
        control = list(
            eval.max = 10000,
            iter.max = 10000,
            trace = 0
        )
    )

    # -----------------------------------------------------------------------------
    # Final diagnostics
    # -----------------------------------------------------------------------------
    par_final <- as.numeric(opt[["par"]])
    objective_final <- as.numeric(obj[["fn"]](par_final))
    grad_final <- as.numeric(obj[["gr"]](par_final))

    final_report <- tryCatch(
        obj[["report"]](par_final),
        error = function(e) NULL
    )

    # -----------------------------------------------------------------------------
    # Parameter movement
    # -----------------------------------------------------------------------------
    parameter_table <- data.frame(
        index = seq_along(par_start),
        start = par_start,
        estimate = par_final,
        change = par_final - par_start,
        abs_change = abs(par_final - par_start)
    )

    if (length(parameter_names) >= length(par_start)) {
        parameter_table$name <- parameter_names[seq_along(par_start)]
        parameter_table <- parameter_table[
            c("index", "name", "start", "estimate", "change", "abs_change")
        ]
    }

    # -----------------------------------------------------------------------------
    # Compact result object for interactive use
    # -----------------------------------------------------------------------------
    result <- list(
        objective_at_start = objective_at_start,
        objective_at_optimum = objective_final,
        objective_decrease = objective_at_start - objective_final,
        gradient_at_start = grad_start,
        gradient_at_optimum = grad_final,
        gradient_max_abs_at_start = max(abs(grad_start)),
        gradient_max_abs_at_optimum = max(abs(grad_final)),
        gradient_norm_at_start = sqrt(sum(grad_start^2)),
        gradient_norm_at_optimum = sqrt(sum(grad_final^2)),
        optimizer = opt,
        parameters = parameter_table,
        initial_report = initial_report,
        final_report = final_report
    )

    # invisible(result)

    print(opt)
}
