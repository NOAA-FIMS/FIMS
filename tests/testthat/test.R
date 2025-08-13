load(test_path("fixtures", "integration_test_data_components.RData"))

  # Generate dataset with only age composition data
  data_age_comp_raw <- rbind(
    landings_data,
    index_data,
    age_data,
    weightatage_data
  )
  data_age_comp <- FIMS::FIMSFrame(data_age_comp_raw)
  saveRDS(
    data_age_comp,
    file = testthat::test_path("fixtures", "data_age_comp.RDS")
  )

  # Generate dataset with only length composition data
  data_length_comp_raw <- rbind(landings_data, index_data, weightatage_data) |>
    dplyr::mutate(
      length = NA,
      .after = "age"
    ) |>
    rbind(length_comp_data, length_age_data)
  data_length_comp <- FIMS::FIMSFrame(data_length_comp_raw)
  saveRDS(
    data_length_comp,
    file = testthat::test_path("fixtures", "data_length_comp.RDS")
  )

  # Missing year for all data sets is year 0002, i.e., yyyy-mm-dd
  na_index <- as.Date("2-01-01")

  # Generate dataset with missing age composition for fleet1
  data_age_comp_na <- data_age_comp_raw |>
    dplyr::filter(!(name == "fleet1" & type == "age" & datestart == na_index)) |>
    FIMS::FIMSFrame()
  saveRDS(
    data_age_comp_na,
    file = testthat::test_path("fixtures", "data_age_comp_na.RDS")
  )

  # Generate dataset with missing length composition, age-to-length-conversion,
  # and index for survey1
  data_length_comp_na <- data_length_comp_raw |>
    dplyr::filter(
      !(name == "survey1" &
        type %in% c("index", "length", "age-to-length-conversion") &
        datestart == na_index
      )
    ) |>
    FIMS::FIMSFrame()
  saveRDS(
    data_length_comp_na,
    file = testthat::test_path("fixtures", "data_length_comp_na.RDS")
  )

  # Generate dataset with missing values in age composition for survey1
  # Missing values for length composition for fleet1 year 0012
  length_na_index <- as.Date("12-01-01")
  data_age_length_comp_raw <- rbind(
    landings_data,
    index_data,
    age_data,
    weightatage_data
  )
  data_age_length_comp_na <- data_age_length_comp_raw |>
    dplyr::filter(
      !(name == "survey1" & type %in% c("age") & datestart == na_index)
    ) |>
    dplyr::filter(
      !(name == "fleet1" &
        type %in% c("length", "age-to-length-conversion") &
        datestart == length_na_index
      )
    ) |>
    FIMS::FIMSFrame()
  saveRDS(
    data_age_length_comp_na,
    file = testthat::test_path("fixtures", "data_age_length_comp_na.RDS")
  )

  # Set up FIMS deterministic and estimation runs results ----
  # This section generates multiple fit results for use in tests located in
  # tests/testthat. For example, it creates fits from FIMS runs with only age
  # composition, only length composition data, or data with missing values.

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
      # to TRUE. We need to debug the issue, then update the line below accordingly. Currently it
      # is set up as fixed_effects for deterministic run and constant for estimation runs.
      BevertonHoltRecruitment.log_devs.estimation_type = "fixed_effects",
      DnormDistribution.log_sd.value = om_input_list[[iter_id]][["logR_sd"]]
    ),
    maturity = list(
      LogisticMaturity.inflection_point.value = om_input_list[[iter_id]][["A50.mat"]],
      LogisticMaturity.inflection_point.estimation_type = "constant",
      LogisticMaturity.slope.value = om_input_list[[iter_id]][["slope.mat"]],
      LogisticMaturity.slope.estimation_type = "constant"
    ),
    population = list(
      Population.log_init_naa.value = log(om_output_list[[iter_id]][["N.age"]][1, ])
    )
  )
  saveRDS(
    modified_parameters,
    file = testthat::test_path("fixtures", "parameters_model_comparison_project.RDS")
  )

modified_parameters[[iter_id]][["recruitment"]][["BevertonHoltRecruitment.log_devs.estimation_type"]] <- "constant"
  saveRDS(
    modified_parameters,
    file = testthat::test_path("fixtures", "parameters_model_comparison_project.RDS")
  )

estimation_mode = TRUE
random_effects = FALSE
map = list()
om_input <- om_input_list[[iter_id]]
  om_output <- om_output_list[[iter_id]]
  em_input <- em_input_list[[iter_id]]

  # Clear any previous FIMS settings
  clear()

  data <- FIMS::FIMSFrame(data1)

  # Set up default parameters
  fleets <- list(
    fleet1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Landings = "DlnormDistribution",
        Index = "DlnormDistribution",
        AgeComp = "DmultinomDistribution",
        LengthComp = "DmultinomDistribution"
      )
    ),
    survey1 = list(
      selectivity = list(form = "LogisticSelectivity"),
      data_distribution = c(
        Landings = "DlnormDistribution",
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