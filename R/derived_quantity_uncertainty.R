#' Extract TMB ADREPORT uncertainty payload
#'
#' @description
#' Extracts the TMB ADREPORT estimates, Jacobians, covariance matrices, and
#' Laplace-adjusted pieces needed by the FIMS backend uncertainty adapters.
#'
#' @param obj An object returned from [TMB::MakeADFun()].
#' @param sdreport An object returned from [TMB::sdreport()].
#' @param par_fixed Optional fixed-effect parameter vector.
#'
#' @return A structured list containing ADREPORT payload pieces, or `NULL` when
#'   extraction is not available for the supplied object.
#' @noRd
extract_tmb_adreport_payload <- function(obj,
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
  if (is.null(estimate) || length(estimate) == 0) {
    return(NULL)
  }

  jacobian <- tryCatch(
    adreport_obj[["gr"]](par),
    error = function(e) NULL
  )
  if (is.null(jacobian)) {
    return(NULL)
  }

  if (!has_random) {
    payload <- assemble_adreport_payload(list(
      estimate = estimate,
      jacobian = jacobian,
      fixed_covariance = sdreport[["cov.fixed"]]
    ))
    names(payload[["estimate"]]) <- names(estimate)
    payload[["par"]] <- par
    payload[["par_fixed"]] <- par_fixed
    payload[["random"]] <- random
    return(payload)
  }

  fixed_indices <- seq_along(par)[-random]
  random_jacobian <- jacobian[, random, drop = FALSE]
  hessian_random <- obj[["env"]]$spHess(par, random = TRUE)
  fixed_jacobian_adjustment <- matrix(
    0,
    nrow = length(estimate),
    ncol = length(fixed_indices)
  )

  if (!all(random_jacobian == 0)) {
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
    fixed_jacobian_adjustment <- t(do.call(
      "cbind",
      lapply(seq_along(estimate), reverse_sweep)
    ))
  }

  payload <- assemble_adreport_payload(list(
    estimate = estimate,
    jacobian = jacobian,
    fixed_covariance = sdreport[["cov.fixed"]],
    random_indices = as.integer(random - 1L),
    random_hessian = as.matrix(hessian_random),
    fixed_jacobian_adjustment = fixed_jacobian_adjustment
  ))
  names(payload[["estimate"]]) <- names(estimate)
  payload[["par"]] <- par
  payload[["par_fixed"]] <- par_fixed
  payload[["random"]] <- random
  payload[["fixed_indices"]] <- fixed_indices
  payload
}

#' Calculate uncertainty from a TMB ADREPORT payload
#'
#' @description
#' Delegates fixed-effect and Laplace ADREPORT payloads to the FIMS C++ backend
#' uncertainty adapters.
#'
#' @param payload A list returned by [extract_tmb_adreport_payload()].
#'
#' @return A matrix with `Estimate` and `Std. Error` columns, or `NULL` when the
#'   payload is `NULL`.
#' @noRd
calculate_tmb_adreport_payload_uncertainty <- function(payload) {
  if (is.null(payload)) {
    return(NULL)
  }

  backend_se <- calculate_adreport_payload_se(payload)

  out <- cbind(
    "Estimate" = unname(payload[["estimate"]]),
    "Std. Error" = unname(backend_se)
  )
  rownames(out) <- names(payload[["estimate"]])
  out
}

#' Calculate ADREPORT uncertainty with the FIMS backend
#'
#' @description
#' Compatibility wrapper that extracts a TMB ADREPORT payload and delegates the
#' uncertainty calculation to the FIMS C++ backend adapters.
#'
#' @param obj An object returned from [TMB::MakeADFun()].
#' @param sdreport An object returned from [TMB::sdreport()].
#' @param par_fixed Optional fixed-effect parameter vector.
#'
#' @return A matrix with `Estimate` and `Std. Error` columns, or `NULL` when
#'   backend calculation is not available for the supplied object.
#' @noRd
calculate_tmb_adreport_uncertainty <- function(obj,
                                               sdreport,
                                               par_fixed = NULL) {
  payload <- extract_tmb_adreport_payload(
    obj = obj,
    sdreport = sdreport,
    par_fixed = par_fixed
  )
  calculate_tmb_adreport_payload_uncertainty(payload)
}
