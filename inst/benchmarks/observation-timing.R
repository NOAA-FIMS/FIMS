# Run with an installed FIMS library, e.g.:
# R_LIBS=/path/to/library Rscript inst/benchmarks/observation-timing.R results.csv
# Compare libraries built with the same compiler flags on the same machine.
library(FIMS)
args <- commandArgs(TRUE)
outfile <- if (length(args)) args[1] else "observation-timing-benchmark.csv"
results <- list()
for (scenario in c("index_only", "shared_index_length")) {
  d <- data_big |>
    dplyr::filter(is.na(timing) | timing <= 3 | (type == "weight_at_age" & timing == 4)) |>
    dplyr::filter(type != "age_to_length_conversion")
  d$timing <- ifelse(is.na(d$timing), NA_character_, sprintf("%04d", d$timing + 2026L))
  source <- d[d$fleet == "survey1" & d$timing %in% "2028" &
    d$type %in% if (scenario == "index_only") "index" else c("index", "length_comp"), ]
  extras <- lapply(seq(10, 340, length.out = 24), function(day) {
    x <- source
    x$timing <- format(as.Date("2028-01-01") + round(day), "%Y-%m-%d")
    x
  })
  d <- dplyr::bind_rows(d, extras)
  # Two fleets share every date but retain separate observation predictions.
  second <- d[d$fleet == "survey1", ]
  second$fleet <- "survey2"
  f <- FIMSFrame(dplyr::bind_rows(d, second))
  p <- setup_default_parameters(f) |>
    dplyr::filter(module_name != "Growth") |>
    dplyr::bind_rows(setup_default_Growth(f, module_type = "VonBertalanffySchnute"))
  p$estimation_type <- "constant"
  p$distribution[p$module_name == "Recruitment"] <- NA_character_
  p$distribution_type[p$module_name == "Recruitment"] <- NA_character_
  p$estimation_type[p$module_name == "Fleet" & p$fleet == "survey1" & p$label == "log_q"] <- "fixed_effects"
  setup <- system.time(fit <- fit_fims(initialize_fims(p, f), optimize = FALSE))[["elapsed"]]
  obj <- get_obj(fit)
  invisible(obj$fn(obj$par))
  invisible(obj$gr(obj$par))
  evaluation <- median(replicate(3, system.time(for (i in seq_len(20)) {
    obj$fn(obj$par)
    obj$gr(obj$par)
  })[["elapsed"]] / 20))
  reporting <- median(replicate(3, system.time(obj$report())[["elapsed"]]))
  report <- get_report(fit)
  work <- report$observation_work
  results[[scenario]] <- data.frame(
    scenario,
    dates = nrow(report$observation_biology) / get_n_ages(f),
    size_rows = if (is.null(work)) NA_real_ else unname(work[2]),
    setup_seconds = setup, objective_gradient_seconds = evaluation,
    report_seconds = reporting, objective = as.numeric(obj$fn(obj$par))
  )
  clear()
}
result <- dplyr::bind_rows(results)
write.csv(result, outfile, row.names = FALSE)
print(result)
