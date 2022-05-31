# test_that("use_module() works", {
#   original_wd <- getwd()
#   path <- find.package("FIMS")
#   setwd(path)
#   on.exit(setwd(original_wd), add = TRUE)

#   module_name <- "test_module_name"
#   module_type <- "test_module_type"

#   FIMS::use_module(
#     path = "inst",
#     module_name = module_name,
#     module_type = module_type
#   )

#   expect_true(file.exists(file.path(
#     "inst", "include",
#     "population_dynamics", module_type, paste0(module_type, ".hpp")
#   )))

#   expect_true(file.exists(file.path(
#     "inst", "include",
#     "population_dynamics", module_type, "functors",
#     paste0(module_name, ".hpp")
#   )))

#   expect_true(file.exists(file.path(
#     "inst", "include",
#     "population_dynamics", module_type, "functors",
#     paste0(module_type, "_base.hpp")
#   )))

#   unlink(file.path(
#     "inst", "include",
#     "population_dynamics", module_type
#   ), recursive = TRUE)
# })
