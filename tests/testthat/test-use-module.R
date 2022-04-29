test_that("use_model creates subfolders correctly", {
  use_module(sub_folder = "growth",  module_name = "ewaa")
  expect_gt(length(system.file("inst", "include", "growth",
  package = "FIMS")), 0)
  expect_gt(length(system.file("inst", "include","growth", "ewaa",
   package="FIMS")), 0)
})