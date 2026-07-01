#' Calculate fixed-effect ADREPORT uncertainty with the FIMS backend
#'
#' @description
#' Extracts fixed-effect ADREPORT estimates, Jacobians, and covariance from a
#' TMB object, then delegates the delta-method standard error calculation to
#' the FIMS C++ backend.
#'
#' @param obj An object returned from [TMB::MakeADFun()].
#' @param sdreport An object returned from [TMB::sdreport()].
#' @param par_fixed Optional fixed-effect parameter vector. When `NULL`, this
#'   function uses `sdreport[["par.fixed"]]`.
#'
#' @return A matrix with `Estimate` and `Std. Error` columns, or `NULL` when
#'   backend calculation is not available for the supplied object.
#' @noRd
calculate_tmb_adreport_uncertainty <- function(obj,
                                               sdreport,
                                               par_fixed = NULL) {
  if (length(sdreport) == 0 || is.null(sdreport[["cov.fixed"]])) {
    return(NULL)
  }

  random <- obj[["env"]][["random"]]
  has_random <- !is.null(random) && length(random) > 0
  if (is.null(par_fixed)) {
    par <- obj[["env"]][["last.par.best"]]
    if (has_random) {
      par_fixed <- par[-random]
    } else {
      par_fixed <- par
    }
  } else {
    par <- obj[["env"]][["last.par"]]
  }
  if (is.null(par_fixed) || length(par_fixed) == 0) {
    return(NULL)
  }

  adreport_args <- list(
    data = obj[["env"]][["data"]],
    parameters = obj[["env"]][["parameters"]],
    type = "ADFun",
    ADreport = TRUE,
    DLL = as.character(obj[["env"]][["DLL"]][[1]]),
    silent = obj[["env"]][["silent"]]
  )
  if (!is.null(obj[["env"]][["map"]])) {
    adreport_args[["map"]] <- obj[["env"]][["map"]]
  }

  adreport_obj <- tryCatch(
    do.call(TMB::MakeADFun, adreport_args),
    error = function(e) NULL
  )
  if (is.null(adreport_obj)) {
    return(NULL)
  }

  estimate <- tryCatch(
    adreport_obj[["fn"]](par),
    error = function(e) NULL
  )
  if (is.null(estimate)) {
    return(NULL)
  }
  if (length(estimate) == 0) {
    return(NULL)
  }

  jacobian <- tryCatch(
    adreport_obj[["gr"]](par),
    error = function(e) NULL
  )
  if (is.null(jacobian)) {
    return(NULL)
  }

  if (has_random) {
    fixed_indices <- seq_along(par)[-random]
    random_jacobian <- jacobian[, random, drop = FALSE]
    fixed_jacobian <- jacobian[, fixed_indices, drop = FALSE]
    if (all(random_jacobian == 0)) {
      backend_se <- calculate_derived_quantity_se(
        estimate = estimate,
        jacobian = as.vector(t(fixed_jacobian)),
        covariance = as.vector(t(sdreport[["cov.fixed"]])),
        n_parameters = length(par_fixed)
      )
    } else {
      hessian_random <- obj[["env"]]$spHess(par, random = TRUE)
      random_covariance <- as.matrix(solve(hessian_random))
      random_tmp <- as.matrix(solve(hessian_random, t(random_jacobian)))

      model_adgrad <- obj[["env"]][["f"]]
      model_adgrad(par, order = 0, type = "ADGrad")
      weight <- rep(0, length(par))
      reverse_sweep <- function(i) {
        weight[random] <- random_tmp[, i]
        -model_adgrad(
          par,
          order = 1,
          type = "ADGrad",
          rangeweight = weight,
          doforward = 0
        )[fixed_indices]
      }
      adjusted_fixed_jacobian <- t(do.call(
        "cbind",
        lapply(seq_along(estimate), reverse_sweep)
      )) + fixed_jacobian

      backend_se <- calculate_derived_quantity_laplace_se(
        estimate = estimate,
        adjusted_fixed_jacobian = as.vector(t(adjusted_fixed_jacobian)),
        fixed_covariance = as.vector(t(sdreport[["cov.fixed"]])),
        random_jacobian = as.vector(t(random_jacobian)),
        random_covariance = as.vector(t(random_covariance)),
        n_fixed_effects = length(par_fixed),
        n_random_effects = length(random)
      )
    }
  } else {
    backend_se <- calculate_derived_quantity_se(
      estimate = estimate,
      jacobian = as.vector(t(jacobian)),
      covariance = as.vector(t(sdreport[["cov.fixed"]])),
      n_parameters = length(par_fixed)
    )
  }

  out <- cbind(
    "Estimate" = unname(estimate),
    "Std. Error" = unname(backend_se)
  )
  rownames(out) <- names(estimate)
  out
}
