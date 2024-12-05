data("data_mile1", package = "FIMS")

test_that("ewaa data can be added to model", {
  ewaa_growth <- new(EWAAgrowth)
  fims_frame <- FIMSFrame(data_mile1)
  ewaa_growth$ages <- m_ages(fims_frame)
  ewaa_growth$weights <- m_weight_at_age(fims_frame)
  expect_equal(ewaa_growth$evaluate(1), 0.00053065552)

  ewaa_growth2 <- new(EWAAgrowth)
  ewaa_growth2$ages <- c(ewaa_growth$ages, 12)
  ewaa_growth2$weights <- m_weight_at_age(fims_frame)
  expect_error(
    ewaa_growth2$evaluate(1),
    regexp = "ages and weights must be the same length",
    ignore.case = FALSE
  )

  clear()
})
