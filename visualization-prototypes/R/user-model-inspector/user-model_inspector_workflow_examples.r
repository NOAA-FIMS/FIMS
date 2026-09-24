# Workflow examples for the FIMS user-model inspector helper.
#
# These are meant to be edited and rerun while testing model configurations.
# They use the helper functions from user-model_inspector_prototype.r rather
# than hand-built graph specifications.
#
# Run from the FIMS repository root after FIMS is available in your R session:
# Rscript visualization-prototypes/R/user-model-inspector/user-model_inspector_workflow_examples.r

script_args <- commandArgs(trailingOnly = FALSE)
script_file_arg <- script_args[grepl("^--file=", script_args)]
script_dir <- if (length(script_file_arg) > 0) {
  dirname(normalizePath(sub("^--file=", "", script_file_arg[[1]])))
} else {
  normalizePath(
    file.path("visualization-prototypes", "R", "user-model-inspector"),
    mustWork = FALSE
  )
}

repo_root <- normalizePath(file.path(script_dir, "..", "..", ".."))
old_wd <- getwd()
setwd(repo_root)
on.exit(setwd(old_wd), add = TRUE)

source(file.path(script_dir, "user-model_inspector_prototype.r"))

if (!requireNamespace("FIMS", quietly = TRUE)) {
  stop("Install or load FIMS before running these workflow examples.")
}

suppressPackageStartupMessages(library(FIMS))

data("data_big", package = "FIMS")

output_dir <- file.path(
  repo_root,
  "visualization-prototypes",
  "examples",
  "user-model-inspector"
)
dir.create(output_dir, showWarnings = FALSE, recursive = TRUE)
metadata_catalog <- write_inspector_metadata_catalog(
  output = file.path(
    repo_root,
    "visualization-prototypes",
    "data",
    "fims_inspector_metadata_catalog.json"
  )
)

default_data <- FIMSFrame(data_big)
default_parameters <- setup_default_parameters(default_data)

# -------------------------------------------------------------------------
# Example 1: basic demo model using default data and default parameters
# -------------------------------------------------------------------------

default_inspector <- inspect_fims_model(
  data = default_data,
  parameters = default_parameters,
  model_label = "Basic demo model",
  output = file.path(output_dir, "01_basic_demo_model.html"),
  open = FALSE
)

# -------------------------------------------------------------------------
# Example 2: same demo model, but change Selectivity from Logistic to
# DoubleLogistic
# -------------------------------------------------------------------------

double_logistic_parameters <- default_parameters[
  default_parameters$module_name != "Selectivity",
  ,
  drop = FALSE
]

double_logistic_selectivity <- do.call(
  rbind,
  lapply(get_fleets(default_data), function(fleet_i) {
    setup_default_Selectivity(
      data = default_data,
      fleet = fleet_i,
      module_type = "DoubleLogistic"
    )
  })
)

double_logistic_parameters <- rbind(
  double_logistic_parameters,
  double_logistic_selectivity
)

double_logistic_inspector <- inspect_fims_model(
  data = default_data,
  parameters = double_logistic_parameters,
  model_label = "Double logistic selectivity",
  output = file.path(output_dir, "02_double_logistic_selectivity.html"),
  open = FALSE
)

# -------------------------------------------------------------------------
# Example 3: multi-fleet demo with different fleet-specific selectivity
# choices and Growth changed from EWAA to VonBertalanffy-Schnute
# -------------------------------------------------------------------------

multi_fleet_vonb_parameters <- default_parameters[
  !default_parameters[["module_name"]] %in% c("Growth", "Selectivity"),
  ,
  drop = FALSE
]

multi_fleet_vonb_selectivity <- do.call(
  rbind,
  lapply(seq_along(get_fleets(default_data)), function(fleet_index) {
    fleet_i <- get_fleets(default_data)[[fleet_index]]
    module_type <- if (fleet_index == 1) {
      "Logistic"
    } else {
      "DoubleLogistic"
    }

    setup_default_Selectivity(
      data = default_data,
      fleet = fleet_i,
      module_type = module_type
    )
  })
)

multi_fleet_vonb_parameters <- rbind(
  multi_fleet_vonb_parameters,
  setup_default_Growth(
    data = default_data,
    module_type = "VonBertalanffySchnute"
  ),
  multi_fleet_vonb_selectivity
)

multi_fleet_vonb_inspector <- inspect_fims_model(
  data = default_data,
  parameters = multi_fleet_vonb_parameters,
  model_label = "Multi-fleet VonBertalanffy-Schnute growth",
  output = file.path(output_dir, "03_vonb_growth.html"),
  open = FALSE
)

workflow_summary <- rbind(
  data.frame(
    example = "Basic demo model",
    nodes = default_inspector$n_nodes,
    edges = default_inspector$n_edges,
    output = default_inspector$output,
    stringsAsFactors = FALSE
  ),
  data.frame(
    example = "Double logistic selectivity",
    nodes = double_logistic_inspector$n_nodes,
    edges = double_logistic_inspector$n_edges,
    output = double_logistic_inspector$output,
    stringsAsFactors = FALSE
  ),
  data.frame(
    example = "Multi-fleet VonBertalanffy-Schnute growth",
    nodes = multi_fleet_vonb_inspector$n_nodes,
    edges = multi_fleet_vonb_inspector$n_edges,
    output = multi_fleet_vonb_inspector$output,
    stringsAsFactors = FALSE
  )
)
