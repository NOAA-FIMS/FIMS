# Source recovery modules through dm-validation.R. Benchmark only.
dm_profile_controls <- function() list(
  original = list(iter.max = 300, eval.max = 600),
  tight = list(iter.max = 1000, eval.max = 2000, rel.tol = 1e-12, x.tol = 1e-10))

dm_profile_fit <- function(fixture, operating, snapshot, owners, start = "saved",
                           control = dm_profile_controls()$tight, fixed_M_sd = NULL) {
  stopifnot(start %in% c("saved", "cold", "template"), length(start) == 1L,
    length(owners) == length(snapshot$p), !anyDuplicated(owners), "M" %in% owners)
  if (!is.null(fixed_M_sd)) stopifnot(length(fixed_M_sd) == 1L,
    is.finite(fixed_M_sd), fixed_M_sd >= .001, fixed_M_sd <= 1)
  if (!identical(fixture$parameters, snapshot$fixture_parameters) ||
      !identical(fixture$initial, snapshot$initial) ||
      !identical(operating$truth, snapshot$truth) ||
      !identical(get_data(operating$data), snapshot$data)) stop("Snapshot input mismatch")
  # A constant .2 template recreates the original high-SD starting basin.
  setup <- mode_model_input(fixture, operating, snapshot$start_sd, snapshot$composition_likelihood)
  input <- setup$input
  fixed <- names(get_parameter_names(as.list(input$parameters$p)))
  random <- names(get_random_names(as.list(input$parameters$re)))
  if (!identical(mode_parameter_layout(fixed), snapshot$fixed_layout) ||
      !identical(mode_parameter_layout(random), snapshot$random_layout) ||
      length(snapshot$p) != length(input$parameters$p) ||
      length(snapshot$re) != length(input$parameters$re) ||
      any(!is.finite(c(snapshot$p, snapshot$re)))) stop("Snapshot layout/value mismatch")
  densities <- list(R = attr(input, "gaussian_recruitment"),
    F = attr(input, "fishing_processes")$fleet1, M = attr(input, "gaussian_mortality"))
  for (key in names(densities)) {
    position <- which(startsWith(fixed, paste0("gaussian_process.", densities[[key]]$get_id(), ".log_sd.")))
    if (!identical(position, which(owners == key))) stop("Snapshot parameter ownership mismatch")
  }
  if (start != "template") input$parameters$p[] <- snapshot$p
  if (start == "template") input$parameters$p[owners %in% c("R", "F", "M")] <- log(.2)
  if (start == "saved") input$parameters$re[] <- snapshot$re
  M <- which(owners == "M")
  free <- seq_along(snapshot$p)
  map <- NULL
  if (!is.null(fixed_M_sd)) {
    input$parameters$p[M] <- log(fixed_M_sd)
    free <- free[-M]
    labels <- seq_along(snapshot$p); labels[M] <- NA_integer_
    map <- list(p = factor(labels))
  }
  obj <- TMB::MakeADFun(data = list(), parameters = input$parameters, map = map,
    random = "re", DLL = "FIMS", silent = TRUE)
  lower <- snapshot$lower[free]; upper <- snapshot$upper[free]
  stopifnot(length(obj$par) == length(free))
  opt <- nlminb(obj$par, obj$fn, obj$gr, lower = lower, upper = upper, control = control)
  first <- as.numeric(obj$fn(opt$par)); gradient <- as.numeric(obj$gr(opt$par))
  value <- as.numeric(obj$fn(opt$par)); joint <- as.numeric(obj$env$last.par)
  full <- obj$env$parList(par = joint)
  inner <- as.numeric(obj$env$f(joint, order = 1))[obj$env$random]
  projected <- sensitivity_projected_gradient(opt$par, gradient, lower, upper)
  valid <- all(is.finite(c(value, first, projected, inner))) && abs(value - first) < 1e-5 &&
    opt$convergence == 0 && max(abs(projected)) < 1e-3 && max(abs(inner)) < 1e-3
  if (!is.null(fixed_M_sd)) stopifnot(abs(full$p[M] - log(fixed_M_sd)) < 1e-12)
  refined <- NULL
  if (is.null(fixed_M_sd)) {
    refined <- snapshot
    refined$p <- as.numeric(full$p); refined$re <- as.numeric(full$re)
    refined$joint <- joint; refined$random_indices <- obj$env$random
    refined$objective <- value; refined$joint_objective <- as.numeric(obj$env$f(joint, order = 0))
    refined$optimizer_code <- opt$convergence; refined$optimizer_message <- opt$message
    refined$optimizer_valid <- valid; refined$evaluation_drift <- value - first
    refined$max_projected_gradient <- max(abs(projected)); refined$max_inner_gradient <- max(abs(inner))
  }
  list(snapshot = refined, p = as.numeric(full$p), re = as.numeric(full$re),
    point = data.frame(parameter = owners, value = as.numeric(full$p), estimate = exp(as.numeric(full$p))),
    diagnostics = data.frame(objective = value, optimizer_code = opt$convergence,
      optimizer_message = opt$message, optimizer_valid = valid, evaluation_drift = value - first,
      max_projected_gradient = max(abs(projected)), max_inner_gradient = max(abs(inner)),
      M_sd = exp(full$p[M]), fixed_M_sd = if (is.null(fixed_M_sd)) NA_real_ else fixed_M_sd,
      nuisance_boundary = any(abs(as.numeric(full$p)[free] - lower) < 1e-3 |
        abs(as.numeric(full$p)[free] - upper) < 1e-3)))
}

# Include every valid free/grid solution in the reference; a grid that beats the
# free optimizer exposes an optimization gap instead of being silently clipped.
dm_profile_summary <- function(refits, profiles) {
  pieces <- lapply(unique(profiles$replicate), function(rep) {
    x <- profiles[profiles$replicate == rep, , drop = FALSE]
    free <- refits[refits$replicate == rep & refits$case == "estimated_dm" & refits$optimizer_valid, , drop = FALSE]
    valid <- x$optimizer_valid & is.finite(x$objective)
    reference <- c(x$objective[valid], free$objective[is.finite(free$objective)])
    reference <- if (length(reference)) min(reference) else NA_real_
    x$reference_objective <- reference
    x$delta_nll <- ifelse(valid, x$objective - reference, NA_real_)
    x$free_optimization_gap <- if (nrow(free)) min(free$objective) - reference else NA_real_
    x
  })
  dplyr::bind_rows(pieces)
}

run_dm_profiles <- function(baseline, checkpoint = NULL,
                             grid = c(.001, .01, .025, .05, .1, .2, .4)) {
  stopifnot(identical(baseline$configuration$replicates, 5L),
    identical(baseline$configuration$years, 18L), length(grid) > 0L,
    !anyDuplicated(grid), all(is.finite(grid)), all(grid >= .001 & grid <= 1))
  fixture <- recovery_fixture(18L)
  cases <- rbind(data.frame(replicate = 1:5, case = "estimated_dm"),
    data.frame(replicate = c(3L, 4L), case = "multinomial"))
  fits <- refits <- profiles <- points <- reports <- metrics <- correlations <- list()
  capture <- function(expr) {
    warnings <- character(); started <- proc.time()[["elapsed"]]
    result <- withCallingHandlers(tryCatch(force(expr), error = identity), warning = function(w) {
      warnings <<- c(warnings, conditionMessage(w)); invokeRestart("muffleWarning")
    })
    row <- if (inherits(result, "error")) data.frame(objective = NA_real_, optimizer_valid = FALSE,
      error = conditionMessage(result)) else result$diagnostics
    list(result = if (inherits(result, "error")) NULL else result,
      row = cbind(row, warnings = paste(unique(warnings), collapse = " | "),
        elapsed_seconds = proc.time()[["elapsed"]] - started))
  }
  for (i in seq_len(nrow(cases))) {
    meta <- cases[i, ]; rep <- meta$replicate; case <- meta$case
    operating <- list(data = FIMSFrame(baseline$inputs[[rep]]$data$strong), truth = baseline$inputs[[rep]]$truth)
    key <- paste(rep, "strong", case, sep = "/"); original <- baseline$snapshots[[key]]
    old_points <- baseline$tables$points[baseline$tables$points$replicate == rep &
      baseline$tables$points$scenario == "strong" & baseline$tables$points$case == case, ]
    stopifnot(!is.null(original), identical(as.numeric(old_points$value), original$p))
    owners <- old_points$parameter
    candidates <- list(); rows <- list()
    for (control_name in names(dm_profile_controls())) for (start in c("saved", "cold", "template")) {
      message(sprintf("refit replicate=%d case=%s control=%s start=%s", rep, case, control_name, start))
      x <- capture(dm_profile_fit(fixture, operating, original, owners, start, dm_profile_controls()[[control_name]]))
      candidates[length(candidates) + 1L] <- list(x$result)
      rows[[length(rows) + 1L]] <- cbind(meta, control = control_name, start = start, x$row)
      gc(verbose = FALSE)
    }
    rows <- dplyr::bind_rows(rows)
    chosen <- order(!rows$optimizer_valid, rows$objective, na.last = TRUE)[1L]
    rows$selected <- seq_len(nrow(rows)) == chosen
    refits[[length(refits) + 1L]] <- rows
    best <- candidates[[chosen]]
    fits[[key]] <- best
    report <- capture({
      if (is.null(best)) stop("No fitted snapshot")
      mode_report_snapshot(fixture, operating, best$snapshot, TRUE)
    })
    d <- report$row
    if (is.null(d$status)) d$status <- "report_error"
    d$eligible <- !is.null(report$result) && isTRUE(d$optimizer_valid) && isTRUE(d$pd_hessian) &&
      d$status == "consistent_local_report" && nrow(report$result$states) > 0 &&
      all(is.finite(report$result$states$se) & report$result$states$se > 0)
    d$regular <- d$eligible && !isTRUE(d$boundary)
    reports[[length(reports) + 1L]] <- cbind(meta, d)
    metric <- if (!is.null(report$result) && nrow(report$result$state_metrics)) report$result$state_metrics else
      data.frame(process = c("R", "F", "M"), coverage = NA_real_, rmse = NA_real_)
    metrics[[length(metrics) + 1L]] <- cbind(meta[rep(1L, nrow(metric)), , drop = FALSE], eligible = d$eligible, metric)
    if (!is.null(report$result)) {
      covariance <- report$result$fixed_covariance
      if (d$eligible && !is.null(covariance) && all(diag(covariance) > 0)) {
        cor <- cov2cor(covariance); M <- which(owners == "M"); k <- grep("concentration/", owners)
        if (length(k)) correlations[[length(correlations) + 1L]] <- data.frame(replicate = rep,
          parameter = owners[k], correlation_log_M_sd = cor[M, k])
      }
    }
    if (!is.null(best)) points[[length(points) + 1L]] <- cbind(meta[rep(1L, nrow(best$point)), , drop = FALSE], stage = "refit", fixed_M_sd = NA_real_,
      start = rows$start[chosen], best$point)
    if (case == "estimated_dm") for (sd in grid) for (start in c("saved", "cold")) {
      message(sprintf("profile replicate=%d M_sd=%.3f start=%s", rep, sd, start))
      x <- capture({
        if (is.null(best)) stop("No fitted snapshot")
        dm_profile_fit(fixture, operating, best$snapshot, owners, start, dm_profile_controls()$tight, sd)
      })
      profiles[[length(profiles) + 1L]] <- cbind(meta, profile_sd = sd, start = start, x$row)
      if (!is.null(x$result)) points[[length(points) + 1L]] <- cbind(meta[rep(1L, nrow(x$result$point)), , drop = FALSE], stage = "profile",
        fixed_M_sd = sd, start = start, x$result$point)
      gc(verbose = FALSE)
    }
    if (!is.null(checkpoint)) saveRDS(list(refits = refits, profiles = profiles,
      points = points, reports = reports, metrics = metrics, correlations = correlations, fits = fits), checkpoint)
  }
  tables <- lapply(list(refits = refits, profiles = profiles, points = points, reports = reports,
    metrics = metrics, correlations = correlations), dplyr::bind_rows)
  tables$profiles <- dm_profile_summary(tables$refits, tables$profiles)
  list(configuration = list(grid = grid, cases = cases, controls = dm_profile_controls(),
    acceptance = "code 0; projected/inner gradients <1e-3; repeated objective drift <1e-5",
    profile_scope = "M innovation SD fixed; R/F SDs, four concentrations and random states reoptimized",
    start_scope = "saved p/re; saved p/default re; template p/default re with process SD .2"),
    tables = tables, fits = fits)
}
