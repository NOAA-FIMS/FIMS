# Model inspector examples using FIMS example data.
# Run from the repository root with FIMS from this branch already loaded.

source(file.path(
  "visualization-prototypes",
  "R",
  "user-model-inspector",
  "user-model_inspector_prototype.r"
))

output_dir <- file.path(
  "visualization-prototypes",
  "examples",
  "user-model-inspector"
)

# -------------------------------------------------------------------------
# Example 1: default model
# -------------------------------------------------------------------------

# Clear objects left in C++ memory by earlier FIMS runs.
FIMS::clear()

# Load FIMS example data and prepare the FIMSFrame.
data("data_big")
fims_frame <- FIMS::FIMSFrame(data_big)

# Create the default parameter table from the data.
parameters <- FIMS::setup_default_parameters(data = fims_frame)

# Inspect the configured model before fitting.
inspect_fims_model(
  data = fims_frame,
  parameters = parameters,
  model_label = "Basic demo model",
  output = file.path(output_dir, "01_basic_demo_model.html"),
  open = FALSE
)

# -------------------------------------------------------------------------
# Example 2: double-logistic selectivity
# -------------------------------------------------------------------------

# Load FIMS example data and prepare the FIMSFrame.
data("data_big")
fims_frame <- FIMS::FIMSFrame(data_big)

# Replace the default selectivity rows for the fishery and survey.
parameters <- FIMS::setup_default_parameters(data = fims_frame) |>
  dplyr::filter(module_name != "Selectivity") |>
  dplyr::bind_rows(
    FIMS::setup_default_Selectivity(
      data = fims_frame,
      fleet = "fleet1",
      module_type = "DoubleLogistic"
    ),
    FIMS::setup_default_Selectivity(
      data = fims_frame,
      fleet = "survey1",
      module_type = "DoubleLogistic"
    )
  )

# Inspect the configured model before fitting.
inspect_fims_model(
  data = fims_frame,
  parameters = parameters,
  model_label = "Double logistic selectivity",
  output = file.path(output_dir, "02_double_logistic_selectivity.html"),
  open = FALSE
)

# -------------------------------------------------------------------------
# Example 3: VonB Growth with fleet-specific selectivity
# -------------------------------------------------------------------------

# Load FIMS example data and prepare the FIMSFrame.
data("data_big")
fims_frame <- FIMS::FIMSFrame(data_big)

# Replace the Growth and Selectivity rows in the default parameter table.
# The three core VonB curve parameters are estimable by default.
# Use logistic selectivity for the fishery and double logistic for the survey.

parameters <- FIMS::setup_default_parameters(data = fims_frame) |>
  dplyr::filter(!module_name %in% c("Growth", "Selectivity")) |>
  dplyr::bind_rows(
    FIMS::setup_default_Growth(
      data = fims_frame,
      module_type = "VonBertalanffySchnute"
    ),
    FIMS::setup_default_Selectivity(
      data = fims_frame,
      fleet = "fleet1",
      module_type = "Logistic"
    ),
    FIMS::setup_default_Selectivity(
      data = fims_frame,
      fleet = "survey1",
      module_type = "DoubleLogistic"
    )
  )

# Inspect the configured model before fitting.
inspect_fims_model(
  data = fims_frame,
  parameters = parameters,
  model_label = "Multi-fleet VonBertalanffy-Schnute growth",
  output = file.path(output_dir, "03_vonb_growth.html"),
  open = FALSE
)

#View your growth parameters

parameters |>
  dplyr::filter(module_name == "Growth") |>
  dplyr::select(label, age, value, estimation_type)

# Replace the default Growth parameter values.
parameters <- parameters |>
  dplyr::rows_update(
    tibble::tibble(
      module_name = "Growth",
      label = c(
        "mean_length_young",
        "mean_length_old",
        "growth_coefficient",
        "reference_age_for_length_young",
        "reference_age_for_length_old"
      ),
      value = c(150, 800, 0.22, 1, 12)
    ),
    by = c("module_name", "label")
  )

# Inspect the configured model before fitting.
inspect_fims_model(
  data = fims_frame,
  parameters = parameters,
  model_label = "Multi-fleet VonBertalanffy-Schnute growth",
  output = file.path(output_dir, "03_vonb_growth.html"),
  open = FALSE
)

