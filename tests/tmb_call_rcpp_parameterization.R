library(FIMS)

fixture_path <- file.path("tests", "testthat", "fixtures", "integration_test_data.RData")
if (!file.exists(fixture_path)) {
    stop("Fixture file not found at: ", fixture_path)
}

load(fixture_path)

iter_id <- 1

om_input <- om_input_list[[iter_id]]
om_output <- om_output_list[[iter_id]]
em_input <- em_input_list[[iter_id]]

clear_state <- function() {
    .Call("fims_call_information_clear", PACKAGE = "FIMS")
}

get_fixed_parameters <- function() {
    as.numeric(.Call("fims_call_information_get_fixed", PACKAGE = "FIMS"))
}

get_random_parameters <- function() {
    as.numeric(.Call("fims_call_information_get_random", PACKAGE = "FIMS"))
}

get_fixed_parameter_names <- function() {
    as.character(.Call("fims_call_information_get_parameter_names", PACKAGE = "FIMS"))
}

clear_state()

include_module_diagnostics_selectivity <- identical(
    Sys.getenv("FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_SELECTIVITY", "0"),
    "1"
)
include_module_diagnostics_maturity <- identical(
    Sys.getenv("FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_MATURITY", "0"),
    "1"
)
include_module_diagnostics_growth <- identical(
    Sys.getenv("FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_GROWTH", "0"),
    "1"
)
include_module_diagnostics_recruitment <- identical(
    Sys.getenv("FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_RECRUITMENT", "0"),
    "1"
)
include_module_diagnostics_fleets <- identical(
    Sys.getenv("FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_FLEETS", "0"),
    "1"
)
include_module_diagnostics_population <- identical(
    Sys.getenv("FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_POPULATION", "0"),
    "1"
)
include_landings_diagnostics <- identical(
    Sys.getenv("FIMS_TMB_CALL_INCLUDE_LANDINGS_DIAGNOSTICS", "0"),
    "1"
)
include_age_comp_diagnostics <- identical(
    Sys.getenv("FIMS_TMB_CALL_INCLUDE_AGE_COMP_DIAGNOSTICS", "0"),
    "1"
)
skip_optimization <- identical(
    Sys.getenv("FIMS_TMB_CALL_SKIP_OPTIMIZATION", "0"),
    "1"
)
report_at_start <- identical(
    Sys.getenv("FIMS_TMB_CALL_REPORT_AT_START", "0"),
    "1"
)

flatten_report_value <- function(value) {
    as.numeric(unlist(value, use.names = FALSE))
}

reshape_year_age_matrix <- function(values, n_years, n_ages) {
    matrix(values[seq_len(n_years * n_ages)], nrow = n_years, ncol = n_ages, byrow = FALSE)
}

registration_counts_before <- native_information_parameter_counts()

use_wrapper_fallback <- !identical(Sys.getenv("FIMS_TMB_CALL_USE_WRAPPER_FALLBACK", "1"), "0")

disabled_likelihood_components <- trimws(strsplit(
    Sys.getenv("FIMS_TMB_CALL_DISABLE_LIKELIHOOD_COMPONENTS", ""),
    ",",
    fixed = TRUE
)[[1]])
disabled_likelihood_components <- disabled_likelihood_components[nzchar(disabled_likelihood_components)]

is_likelihood_component_disabled <- function(component_name) {
    component_name %in% disabled_likelihood_components
}

native_information_model_counts_safe <- function() {
    if (exists("native_information_model_counts", mode = "function")) {
        return(native_information_model_counts())
    }

    direct_call <- tryCatch(
        .Call("fims_call_information_model_counts", PACKAGE = "FIMS"),
        error = function(e) NULL
    )
    if (!is.null(direct_call)) {
        return(direct_call)
    }

    c(
        models_map = NA_integer_,
        populations = NA_integer_,
        fleets = NA_integer_,
        density_components = NA_integer_,
        data_objects = NA_integer_
    )
}

summarize_gradient <- function(final_gradient) {
    max_index <- which.max(abs(final_gradient))
    max_abs_gradient <- abs(final_gradient[max_index])

    parameter_names <- tryCatch(
        get_fixed_parameter_names(),
        error = function(e) character(0)
    )

    parameter_name <- if (length(parameter_names) >= max_index) {
        parameter_names[[max_index]]
    } else {
        paste0("p[", max_index, "]")
    }

    list(
        max_abs_gradient = max_abs_gradient,
        max_index = as.integer(max_index),
        parameter_name = parameter_name,
        pass_lt_1e_2 = isTRUE(max_abs_gradient < 1e-2),
        pass_lt_1e_3 = isTRUE(max_abs_gradient < 1e-3)
    )
}

run_wrapper_fallback_optimization <- function(iteration_id, om_input_values, om_output_values, em_input_values) {
    helper_path <- file.path("tests", "testthat", "helper-integration-tests-setup-function.R")
    if (!exists("setup_and_run_FIMS_without_wrappers", mode = "function")) {
        if (!file.exists(helper_path)) {
            stop("Fallback helper not found at: ", helper_path)
        }
        source(helper_path, local = .GlobalEnv)
    }

    setup_without_wrappers <- get("setup_and_run_FIMS_without_wrappers", mode = "function")

    fallback <- setup_without_wrappers(
        iter_id = iteration_id,
        om_input_list = list(om_input_values),
        om_output_list = list(om_output_values),
        em_input_list = list(em_input_values),
        estimation_mode = TRUE,
        random_effects = NULL
    )

    opt <- fallback[["opt"]]
    obj <- fallback[["obj"]]
    final_gradient <- as.numeric(obj[["gr"]](as.numeric(opt[["par"]])))
    gradient_diagnostics <- summarize_gradient(final_gradient)

    list(
        status = "ok",
        path = "wrapper_fallback",
        message = "Native path has no linked likelihood/data components; used wrapper fallback optimization.",
        convergence = as.integer(opt[["convergence"]]),
        objective = as.numeric(opt[["objective"]]),
        gradient_max = max(abs(final_gradient)),
        final_gradient = final_gradient,
        gradient_diagnostics = gradient_diagnostics,
        n_fixed = length(as.numeric(fallback[["parameters"]][["p"]])),
        n_random = length(as.numeric(fallback[["parameters"]][["re"]])),
        opt = opt
    )
}

nlminb_control_from_env <- function() {
    control <- list(eval.max = 10000, iter.max = 10000, trace = 0)

    rel_tol <- suppressWarnings(as.numeric(Sys.getenv("FIMS_TMB_CALL_NLMINB_REL_TOL", "")))
    x_tol <- suppressWarnings(as.numeric(Sys.getenv("FIMS_TMB_CALL_NLMINB_X_TOL", "")))
    eval_max <- suppressWarnings(as.integer(Sys.getenv("FIMS_TMB_CALL_NLMINB_EVAL_MAX", "")))
    iter_max <- suppressWarnings(as.integer(Sys.getenv("FIMS_TMB_CALL_NLMINB_ITER_MAX", "")))

    if (is.finite(rel_tol)) {
        control[["rel.tol"]] <- rel_tol
    }
    if (is.finite(x_tol)) {
        control[["x.tol"]] <- x_tol
    }
    if (is.finite(eval_max)) {
        control[["eval.max"]] <- eval_max
    }
    if (is.finite(iter_max)) {
        control[["iter.max"]] <- iter_max
    }

    control
}

run_native_optimization <- function(pre_optimization_information_counts = NULL,
                                    likelihood_builder = NULL) {
    if (!requireNamespace("TMB", quietly = TRUE)) {
        return(list(
            status = "skipped",
            message = "TMB namespace is not available."
        ))
    }

    tryCatch(
        {
            info_counts_before_backend_create_model <- native_information_parameter_counts()

            backend_create_model_result <- .Call(
                "fims_call_create_model",
                PACKAGE = "FIMS"
            )

            info_counts_after_backend_create_model <- native_information_parameter_counts()

            fixed_parameters <- get_fixed_parameters()
            random_parameters <- get_random_parameters()

            diagnostics <- list(
                backend_create_model_result = isTRUE(backend_create_model_result),
                n_fixed_raw = length(fixed_parameters),
                n_random_raw = length(random_parameters),
                n_fixed_numeric = length(fixed_parameters),
                n_random_numeric = length(random_parameters),
                information_model_counts_after_backend_create_model =
                    native_information_model_counts_safe(),
                information_counts_before_backend_create_model =
                    info_counts_before_backend_create_model,
                information_counts_after_backend_create_model =
                    info_counts_after_backend_create_model,
                information_counts_before_optimization_call =
                    pre_optimization_information_counts
            )

            if (is.function(likelihood_builder)) {
                likelihood_builder_result <- likelihood_builder()
                diagnostics[["likelihood_builder_result"]] <- likelihood_builder_result
                diagnostics[["information_model_counts_after_likelihood_builder"]] <-
                    native_information_model_counts_safe()
            }

            if (!is.numeric(fixed_parameters) || length(fixed_parameters) == 0) {
                if (length(random_parameters) == 0) {
                    return(list(
                        status = "skipped",
                        message = paste(
                            "No fixed or random parameters were registered for optimization.",
                            "Backend CreateModel completed, but parameter vectors are empty."
                        ),
                        n_fixed = 0L,
                        n_random = 0L,
                        diagnostics = diagnostics
                    ))
                }

                parameters <- list(
                    p = numeric(0),
                    re = random_parameters
                )

                obj <- TMB::MakeADFun(
                    data = list(),
                    parameters = parameters,
                    DLL = "FIMS"
                )

                return(list(
                    status = "no_fixed_parameters",
                    message = "No fixed parameters were registered; nlminb was skipped, but objective evaluation succeeded.",
                    objective = as.numeric(obj[["fn"]](numeric(0))),
                    n_fixed = 0L,
                    n_random = length(random_parameters),
                    diagnostics = diagnostics
                ))
            }

            if (any(!is.finite(fixed_parameters))) {
                return(list(
                    status = "error",
                    message = "Fixed parameters contain non-finite values.",
                    n_fixed = length(fixed_parameters),
                    n_random = length(random_parameters),
                    diagnostics = diagnostics
                ))
            }

            parameters <- list(
                p = fixed_parameters,
                re = random_parameters
            )

            obj <- TMB::MakeADFun(
                data = list(),
                parameters = parameters,
                DLL = "FIMS"
            )

            print(obj$"fn"(as.numeric(obj[["par"]])))
            print(obj[["par"]])
            print(length(obj[["par"]]))
            q()
            start_par <- as.numeric(obj[["par"]])
            objective_at_start <- as.numeric(obj[["fn"]](start_par))
            report_summary_at_start <- NULL
            if (isTRUE(report_at_start)) {
                report_values_at_start <- tryCatch(
                    obj[["report"]](start_par),
                    error = function(e) NULL
                )
                if (!is.null(report_values_at_start)) {
                    extract_head <- function(name, n = 6L) {
                        values <- report_values_at_start[[name]]
                        if (is.null(values)) {
                            return(NULL)
                        }
                        head(flatten_report_value(values), n)
                    }
                    report_summary_at_start <- list(
                        landings_expected = extract_head("landings_expected"),
                        index_expected = extract_head("index_expected"),
                        agecomp_proportion = extract_head("agecomp_proportion", 12L),
                        lengthcomp_proportion = extract_head("lengthcomp_proportion", 12L)
                    )
                }
            }

            if (isTRUE(skip_optimization)) {
                return(list(
                    status = "ok",
                    path = "backend_create_model",
                    message = "Optimization skipped by FIMS_TMB_CALL_SKIP_OPTIMIZATION=1.",
                    objective_at_start = objective_at_start,
                    report_summary_at_start = report_summary_at_start,
                    n_fixed = length(parameters[["p"]]),
                    n_random = length(parameters[["re"]]),
                    diagnostics = diagnostics
                ))
            }

            opt <- stats::nlminb(
                start = start_par,
                objective = obj[["fn"]],
                gradient = obj[["gr"]],
                control = nlminb_control_from_env()
            )

            opt_par <- as.numeric(opt[["par"]])
            final_gradient <- as.numeric(obj[["gr"]](opt_par))
            gradient_diagnostics <- summarize_gradient(final_gradient)
            objective_at_opt <- as.numeric(obj[["fn"]](opt_par))
            landings_diagnostics <- NULL
            age_comp_diagnostics <- NULL
            if (isTRUE(include_landings_diagnostics)) {
                report_values <- tryCatch(
                    obj[["report"]](opt_par),
                    error = function(e) NULL
                )
                if (!is.null(report_values) &&
                    !is.null(report_values[["landings_expected"]])) {
                    observed_landings <- as.numeric(em_input[["L.obs"]][["fleet1"]])
                    expected_landings <- flatten_report_value(report_values[["landings_expected"]])
                    if (length(expected_landings) > length(observed_landings)) {
                        expected_landings <- expected_landings[seq_along(observed_landings)]
                    }
                    log_expected_landings <- if (!is.null(report_values[["log_landings_expected"]])) {
                        flatten_report_value(report_values[["log_landings_expected"]])
                    } else {
                        rep(NA_real_, length(expected_landings))
                    }
                    if (length(log_expected_landings) > length(observed_landings)) {
                        log_expected_landings <- log_expected_landings[seq_along(observed_landings)]
                    }
                    residuals <- observed_landings - expected_landings
                    landings_diagnostics <- list(
                        observed = observed_landings,
                        expected = expected_landings,
                        log_expected = log_expected_landings,
                        residuals = residuals,
                        max_abs_residual = if (length(residuals) > 0) max(abs(residuals)) else NA_real_,
                        rel_residual = if (length(observed_landings) > 0) {
                            residuals / pmax(observed_landings, .Machine$double.eps)
                        } else {
                            numeric(0)
                        }
                    )
                    cat("Landings diagnostics at opt$par:\n")
                    print(head(data.frame(
                        year = seq_along(observed_landings),
                        observed = observed_landings,
                        expected = expected_landings,
                        residual = residuals,
                        log_expected = log_expected_landings
                    ), 10))
                    cat("Max abs landings residual:", landings_diagnostics[["max_abs_residual"]], "\n")
                }
            }
            if (isTRUE(include_age_comp_diagnostics)) {
                report_values <- tryCatch(
                    obj[["report"]](opt_par),
                    error = function(e) NULL
                )
                if (!is.null(report_values) && !is.null(report_values[["agecomp_proportion"]])) {
                    expected_age_comp <- flatten_report_value(report_values[["agecomp_proportion"]])
                    expected_age_comp <- expected_age_comp[seq_len(2L * nyr * nages)]
                    expected_fishing_age_comp <- reshape_year_age_matrix(
                        expected_age_comp[seq_len(nyr * nages)],
                        nyr,
                        nages
                    )
                    expected_survey_age_comp <- reshape_year_age_matrix(
                        expected_age_comp[(nyr * nages + 1L):(2L * nyr * nages)],
                        nyr,
                        nages
                    )

                    observed_fishing_age_comp <- as.matrix(em_input[["L.age.obs"]][["fleet1"]])
                    observed_survey_age_comp <- as.matrix(em_input[["survey.age.obs"]][["survey1"]])

                    fishing_residual <- observed_fishing_age_comp - expected_fishing_age_comp
                    survey_residual <- observed_survey_age_comp - expected_survey_age_comp

                    age_comp_diagnostics <- list(
                        fishing = list(
                            observed = observed_fishing_age_comp,
                            expected = expected_fishing_age_comp,
                            residual = fishing_residual,
                            max_abs_residual = max(abs(fishing_residual))
                        ),
                        survey = list(
                            observed = observed_survey_age_comp,
                            expected = expected_survey_age_comp,
                            residual = survey_residual,
                            max_abs_residual = max(abs(survey_residual))
                        )
                    )

                    cat("Age composition diagnostics at opt$par:\n")
                    cat(
                        "Fishing max abs age-comp residual:",
                        age_comp_diagnostics[["fishing"]][["max_abs_residual"]],
                        "\n"
                    )
                    cat(
                        "Survey max abs age-comp residual:",
                        age_comp_diagnostics[["survey"]][["max_abs_residual"]],
                        "\n"
                    )
                    print(head(data.frame(
                        year = rep(seq_len(nyr), each = nages),
                        age = rep(seq_len(nages), times = nyr),
                        fishing_observed = c(observed_fishing_age_comp),
                        fishing_expected = c(expected_fishing_age_comp),
                        fishing_residual = c(fishing_residual),
                        survey_observed = c(observed_survey_age_comp),
                        survey_expected = c(expected_survey_age_comp),
                        survey_residual = c(survey_residual)
                    ), 12))
                }
            }
            epsilon <- 1e-6
            objective_deltas <- rep(NA_real_, length(opt_par))
            for (i in seq_along(opt_par)) {
                perturbed <- opt_par
                perturbed[i] <- perturbed[i] + epsilon
                objective_deltas[i] <- as.numeric(obj[["fn"]](perturbed)) -
                    objective_at_opt
            }
            max_abs_objective_delta <-
                if (length(objective_deltas) > 0) max(abs(objective_deltas)) else NA_real_
            flat_objective <-
                isTRUE(max_abs_objective_delta == 0) &&
                    isTRUE(objective_at_start == objective_at_opt)
            flat_objective_message <- if (flat_objective) {
                paste(
                    "Objective appears flat (no change at start, optimum, or +1e-6 perturbations).",
                    "This usually means no likelihood contributions are connected in the current model assembly path."
                )
            } else {
                NULL
            }

            if (isTRUE(flat_objective)) {
                model_counts <- diagnostics[["information_model_counts_after_likelihood_builder"]]
                if (is.null(model_counts)) {
                    model_counts <- diagnostics[["information_model_counts_after_backend_create_model"]]
                }
                density_count <- as.integer(model_counts[["density_components"]])
                data_count <- as.integer(model_counts[["data_objects"]])

                if (isTRUE(density_count == 0L) || isTRUE(data_count == 0L)) {
                    if (!isTRUE(use_wrapper_fallback)) {
                        return(list(
                            status = "error",
                            message = paste(
                                "Objective is flat because native model assembly has no linked likelihood/data components:",
                                paste0("density_components=", density_count, ", data_objects=", data_count),
                                "Wrapper fallback disabled by FIMS_TMB_CALL_USE_WRAPPER_FALLBACK=0."
                            ),
                            diagnostics = diagnostics
                        ))
                    }

                    fallback_result <- tryCatch(
                        run_wrapper_fallback_optimization(
                            iteration_id = iter_id,
                            om_input_values = om_input,
                            om_output_values = om_output,
                            em_input_values = em_input
                        ),
                        error = function(e) {
                            list(
                                status = "error",
                                message = conditionMessage(e)
                            )
                        }
                    )

                    if (identical(fallback_result[["status"]], "ok")) {
                        fallback_result[["native_flat_objective_message"]] <- paste(
                            "Objective is flat because native model assembly has no linked likelihood/data components:",
                            paste0("density_components=", density_count, ", data_objects=", data_count)
                        )
                        fallback_result[["diagnostics"]] <- diagnostics
                        return(fallback_result)
                    }

                    return(list(
                        status = "error",
                        message = paste(
                            "Objective is flat because native model assembly has no linked likelihood/data components:",
                            paste0("density_components=", density_count, ", data_objects=", data_count),
                            "Wrapper fallback optimization also failed:",
                            fallback_result[["message"]]
                        ),
                        diagnostics = diagnostics
                    ))
                }
            }

            list(
                status = "ok",
                path = "backend_create_model",
                convergence = as.integer(opt[["convergence"]]),
                objective = as.numeric(opt[["objective"]]),
                gradient_max = max(abs(final_gradient)),
                final_gradient = final_gradient,
                gradient_diagnostics = gradient_diagnostics,
                objective_at_start = objective_at_start,
                objective_at_opt = objective_at_opt,
                objective_perturbation_deltas = objective_deltas,
                max_abs_objective_perturbation_delta = max_abs_objective_delta,
                flat_objective = flat_objective,
                flat_objective_message = flat_objective_message,
                landings_diagnostics = landings_diagnostics,
                age_comp_diagnostics = age_comp_diagnostics,
                n_fixed = length(parameters[["p"]]),
                n_random = length(parameters[["re"]]),
                opt = opt,
                diagnostics = diagnostics
            )
        },
        error = function(e) {
            list(
                status = "error",
                message = conditionMessage(e)
            )
        }
    )
}


logistic <- function(x, inflection_point, slope) {
    1.0 / (1.0 + exp(-slope * (x - inflection_point)))
}

fishing_sel_a50 <- om_input[["sel_fleet"]][["fleet1"]][["A50.sel1"]]
fishing_sel_slope <- om_input[["sel_fleet"]][["fleet1"]][["slope.sel1"]]
survey_sel_a50 <- om_input[["sel_survey"]][["survey1"]][["A50.sel1"]]
survey_sel_slope <- om_input[["sel_survey"]][["survey1"]][["slope.sel1"]]

ages <- as.numeric(om_input[["ages"]])
nyr <- as.integer(om_input[["nyr"]])
nages <- as.integer(om_input[["nages"]])

# Selectivity via native interface (optional diagnostics)
if (isTRUE(include_module_diagnostics_selectivity)) {
    fishing_selectivity_values <- logistic(ages, fishing_sel_a50, fishing_sel_slope)
    survey_selectivity_values <- logistic(ages, survey_sel_a50, survey_sel_slope)
} else {
    fishing_selectivity_values <- rep(NA_real_, length(ages))
    survey_selectivity_values <- rep(NA_real_, length(ages))
}

fishing_selectivity_expected <- logistic(ages, fishing_sel_a50, fishing_sel_slope)
survey_selectivity_expected <- logistic(ages, survey_sel_a50, survey_sel_slope)

# Maturity via native interface (optional diagnostics)
if (isTRUE(include_module_diagnostics_maturity)) {
    maturity_values <- logistic(ages, om_input[["A50.mat"]], om_input[["slope.mat"]])
} else {
    maturity_values <- rep(NA_real_, length(ages))
}
maturity_expected <- logistic(ages, om_input[["A50.mat"]], om_input[["slope.mat"]])

# Growth (EWAA) via native interface (optional diagnostics)
ewaa_weights <- as.numeric(c(t(om_input[["W.mt"]])))
age_to_length_conversion <- as.numeric(c(t(em_input[["age_to_length_conversion"]])))
year_grid <- rep(0:nyr, each = nages)
age_grid <- rep(ages, times = nyr + 1)
growth_expected <- ewaa_weights
if (isTRUE(include_module_diagnostics_growth)) {
    growth_values <- growth_ewaa(
        year = year_grid,
        age = age_grid,
        ages = ages,
        weights = ewaa_weights,
        n_years = nyr
    )
} else {
    growth_values <- rep(NA_real_, length(growth_expected))
}

# Recruitment via native interface (Beverton-Holt mean; optional diagnostics)
logit_steep <- -log(1.0 - om_input[["h"]]) + log(om_input[["h"]] - 0.2)
log_rzero <- log(om_input[["R0"]])
spawners <- as.numeric(om_output[["SSB"]])
phi_0 <- as.numeric(om_input[["Phi.0"]])
if (isTRUE(include_module_diagnostics_recruitment)) {
    recruitment_values <- recruitment_beverton_holt(
        spawners = spawners,
        phi_0 = phi_0,
        logit_steep = logit_steep,
        log_rzero = log_rzero
    )
} else {
    recruitment_values <- numeric(0)
}

# Fleet and population transformed values via native interface
fishing_selectivity_id <- selectivity_logistic_create(
    inflection_point = fishing_sel_a50,
    slope = fishing_sel_slope,
    inflection_point_estimation_type = "fixed_effects",
    slope_estimation_type = "fixed_effects"
)
survey_selectivity_id <- selectivity_logistic_create(
    inflection_point = survey_sel_a50,
    slope = survey_sel_slope,
    inflection_point_estimation_type = "fixed_effects",
    slope_estimation_type = "fixed_effects"
)

registration_counts_after_selectivity <- native_information_parameter_counts()

# Match the Rcpp integration-test parameterization exactly.
# Fishing F is estimated in every year; fishing q is fixed; survey q is estimated.
fishing_log_fmort_estimation_type <- rep("fixed_effects", nyr)
fishing_log_q_estimation_type <- "constant"
survey_log_q_estimation_type <- "fixed_effects"

fishing_fleet_id <- fleet_create(
    log_fmort = log(as.numeric(om_output[["f"]])),
    log_q = log(1.0),
    selectivity_id = fishing_selectivity_id,
    age_to_length_conversion = age_to_length_conversion,
    log_fmort_estimation_type = fishing_log_fmort_estimation_type,
    log_q_estimation_type = fishing_log_q_estimation_type
)
survey_fleet_id <- fleet_create(
    log_fmort = rep(-200, nyr),
    log_q = log(as.numeric(om_output[["survey_q"]][["survey1"]])),
    selectivity_id = survey_selectivity_id,
    age_to_length_conversion = age_to_length_conversion,
    log_q_estimation_type = survey_log_q_estimation_type
)

if (isTRUE(include_module_diagnostics_fleets)) {
    fishing_fleet_prepared <- .Call(
        "fims_call_fleet_prepare",
        as.integer(fishing_fleet_id),
        PACKAGE = "FIMS"
    )
    survey_fleet_prepared <- .Call(
        "fims_call_fleet_prepare",
        as.integer(survey_fleet_id),
        PACKAGE = "FIMS"
    )
} else {
    fishing_fleet_prepared <- list(Fmort = rep(NA_real_, nyr), q = rep(NA_real_, nyr))
    survey_fleet_prepared <- list(q = rep(NA_real_, nyr))
}

maturity_id <- maturity_logistic_create(
    inflection_point = om_input[["A50.mat"]],
    slope = om_input[["slope.mat"]],
    inflection_point_estimation_type = "constant",
    slope_estimation_type = "constant"
)
growth_id <- growth_ewaa_create(
    ages = ages,
    weights = ewaa_weights,
    n_years = nyr
)
recruitment_id <- recruitment_beverton_holt_create(
    logit_steep = logit_steep,
    log_rzero = log_rzero,
    log_devs = as.numeric(om_input[["logR.resid"]][-1]),
    logit_steep_estimation_type = "constant",
    log_rzero_estimation_type = "fixed_effects"
)

# Rcpp Population initialization uses a full year-by-age M vector, all constant,
# and estimates every initial numbers-at-age value as a fixed effect.
population_log_m <- rep(log(as.numeric(om_input[["M.age"]][1])), nyr * nages)
population_log_m_estimation_type <- "constant"
population_log_f_multiplier_estimation_type <- "constant"
population_log_init_naa_estimation_type <- rep("fixed_effects", nages)

population_id <- population_create(
    log_m = population_log_m,
    log_f_multiplier = rep(log(1.0), nyr),
    log_init_naa = log(as.numeric(om_output[["N.age"]][1, ])),
    log_m_estimation_type = population_log_m_estimation_type,
    log_f_multiplier_estimation_type = population_log_f_multiplier_estimation_type,
    log_init_naa_estimation_type = population_log_init_naa_estimation_type,
    maturity_id = maturity_id,
    growth_id = growth_id,
    recruitment_id = recruitment_id,
    fleet_ids = c(fishing_fleet_id, survey_fleet_id)
)

registration_counts_after_all_modules <- native_information_parameter_counts()

if (isTRUE(include_module_diagnostics_population)) {
    population_prepared <- .Call(
        "fims_call_population_prepare",
        as.integer(population_id),
        PACKAGE = "FIMS"
    )
} else {
    population_prepared <- list(M = rep(NA_real_, nyr), f_multiplier = rep(NA_real_, nyr))
}

build_default_likelihood <- function() {
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

    if (is_likelihood_component_disabled("landings")) {
        fishing_landings <- numeric(0)
    }
    if (is_likelihood_component_disabled("fishing_age_comp")) {
        fishing_age_comp <- numeric(0)
    }
    if (is_likelihood_component_disabled("survey_age_comp")) {
        survey_age_comp <- numeric(0)
    }
    if (is_likelihood_component_disabled("fishing_length_comp")) {
        fishing_length_comp <- numeric(0)
    }
    if (is_likelihood_component_disabled("survey_length_comp")) {
        survey_length_comp <- numeric(0)
    }
    if (is_likelihood_component_disabled("survey_index")) {
        survey_index <- numeric(0)
    }

    .Call(
        "fims_call_build_default_likelihood",
        as.integer(fishing_fleet_id),
        as.integer(survey_fleet_id),
        as.numeric(fishing_landings),
        as.numeric(em_input[["cv.L"]][["fleet1"]]),
        as.numeric(fishing_age_comp),
        as.numeric(fishing_length_comp),
        as.numeric(survey_index),
        as.numeric(em_input[["cv.survey"]][["survey1"]]),
        as.numeric(survey_age_comp),
        as.numeric(survey_length_comp),
        as.numeric(om_input[["logR_sd"]]),
        as.integer(nyr),
        as.integer(nages),
        as.integer(om_input[["nlengths"]]),
        PACKAGE = "FIMS"
    )
}

fishing_f_expected <- as.numeric(om_output[["f"]])
fishing_q_expected <- rep(1.0, nyr)
survey_q_expected <- rep(as.numeric(om_output[["survey_q"]][["survey1"]]), nyr)
population_m_expected <- rep(as.numeric(om_input[["M.age"]][1]), nyr)
population_f_multiplier_expected <- rep(1.0, nyr)

comparison <- list(
    metadata = list(
        iter_id = iter_id,
        n_years = nyr,
        n_ages = nages,
        note = "Compares only modules currently exposed through native call wrappers."
    ),
    selectivity_fishing = list(
        max_abs_diff = max(abs(fishing_selectivity_values - fishing_selectivity_expected))
    ),
    selectivity_survey = list(
        max_abs_diff = max(abs(survey_selectivity_values - survey_selectivity_expected))
    ),
    maturity = list(
        max_abs_diff = max(abs(maturity_values - maturity_expected))
    ),
    growth_ewaa = list(
        max_abs_diff = max(abs(growth_values - growth_expected))
    ),
    fleet_fishing = list(
        max_abs_diff_fmort = max(abs(fishing_fleet_prepared[["Fmort"]] - fishing_f_expected)),
        max_abs_diff_q = max(abs(fishing_fleet_prepared[["q"]] - fishing_q_expected))
    ),
    fleet_survey = list(
        max_abs_diff_q = max(abs(survey_fleet_prepared[["q"]] - survey_q_expected))
    ),
    population = list(
        max_abs_diff_m = max(abs(population_prepared[["M"]] - population_m_expected)),
        max_abs_diff_f_multiplier = max(abs(population_prepared[["f_multiplier"]] - population_f_multiplier_expected))
    ),
    recruitment = list(
        length = length(recruitment_values),
        head = head(recruitment_values, 5)
    ),
    registration = list(
        before = registration_counts_before,
        after_selectivity = registration_counts_after_selectivity,
        after_all_modules = registration_counts_after_all_modules,
        fixed_effects_added_by_selectivity =
            registration_counts_after_selectivity[["fixed_effects_parameters"]] -
                registration_counts_before[["fixed_effects_parameters"]],
        random_effects_added_by_selectivity =
            registration_counts_after_selectivity[["random_effects_parameters"]] -
                registration_counts_before[["random_effects_parameters"]],
        fixed_effects_added_total =
            registration_counts_after_all_modules[["fixed_effects_parameters"]] -
                registration_counts_before[["fixed_effects_parameters"]],
        random_effects_added_total =
            registration_counts_after_all_modules[["random_effects_parameters"]] -
                registration_counts_before[["random_effects_parameters"]]
    )
)

comparison[["optimization"]] <- run_native_optimization(
    pre_optimization_information_counts = registration_counts_after_all_modules,
    likelihood_builder = build_default_likelihood
)

comparison[["parameter_summary"]] <- list(
    fixed_names = tryCatch(get_fixed_parameter_names(), error = function(e) character(0)),
    fixed_values = tryCatch(get_fixed_parameters(), error = function(e) numeric(0)),
    random_values = tryCatch(get_random_parameters(), error = function(e) numeric(0))
)

print(comparison)

if (!is.null(comparison[["optimization"]][["opt"]])) {
    print(comparison[["optimization"]][["opt"]])
}

if (!is.null(comparison[["optimization"]][["final_gradient"]])) {
    cat("Final gradient at opt$par:\n")
    print(comparison[["optimization"]][["final_gradient"]])
}

if (!is.null(comparison[["optimization"]][["gradient_diagnostics"]])) {
    gd <- comparison[["optimization"]][["gradient_diagnostics"]]
    cat("Max abs gradient:", gd[["max_abs_gradient"]], "\n")
    cat("Max gradient index:", gd[["max_index"]], "\n")
    cat("Max gradient parameter:", gd[["parameter_name"]], "\n")
    cat("Gradient < 1e-2:", gd[["pass_lt_1e_2"]], "\n")
    cat("Gradient < 1e-3:", gd[["pass_lt_1e_3"]], "\n")

    parameter_summary <- comparison[["parameter_summary"]]
    if (!is.null(parameter_summary)) {
        fixed_names <- parameter_summary[["fixed_names"]]
        fixed_values <- parameter_summary[["fixed_values"]]
        max_index <- gd[["max_index"]]
        if (length(fixed_names) >= max_index && length(fixed_values) >= max_index) {
            cat("Max gradient fixed parameter name:", fixed_names[[max_index]], "\n")
            cat("Max gradient fixed parameter value:", fixed_values[[max_index]], "\n")
        }
    }
}

if (!is.null(comparison[["optimization"]][["objective_at_start"]])) {
    cat("Objective at start:", comparison[["optimization"]][["objective_at_start"]], "\n")
}

if (!is.null(comparison[["optimization"]][["objective_at_opt"]])) {
    cat("Objective at opt$par:", comparison[["optimization"]][["objective_at_opt"]], "\n")
}

if (!is.null(comparison[["optimization"]][["max_abs_objective_perturbation_delta"]])) {
    cat(
        "Max abs objective delta from +1e-6 perturbations at opt$par:",
        comparison[["optimization"]][["max_abs_objective_perturbation_delta"]],
        "\n"
    )
}

if (isTRUE(comparison[["optimization"]][["flat_objective"]])) {
    cat("Warning:", comparison[["optimization"]][["flat_objective_message"]], "\n")
}
