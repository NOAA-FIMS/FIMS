test_that("Quadra sdreport summaries match the TMB summary contract", {
  report <- structure(
    list(
      par.fixed = c(alpha = 1, beta = 2),
      par.random = c(3, 4),
      std.error = c(0.1, 0.2, 0.3, 0.4)
    ),
    class = c("quadra_sdreport", "list")
  )

  expect_equal(
    summary(report, "fixed"),
    cbind("Estimate" = c(alpha = 1, beta = 2), "Std. Error" = c(0.1, 0.2))
  )
  expect_equal(
    unname(summary(report, "random")),
    unname(cbind("Estimate" = c(3, 4), "Std. Error" = c(0.3, 0.4)))
  )
  expect_identical(rownames(summary(report, "random")), c("re", "re"))
  expect_identical(dim(summary(report, "report")), c(0L, 2L))
  expect_equal(nrow(summary(report)), 4L)
})
