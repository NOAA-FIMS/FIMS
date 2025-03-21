# Load necessary data for the integration test
load(test_path("fixtures", "integration_test_data.RData"))

# Set the iteration ID to 1 for accessing specific input/output list
iter_id <- 1

# Extract model input and output data for the specified iteration
om_input <- om_input_list[[iter_id]]
om_output <- om_output_list[[iter_id]]
em_input <- em_input_list[[iter_id]]

# Define modified parameters for different modules
modified_parameters <- vector(mode = "list", length = length(iter_id))
modified_parameters[[iter_id]] <- list(
  fleet1 = list(
    Fleet.log_Fmort.value = log(om_output_list[[iter_id]][["f"]])
  ),
  survey1 = list(
    LogisticSelectivity.inflection_point.value = 1.5,
    LogisticSelectivity.slope.value = 2,
    Fleet.log_q.value = log(om_output_list[[iter_id]][["survey_q"]][["survey1"]])
  ),
  recruitment = list(
    BevertonHoltRecruitment.log_rzero.value = log(om_input_list[[iter_id]][["R0"]]),
    BevertonHoltRecruitment.log_devs.value = om_input_list[[iter_id]][["logR.resid"]][-1],
    # TODO: integration tests fail after setting BevertonHoltRecruitment.log_devs.estimated
    # to TRUE. We need to debug the issue, then update the line below accordingly.
    BevertonHoltRecruitment.log_devs.estimated = FALSE,
    DnormDistribution.log_sd.value = om_input_list[[iter_id]][["logR_sd"]]
  ),
  maturity = list(
    LogisticMaturity.inflection_point.value = om_input_list[[iter_id]][["A50.mat"]],
    LogisticMaturity.inflection_point.estimated = FALSE,
    LogisticMaturity.slope.value = om_input_list[[iter_id]][["slope.mat"]],
    LogisticMaturity.slope.estimated = FALSE
  ),
  population = list(
    Population.log_init_naa.value = log(om_output_list[[iter_id]][["N.age"]][1, ])
  )
)
saveRDS(
  modified_parameters,
  file = testthat::test_path("fixtures", "parameters_model_comparison_project.RDS")
)

estimation_mode = TRUE

map = list()

# Clear any previous FIMS settings
  clear()

  data <- FIMS::FIMSFrame(data1)

  # Set up default parameters
  fleets <- list(
    fleet1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    ),
    survey1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    )
  )

  default_parameters <- data |>
    create_default_parameters(
      fleets = fleets,
      recruitment = list(
        form = "BevertonHoltRecruitment",
        process_distribution = c(log_devs = "DnormDistribution")
      ),
      growth = list(form = "EWAAgrowth"),
      maturity = list(form = "LogisticMaturity")
    )

  parameters <- default_parameters |>
    update_parameters(
      modified_parameters = modified_parameters[[iter_id]]
    )

  parameter_list <- initialize_fims(
    parameters = parameters,
    data = data
  )

input = parameter_list

optimize = estimation_mode

get_sd = TRUE
                     save_sd = TRUE
                     number_of_loops = 3
                     optimize = TRUE
                     number_of_newton_steps = 0
                     control = list(
                       eval.max = 10000,
                       iter.max = 10000,
                       trace = 0
                     )
                     filename = NULL

if (!is.null(input$random)) {
    cli::cli_abort("Random effects declared but are not implemented yet.")
  }
  if (number_of_newton_steps > 0) {
    cli::cli_abort("Newton steps not implemented yet.")
  }
  if (number_of_loops < 0) {
    cli::cli_abort("number_of_loops ({.par {number_of_loops}}) must be >= 0.")
  }
  obj <- TMB::MakeADFun(
    data = list(),
    parameters = input$parameters,
    map = input$map,
    random = input$random,
    DLL = "FIMS",
    silent = TRUE
  )
  if (!optimize) {
    initial_fit <- FIMSFit(
      input = input,
      obj = obj,
      timing = c("time_total" = as.difftime(0, units = "secs"))
    )
    return(initial_fit)
  }

  if (!is_fims_verbose()) {
    control$trace <- 0
  }
  ## optimize and compare
  cli::cli_inform(c("v" = "Starting optimization ..."))
  t0 <- Sys.time()
  opt <- with(
    obj,
    nlminb(
      start = par,
      objective = fn,
      gradient = gr,
      control = control
    )
  )
  maxgrad0 <- maxgrad <- max(abs(obj$gr(opt$par)))
  if (number_of_loops > 0) {
    cli::cli_inform(c(
      "i" = "Restarting optimizer {number_of_loops} times to improve gradient."
    ))
    for (ii in 1:number_of_loops) {
      # control$trace is reset to zero regardless of verbosity because the
      # differences in values printed out using control$trace will be
      # negligible between these different runs and is not worth printing
      control$trace <- 0
      opt <- with(
        obj,
        nlminb(
          start = opt[["par"]],
          objective = fn,
          gradient = gr,
          control = control
        )
      )
      maxgrad <- max(abs(obj[["gr"]](opt[["par"]])))
    }
    div_digit <- cli::cli_div(theme = list(.val = list(digits = 5)))
    cli::cli_inform(c(
      "i" = "Maximum gradient went from {.val {maxgrad0}} to
            {.val {maxgrad}} after {number_of_loops} steps."
    ))
    cli::cli_end(div_digit)
  }
  time_optimization <- Sys.time() - t0
  cli::cli_inform(c("v" = "Finished optimization"))

  time_sdreport <- NA
  if (get_sd) {
    t2 <- Sys.time()
    sdreport <- TMB::sdreport(obj)
    cli::cli_inform(c("v" = "Finished sdreport"))
    time_sdreport <- Sys.time() - t2
  } else {
    sdreport <- list()
    time_sdreport <- as.difftime(0, units = "secs")
  }

  timing <- c(
    time_optimization = time_optimization,
    time_sdreport = time_sdreport,
    time_total = Sys.time() - t0
  )

  version = utils::packageVersion("FIMS")

estimates_outline <- dplyr::tibble(
    # The FIMS Rcpp module
    module = character(),
    # The unique ID of the module
    id = integer(),
    # The name of the parameter or derived quantity
    label = character(),
    # The index number corresponding to the label
    index = integer(),
    # The fleet name associated with the parameter or derived quantity
    fleet = character(),
    # The age associated with the parameter or derived quantity
    age = numeric(),
    # The length associated with the parameter or derived quantity
    length = numeric(),
    # The modeled time perioed that the value pertains to
    time = integer(),
    # The initial value use to start the optimization procedure
    initial = numeric(),
    # The estaimted parameter value, which would be the MLE estimate or the value
    # used for a given MCMC iteration
    estimate = numeric(),
    # Estimated uncertainty, reported as a standard deviation
    uncertainty = numeric(),
    # The pointwise log-likelihood used for the estimation model
    log_lik = numeric(),
    # The pointwise log-likelihood used for the test or holdout data
    log_lik_cv = numeric(),
    # The gradient component for that parameter, NA for derived quantities
    gradient = numeric(),
    # A TRUE/FALSE indicator of whether the parameter was estimated (and not fixed),
    # with NA for derived quantities
    estimated = logical()
  )

  # Determine the number of parameters
  n_total <- length(obj[["env"]][["last.par.best"]])
  n_fixed_effects <- length(obj[["par"]])
  number_of_parameters <- c(
    total = n_total,
    fixed_effects = n_fixed_effects,
    random_effects = n_total - n_fixed_effects
  )
  rm(n_total, n_fixed_effects)

  # Calculate the maximum gradient
  max_gradient <- if (length(opt) > 0) {
    max(abs(obj[["gr"]](opt[["par"]])))
  } else {
    NA_real_
  }

  # Rename parameters instead of "p"
  parameter_names <- names(get_parameter_names(obj[["par"]]))
  names(obj[["par"]]) <- parameter_names

  # Get the report
  report <- if (length(opt) == 0) {
    obj[["report"]](obj[["env"]][["last.par.best"]])
  } else {
    obj[["report"]]()
  }
  
  # Unlist module ids and use the ids to map selectivity fleet name later 
  unlist_module_ids <- unlist(input$module_ids)
  # Total number of fleets
  # 3 represents recruitment, growth, and maturity modules
  total_fleet_num <- length(input[["module_ids"]]) - 3
  # Define common variables for model years, one projection year, and all years
  model_years <- FIMS::get_start_year(input[["data"]]):FIMS::get_end_year(input[["data"]])
  projection_year <- tail(model_years, 1) + 1
  all_years <- c(model_years, projection_year)

 names(sdreport[["par.fixed"]]) <- parameter_names
    dimnames(sdreport[["cov.fixed"]]) <- list(parameter_names, parameter_names)
    std <- summary(sdreport)

    # Number of rows for derived quantities: based on the difference
    # between the total number of rows in std and the length of parameter_names.
    derived_quantity_nrow <- nrow(std) - length(parameter_names)

estimates <- estimates_outline |>
      tibble::add_row(
        label = dimnames(std)[[1]],
        estimate = std[, "Estimate"],
        uncertainty = std[, "Std. Error"],
        # Use obj[["env"]][["parameters"]][["p"]] as this will return both initial
        # fixed and random effects while obj[["par"]] only returns initial fixed
        # effects
        initial = c(
          obj[["env"]][["parameters"]][["p"]], 
          rep(NA_real_, derived_quantity_nrow)
        ),
        gradient = c(
          obj[["gr"]](opt[["par"]]), 
          rep(NA_real_, derived_quantity_nrow)
        ),
        estimated = c(
          rep(TRUE, length(parameter_names)),
          rep(NA, derived_quantity_nrow)
        )
      ) 

fit_output<- finalize(opt$par, obj$fn, obj$gr)
# Convert the JSON-formatted string `fit_output` into an R list object (`json_list`) 
# for easier manipulation and extraction of data.
json_list <- jsonlite::fromJSON(fit_output)
# Identify the index of the "modules" element in `json_list` by matching its name.
# This is used to locate the relevant part of the JSON structure for further processing.
modules_id <- which(names(json_list) == "modules")

# Extract and process the "parameters" from each module in `json_list`
estimates <- purrr::map(seq_along(json_list[[modules_id]][["parameters"]]), ~{
  # If the current module's "parameters" is NULL, return NULL to skip processing.
  if (is.null(json_list[[modules_id]][["parameters"]][[.x]])) {
    NULL
  } else{
    # Convert the current module's "parameters" into a tibble for easier manipulation.
    tibble::as_tibble(json_list[[modules_id]][["parameters"]][[.x]]) |>
      # Rename the columns of the tibble.
      dplyr::rename_with(~c("parameter_name", "parameter_type_id", "parameter_type", "parameter")) |>
      # Expand the "parameter" column into multiple rows if it has more than one parameter.
      tidyr::unnest_longer(parameter) |>
      # Expand the nested structure in the "parameter" column into multiple columns,
      # using an underscore (`_`) as a separator for the new column names.
      tidyr::unnest_wider(
        parameter,
        names_sep = "_"
      )
  }
}) 

# Combine the processed estimates with additional information from `json_list`
combined_estimates <- purrr::pmap(
  list(estimates, json_list[[modules_id]][["name"]], json_list[[modules_id]][["id"]], json_list[[modules_id]][["type"]]),
  ~ {
    # Skip processing if the current estimate is NULL.
    if (is.null(..1)) {
      NULL
    } else {
      ..1 |> 
        dplyr::mutate(
          # Add the module name from `json_list` as a new column of the current estimates.
          module_name = ..2,  
          # Add the module ID from `json_list` as a new column.
          module_id = ..3,  
          # Add the module type from `json_list` as a new column.         
          module_type = ..4
        )
    }
  }
) |>
  # Combine all the processed tibbles into a single tibble by stacking rows.
  dplyr::bind_rows() |>
  # Reorder the columns to place `module_name`, `module_id`, and `module_type` at the beginning.
  dplyr::relocate(module_name, module_id, module_type, .before = everything())

combined_estimates |>
  dplyr::filter(parameter_estimated == 1) 

# Extract and process the "derived_quantities" from each module in `json_list`
which(sapply(json_list[[modules_id]][["derived_quantities"]], is.null))
which(sapply(json_list[[modules_id]][["density_component"]][[2]], is.null))
cbind(
  1:21, 
  json_list[[modules_id]][["name"]], 
  json_list[[modules_id]][["type"]]
)


derived_quantity <- purrr::map(seq_along(json_list[[modules_id]][["derived_quantities"]]), ~{
  # If the current module's "derived_quantities" is NULL, return NULL to skip processing.
  if (is.null(json_list[[modules_id]][["derived_quantities"]][[.x]])) {
    NULL
  } else {
    # Convert the current module's "derived_quantities" into a tibble for easier manipulation.
    tibble::as_tibble(json_list[[modules_id]][["derived_quantities"]][[.x]]) |>
      # Expand the "values" column into multiple rows.
      tidyr::unnest_longer(values) 
  }
}) 

combined_derived_quantity <- purrr::pmap(
  list(derived_quantity, json_list[[modules_id]][["name"]], json_list[[modules_id]][["id"]], json_list[[modules_id]][["type"]]),
  ~ {
    # Skip processing if the current derived_quantity is NULL.
    if (is.null(..1)) {
      NULL
    } else {
      ..1 |> 
        dplyr::mutate(
          # Add the module name from `json_list` as a new column of the current estimates.
          module_name = ..2,  
          # Add the module ID from `json_list` as a new column.
          module_id = ..3,  
          # Add the module type from `json_list` as a new column.         
          module_type = ..4
        )
    }
  }
) |>
  # Combine all the processed tibbles into a single tibble by stacking rows.
  dplyr::bind_rows() |>
  # Reorder the columns to place `module_name`, `module_id`, and `module_type` at the beginning.
  dplyr::relocate(module_name, module_id, module_type, .before = everything())

density_component <- purrr::map(seq_along(json_list[[modules_id]][["density_component"]][[2]]), ~{
  # If the current module's "derived_quantities" is NULL, return NULL to skip processing.
  if (is.null(json_list[[modules_id]][["derived_quantities"]][[.x]])) {
    NULL
  } else {
    # Convert the current module's "derived_quantities" into a tibble for easier manipulation.
    tibble::as_tibble(json_list[[modules_id]][["derived_quantities"]][[.x]]) |>
      # Expand the "values" column into multiple rows.
      tidyr::unnest_longer(values) 
  }
}) 