# Rscript tests/integration/process-recovery/plot-calibration.R OUTPUT_DIRECTORY
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) == 1L)
output <- args[[1L]]
summary <- read.csv(file.path(output, "state_summary.csv"))
configuration <- dget(file.path(output, "configuration.R"))
rows <- summary[summary$process == "M", ]
stopifnot(nrow(rows) > 0L)
curves <- data.frame(mode = c("known", "estimated", "estimated"),
  stratum = c("all", "all", "regular"), color = c("#145b80", "#a13b28", "#387542"),
  label = c("Known SDs", "Estimated SDs: all", "Estimated SDs: regular subset"))
bitmap <- if (Sys.info()[["sysname"]] == "Darwin") "quartz" else getOption("bitmapType")
png(file.path(output, "mortality-interval-calibration.png"), width = 1400, height = 650, res = 140, type = bitmap)
par(mfrow = c(1, 2), mar = c(4.3, 4.5, 3, 1), oma = c(0, 0, 1.7, 0), las = 1)
for (quantity in c("mean_coverage", "mean_width")) {
  coverage_min <- min(c(rows$mean_coverage, rows$mean_coverage - rows$coverage_mcse), na.rm = TRUE)
  limit <- if (quantity == "mean_coverage")
    c(max(0, floor((coverage_min - .025) * 20) / 20), 1) else range(rows$mean_width, finite = TRUE)
  plot(NA, xlim = range(configuration$mortality_sds) * c(.9, 1.1), ylim = limit,
    log = "x", xaxt = "n", xlab = "True mortality innovation SD",
    ylab = if (quantity == "mean_coverage") "Mean pointwise coverage" else "Mean 95% interval width (log M)",
    main = if (quantity == "mean_coverage") "Nominal 95% state intervals (zoomed)" else "Mortality interval width")
  axis(1, at = configuration$mortality_sds, labels = configuration$mortality_sds)
  if (quantity == "mean_coverage") abline(h = .95, lty = 2, col = "#555555")
  for (i in seq_len(nrow(curves))) {
    r <- rows[rows$sd_mode == curves$mode[i] & rows$stratum == curves$stratum[i], ]
    r <- r[order(r$true_M_sd), ]
    x <- r$true_M_sd * exp((i - 2) * .025)
    lines(x, r[[quantity]], type = "b", pch = 15 + i, col = curves$color[i])
    if (quantity == "mean_coverage") {
      available <- is.finite(r$mean_coverage) & is.finite(r$coverage_mcse) & r$coverage_mcse > 0
      for (j in which(available)) arrows(x[j], max(0, r$mean_coverage[j] - r$coverage_mcse[j]),
        x[j], min(1, r$mean_coverage[j] + r$coverage_mcse[j]),
        angle = 90, code = 3, length = .04, col = curves$color[i])
    }
  }
  if (quantity == "mean_coverage") legend("bottomleft", legend = c(curves$label, "Nominal 0.95"),
    col = c(curves$color, "#555555"), lty = c(1, 1, 1, 2), pch = c(16, 17, 18, NA), bty = "n", cex = .8)
}
mtext("Error bars: one replicate-level MCSE; regular fits are a selected subset, not a calibration guarantee", outer = TRUE, cex = .9)
invisible(dev.off())

# Paired differences use the same dataset, so their MCSE does not assume the
# known/estimated coverage estimates are independent.
metrics <- read.csv(file.path(output, "state_metrics.csv"))
known <- metrics[metrics$sd_mode == "known", ]
estimated <- metrics[metrics$sd_mode == "estimated", ]
pairs <- merge(known, estimated, by = c("true_M_sd", "replicate", "process"),
  suffixes = c("_known", "_estimated"))
groups <- split(pairs, interaction(pairs$true_M_sd, pairs$process, drop = TRUE))
paired_summary <- do.call(rbind, lapply(groups, function(r) {
  difference <- r$coverage_estimated - r$coverage_known
  available <- is.finite(difference)
  data.frame(true_M_sd = r$true_M_sd[1], process = r$process[1],
    planned_pairs = configuration$replicates, computed_pairs = nrow(r), coverage_pairs = sum(available),
    mean_coverage_difference = if (any(available)) mean(difference[available]) else NA_real_,
    difference_mcse = if (sum(available) > 1L) sd(difference[available]) / sqrt(sum(available)) else NA_real_,
    mean_rmse_difference = mean(r$rmse_estimated - r$rmse_known))
}))
write.csv(paired_summary, file.path(output, "paired-summary.csv"), row.names = FALSE)
