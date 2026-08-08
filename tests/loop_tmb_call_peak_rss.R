#!/usr/bin/env Rscript

iterations <- 50L
if (length(commandArgs(trailingOnly = TRUE)) >= 1) {
    iterations <- as.integer(commandArgs(trailingOnly = TRUE)[[1]])
}

if (is.na(iterations) || iterations < 1L) {
    stop("iterations must be a positive integer")
}

target_script <- file.path("tests", "tmb_call.R")
if (!file.exists(target_script)) {
    stop("Target script not found: ", target_script)
}

gc_mode <- Sys.getenv("FIMS_LOOP_GC_MODE", "none")
if (!gc_mode %in% c("none", "before_each", "after_each", "both")) {
    stop("FIMS_LOOP_GC_MODE must be one of: none, before_each, after_each, both")
}

gc_snapshot <- function(gc_matrix) {
    if (is.null(gc_matrix) || !is.matrix(gc_matrix) || ncol(gc_matrix) < 6) {
        return(list(used_mb = NA_real_, max_used_mb = NA_real_))
    }

    list(
        used_mb = suppressWarnings(max(as.numeric(gc_matrix[, 2]), na.rm = TRUE)),
        max_used_mb = suppressWarnings(max(as.numeric(gc_matrix[, 6]), na.rm = TRUE))
    )
}

# Force pure native path for RSS comparison by disabling wrapper fallback.
Sys.setenv(FIMS_TMB_CALL_USE_WRAPPER_FALLBACK = "0")
# Bypass Rcpp utility bridge (clear/get_fixed/get_random/get_parameter_names)
# so this loop exercises native .Call info access only.
Sys.setenv(FIMS_TMB_CALL_USE_NATIVE_INFO_CALLS = "1")

diagnostic_module <- Sys.getenv("FIMS_TMB_CALL_DIAGNOSTIC_MODULE", "population")
if (!diagnostic_module %in% c("none", "selectivity", "maturity", "growth", "recruitment", "fleets", "population", "all")) {
    stop(
        "FIMS_TMB_CALL_DIAGNOSTIC_MODULE must be one of: none, selectivity, maturity, growth, recruitment, fleets, population, all"
    )
}

module_flags <- list(
    selectivity = FALSE,
    maturity = FALSE,
    growth = FALSE,
    recruitment = FALSE,
    fleets = FALSE,
    population = FALSE
)

if (identical(diagnostic_module, "all")) {
    module_flags[] <- TRUE
} else if (!identical(diagnostic_module, "none")) {
    module_flags[[diagnostic_module]] <- TRUE
}

Sys.setenv(
    FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_SELECTIVITY = if (isTRUE(module_flags$selectivity)) "1" else "0",
    FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_MATURITY = if (isTRUE(module_flags$maturity)) "1" else "0",
    FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_GROWTH = if (isTRUE(module_flags$growth)) "1" else "0",
    FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_RECRUITMENT = if (isTRUE(module_flags$recruitment)) "1" else "0",
    FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_FLEETS = if (isTRUE(module_flags$fleets)) "1" else "0",
    FIMS_TMB_CALL_INCLUDE_MODULE_DIAGNOSTICS_POPULATION = if (isTRUE(module_flags$population)) "1" else "0"
)

get_rss_kb_raw <- function(samples = 3L) {
    readings <- rep(NA_real_, samples)
    for (sample_i in seq_len(samples)) {
        rss_lines <- suppressWarnings(
            system2("ps", c("-o", "rss=", "-p", as.character(Sys.getpid())), stdout = TRUE)
        )
        if (length(rss_lines) < 1) {
            next
        }

        parsed <- suppressWarnings(as.numeric(gsub("[^0-9]", "", trimws(rss_lines))))
        parsed <- parsed[is.finite(parsed) & parsed > 0]
        if (length(parsed) > 0) {
            readings[[sample_i]] <- max(parsed)
        }
    }

    valid <- readings[is.finite(readings) & readings > 0]
    if (length(valid) < 1) {
        return(NA_real_)
    }

    max(valid)
}

scalar_character_or_na <- function(x) {
    if (is.null(x) || length(x) < 1) {
        return(NA_character_)
    }
    as.character(x[[1]])
}

scalar_numeric_or_na <- function(x) {
    if (is.null(x) || length(x) < 1) {
        return(NA_real_)
    }
    suppressWarnings(as.numeric(x[[1]]))
}

results <- data.frame(
    iteration = integer(0),
    status = character(0),
    message = character(0),
    optimization_status = character(0),
    optimization_path = character(0),
    max_gradient_component = numeric(0),
    elapsed_sec = numeric(0),
    rss_kb_raw = numeric(0),
    rss_kb = numeric(0),
    delta_rss_kb = numeric(0),
    gc_used_mb = numeric(0),
    gc_max_used_mb = numeric(0),
    stringsAsFactors = FALSE
)

peak_rss_kb <- -Inf
peak_gc_used_mb <- -Inf
peak_gc_max_used_mb <- -Inf
peak_max_gradient_component <- -Inf
optimized_iterations <- 0L
start_rss_kb <- get_rss_kb_raw()
prev_rss_kb <- start_rss_kb

timestamp <- format(Sys.time(), "%Y%m%d-%H%M%S")
out_csv <- file.path("tests", paste0("rss_loop_tmb_call_", timestamp, ".csv"))

cat("Running", iterations, "in one R session sourcing", target_script, "\n")
cat("GC mode:", gc_mode, "\n")
cat("Wrapper fallback disabled: FIMS_TMB_CALL_USE_WRAPPER_FALLBACK=0\n")
cat("Native info calls enabled: FIMS_TMB_CALL_USE_NATIVE_INFO_CALLS=1\n")
cat("Diagnostic module:", diagnostic_module, "\n")

for (i in seq_len(iterations)) {
    gc_used_mb <- NA_real_
    gc_max_used_mb <- NA_real_

    if (gc_mode %in% c("before_each", "both")) {
        gc_before <- gc(verbose = FALSE)
        gc_before_snapshot <- gc_snapshot(gc_before)
        gc_used_mb <- gc_before_snapshot$used_mb
        gc_max_used_mb <- gc_before_snapshot$max_used_mb
    }

    status <- "ok"
    message <- ""
    optimization_status <- NA_character_
    optimization_path <- NA_character_
    max_gradient_component <- NA_real_
    t0 <- proc.time()[["elapsed"]]

    tryCatch(
        {
            run_env <- new.env(parent = globalenv())
            source(target_script, local = run_env, chdir = FALSE)

            if (exists("comparison", envir = run_env, inherits = FALSE)) {
                comparison_obj <- get("comparison", envir = run_env, inherits = FALSE)
                optimization_obj <- comparison_obj[["optimization"]]
                optimization_status <- scalar_character_or_na(optimization_obj[["status"]])
                optimization_path <- scalar_character_or_na(optimization_obj[["path"]])

                if (!is.null(optimization_obj[["gradient_diagnostics"]][["max_abs_gradient"]])) {
                    max_gradient_component <- scalar_numeric_or_na(
                        optimization_obj[["gradient_diagnostics"]][["max_abs_gradient"]]
                    )
                } else if (!is.null(optimization_obj[["gradient_max"]])) {
                    max_gradient_component <- scalar_numeric_or_na(optimization_obj[["gradient_max"]])
                } else if (!is.null(optimization_obj[["final_gradient"]])) {
                    max_gradient_component <- max(abs(as.numeric(optimization_obj[["final_gradient"]])))
                }
            }
        },
        error = function(e) {
            status <<- "error"
            message <<- conditionMessage(e)
        }
    )

    if (gc_mode %in% c("after_each", "both")) {
        gc_after <- gc(verbose = FALSE)
        gc_after_snapshot <- gc_snapshot(gc_after)
        gc_used_mb <- if (is.finite(gc_after_snapshot$used_mb)) gc_after_snapshot$used_mb else gc_used_mb
        gc_max_used_mb <- if (is.finite(gc_after_snapshot$max_used_mb)) gc_after_snapshot$max_used_mb else gc_max_used_mb
    }

    elapsed <- proc.time()[["elapsed"]] - t0
    rss_kb_raw <- get_rss_kb_raw()
    rss_kb <- if (is.finite(rss_kb_raw) && rss_kb_raw > 0) rss_kb_raw else prev_rss_kb
    delta_rss_kb <- if (is.finite(rss_kb) && is.finite(prev_rss_kb)) rss_kb - prev_rss_kb else NA_real_
    prev_rss_kb <- rss_kb

    if (is.finite(rss_kb)) {
        peak_rss_kb <- max(peak_rss_kb, rss_kb)
    }

    if (is.finite(gc_used_mb)) {
        peak_gc_used_mb <- max(peak_gc_used_mb, gc_used_mb)
    }

    if (is.finite(gc_max_used_mb)) {
        peak_gc_max_used_mb <- max(peak_gc_max_used_mb, gc_max_used_mb)
    }

    if (is.finite(max_gradient_component)) {
        peak_max_gradient_component <- max(peak_max_gradient_component, max_gradient_component)
    }

    if (!is.na(optimization_status) && identical(optimization_status, "ok")) {
        optimized_iterations <- optimized_iterations + 1L
    }

    results[nrow(results) + 1, ] <- list(
        i,
        status,
        message,
        optimization_status,
        optimization_path,
        max_gradient_component,
        elapsed,
        rss_kb_raw,
        rss_kb,
        delta_rss_kb,
        gc_used_mb,
        gc_max_used_mb
    )

    # Persist every iteration so data remains useful if a native crash terminates early.
    write.csv(results, out_csv, row.names = FALSE)

    cat(sprintf(
        "[%d/%d] status=%s opt_status=%s max_grad=%s elapsed=%.3fs rss_kb=%s raw_rss_kb=%s delta_rss_kb=%s gc_max_used_mb=%s\n",
        i,
        iterations,
        status,
        ifelse(is.na(optimization_status), "NA", optimization_status),
        ifelse(is.na(max_gradient_component), "NA", format(max_gradient_component, scientific = TRUE)),
        elapsed,
        ifelse(is.na(rss_kb), "NA", format(rss_kb, scientific = FALSE)),
        ifelse(is.na(rss_kb_raw), "NA", format(rss_kb_raw, scientific = FALSE)),
        ifelse(is.na(delta_rss_kb), "NA", format(delta_rss_kb, scientific = FALSE)),
        ifelse(is.na(gc_max_used_mb), "NA", format(gc_max_used_mb, scientific = FALSE))
    ))
    flush.console()
}

cat("\nSummary\n")
cat("- target:", target_script, "\n")
cat("- iterations:", iterations, "\n")
cat("- start_rss_kb:", start_rss_kb, "\n")
cat("- end_rss_kb:", prev_rss_kb, "\n")
cat("- peak_rss_kb:", if (is.finite(peak_rss_kb)) peak_rss_kb else NA_real_, "\n")
cat("- peak_rss_mb:", if (is.finite(peak_rss_kb)) round(peak_rss_kb / 1024, 3) else NA_real_, "\n")
cat("- peak_gc_used_mb:", if (is.finite(peak_gc_used_mb)) peak_gc_used_mb else NA_real_, "\n")
cat("- peak_gc_max_used_mb:", if (is.finite(peak_gc_max_used_mb)) peak_gc_max_used_mb else NA_real_, "\n")
cat("- optimized_iterations:", optimized_iterations, "\n")
cat("- peak_max_gradient_component:", if (is.finite(peak_max_gradient_component)) peak_max_gradient_component else NA_real_, "\n")
cat("- rss_growth_kb:", if (is.finite(start_rss_kb) && is.finite(prev_rss_kb)) prev_rss_kb - start_rss_kb else NA_real_, "\n")
cat("- error_iterations:", sum(results$status != "ok", na.rm = TRUE), "\n")
cat("- output_csv:", out_csv, "\n")
