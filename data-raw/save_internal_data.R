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

# The saved fit changes on every run because it stores the Rcpp model object,
# but the unoptimized estimates are the same on every run. Saving
# R/sysdata.rda only when the version, estimates or input types change keeps
# pull requests from carrying an .rda file that is not their own.
saved <- new.env()
if (file.exists(file.path("R", "sysdata.rda"))) {
  load(file.path("R", "sysdata.rda"), envir = saved)
}
internal_data_changed <- is.null(saved[["fit_big_unoptimized"]]) ||
  !identical(
    FIMS::get_version(fit_big_unoptimized),
    FIMS::get_version(saved[["fit_big_unoptimized"]])
  ) ||
  !identical(
    estimates_big_unoptimized,
    saved[["estimates_big_unoptimized"]]
  ) ||
  !identical(fims_input_types, saved[["fims_input_types"]])
if (internal_data_changed) {
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
