# Find path to FIMS.dll and TMB.dll, then load and unload dlls in test-fims-estimation.R

find_dll_path <- function(package_name) {
  dll_name <- paste(package_name, .Platform$dynlib.ext, sep = "")
  if (dir.exists(file.path(find.package(package_name), "src"))){
    dll_path <- file.path(find.package(package_name), "src", dll_name)
  } else {
    libs_path <- file.path(find.package(package_name), "libs")

    if (.Platform$OS.type == "windows") {
      dll_path <- file.path(libs_path, .Platform$r_arch, dll_name)
    } else {
      dll_path <- file.path(libs_path, dll_name)
    }
  }
}
