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

  TMB::compile(model_file, framework = "TMBad")
  dynlib <- TMB::dynlib(tools::file_path_sans_ext(model_file))
  dyn.load(dynlib)
  test_env <- environment()
  withr::defer({
    rm(
      list = intersect(
        c("obj", "sdr", "adreport_obj", "payload"),
        ls(envir = test_env)
      ),
      envir = test_env
    )
    invisible(gc())
    dyn.unload(dynlib)
  })

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
  static_payload <- assemble_adreport_payload(list(
    estimate = estimate,
    jacobian = jacobian,
    fixed_covariance = sdr$cov.fixed
  ))
  native_payload <- assemble_adreport_payload_from_tmb_adfun(
    adreport_obj,
    FIMS:::get_adreport_parameters(adreport_obj, theta_hat),
    sdr$cov.fixed,
    list()
  )
  payload <- FIMS:::extract_tmb_adreport_payload(
    obj = obj,
    sdreport = sdr
  )
  payload_se <- calculate_adreport_payload_se(payload)

  expect_equal(unname(tmb_se), c(sqrt(40), 5), tolerance = 1e-8)
  expect_equal(payload$method, "fixed")
  expect_equal(native_payload$method, "fixed")
  expect_equal(native_payload$estimate, static_payload$estimate)
  expect_equal(native_payload$jacobian, static_payload$jacobian)
  expect_equal(
    native_payload$fixed_covariance,
    static_payload$fixed_covariance
  )
  expect_equal(native_payload$n_fixed_effects, static_payload$n_fixed_effects)
  expect_equal(unname(payload$estimate), unname(estimate), tolerance = 1e-12)
  expect_equal(
    unname(as.vector(t(payload$jacobian))),
    c(1, 2, 2, 1),
    tolerance = 1e-12
  )
  expect_equal(unname(backend_se), unname(tmb_se), tolerance = 1e-8)
  expect_equal(unname(payload_se), unname(tmb_se), tolerance = 1e-8)
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

  TMB::compile(model_file, framework = "TMBad")
  dynlib <- TMB::dynlib(tools::file_path_sans_ext(model_file))
  dyn.load(dynlib)
  test_env <- environment()
  withr::defer({
    rm(
      list = intersect(
        c("obj", "sdr", "adreport_obj", "payload"),
        ls(envir = test_env)
      ),
      envir = test_env
    )
    invisible(gc())
    dyn.unload(dynlib)
  })

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
  payload <- FIMS:::extract_tmb_adreport_payload(
    obj = obj,
    sdreport = sdr
  )
  adreport_obj <- TMB::MakeADFun(
    data = list(),
    parameters = list(beta = 0, u = 0),
    type = "ADFun",
    ADreport = TRUE,
    DLL = "derived_quantity_laplace",
    silent = TRUE
  )
  native_payload <- assemble_adreport_payload_from_tmb_adfun(
    adreport_obj,
    FIMS:::get_adreport_parameters(
      adreport_obj,
      obj$env$last.par.best
    ),
    sdr$cov.fixed,
    list(
      random_indices = as.integer(obj$env$random - 1L),
      random_hessian = payload$random_hessian,
      fixed_jacobian_adjustment =
        payload$adjusted_fixed_jacobian - payload$fixed_jacobian
    )
  )
  payload_se <- calculate_adreport_payload_se(payload)

  expect_true(opt$convergence == 0)
  expect_equal(payload$method, "laplace")
  expect_equal(native_payload$method, "laplace")
  expect_equal(unname(native_payload$estimate), unname(payload$estimate))
  expect_equal(native_payload$fixed_jacobian, payload$fixed_jacobian)
  expect_equal(native_payload$random_jacobian, payload$random_jacobian)
  expect_equal(
    native_payload$adjusted_fixed_jacobian,
    payload$adjusted_fixed_jacobian
  )
  expect_equal(ncol(payload$fixed_jacobian), length(sdr$par.fixed))
  expect_equal(ncol(payload$random_jacobian), length(obj$env$random))
  expect_equal(nrow(payload$random_hessian), length(obj$env$random))
  expect_equal(ncol(payload$random_covariance), length(obj$env$random))
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
  expect_equal(
    unname(payload_se),
    unname(tmb_report[, "Std. Error"]),
    tolerance = 1e-8
  )
})
