# Rebuild internal package data in R/sysdata.rda. Source this script from the
# package root after running data-raw/data_big.R to generate the unoptimized fit
# and estimates. Optionally run data-raw/fims_input_types.R first to update the
# input types; otherwise the checked-in input types are retained.

# Prefer regenerated input types; otherwise use the checked-in package data.
if (file.exists(file.path("data-raw", "fims_input_types.rda"))) {
  load(file.path("data-raw", "fims_input_types.rda"))
} else {
  load(file.path("data", "fims_input_types.rda"))
}

# data_big.R saves these objects temporarily rather than as public datasets.
load(file.path("data-raw", "fit_estimates_big_unoptimized.rda"))

# Schedule removal of all temporary .rda files in data-raw when sourcing exits.
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

# The saved fit changes on every run because it stores the Rcpp model object,
# but the unoptimized estimates can differ by floating-point noise. Saving
# R/sysdata.rda only when the version, estimates or input types change keeps
# pull requests from carrying an .rda file that is not their own.
saved <- new.env(parent = emptyenv())
if (file.exists(file.path("R", "sysdata.rda"))) {
  load(file.path("R", "sysdata.rda"), envir = saved)
  saved_fit <- saved[["fit_big_unoptimized"]]
  saved_estimates <- saved[["estimates_big_unoptimized"]]
}
internal_data_changed <- is.null(saved_fit) ||
  !identical(
    FIMS::get_version(fit_big_unoptimized),
    FIMS::get_version(saved_fit)
  ) ||
  !isTRUE(all.equal(
    estimates_big_unoptimized,
    saved_estimates
  )) ||
  !identical(fims_input_types, saved[["fims_input_types"]])
if (internal_data_changed) {
  # Replace R/sysdata.rda with these objects for internal use by the package.
  usethis::use_data(
    fit_big_unoptimized,
    estimates_big_unoptimized,
    fims_input_types,
    internal = TRUE,
    overwrite = TRUE,
    compress = "xz"
  )
}
rm(saved, internal_data_changed)
