# test_that("use_model creates subfolders correctly", {
#   library(usethis)
#   create_local_package()
#   desc_lines_before <- read_utf8(proj_path("DESCRIPTION"))
#   dir.create("inst/include/population_dynamics/")
#   expect_error_free(
#     use_module(path = "inst/include/population_dynamics",
#       module_name = "test_module", module_type = "test_type")
#   )
#   desc_lines_after <- read_utf8(proj_path("DESCRIPTION"))
#   expect_identical(desc_lines_before, desc_lines_after)
# })