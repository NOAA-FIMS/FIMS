fims_input_types <- c(
  "age_comp",
  "age_to_length_conversion",
  "index",
  "catch",
  "length_bin",
  "length_comp",
  "maturity_at_age",
  "weight_at_age",
  "spawning_weight_at_age",
  "fecundity_at_age",
  "fecundity_at_length"
)

usethis::use_data(fims_input_types, overwrite = TRUE)
save(fims_input_types, file = file.path("data-raw", "fims_input_types.rda"))
on.exit(rm(fims_input_types), add = TRUE)
