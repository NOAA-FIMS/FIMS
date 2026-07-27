fims_distributions <- tibble::tribble(
  ~family,     ~sample_function, ~central_parameter,  ~other_parameters,
  "dnorm",     "rnorm",          "mean",              list("sd"),
  "dlnorm",    "rlnorm",         "meanlog",           list("sdlog"),
  "dmultinom", "rmultinom",      "prob",              list("size")
)

usethis::use_data(fims_distributions, overwrite = TRUE)
on.exit(rm(fims_distributions), add = TRUE)
