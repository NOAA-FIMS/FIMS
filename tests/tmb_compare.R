run_script_env <- function(path) {
  env <- new.env(parent = globalenv())
  source(path, local = env)
  env
}

safe_extract <- function(expr, default = NA_real_) {
  tryCatch(expr, error = function(e) default)
}

compare_max_abs_diff <- function(x, y) {
  if (length(x) != length(y)) {
    return(NA_real_)
  }
  max(abs(as.numeric(x) - as.numeric(y)))
}

legacy_path <- file.path("tests", "tmb_Rcpp.R")
native_path <- file.path("tests", "tmb_call.R")

if (!file.exists(legacy_path)) {
  stop("Legacy script not found at: ", legacy_path)
}

if (!file.exists(native_path)) {
  stop("Native script not found at: ", native_path)
}

legacy_env <- run_script_env(legacy_path)
native_env <- run_script_env(native_path)

legacy_gradient_max <- safe_extract(max(abs(legacy_env$obj$gr())))
legacy_objective <- safe_extract(legacy_env$opt$objective)
legacy_convergence <- safe_extract(legacy_env$opt$convergence, default = NA_integer_)

native_comparison <- if (exists("comparison", envir = native_env, inherits = FALSE)) {
  get("comparison", envir = native_env, inherits = FALSE)
} else {
  NULL
}

native_recruitment_head <- if (!is.null(native_comparison)) {
  native_comparison$recruitment$head
} else {
  NA_real_
}

legacy_ssb_head <- safe_extract(head(as.numeric(legacy_env$om_output[["SSB"]]), 5),
                                default = rep(NA_real_, 5))

summary <- list(
  legacy = list(
    gradient_max = legacy_gradient_max,
    objective = legacy_objective,
    convergence = legacy_convergence
  ),
  native = list(
    comparison = native_comparison
  ),
  cross_checks = list(
    recruitment_head_vs_ssb_head_max_abs_diff =
      compare_max_abs_diff(native_recruitment_head, legacy_ssb_head)
  ),
  notes = c(
    "The native script currently covers module-level wrappers and transformed values.",
    "The legacy script includes full CatchAtAge and TMB optimization workflow.",
    "Cross-check fields are intentionally limited until the native interface includes full model assembly and objective evaluation."
  )
)

print(summary)
