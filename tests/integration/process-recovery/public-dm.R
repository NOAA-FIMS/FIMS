# Source recovery modules through dm-profiles.R. No production changes.
public_dm_fixture <- function(years) {
  fixture <- recovery_fixture(years)
  raw <- get_data(fixture$data)
  raw$uncertainty[raw$type == "index"] <- "~ dlnorm(meanlog = log_index_expected, sdlog = 0.05)"
  raw$uncertainty[raw$type == "age_comp"] <- "~ dmultinom(prob = agecomp_proportion, size = 2000)"
  raw$uncertainty[raw$type == "length_comp"] <- "~ dmultinom(prob = lengthcomp_proportion, size = 2000)"
  fixture$data <- FIMSFrame(raw)
  fixture
}

public_dm_generate <- function(fixture, seed, concentration, nsim = 1L, fixed = FALSE) {
  walk <- function(sd) list(type = "rw1", sd = sd, estimation_type = "constant", initial = fixture$initial)
  spec <- dm_composition_spec(list(data = fixture$data), concentration)
  simulate_fims(fixture$parameters, fixture$data, list(type = "iid"), seed = seed,
    nsim = nsim, process_mode = if (fixed) "fixed" else "simulate",
    composition_likelihood = spec, fishing_process = list(fleet1 = walk(fixture$sigma[["F"]])),
    mortality_process = walk(fixture$sigma[["M"]]))
}

public_dm_roundtrip <- function(simulation, replicate = 1L) {
  raw <- simulation$configuration$data; draw <- simulation$replicates[[replicate]]
  idx <- which(raw$type %in% c("catch", "index", "age_comp", "length_comp"))
  obs <- draw$observations
  stopifnot(length(idx) == nrow(obs), all(obs$status == "simulated"),
    all(is.finite(obs$simulated)), all(vapply(c("fleet", "type", "timing", "age", "length"),
      function(key) identical(raw[[key]][idx], obs[[key]]), logical(1))))
  comp <- which(obs$type %in% c("age_comp", "length_comp"))
  groups <- split(comp, interaction(obs$fleet[comp], obs$type[comp], obs$timing[comp], drop = TRUE))
  for (rows in groups) stopifnot(abs(sum(obs$simulated[rows]) - 1) < 1e-12,
    all(abs(2000 * obs$simulated[rows] - round(2000 * obs$simulated[rows])) < 1e-8))
  raw$observed[idx] <- obs$simulated
  list(data = FIMSFrame(raw), truth = list(R = draw$recruitment$value,
    F = draw$fishing_mortality$log_Fmort, M = draw$natural_mortality$log_M))
}

# Fixed paths make repeated observation draws independent conditional on one p.
# Reference draws use the old qgamma/inverse-CDF sampler, not the production helper.
public_dm_moments <- function(nsim = 1000L, seed = 8801L) {
  stopifnot(nsim >= 2L, nsim == as.integer(nsim))
  had <- exists(".Random.seed", .GlobalEnv, inherits = FALSE)
  saved <- if (had) get(".Random.seed", .GlobalEnv) else NULL
  on.exit({if (had) assign(".Random.seed", saved, .GlobalEnv)
    else if (exists(".Random.seed", .GlobalEnv, inherits = FALSE)) rm(".Random.seed", envir = .GlobalEnv)})
  fixture <- public_dm_fixture(3L); bins <- covariances <- differences <- list()
  for (kappa in c(Inf, 2000, 200)) {
    message("distribution check kappa=", kappa)
    simulation <- public_dm_generate(fixture, seed, kappa, nsim, TRUE)
    first <- simulation$replicates[[1]]$observations
    idx <- which(first$type %in% c("age_comp", "length_comp") & first$timing == 1L)
    groups <- split(idx, interaction(first$fleet[idx], first$type[idx], drop = TRUE))
    for (j in seq_along(groups)) {
      rows <- groups[[j]]; probability <- first$expected[rows]; size <- 2000
      public <- vapply(simulation$replicates, function(draw) {
        stopifnot(identical(draw$observations$expected[rows], probability))
        size * draw$observations$simulated[rows]
      }, numeric(length(rows)))
      set.seed(seed + 100000L + j)
      reference <- replicate(nsim, robustness_counts(robustness_probabilities(probability,
        kappa, runif(length(rows))), runif(size)))
      stopifnot(all(abs(colSums(public) - size) < 1e-8), all(colSums(reference) == size),
        all(public[probability == 0, , drop = FALSE] == 0), all(reference[probability == 0, , drop = FALSE] == 0))
      inflation <- if (is.infinite(kappa)) 1 else (size + kappa) / (1 + kappa)
      mu <- size * probability; variance <- size * probability * (1 - probability) * inflation
      meta <- data.frame(concentration = kappa, fleet = first$fleet[rows[1]], type = first$type[rows[1]])
      summaries <- list()
      for (generator in c("public", "reference")) {
        draws <- if (generator == "public") public else reference
        center <- draws - mu; squared <- center^2
        se_mean <- sqrt(variance / nsim); se_variance <- apply(squared, 1, sd) / sqrt(nsim)
        table <- data.frame(bin = seq_along(rows), probability = probability, structural_zero = probability == 0,
          n = nsim, expected_mean = mu, mean = rowMeans(draws), expected_variance = variance,
          second_moment = rowMeans(squared), mean_mcse = se_mean, second_moment_mcse = se_variance)
        table$mean_z <- ifelse(se_mean > 0, (table$mean - mu) / se_mean, NA_real_)
        table$variance_z <- ifelse(se_variance > 0, (table$second_moment - variance) / se_variance, NA_real_)
        summaries[[generator]] <- table
        bins[[length(bins) + 1L]] <- cbind(meta[rep(1L, nrow(table)), ], generator = generator, table)
        top <- head(order(probability, decreasing = TRUE), 2L)
        if (length(top) == 2L && all(probability[top] > 0)) {
          product <- center[top[1], ] * center[top[2], ]
          covariances[[length(covariances) + 1L]] <- cbind(meta, generator = generator,
            bin1 = top[1], bin2 = top[2], expected = -size * prod(probability[top]) * inflation,
            estimate = mean(product), mcse = sd(product) / sqrt(nsim), n = nsim)
        }
      }
      a <- summaries$public; b <- summaries$reference
      se_v <- sqrt(a$second_moment_mcse^2 + b$second_moment_mcse^2)
      d <- data.frame(bin = a$bin, structural_zero = a$structural_zero,
        mean_difference = a$mean - b$mean, mean_mcse = sqrt(2) * a$mean_mcse,
        second_moment_difference = a$second_moment - b$second_moment, second_moment_mcse = se_v)
      d$mean_z <- ifelse(d$mean_mcse > 0, d$mean_difference / d$mean_mcse, NA_real_)
      d$variance_z <- ifelse(se_v > 0, d$second_moment_difference / se_v, NA_real_)
      differences[[length(differences) + 1L]] <- cbind(meta[rep(1L, nrow(d)), ], d)
    }
    rm(simulation); gc(verbose = FALSE)
  }
  list(bins = dplyr::bind_rows(bins), covariance = dplyr::bind_rows(covariances),
    generator_differences = dplyr::bind_rows(differences))
}

public_dm_capture <- function(expr) {
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

public_dm_summaries <- function(tables) {
  result <- dm_validation_summaries(tables)
  m <- tables$metrics; pairs <- list()
  for (scenario in unique(m$scenario)) for (process in c("R", "F", "M")) {
    x <- m[m$scenario == scenario & m$process == process & m$eligible, ]
    both <- merge(x[x$case == "known_dm", ], x[x$case == "estimated_dm", ],
      by = "replicate", suffixes = c("_known", "_estimated"))
    for (metric in c("coverage", "rmse")) {
      s <- dm_mean_mcse(both[[paste0(metric, "_estimated")]] - both[[paste0(metric, "_known")]])
      pairs[[length(pairs) + 1L]] <- data.frame(scenario = scenario, process = process, metric = metric,
        comparison = "estimated minus known concentration", n = s[["n"]],
        mean_difference = s[["mean"]], mcse = s[["mcse"]])
    }
  }
  result$paired_summary <- dplyr::bind_rows(pairs)
  result
}

run_public_dm_recovery <- function(replicates = 5L, years = 18L, seed = 6301L, checkpoint = NULL) {
  stopifnot(replicates >= 1L, replicates == as.integer(replicates))
  fixture <- public_dm_fixture(years)
  starts <- expand.grid(start_sd = c(.05, .2), shifted = c(FALSE, TRUE))
  attempts <- reports <- metrics <- points <- inputs <- snapshots <- checks <- list()
  for (replicate in seq_len(replicates)) {
    previous <- NULL
    for (kappa in c(2000, 200)) {
      scenario <- if (kappa == 2000) "moderate" else "strong"
      simulation <- public_dm_generate(fixture, seed + replicate - 1L, kappa)
      model <- public_dm_roundtrip(simulation)
      obs <- simulation$replicates[[1]]$observations
      scalar <- obs$simulated[obs$type %in% c("catch", "index")]
      if (!is.null(previous)) stopifnot(identical(model$truth, previous$truth), identical(scalar, previous$scalar))
      previous <- list(truth = model$truth, scalar = scalar)
      inputs[[paste(replicate, scenario, sep = "/")]] <- list(data = get_data(model$data), truth = model$truth)
      checks[[length(checks) + 1L]] <- data.frame(replicate = replicate, scenario = scenario,
        seed = seed + replicate - 1L, concentration = kappa, aligned = TRUE, integer_counts = TRUE,
        shared_paths_and_scalars = TRUE)
      for (case in c("known_dm", "estimated_dm")) {
        message(sprintf("public recovery replicate=%d/%d scenario=%s case=%s", replicate, replicates, scenario, case))
        meta <- data.frame(replicate = replicate, seed = seed + replicate - 1L, scenario = scenario, case = case)
        spec <- dm_composition_spec(model, if (case == "known_dm") kappa else 2000, case == "estimated_dm")
        candidates <- rows <- list()
        for (j in seq_len(nrow(starts))) {
          x <- public_dm_capture(dm_fit_snapshot(fixture, model, starts$start_sd[j], starts$shifted[j], spec))
          candidates[j] <- list(x$result)
          rows[[j]] <- cbind(meta, start_id = j, stage = "initial", starts[j, ], x$row)
          gc(verbose = FALSE)
        }
        table <- dplyr::bind_rows(rows)
        # Predeclared, recorded rescue only when all initial starts fail checks.
        if (!any(table$optimizer_valid) && any(vapply(candidates, Negate(is.null), logical(1)))) {
          best <- order(table$objective, na.last = TRUE)[1L]; original <- candidates[[best]]
          if (!is.null(original)) for (control in names(dm_profile_controls())) {
            x <- public_dm_capture(dm_profile_fit(fixture, model, original$snapshot,
              original$point$parameter, "saved", dm_profile_controls()[[control]]))
            fit <- x$result
            if (!is.null(fit)) fit$point <- data.frame(parameter = original$point$parameter,
              value = fit$snapshot$p, lower = fit$snapshot$lower, upper = fit$snapshot$upper)
            candidates[length(candidates) + 1L] <- list(fit)
            rows[[length(rows) + 1L]] <- cbind(meta, start_id = length(rows) + 1L,
              stage = paste0("refine_", control), start_sd = original$snapshot$start_sd, shifted = NA, x$row)
          }
        }
        table <- dplyr::bind_rows(rows)
        selected <- order(!table$optimizer_valid, table$objective, na.last = TRUE)[1L]
        table$selected <- seq_len(nrow(table)) == selected
        attempts[[length(attempts) + 1L]] <- table
        fit <- candidates[[selected]]
        report <- public_dm_capture({if (is.null(fit)) stop("No fitted snapshot")
          mode_report_snapshot(fixture, model, fit$snapshot, TRUE)})
        d <- report$row
        if (is.null(d$status)) d$status <- "report_error"
        d$warm <- TRUE
        d$eligible <- !is.null(report$result) && isTRUE(d$optimizer_valid) && isTRUE(d$pd_hessian) &&
          d$status == "consistent_local_report" && nrow(report$result$states) > 0 &&
          all(is.finite(report$result$states$se) & report$result$states$se > 0)
        d$regular <- d$eligible && !isTRUE(d$boundary)
        reports[[length(reports) + 1L]] <- cbind(meta, selected_start = selected, d)
        m <- if (!is.null(report$result) && nrow(report$result$state_metrics)) report$result$state_metrics else
          data.frame(process = c("R", "F", "M"), coverage = NA_real_, rmse = NA_real_, bias = NA_real_, mean_width = NA_real_)
        metrics[[length(metrics) + 1L]] <- cbind(meta[rep(1L, nrow(m)), ], eligible = d$eligible, regular = d$regular, m)
        if (!is.null(fit)) {
          p <- fit$point
          p$truth <- ifelse(grepl("concentration", p$parameter), kappa, fixture$sigma[match(p$parameter, names(fixture$sigma))])
          p$estimate <- exp(p$value); p$boundary <- abs(p$value - p$lower) < 1e-3 | abs(p$value - p$upper) < 1e-3
          covariance <- if (!is.null(report$result)) report$result$fixed_covariance else NULL
          variance_fixed <- if (!is.null(covariance)) diag(covariance) else rep(NA_real_, nrow(p))
          p$variance_fixed <- variance_fixed
          p$log_se <- sqrt(ifelse(is.finite(variance_fixed) & variance_fixed > 0, variance_fixed, NA_real_))
          p$covered <- ifelse(is.finite(p$log_se), abs(p$value - log(p$truth)) <= qnorm(.975) * p$log_se, NA)
          points[[length(points) + 1L]] <- cbind(meta[rep(1L, nrow(p)), ], eligible = d$eligible, regular = d$regular, p)
          snapshots[[paste(replicate, scenario, case, sep = "/")]] <- fit$snapshot
        }
        if (!is.null(checkpoint)) saveRDS(list(inputs = inputs, snapshots = snapshots, attempts = attempts,
          reports = reports, metrics = metrics, points = points, checks = checks), checkpoint)
      }
    }
  }
  tables <- lapply(list(attempts = attempts, reports = reports, metrics = metrics, points = points, checks = checks), dplyr::bind_rows)
  if (!nrow(tables$points)) tables$points <- data.frame(scenario = character(), case = character(),
    parameter = character(), eligible = logical(), regular = logical())
  list(configuration = list(replicates = replicates, years = years, seed = seed,
    concentrations = c(2000, 200), sample_size = 2000L, starts = starts, controls = dm_profile_controls(),
    rescue = "original/tight saved-state restart only if no initial start passes",
    true_process_sd = fixture$sigma, initial = fixture$initial), inputs = inputs, snapshots = snapshots,
    tables = tables, summaries = public_dm_summaries(tables))
}
