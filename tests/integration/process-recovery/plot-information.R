# Rscript tests/integration/process-recovery/plot-information.R OUTPUT_DIRECTORY
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) == 1L)
output <- args[[1L]]
best <- read.csv(file.path(output, "best.csv"))
configuration <- dget(file.path(output, "configuration.R"))
designs <- configuration$design$design
bitmap <- if (Sys.info()[["sysname"]] == "Darwin") "quartz" else getOption("bitmapType")
free <- best[is.na(best$profile_sd), ]
colors <- c("#145b80", "#a13b28", "#387542", "#8054a1", "#806326")
png(file.path(output, "mortality-sd-by-design.png"), width = 1400, height = 650, res = 140, type = bitmap)
par(mfrow = c(1, 2), mar = c(4.5, 4.5, 3, 1), oma = c(0, 0, 1.5, 0), las = 1)
for (initial in c("fixed", "normal")) {
  rows <- free[free$initialization == initial, ]
  plot(NA, xlim = c(1, length(designs)), ylim = range(c(.001, .05, rows$M_sd[rows$valid])),
    log = "y", xaxt = "n", xlab = "Observation design", ylab = "Estimated mortality innovation SD",
    main = if (initial == "fixed") "Known initial anchors" else "Proper initial prior")
  axis(1, at = seq_along(designs), labels = designs)
  abline(h = .05, lty = 2, col = "#555555")
  for (replicate in sort(unique(rows$replicate))) {
    r <- rows[rows$replicate == replicate, ]
    y <- r$M_sd[match(designs, r$design)]
    y[!r$valid[match(designs, r$design)]] <- NA_real_
    lines(seq_along(designs), y, type = "b", pch = 15 + replicate %% 5,
      col = colors[(replicate - 1L) %% length(colors) + 1L])
  }
  legend("bottomright", legend = c(paste("Dataset", sort(unique(rows$replicate))), "True SD = 0.05"),
    col = c(colors[(seq_along(unique(rows$replicate)) - 1L) %% length(colors) + 1L], "#555555"),
    lty = c(rep(1, length(unique(rows$replicate))), 2), bty = "n", cex = .8)
}
mtext("Paired trajectories and sampling errors; invalid fits omitted from lines", outer = TRUE, cex = .95)
invisible(dev.off())

profile <- best[best$replicate == 1 & !is.na(best$profile_sd) & best$valid, ]
if (nrow(profile)) {
  png(file.path(output, "mortality-information-profiles.png"), width = 1400, height = 650, res = 140, type = bitmap)
  par(mfrow = c(1, 2), mar = c(4.3, 4.5, 3, 1), oma = c(0, 0, 1.5, 0), las = 1)
  for (initial in c("fixed", "normal")) {
    rows <- profile[profile$initialization == initial, ]
    optima <- free[free$replicate == 1 & free$initialization == initial &
      free$design %in% configuration$profile_designs & free$valid, ]
    plot(NA, xlim = range(c(rows$profile_sd, optima$M_sd)),
      ylim = range(c(0, rows$delta_nll, optima$delta_nll)), log = "x",
      xlab = "Mortality innovation SD", ylab = "Relative negative log likelihood",
      main = if (initial == "fixed") "Known initial anchors" else "Proper initial prior")
    for (i in seq_along(configuration$profile_designs)) {
      name <- configuration$profile_designs[i]
      r <- rows[rows$design == name, ]
      r <- r[order(r$profile_sd), ]
      lines(r$profile_sd, r$delta_nll, type = "b", pch = 16, col = colors[i])
      optimum <- optima[optima$design == name, ]
      if (nrow(optimum)) points(optimum$M_sd, optimum$delta_nll, pch = 17, col = colors[i], cex = 1.2)
    }
    abline(v = .05, lty = 2, col = "#555555")
    legend("topleft", legend = c(configuration$profile_designs, "True SD = 0.05"),
      col = c(colors[seq_along(configuration$profile_designs)], "#555555"),
      lty = c(rep(1, length(configuration$profile_designs)), 2), bty = "n", cex = .8)
  }
  mtext("Dataset 1 only; separate likelihood references; triangles mark free optima; no confidence cutoff", outer = TRUE, cex = .95)
  invisible(dev.off())
}
