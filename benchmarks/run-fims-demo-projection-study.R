#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = TRUE)
max_years <- if (length(args) >= 1L) as.integer(args[[1]]) else 15L
max_iterations <- if (length(args) >= 2L) as.integer(args[[2]]) else 1L
gradient_tolerance <- if (length(args) >= 3L) as.numeric(args[[3]]) else 1e-5
output_csv <- if (length(args) >= 4L) {
  args[[4]]
} else {
  file.path("benchmarks", "fims-demo-projection-study.csv")
}
requested_backends <- if (length(args) >= 5L) {
  strsplit(args[[5]], ",", fixed = TRUE)[[1]]
} else {
  c("quadra", "tmb")
}
start_year <- if (length(args) >= 6L) as.integer(args[[6]]) else 1L

if (is.na(max_years) || max_years < 1L ||
    is.na(start_year) || start_year < 1L || start_year > max_years ||
    is.na(max_iterations) || max_iterations < 1L ||
    !is.finite(gradient_tolerance) || gradient_tolerance <= 0) {
  stop(
    "Usage: Rscript SCRIPT [max-years>=1] [max-iterations>=1] ",
    "[gradient-tolerance>0] [output.csv]"
  )
}

scripts <- c(
  quadra = file.path("benchmarks", "fims-demo-projections-quadra-laplace.R"),
  tmb = file.path("benchmarks", "fims-demo-projections-tmb-laplace.R")
)
if (!all(requested_backends %in% names(scripts))) {
  stop("Backends must be a comma-separated subset of: quadra,tmb.")
}
scripts <- scripts[requested_backends]
if (any(!file.exists(scripts))) {
  stop("Run this study from the FIMS repository root.")
}

output_dir <- dirname(output_csv)
dir.create(output_dir, recursive = TRUE, showWarnings = FALSE)
artifact_dir <- file.path(
  output_dir,
  paste0(tools::file_path_sans_ext(basename(output_csv)), "-artifacts")
)
dir.create(artifact_dir, recursive = TRUE, showWarnings = FALSE)

extract_number <- function(lines, pattern) {
  hit <- grep(pattern, lines, value = TRUE)
  if (!length(hit)) return(NA_real_)
  as.numeric(strsplit(trimws(hit[[1]]), "[[:space:]]+")[[1]][[1]])
}

run_one <- function(backend, years) {
  stem <- sprintf("%s-years-%02d", backend, years)
  result_file <- file.path(artifact_dir, paste0(stem, ".rds"))
  log_file <- file.path(artifact_dir, paste0(stem, ".log"))
  command_args <- c(
    "-l",
    file.path(R.home("bin"), "Rscript"),
    scripts[[backend]],
    years,
    result_file,
    max_iterations,
    format(gradient_tolerance, scientific = TRUE)
  )

  started <- Sys.time()
  status <- system2(
    "/usr/bin/time",
    args = command_args,
    stdout = log_file,
    stderr = log_file
  )
  wall_seconds <- as.numeric(difftime(Sys.time(), started, units = "secs"))
  log <- if (file.exists(log_file)) readLines(log_file, warn = FALSE) else character()
  fit <- if (identical(status, 0L) && file.exists(result_file)) {
    tryCatch(readRDS(result_file), error = function(error) NULL)
  } else {
    NULL
  }

  data.frame(
    backend = backend,
    projection_years = years,
    status = if (!is.null(fit)) "completed" else "failed",
    exit_code = status,
    wall_seconds = wall_seconds,
    timed_real_seconds = extract_number(log, " real[[:space:]]"),
    max_rss_bytes = extract_number(log, "maximum resident set size"),
    peak_memory_footprint_bytes = extract_number(log, "peak memory footprint"),
    initial_objective = if (!is.null(fit)) fit$initial_objective else NA_real_,
    objective = if (!is.null(fit)) fit$objective else NA_real_,
    gradient_norm = if (!is.null(fit)) fit$gradient_norm else NA_real_,
    fixed_effects = if (!is.null(fit)) length(fit$fixed) else NA_integer_,
    random_effects = if (!is.null(fit) && !is.null(fit$random)) length(fit$random) else NA_integer_,
    iterations = if (!is.null(fit)) fit$iterations else NA_integer_,
    evaluations = if (!is.null(fit)) paste(fit$evaluations, collapse = ";") else NA_character_,
    converged = if (!is.null(fit)) fit$converged else NA,
    message = if (!is.null(fit)) fit$message else paste(tail(log, 3L), collapse = " | "),
    log_file = log_file,
    result_file = if (file.exists(result_file)) result_file else NA_character_,
    stringsAsFactors = FALSE
  )
}

results <- list()
index <- 0L
for (years in seq.int(start_year, max_years)) {
  for (backend in names(scripts)) {
    cat(sprintf("Running %-6s projection year %d... ", backend, years))
    index <- index + 1L
    results[[index]] <- run_one(backend, years)
    cat(results[[index]]$status, "\n")
    utils::write.csv(do.call(rbind, results), output_csv, row.names = FALSE)
  }
}

results <- do.call(rbind, results)
print(results[, c(
  "backend", "projection_years", "status", "exit_code",
  "wall_seconds", "max_rss_bytes", "objective", "gradient_norm"
)], row.names = FALSE)
cat("\nResults:", normalizePath(output_csv, mustWork = FALSE), "\n")
