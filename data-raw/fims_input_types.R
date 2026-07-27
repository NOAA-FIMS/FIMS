fims_input_types <- c(
  "AgeComp",
  "age_to_length_conversion",
  "index",
  # TODO: change to singular landing
  "landings",
  "length_comp",
  "weight_at_age"
)

usethis::use_data(fims_input_types, overwrite = TRUE)
on.exit(rm(fims_input_types), add = TRUE)
