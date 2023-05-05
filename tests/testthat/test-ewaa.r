data(package = "FIMS")
dll_name <- paste("FIMS", .Platform$dynlib.ext, sep = "")
if (dir.exists(file.path(find.package("FIMS"), "src"))){
  dll_path <- file.path(find.package("FIMS"), "src", dll_name)
} else {
  libs_path <- file.path(find.package("FIMS"), "libs")

  if (.Platform$OS.type == "windows") {
    dll_path <- file.path(libs_path, .Platform$r_arch, dll_name)
  } else {
    dll_path <- file.path(libs_path, dll_name)
  }
}
test_that("ewaa data can be added to model", {
  dyn.load(dll_path)
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  ewaa_growth <- new(fims$EWAAgrowth)
  age_frame <- FIMSFrameAge(data_mile1)
  ewaa_growth$ages <- m_ages(age_frame)
  ewaa_growth$weights <- m_weightatage(age_frame)
  expect_equal(ewaa_growth$evaluate(1), 0.00053065552)

  ewaa_growth2 <- new(fims$EWAAgrowth)
  ewaa_growth2$ages <- c(ewaa_growth$ages, 12)
  ewaa_growth2$weights <- m_weightatage(age_frame)
  expect_error(
    ewaa_growth2$evaluate(1),
    regexp = "ages and weights must be the same length",
    ignore.case = FALSE
  )

  fims$clear()
})
