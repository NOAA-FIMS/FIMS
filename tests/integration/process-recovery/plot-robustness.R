# Rscript tests/integration/process-recovery/plot-robustness.R OUTPUT_DIRECTORY
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) == 1L)
output <- args[[1L]]
configuration <- dget(file.path(output, "configuration.R"))
metrics <- read.csv(file.path(output, "state_metrics.csv"))
diagnostics <- read.csv(file.path(output, "diagnostics.csv"))
status <- diagnostics[c("replicate", "scenario", "sd_mode", "true_M_sd", "optimizer_valid")]
scored <- merge(metrics, status, by = c("replicate", "scenario", "sd_mode", "true_M_sd"))
scored <- scored[scored$process == "M" & !is.na(scored$optimizer_valid) & scored$optimizer_valid, ]
groups <- split(scored, interaction(scored$scenario, scored$sd_mode, drop = TRUE))
rows <- do.call(rbind, lapply(groups, function(r) {
  available <- is.finite(r$coverage)
  data.frame(scenario = r$scenario[1], sd_mode = r$sd_mode[1],
    mean_coverage = if (any(available)) mean(r$coverage[available]) else NA_real_,
    coverage_mcse = if (sum(available) > 1L) sd(r$coverage[available]) / sqrt(sum(available)) else NA_real_,
    mean_width = if (any(is.finite(r$mean_width))) mean(r$mean_width, na.rm = TRUE) else NA_real_)
}))
stopifnot(nrow(rows) > 0L)
colors <- c(known = "#145b80", estimated = "#a13b28")
bitmap <- if (Sys.info()[["sysname"]] == "Darwin") "quartz" else getOption("bitmapType")
png(file.path(output, "composition-misspecification.png"), width = 1400, height = 650, res = 140, type = bitmap)
par(mfrow = c(1, 2), mar = c(4.5, 4.5, 3, 1), oma = c(0, 0, 1.8, 0), las = 1)
for (quantity in c("mean_coverage", "mean_width")) {
  ylim <- if (quantity == "mean_coverage") c(0, 1) else range(rows$mean_width, finite = TRUE)
  plot(NA, xlim = range(configuration$design$variance_inflation) * c(.9, 1.1),
    ylim = ylim, log = "x", xaxt = "n", xlab = "True composition variance inflation",
    ylab = if (quantity == "mean_coverage") "Mean pointwise M coverage" else "Mean 95% interval width (log M)",
    main = if (quantity == "mean_coverage") "Nominal 95% fitted-state intervals" else "Mortality interval width")
  axis(1, at = configuration$design$variance_inflation,
    labels = format(round(configuration$design$variance_inflation, 2), nsmall = 2))
  if (quantity == "mean_coverage") abline(h = .95, lty = 2, col = "#555555")
  for (mode in names(colors)) {
    r <- rows[rows$sd_mode == mode, ]
    r$inflation <- configuration$design$variance_inflation[match(r$scenario, configuration$design$scenario)]
    r <- r[order(r$inflation), ]
    x <- r$inflation * if (mode == "known") .99 else 1.01
    lines(x, r[[quantity]], type = "b", pch = if (mode == "known") 16 else 17, col = colors[[mode]])
    if (quantity == "mean_coverage") {
      for (j in which(is.finite(r$coverage_mcse) & r$coverage_mcse > 0))
        arrows(x[j], max(0, r$mean_coverage[j] - r$coverage_mcse[j]),
          x[j], min(1, r$mean_coverage[j] + r$coverage_mcse[j]), angle = 90, code = 3, length = .04, col = colors[[mode]])
    }
  }
  if (quantity == "mean_coverage") {
    missing <- setdiff(configuration$design$scenario, rows$scenario[rows$sd_mode == "estimated" & is.finite(rows$mean_coverage)])
    for (name in missing) text(configuration$design$variance_inflation[configuration$design$scenario == name],
      .8, labels = paste("Estimated-SD intervals unavailable:", name),
      pos = 2, cex = .7, col = colors[["estimated"]])
  }
  if (quantity == "mean_coverage") legend("bottomleft", legend = c("Known process SDs", "Estimated process SDs", "Nominal 0.95"),
    col = c(colors, "#555555"), lty = c(1, 1, 2), pch = c(16, 17, NA), bty = "n", cex = .8)
}
mtext("Optimizer-valid reports; one replicate-level MCSE; every fit assumes multinomial size 2000", outer = TRUE, cex = .9)
invisible(dev.off())

metrics <- read.csv(file.path(output, "state_metrics.csv"))
baseline <- metrics[metrics$scenario == "multinomial", setdiff(names(metrics), "scenario")]
changed <- metrics[metrics$scenario != "multinomial", ]
pairs <- merge(changed, baseline, by = c("true_M_sd", "replicate", "sd_mode", "process"), suffixes = c("_changed", "_baseline"))
diagnostics <- read.csv(file.path(output, "diagnostics.csv"))
status <- diagnostics[c("replicate", "scenario", "sd_mode", "optimizer_valid")]
pairs <- merge(pairs, status, by = c("replicate", "scenario", "sd_mode"), all.x = TRUE)
names(pairs)[names(pairs) == "optimizer_valid"] <- "changed_valid"
control <- status[status$scenario == "multinomial", c("replicate", "sd_mode", "optimizer_valid")]
pairs <- merge(pairs, control, by = c("replicate", "sd_mode"), all.x = TRUE)
names(pairs)[names(pairs) == "optimizer_valid"] <- "control_valid"
groups <- split(pairs, interaction(pairs$scenario, pairs$sd_mode, pairs$process, drop = TRUE))
paired <- do.call(rbind, lapply(groups, function(r) {
  difference <- r$coverage_changed - r$coverage_baseline
  computed <- is.finite(difference)
  available <- computed & !is.na(r$changed_valid) & r$changed_valid & !is.na(r$control_valid) & r$control_valid
  data.frame(scenario = r$scenario[1], sd_mode = r$sd_mode[1], process = r$process[1],
    planned_pairs = configuration$replicates, computed_pairs = nrow(r), numerical_interval_pairs = sum(computed), coverage_pairs = sum(available),
    all_computed_difference = if (any(computed)) mean(difference[computed]) else NA_real_,
    coverage_difference = if (any(available)) mean(difference[available]) else NA_real_,
    difference_mcse = if (sum(available) > 1L) sd(difference[available]) / sqrt(sum(available)) else NA_real_,
    rmse_difference = if (any(available)) mean((r$rmse_changed - r$rmse_baseline)[available]) else NA_real_)
}))
write.csv(paired, file.path(output, "paired-summary.csv"), row.names = FALSE)
