fims_input_types <- c(
  "age_comp",
  "age_to_length_conversion",
  "ageing_error",
  "index",
  "catch",
  "length_bin",
  "length_comp",
  "weight_at_age"
)

usethis::use_data(fims_input_types, overwrite = TRUE)
usethis::use_data(fims_input_types, overwrite = TRUE, internal = TRUE)
on.exit(rm(fims_input_types), add = TRUE)
