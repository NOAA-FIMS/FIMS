#' use_module
#'
#' @description This function will generate the necessary files to
#' add a new module to the FIMS system.
#'
#' @param path The path to the FIMS project directory.
#' @param module_name the name of the module. This creates a subfolder in
#' \code{inst/include} of this name with folders \code{functors} and 
#' \code{module_name.hpp}.
#' @param sub_folder the folder of \code{inst/include} to put the files in
#' @param sub_module_name optional, the name of the submodule. This is used
#'  to create submodules of a type, for example, a logistic submodule for a 
#' selectivity module.
#' @param global_equation = TRUE if the module should export
#' a function from fims_math, FALSE otherwise.
#' @return
#' @export
#'
#' @examples
use_module <- function(path, module_name, sub_folder, sub_module_name = NA) {

  if (typeof(module_name) != "character") {
   usethis::ui_stop(paste("The module name, ", module_name, " is not of the
   correct type, please enter a string.", sep = ""))
  }

  if (typeof(sub_folder) != "character") {
   usethis::ui_stop(paste("The subfolder name, ", sub_folder, " is not of
   the correct type, please enter a string.", sep = ""))
  }

  old_wd <- getwd()
  subdir <- file.path(path, "inst", "include", "population_dynamics")
  setwd(subdir)

  #Create subfolder in inst/include if it does not exist
  if (!file.exists(sub_folder)) {
     try(dir.create(sub_folder))
  }

  #Create subfolder named module_name in inst/include if it does not exist
 if (!file.exists(file.path(sub_folder, module_name))) {
     try(dir.create(file.path(sub_folder, module_name)))
  }

   if (!file.exists(file.path(sub_folder, module_name,
   paste(module_name, ".hpp", sep = "")))) {
     try(usethis::use_template(save_as = file.path(subdir, sub_folder,
      module_name, paste(module_name, ".hpp", sep = "")),
     template = "module_template.hpp",
     package = "FIMS", data = list(module_name = module_name)))
  }

  #Create subfolder under module_name/functors if it does not exist
  if (!file.exists(file.path(sub_folder, module_name, "functors"))) {
     try(dir.create(file.path(sub_folder, module_name, "functors")))
  }

  # create module_name_base.hpp in
  # the folder inst/include/sub_folder/module_name/functors
  if (!file.exists(file.path(sub_folder, module_name, "functors",
    paste(module_name, "_base.hpp", sep = "")))) {
     try(usethis::use_template(save_as = file.path(subdir, sub_folder,
      module_name, "functors", paste(module_name, "_base.hpp", sep = "")),
      template = "module_base_template.hpp", package = "FIMS"))
  }

  if (!is.na(sub_module_name)){
    if(!file.exists(file.path(sub_folder, module_name, "functors",
      paste(sub_module_name, ".hpp", sep = "")))) {
      try(usethis::use_template(
          save_as = file.path(subdir, sub_folder, module_name, "functors",
        paste(sub_module_name, ".hpp", sep = "")),
        template = "module_functor_template.hpp", package = "FIMS"))
    }
  }

  # create module_name.hpp in inst/include/sub_folder/module_name/
  if (!file.exists(file.path(sub_folder, module_name,
    paste(module_name, ".hpp", sep = "")))) {
     try(usethis::use_template(save_as = file.path(subdir, sub_folder,
     module_name, paste(module_name, ".hpp", sep = "")),
      template = "module_template.hpp", package = "FIMS"))
  }

  setwd(old_wd)
  return(TRUE)
}
