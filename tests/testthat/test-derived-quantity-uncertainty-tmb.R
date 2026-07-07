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
  model_dll_hessian <- FIMS:::calculate_tmb_model_dll_fixed_hessian(
    obj,
    theta_hat
  )
  backend_hessian <- calculate_fixed_effect_hessian(
    theta_hat,
    obj$gr,
    .Machine$double.eps^(1 / 3)
  )

  expect_equal(unname(estimate), c(-3, -1), tolerance = 1e-12)
  expect_equal(model_dll_hessian, hessian, tolerance = 1e-12)
  expect_equal(backend_hessian, hessian, tolerance = 1e-8)
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
  backend_fixed_covariance <- calculate_fixed_effect_covariance(hessian)
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
  expect_equal(backend_fixed_covariance, sdr$cov.fixed)
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
  expect_equal(payload$fixed_covariance, sdr$cov.fixed)
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

test_that("fixed Hessian helpers fall back safely", {
  par_fixed <- c(a = 1, b = -2)
  hessian <- matrix(c(2, 0.5, 0.5, 3), nrow = 2, byrow = TRUE)
  obj <- list(
    he = function(x) stop("native Hessian unavailable"),
    gr = function(x) as.vector(hessian %*% x),
    fn = function(x) 0.5 * sum(x * as.vector(hessian %*% x))
  )

  expect_null(
    FIMS:::calculate_tmb_model_dll_fixed_hessian(
      list(he = function(x) matrix(1, nrow = 1, ncol = 1)),
      par_fixed
    )
  )
  expect_equal(
    FIMS:::calculate_tmb_fixed_hessian(
      obj = obj,
      par_fixed = par_fixed,
      has_random = FALSE
    ),
    hessian,
    tolerance = 1e-8
  )
  expect_equal(
    unname(FIMS:::calculate_tmb_fixed_hessian(
      obj = list(fn = function(x) 0.5 * sum(x^2), gr = function(x) x),
      par_fixed = par_fixed,
      has_random = TRUE
    )),
    diag(2),
    tolerance = 1e-8
  )
  expect_equal(
    unname(FIMS:::calculate_tmb_fixed_hessian(
      obj = list(
        env = list(ff = function(x, order = 1) stop("Laplace gradient failed")),
        fn = function(x) 0.5 * sum(x^2),
        gr = function(x) x
      ),
      par_fixed = par_fixed,
      has_random = TRUE
    )),
    diag(2),
    tolerance = 1e-8
  )
})

test_that("fixed covariance falls back when backend inversion fails", {
  par_fixed <- c(a = 1, b = 2)
  sdreport_covariance <- matrix(c(11, 2, 2, 7), nrow = 2, byrow = TRUE)
  obj <- list(
    env = list(random = NULL, last.par.best = par_fixed),
    he = function(x) matrix(c(1, 0, 0, 0), nrow = 2, byrow = TRUE)
  )

  expect_equal(
    FIMS:::calculate_tmb_fixed_covariance(
      obj = obj,
      sdreport = list(cov.fixed = sdreport_covariance)
    ),
    sdreport_covariance
  )
})

test_that("ADREPORT uncertainty is added to model JSON reports", {
  sdreport <- list()
  backend_report <- cbind(
    "Estimate" = c(1, 2, 5),
    "Std. Error" = c(0.1, 0.2, 0.5)
  )
  rownames(backend_report) <- c("report_a", "report_a", "report_b")
  attr(sdreport, "fims_backend_report") <- backend_report

  model_output <- FIMS:::add_adreport_uncertainty_to_model_output(
    model_output = "{\"name\":\"CatchAtAge\"}",
    sdreport = sdreport
  )
  model_output_list <- jsonlite::fromJSON(
    model_output,
    simplifyVector = FALSE
  )

  expect_length(model_output_list$reports, 2)
  expect_equal(model_output_list$reports[[1]]$name, "report_a")
  expect_equal(unlist(model_output_list$reports[[1]]$value), c(1, 2))
  expect_equal(
    unlist(model_output_list$reports[[1]]$uncertainty),
    c(0.1, 0.2)
  )
  expect_equal(model_output_list$reports[[2]]$name, "report_b")
  expect_equal(unlist(model_output_list$reports[[2]]$value), 5)
})

test_that("random Hessian extraction falls back safely", {
  random_hessian <- matrix(c(2, 0.25, 0.25, 4), nrow = 2, byrow = TRUE)
  obj <- list(
    env = list(
      random = 2:3,
      last.par.best = c(beta = 1, u1 = 0.5, u2 = -0.25),
      f = function(...) stop("ADGrad Hessian unavailable"),
      spHess = function(par, random = FALSE) {
        if (random) {
          return(random_hessian)
        }
        stop("full Hessian not needed")
      }
    )
  )

  expect_equal(
    FIMS:::calculate_tmb_random_hessian(obj),
    random_hessian
  )
})

test_that("model-DLL fixed Hessian handles non-diagonal fixed-only models", {
  skip_on_cran()
  skip_if_not_installed("TMB")

  oldwd <- getwd()
  tmp <- withr::local_tempdir()
  setwd(tmp)
  withr::defer(setwd(oldwd))

  model_file <- file.path(tmp, "derived_quantity_cross_hessian.cpp")
  writeLines(
    c(
      "#include <TMB.hpp>",
      "template<class Type>",
      "Type objective_function<Type>::operator() () {",
      "  PARAMETER_VECTOR(theta);",
      "  Type d0 = theta(0) - Type(1);",
      "  Type d1 = theta(1) + Type(2);",
      "  Type d2 = theta(2) - Type(0.5);",
      "  Type nll = Type(0);",
      "  nll += Type(0.5) * Type(2.0) * d0 * d0;",
      "  nll += Type(0.5) * Type(1.5) * d1 * d1;",
      "  nll += Type(0.5) * Type(3.0) * d2 * d2;",
      "  nll += Type(0.3) * d0 * d1;",
      "  nll -= Type(0.2) * d0 * d2;",
      "  nll += Type(0.4) * d1 * d2;",
      "  vector<Type> derived(2);",
      "  derived(0) = theta(0) + theta(1) - theta(2);",
      "  derived(1) = theta(0) * theta(2) + Type(2) * theta(1);",
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
    parameters = list(theta = c(1, -2, 0.5)),
    DLL = "derived_quantity_cross_hessian",
    silent = TRUE
  )
  hessian <- matrix(
    c(2, 0.3, -0.2, 0.3, 1.5, 0.4, -0.2, 0.4, 3),
    nrow = 3,
    byrow = TRUE
  )
  sdr <- TMB::sdreport(
    obj,
    par.fixed = obj$par,
    hessian.fixed = hessian
  )
  payload <- FIMS:::extract_tmb_adreport_payload(obj = obj, sdreport = sdr)
  backend_summary <- FIMS:::calculate_tmb_adreport_uncertainty(
    obj = obj,
    sdreport = sdr
  )
  tmb_report <- summary(sdr, "report")

  expect_equal(
    FIMS:::calculate_tmb_model_dll_fixed_hessian(obj, obj$par),
    hessian,
    tolerance = 1e-12
  )
  expect_equal(
    payload$fixed_covariance,
    solve(hessian),
    tolerance = 1e-10
  )
  expect_equal(payload$method, "fixed")
  expect_equal(
    unname(backend_summary[, "Std. Error"]),
    unname(tmb_report[, "Std. Error"]),
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
      "  nll += Type(0.5) * pow((u(1) + Type(0.5) * beta) / Type(2), 2);",
      "  nll += Type(0.05) * (u(0) - beta) *",
      "    (u(1) + Type(0.5) * beta);",
      "  vector<Type> derived(2);",
      "  derived(0) = beta + u(0) + u(1);",
      "  derived(1) = beta * beta + u(0) - u(1);",
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
    parameters = list(beta = 0, u = c(0, 0)),
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
  backend_random_hessian <- FIMS:::calculate_tmb_random_hessian(
    obj = obj,
    par = obj$env$last.par.best,
    random = obj$env$random
  )
  backend_fixed_hessian <- FIMS:::calculate_tmb_fixed_hessian(
    obj = obj,
    par_fixed = obj$env$last.par.best[-obj$env$random],
    has_random = TRUE
  )
  backend_fixed_covariance <- FIMS:::calculate_tmb_fixed_covariance(
    obj = obj,
    sdreport = sdr
  )
  adreport_obj <- TMB::MakeADFun(
    data = list(),
    parameters = list(beta = 0, u = c(0, 0)),
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
  expect_equal(
    backend_random_hessian,
    as.matrix(obj$env$spHess(obj$env$last.par.best, random = TRUE)),
    tolerance = 1e-12
  )
  expect_equal(
    unname(backend_fixed_hessian),
    unname(stats::optimHess(
      obj$env$last.par.best[-obj$env$random],
      obj$fn,
      obj$gr
    )),
    tolerance = 1e-8
  )
  expect_equal(payload$random_hessian, backend_random_hessian)
  expect_equal(backend_fixed_covariance, sdr$cov.fixed)
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
