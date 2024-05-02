test_that("use_module() works", {
  project_path <- find.package("FIMS")
  old_wd <- getwd()
  setwd(project_path)
  on.exit(setwd(old_wd), add = TRUE)


  if (!file.exists(file.path(project_path, "inst"))) {
    path <- file.path("include", "population_dynamics")
  } else {
    path <- file.path("inst", "include", "population_dynamics")
  }

  module_name <- "test_module_name"
  module_type <- "test_module_type"

  FIMS::use_module(
    path = path,
    module_name = module_name,
    module_type = module_type
  )

  expect_true(file.exists(file.path(
    project_path, path, module_type, paste0(module_type, ".hpp")
  )))

  expect_true(file.exists(file.path(
    project_path, path, module_type, "functors",
    paste0(module_name, ".hpp")
  )))

  expect_true(file.exists(file.path(
    project_path, path, module_type, "functors",
    paste0(module_type, "_base.hpp")
  )))

  unlink(file.path(
    project_path, path, module_type
  ), recursive = TRUE)

  # Expect use_module throws an error if module_name is not a string
  expect_error(
    FIMS::use_module(
      path = path,
      module_name = 1,
      module_type = "growth"
    )
  )

  # Expect use_module throws an error if module_type is not a string
  expect_error(
    FIMS::use_module(
      path = path,
      module_name = "ewaa",
      module_type = c(1, 2)
    )
  )
})
