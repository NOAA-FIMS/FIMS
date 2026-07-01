test_that("backend derived quantity SEs match TMB sdreport", {
  skip_on_cran()
  skip_if_not_installed("TMB")

  oldwd <- getwd()
  tmp <- withr::local_tempdir()
  setwd(tmp)
  withr::defer(setwd(oldwd))

  model_file <- file.path(tmp, "derived_quantity_delta.cpp")
  writeLines(
    c(
      "#include <TMB.hpp>",
      "template<class Type>",
      "Type objective_function<Type>::operator() () {",
      "  PARAMETER_VECTOR(theta);",
      "  Type nll = Type(0);",
      "  nll += Type(0.5) * pow(theta(0) - Type(1), 2) / Type(4);",
      "  nll += Type(0.5) * pow(theta(1) + Type(2), 2) / Type(9);",
      "  vector<Type> derived(2);",
      "  derived(0) = theta(0) + Type(2) * theta(1);",
      "  derived(1) = theta(0) * theta(0) + theta(1);",
      "  ADREPORT(derived);",
      "  return nll;",
      "}"
    ),
    model_file
  )

  TMB::compile(model_file)
  dynlib <- TMB::dynlib(tools::file_path_sans_ext(model_file))
  dyn.load(dynlib)
  withr::defer(dyn.unload(dynlib))

  obj <- TMB::MakeADFun(
    data = list(),
    parameters = list(theta = c(1, -2)),
    DLL = "derived_quantity_delta",
    silent = TRUE
  )
  theta_hat <- obj$par
  covariance <- matrix(c(4, 0, 0, 9), nrow = 2, byrow = TRUE)
  hessian <- solve(covariance)
  sdr <- TMB::sdreport(
    obj,
    par.fixed = theta_hat,
    hessian.fixed = hessian
  )
  tmb_se <- summary(sdr, "report")[, "Std. Error"]

  adreport_obj <- TMB::MakeADFun(
    data = list(),
    parameters = list(theta = c(1, -2)),
    type = "ADFun",
    ADreport = TRUE,
    DLL = "derived_quantity_delta",
    silent = TRUE
  )
  estimate <- adreport_obj$fn(theta_hat)
  jacobian <- adreport_obj$gr(theta_hat)

  expect_equal(unname(estimate), c(-3, -1), tolerance = 1e-12)
  expect_equal(
    unname(as.vector(t(jacobian))),
    c(1, 2, 2, 1),
    tolerance = 1e-12
  )

  backend_se <- calculate_derived_quantity_se(
    estimate = estimate,
    jacobian = as.vector(t(jacobian)),
    covariance = as.vector(t(sdr$cov.fixed)),
    n_parameters = 2
  )
  backend_summary <- FIMS:::calculate_tmb_adreport_uncertainty(
    obj = obj,
    sdreport = sdr
  )

  expect_equal(unname(tmb_se), c(sqrt(40), 5), tolerance = 1e-8)
  expect_equal(unname(backend_se), unname(tmb_se), tolerance = 1e-8)
  expect_equal(
    unname(backend_summary[, "Estimate"]),
    unname(estimate),
    tolerance = 1e-12
  )
  expect_equal(
    unname(backend_summary[, "Std. Error"]),
    unname(tmb_se),
    tolerance = 1e-8
  )
})

test_that("backend random-effect derived quantity SEs match TMB sdreport", {
  skip_on_cran()
  skip_if_not_installed("TMB")

  oldwd <- getwd()
  tmp <- withr::local_tempdir()
  setwd(tmp)
  withr::defer(setwd(oldwd))

  model_file <- file.path(tmp, "derived_quantity_laplace.cpp")
  writeLines(
    c(
      "#include <TMB.hpp>",
      "template<class Type>",
      "Type objective_function<Type>::operator() () {",
      "  PARAMETER(beta);",
      "  PARAMETER_VECTOR(u);",
      "  Type nll = Type(0);",
      "  nll += Type(0.5) * pow((beta - Type(1)) / Type(2), 2);",
      "  nll += Type(0.5) * pow((u(0) - beta) / Type(3), 2);",
      "  vector<Type> derived(2);",
      "  derived(0) = beta + u(0);",
      "  derived(1) = beta * beta + u(0);",
      "  ADREPORT(derived);",
      "  return nll;",
      "}"
    ),
    model_file
  )

  TMB::compile(model_file)
  dynlib <- TMB::dynlib(tools::file_path_sans_ext(model_file))
  dyn.load(dynlib)
  withr::defer(dyn.unload(dynlib))

  obj <- TMB::MakeADFun(
    data = list(),
    parameters = list(beta = 0, u = 0),
    random = "u",
    DLL = "derived_quantity_laplace",
    silent = TRUE
  )
  opt <- stats::nlminb(obj$par, obj$fn, obj$gr)
  sdr <- TMB::sdreport(obj)
  tmb_report <- summary(sdr, "report")
  backend_summary <- FIMS:::calculate_tmb_adreport_uncertainty(
    obj = obj,
    sdreport = sdr
  )

  expect_true(opt$convergence == 0)
  expect_equal(
    unname(backend_summary[, "Estimate"]),
    unname(tmb_report[, "Estimate"]),
    tolerance = 1e-8
  )
  expect_equal(
    unname(backend_summary[, "Std. Error"]),
    unname(tmb_report[, "Std. Error"]),
    tolerance = 1e-8
  )
})
