library(FIMS)
# Bring the package data into your environment
data("data1")
# Prepare the package data for being used in a FIMS model
data_4_model <- FIMS::FIMSFrame(data1)
# Create default configurations based on the data
default_configurations <- FIMS::create_default_configurations(data = data_4_model) 
# The output is a nested tibble, with details in the `data` column.
default_configurations_unnested <- default_configurations |>
  tidyr::unnest(cols = data)
# Update the module_type for survey1's selectivity
updated_configurations <- default_configurations_unnested |>
  dplyr::rows_update(
    y = tibble::tibble(
      module_name = c("Selectivity"),
      fleet_name = c("survey1"),
      module_type = c("DoubleLogistic")
    ),
    by = c("module_name", "fleet_name")
  )
# Nest updated_configurations
updated_configurations_nested <- updated_configurations |>
  tidyr::nest(.by = c(model_family, module_name, fleet_name))
# Create default parameters based on default_configurations and data
default_parameters <- FIMS::create_default_parameters(
  configurations = default_configurations,
  data = data_4_model
)
parameters_4_model <- default_parameters |>
  tidyr::unnest(cols = data) |>
  # Update log_Fmort initial values for Fleet1
  dplyr::rows_update(
    tibble::tibble(
      fleet_name = "fleet1",
      label = "log_Fmort",
      time = 1:30,
      value = log(c(
        0.009459165, 0.027288858, 0.045063639,
        0.061017825, 0.048600752, 0.087420554,
        0.088447204, 0.186607929, 0.109008958,
        0.132704335, 0.150615473, 0.161242955,
        0.116640187, 0.169346119, 0.180191913,
        0.161240483, 0.314573212, 0.257247574,
        0.254887252, 0.251462108, 0.349101406,
        0.254107720, 0.418478117, 0.345721184,
        0.343685540, 0.314171227, 0.308026829,
        0.431745298, 0.328030899, 0.499675368
      ))
    ),
    by = c("fleet_name", "label", "time")
  ) |>
  # Update selectivity parameters and log_q for survey1
  dplyr::rows_update(
    tibble::tibble(
      fleet_name = "survey1",
      label = c("inflection_point", "slope", "log_q"),
      value = c(1.5, 2, log(3.315143e-07))
    ),
    by = c("fleet_name", "label")
  ) |>
  # Update log_devs in the Recruitment module (time steps 2-30)
  dplyr::rows_update(
    tibble::tibble(
      label = "log_devs",
      time = 2:30,
      value = c(
        0.43787763, -0.13299042, -0.43251973, 0.64861200, 0.50640852,
        -0.06958319, 0.30246260, -0.08257384, 0.20740372, 0.15289604,
        -0.21709207, -0.13320626, 0.11225374, -0.10650836, 0.26877132,
        0.24094126, -0.54480751, -0.23680557, -0.58483386, 0.30122785,
        0.21930545, -0.22281699, -0.51358369, 0.15740234, -0.53988240,
        -0.19556523, 0.20094360, 0.37248740, -0.07163145
      )
    ),
    by = c("label", "time")
  ) |>
  # Update log_sd for log_devs in the Recruitment module
  dplyr::rows_update(
    tibble::tibble(
      module_name = "Recruitment",
      label = "log_sd",
      value = 0.4
    ),
    by = c("module_name", "label")
  ) |>
  # Update inflection point and slope parameters in the Maturity module
  dplyr::rows_update(
    tibble::tibble(
      module_name = "Maturity",
      label = c("inflection_point", "slope"),
      value = c(2.25, 3)
    ),
    by = c("module_name", "label")
  ) |>
  # Update log_init_naa values in the Population module
  dplyr::rows_update(
    tibble::tibble(
      label = "log_init_naa",
      age = 1:12,
      value = c(
        13.80944, 13.60690, 13.40217, 13.19525, 12.98692, 12.77791,
        12.56862, 12.35922, 12.14979, 11.94034, 11.73088, 13.18755
      )
    ),
    by = c("label", "age")
  )

# Run the  model with optimization
fit <- parameters_4_model |>
  FIMS::initialize_fims(data = data_4_model) |>
  FIMS::fit_fims(optimize = TRUE)

clear()
# Temporary manipulation to the returned estimates to get them
# to work with stockplotr
output <- get_estimates(fit) |>
  dplyr::mutate(
    uncertainty_label = "se",
    year = year_i,
    estimate = estimated
  )

# Plot of log fishing mortality
stockplotr::plot_timeseries(
  stockplotr::filter_data(
    output |> dplyr::filter(module_id == 1),
    label_name = "log_Fmort$",
    geom = "line"
  ),
  x = "year",
  y = "estimate",
  ylab = "natural log of Fishing Mortality"
) +
  stockplotr::theme_noaa()

ggplot2::ggsave(filename = "ts_plot.png")