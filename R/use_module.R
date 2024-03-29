#' use_module
#'
#' @description This function will generate the necessary files to
#' add a new module to the FIMS system. For now, this only works for
#' modules under the \code{population_dynamics} folder of FIMS.
#'
#' @param path The path to the FIMS project directory \code{inst} folder.
#' @param module_name the name of the module. This creates a subfolder in
#' \code{inst/include} of this name with folders \code{functors} and
#' \code{module_name.hpp}.
#' @param module_type the folder of
#' \code{inst/include/population_dynamics} to put the files in
#' @return TRUE
#' @examples
#' \dontrun{
#' # create a new empirical weight-at-age module (EWAA) under growth
#' use_module(file.path("inst", "include", "population_dynamics"),
#'   module_name = "ewaa",
#'   module_type = "growth"
#' )
#' }
#' @export
#'
use_module <- function(path = file.path("inst", "include", "population_dynamics"), module_name, module_type) {
  if (typeof(module_name) != "character") {
    usethis::ui_stop(paste("The module name, ", module_name, " is not of
   the correct type, please enter a string.", sep = ""))
  }

  if (typeof(module_type) != "character") {
    usethis::ui_stop(paste("The subfolder name, ", module_type, " is not
   of the correct type, please enter a string.", sep = ""))
  }

  project_path <- find.package("FIMS")
  subdir <- path
  full_subdir <- file.path(project_path, subdir)

  # Create subfolder in inst/include/population_dynamics if it does not exist
  if (!file.exists(file.path(full_subdir, module_type))) {
    try(dir.create(file.path(full_subdir, module_type)))
  }

  # Create subfolder named module_name/functors in inst/include/population_dynamics
  # if it does not exist
  if (!file.exists(file.path(full_subdir, module_type, "functors"))) {
    try(dir.create(file.path(full_subdir, module_type, "functors")))
  }

  # create a module_name.hpp file in inst/include/population_dynamics/module_type/functors
  if (!file.exists(file.path(
    full_subdir, module_type, "functors",
    paste(module_type, ".hpp", sep = "")
  ))) {
    try(usethis::use_template(
      save_as = file.path(
        subdir, module_type,
        "functors", paste(module_name, ".hpp", sep = "")
      ),
      template = "module_functor_template.hpp",
      package = "FIMS", data = list(
        module_type = module_type,
        module_name = module_name
      ), open = TRUE
    ))
  }

  # create module_type_base.hpp in
  # the folder inst/include/population_dynamics/module_type/functors
  if (!file.exists(file.path(
    module_type, "functors",
    paste(module_type, "_base.hpp", sep = "")
  ))) {
    try(usethis::use_template(
      save_as = file.path(
        subdir, module_type,
        "functors", paste(module_type, "_base.hpp", sep = "")
      ),
      template = "module_base_template.hpp", package = "FIMS",
      data = list(module_type = module_type), open = TRUE
    ))
  }

  # create a module_type.hpp file in inst/include/population_dynamics/module_type if it doesn't already exist
  if (!is.na(module_type)) {
    if (!file.exists(file.path(
      subdir, module_type,
      paste(module_type, ".hpp", sep = "")
    ))) {
      try(usethis::use_template(
        save_as = file.path(
          subdir, module_type,
          paste(module_type, ".hpp", sep = "")
        ),
        template = "module_template.hpp", package = "FIMS",
        data = list(module_type = module_type, module_name = module_name),
        open = TRUE
      ))
    }
  }

  return(TRUE)
}
