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
get_adreport_parameters <- function(adreport_obj, par) {
  adreport_par <- adreport_obj[["par"]]
  if (is.null(adreport_par)) {
    adreport_ptr <- tryCatch(
      adreport_obj[["env"]][["ADFun"]][["ptr"]],
      error = function(e) adreport_obj
    )
    adreport_par <- attr(adreport_ptr, "par")
  }
  if (is.null(adreport_par) || length(adreport_par) == 0) {
    return(par)
  }
  if (length(adreport_par) == length(par)) {
    adreport_par[] <- par
    return(adreport_par)
  }

  par_names <- names(par)
  adreport_names <- names(adreport_par)
  if (!is.null(par_names) &&
      !is.null(adreport_names) &&
      all(adreport_names %in% par_names)) {
    adreport_par[] <- par[adreport_names]
    return(adreport_par)
  }

  adreport_par
}

#' Assemble TMB ADREPORT payload from a TMB ADREPORT object
#'
#' @param adreport_obj A TMB object created with `ADreport = TRUE`.
#' @param parameters Parameter vector used to evaluate the ADREPORT tape.
#' @param fixed_covariance Fixed-effect covariance matrix.
#' @param random_payload Optional list with random-effect Laplace pieces.
#'
#' @return A structured ADREPORT payload list.
#' @export
assemble_adreport_payload_from_tmb_adfun <- function(adreport_obj,
                                                     parameters,
                                                     fixed_covariance,
                                                     random_payload = list()) {
  adreport_ptr <- tryCatch(
    adreport_obj[["env"]][["ADFun"]][["ptr"]],
    error = function(e) NULL
  )
  if (!is.null(adreport_ptr)) {
    native_payload <- tryCatch(
      assemble_adreport_payload_from_tmb_adfun_native(
        adreport_ptr,
        parameters,
        fixed_covariance,
        random_payload
      ),
      error = function(e) NULL
    )
    if (!is.null(native_payload)) {
      return(native_payload)
    }
  }

  estimate <- adreport_obj[["fn"]](parameters)
  jacobian <- adreport_obj[["gr"]](parameters)
  payload <- c(
    list(
      estimate = estimate,
      jacobian = jacobian,
      fixed_covariance = fixed_covariance
    ),
    random_payload
  )
  assemble_adreport_payload(payload)
}

calculate_tmb_native_fixed_hessian <- function(obj,
                                               par_fixed) {
  hessian_fixed <- tryCatch(
    obj[["he"]](par_fixed),
    error = function(e) NULL
  )
  if (is.null(hessian_fixed)) {
    return(NULL)
  }

  hessian_fixed <- as.matrix(hessian_fixed)
  if (!identical(dim(hessian_fixed), rep(length(par_fixed), 2))) {
    return(NULL)
  }

  hessian_fixed
}

calculate_tmb_fixed_hessian <- function(obj,
                                        par_fixed,
                                        has_random = FALSE) {
  hessian_fixed <- NULL
  if (!has_random) {
    hessian_fixed <- calculate_tmb_native_fixed_hessian(obj, par_fixed)
  }
  if (is.null(hessian_fixed) && has_random) {
    laplace_gradient <- function(x) {
      obj[["env"]][["ff"]](x, order = 1)
    }
    hessian_fixed <- tryCatch(
      calculate_fixed_effect_hessian(
        par_fixed,
        laplace_gradient,
        .Machine$double.eps^(1 / 3)
      ),
      error = function(e) NULL
    )
  }
  if (is.null(hessian_fixed) && !has_random) {
    hessian_fixed <- tryCatch(
      calculate_fixed_effect_hessian(
        par_fixed,
        obj[["gr"]],
        .Machine$double.eps^(1 / 3)
      ),
      error = function(e) NULL
    )
  }
  if (is.null(hessian_fixed)) {
    hessian_fixed <- tryCatch(
      stats::optimHess(par_fixed, obj[["fn"]], obj[["gr"]]),
      error = function(e) NULL
    )
  }

  hessian_fixed
}

calculate_tmb_fixed_covariance <- function(obj,
                                           sdreport = NULL,
                                           par_fixed = NULL,
                                           hessian_fixed = NULL) {
  random <- obj[["env"]][["random"]]
  has_random <- !is.null(random) && length(random) > 0
  if (is.null(par_fixed)) {
    par <- obj[["env"]][["last.par.best"]]
    if (has_random) {
      par_fixed <- par[-random]
    } else {
      par_fixed <- par
    }
  }

  if (is.null(par_fixed) || length(par_fixed) == 0) {
    return(matrix(0, 0, 0))
  }

  if (is.null(hessian_fixed)) {
    hessian_fixed <- calculate_tmb_fixed_hessian(
      obj = obj,
      par_fixed = par_fixed,
      has_random = has_random
    )
  }
  if (!is.null(hessian_fixed)) {
    fixed_covariance <- tryCatch(
      calculate_fixed_effect_covariance(hessian_fixed),
      error = function(e) NULL
    )
    if (!is.null(fixed_covariance)) {
      dimnames(fixed_covariance) <- list(names(par_fixed), names(par_fixed))
      return(fixed_covariance)
    }
  }

  if (length(sdreport) > 0 && !is.null(sdreport[["cov.fixed"]])) {
    return(sdreport[["cov.fixed"]])
  }

  NULL
}

calculate_tmb_random_hessian <- function(obj,
                                         par = NULL,
                                         random = NULL) {
  if (is.null(random)) {
    random <- obj[["env"]][["random"]]
  }
  if (is.null(random) || length(random) == 0) {
    return(matrix(0, 0, 0))
  }
  if (is.null(par)) {
    par <- obj[["env"]][["last.par.best"]]
  }

  hessian_random <- tryCatch(
    {
      model_adgrad <- obj[["env"]][["f"]]
      hessian <- model_adgrad(par, order = 1, type = "ADGrad")
      hessian <- as.matrix(hessian)
      hessian[random, random, drop = FALSE]
    },
    error = function(e) NULL
  )
  if (!is.null(hessian_random) &&
      identical(dim(hessian_random), rep(length(random), 2))) {
    return(hessian_random)
  }

  hessian_random <- tryCatch(
    as.matrix(obj[["env"]]$spHess(par, random = TRUE)),
    error = function(e) NULL
  )
  if (!is.null(hessian_random) &&
      identical(dim(hessian_random), rep(length(random), 2))) {
    return(hessian_random)
  }

  NULL
}

extract_tmb_adreport_payload <- function(obj,
                                         sdreport,
                                         par_fixed = NULL) {
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
  fixed_covariance <- calculate_tmb_fixed_covariance(
    obj = obj,
    sdreport = sdreport,
    par_fixed = par_fixed
  )
  if (is.null(fixed_covariance)) {
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

  adreport_ptr <- tryCatch(
    adreport_obj[["env"]][["ADFun"]][["ptr"]],
    error = function(e) NULL
  )
  if (is.null(adreport_ptr)) {
    return(NULL)
  }
  adreport_par <- get_adreport_parameters(adreport_obj, par)

  estimate <- tryCatch(
    adreport_obj[["fn"]](adreport_par),
    error = function(e) NULL
  )
  if (is.null(estimate) || length(estimate) == 0) {
    return(NULL)
  }

  if (!has_random) {
    payload <- assemble_adreport_payload_from_tmb_adfun(
      adreport_obj,
      adreport_par,
      fixed_covariance,
      list()
    )
    names(payload[["estimate"]]) <- names(estimate)
    payload[["par"]] <- par
    payload[["par_fixed"]] <- par_fixed
    payload[["random"]] <- random
    return(payload)
  }

  jacobian <- tryCatch(
    adreport_obj[["gr"]](adreport_par),
    error = function(e) NULL
  )
  if (is.null(jacobian)) {
    return(NULL)
  }

  fixed_indices <- seq_along(par)[-random]
  random_jacobian <- jacobian[, random, drop = FALSE]
  hessian_random <- calculate_tmb_random_hessian(
    obj = obj,
    par = par,
    random = random
  )
  if (is.null(hessian_random)) {
    return(NULL)
  }
  fixed_jacobian_adjustment <- matrix(
    0,
    nrow = length(estimate),
    ncol = length(fixed_indices)
  )

  if (!all(random_jacobian == 0)) {
    model_adgrad <- obj[["env"]][["f"]]
    model_adgrad(par, order = 0, type = "ADGrad")
    adgrad_ptr <- tryCatch(
      obj[["env"]][["ADGrad"]][["ptr"]],
      error = function(e) NULL
    )
    if (!is.null(adgrad_ptr)) {
      fixed_jacobian_adjustment <- tryCatch(
        calculate_laplace_fixed_jacobian_adjustment_native(
          random_hessian = as.matrix(hessian_random),
          random_jacobian = random_jacobian,
          random_indices = as.integer(random - 1L),
          fixed_indices = as.integer(fixed_indices - 1L),
          parameters = par,
          adgrad_ptr = adgrad_ptr
        ),
        error = function(e) NULL
      )
    }
    if (is.null(fixed_jacobian_adjustment)) {
      reverse_sweep <- function(weight, fixed_indices) {
        model_adgrad(
          par,
          order = 1,
          type = "ADGrad",
          rangeweight = weight,
          doforward = 0
        )[fixed_indices]
      }
      fixed_jacobian_adjustment <- calculate_laplace_fixed_jacobian_adjustment(
        random_hessian = as.matrix(hessian_random),
        random_jacobian = random_jacobian,
        random_indices = as.integer(random - 1L),
        fixed_indices = as.integer(fixed_indices - 1L),
        n_parameters = length(par),
        reverse_sweep_function = reverse_sweep
      )
    }
  }

  payload <- assemble_adreport_payload_from_tmb_adfun(
    adreport_obj,
    adreport_par,
    fixed_covariance,
    list(
      random_indices = as.integer(random - 1L),
      random_hessian = as.matrix(hessian_random),
      fixed_jacobian_adjustment = fixed_jacobian_adjustment
    )
  )
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
