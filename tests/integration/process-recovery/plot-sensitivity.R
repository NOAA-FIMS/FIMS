# Rscript tests/integration/process-recovery/plot-sensitivity.R OUTPUT_DIRECTORY
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) == 1L)
output <- args[[1L]]
best <- read.csv(file.path(output, "best.csv"))
profile <- best[best$replicate == 1 & !is.na(best$profile_sd) & best$valid, ]
stopifnot(nrow(profile) > 0L)
png(file.path(output, "mortality-sd-profiles.png"), width = 1400, height = 1000,
    res = 140, type = if (Sys.info()[["sysname"]] == "Darwin") "quartz" else getOption("bitmapType"))
par(mfrow = c(length(unique(profile$years)), 2), mar = c(4.2, 4.3, 3, 1),
    oma = c(0, 0, 1.8, 0), las = 1)
for (years in sort(unique(profile$years))) for (initial in c("fixed", "normal")) {
  rows <- profile[profile$years == years & profile$initialization == initial, ]
  rows <- rows[order(rows$profile_sd), ]
  free <- best[best$years == years & best$initialization == initial &
                 best$replicate == 1 & is.na(best$profile_sd) & best$valid, ]
  plot(rows$profile_sd, rows$delta_nll, type = "b", log = "x", pch = 16,
    col = "#145b80", xlim = range(c(rows$profile_sd, free$M_sd)),
    ylim = range(c(0, rows$delta_nll, free$delta_nll)),
    xlab = "Mortality innovation SD", ylab = "Relative negative log likelihood",
    main = paste(years, "years /", if (initial == "fixed") "known initial anchors" else "proper initial prior"))
  abline(v = 0.05, lty = 2, col = "#555555")
  if (nrow(free)) points(free$M_sd, free$delta_nll, pch = 17, cex = 1.2, col = "#a13b28")
  legend("topleft", legend = c("Reoptimized profile grid", "Free M-SD fit", "True SD = 0.05"),
    col = c("#145b80", "#a13b28", "#555555"), pch = c(16, 17, NA),
    lty = c(1, NA, 2), bty = "n", cex = .75)
}
mtext("First replicate only; coarse profile grid, no confidence cutoff", outer = TRUE, cex = 1)
invisible(dev.off())
