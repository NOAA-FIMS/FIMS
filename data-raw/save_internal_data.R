load(file.path("data-raw", "fims_input_types.rda"))
load(file.path("data-raw", "fit_estimates_big_unoptimized.rda"))
on.exit(
  unlink(
    list.files(
      "data-raw",
      pattern = "\\.rda$",
      full.names = TRUE
    )
  ),
  add = TRUE
)

usethis::use_data(
  fit_big_unoptimized,
  estimates_big_unoptimized,
  fims_input_types,
  internal = TRUE,
  overwrite = TRUE,
  compress = "xz"
)
