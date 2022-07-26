test_that("create_fims_rcpp_interface() works with recruitment rcpp", {
  project_path <- find.package("FIMS")

  if (!file.exists(file.path(project_path, "inst"))) {
    path <- file.path(project_path, "extdata", "testthat_data", "create_rcpp_interface_object_data.txt")
  } else {
    path <- file.path(project_path, "inst", "extdata", "testthat_data", "create_rcpp_interface_object_data.txt")
  }

  expect_text <- readLines(path)

  object_text <- capture.output(FIMS::create_fims_rcpp_interface(
    interface_name = "BevertonHoltRecruitmentInterface",
    model= "SRBevertonHolt",
    base_class = "RecruitmentInterfaceBase",
    container = "recruitment_models",
    parameters = c("steep", "rzero", "phizero")
  ))

  expect_true(all.equal(expect_text, object_text))

})
