# R code to read and write data to Google Cloud Storage Bucket
# - This R script uses a "cloud mount", meaning the Google Bucket behaves like a local
#   folder on your workstation located at "~/my_gcs_bucket".
# - Current code links to a data bucket set up by FIMS located at
#   https://console.cloud.google.com/storage/browser/nsaw-2026-fims-training.
# - Permissions to view, read, and write to this data bucket must be amended by FIMS team.

# Define the root path for the shared Google Cloud Storage bucket
gcs_bucket_root <- "~/my_gcs_bucket"

# List all files and directories in the top level of the bucket
list.files(gcs_bucket_root)

# List contents of the shared training materials folder
training_folder <- file.path(gcs_bucket_root, "training_materials")
list.files(training_folder)

# Create a personal folder and change 'firstname_lastname' to your name
user_folder_name <- "firstname_lastname"
user_folder <- file.path(gcs_bucket_root, user_folder_name)
# Create the directory if it does not already exist
if (!dir.exists(user_folder)) {
  dir.create(user_folder)
}

# Pull the latest FIMS demo and save it directly to the cloud folder
demo_url <- "https://raw.githubusercontent.com/NOAA-FIMS/FIMS/refs/heads/main/vignettes/fims-demo.Rmd"
local_script_path <- file.path(user_folder, "fims_demo.R")

knitr::purl(
  input = demo_url, 
  output = local_script_path,
  documentation = 0
)

## Execute the downloaded demo script
source(local_script_path)

# Define the file path for saving the model fit
fit_output_path <- file.path(user_folder, "fit.RDS")
saveRDS(
  object = fit,
  file = fit_output_path ,
  compress = FALSE
)

# Verify the save by removing the local object and reading it back from the bucket
rm(fit)
fit <- readRDS(file.path(folder_path, "fit.RDS"))
