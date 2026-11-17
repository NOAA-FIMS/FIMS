# Input directory containing the new profile CSVs; creates two standalone PNGs.
args <- commandArgs(trailingOnly = TRUE); stopifnot(length(args) == 1L)
directory <- args[[1L]]
profiles <- read.csv(file.path(directory, "profiles.csv"))
points <- read.csv(file.path(directory, "points.csv"))
refits <- read.csv(file.path(directory, "refits.csv"))
colors <- c("#0072B2", "#D55E00", "#009E73", "#CC79A7")
png(file.path(directory, "M-sd-profiles.png"), width = 1800, height = 1300, res = 160)
par(mfrow = c(2, 3), mar = c(4.5, 4.5, 3.5, 1))
for (rep in 1:5) {
  x <- profiles[profiles$replicate == rep, ]
  valid <- x$optimizer_valid & is.finite(x$delta_nll)
  top <- if (any(valid)) max(1, max(x$delta_nll[valid]) * 1.05) else 1
  plot(NA, xlim = range(x$profile_sd), ylim = c(0, top), log = "x",
    xlab = "Fixed M innovation SD", ylab = "Delta NLL (local envelope)",
    main = paste("Dataset", rep))
  mtext(sprintf("%d/%d valid profile attempts", sum(valid), nrow(x)), side = 3, line = .3, cex = .7)
  abline(v = .05, col = "grey65", lty = 3)
  for (i in 1:2) {
    z <- x[x$start == c("saved", "cold")[i], ]; z <- z[order(z$profile_sd), ]
    lines(z$profile_sd, z$delta_nll, type = "b", pch = 15 + i, col = colors[i], lty = i)
  }
  free <- refits[refits$replicate == rep & refits$case == "estimated_dm" & refits$optimizer_valid, ]
  if (nrow(free) && any(valid)) {
    best <- free[which.min(free$objective), ]
    points(best$M_sd, best$objective - x$reference_objective[1], pch = 18, cex = 1.4)
  }
}
plot.new(); legend("center", legend = c("Saved latent states", "Cold latent states", "Best valid free fit", "True M SD = .05"),
  col = c(colors[1:2], "black", "grey65"), pch = c(16, 17, 18, NA), lty = c(1, 2, NA, 3), bty = "n")
dev.off()
png(file.path(directory, "concentration-traces.png"), width = 1800, height = 1300, res = 160)
par(mfrow = c(2, 3), mar = c(5.5, 5, 4.5, 1), xpd = FALSE)
concentrations <- points[points$stage == "profile" & grepl("concentration/", points$parameter), ]
owners <- sort(unique(concentrations$parameter))
for (rep in 1:5) {
  x <- concentrations[concentrations$replicate == rep, ]
  plot(NA, xlim = range(profiles$profile_sd), ylim = range(concentrations$estimate), log = "xy",
    axes = FALSE, ann = FALSE)
  abline(h = 200, col = "grey65", lty = 3)
  for (j in seq_along(owners)) for (i in 1:2) {
    z <- x[x$parameter == owners[j] & x$start == c("saved", "cold")[i], ]
    z <- merge(z, profiles[profiles$replicate == rep, c("profile_sd", "start", "optimizer_valid")],
      by.x = c("fixed_M_sd", "start"), by.y = c("profile_sd", "start"))
    z <- z[order(z$fixed_M_sd), ]; z$estimate[!z$optimizer_valid] <- NA_real_
    lines(z$fixed_M_sd, z$estimate, type = "b", pch = 15 + i, col = colors[j], lty = i)
  }
  axis(1, at = c(.001, .01, .1, .4), labels = c("0.001", "0.01", "0.1", "0.4"), xpd = NA)
  axis(2, xpd = NA); box()
  mtext(paste("Dataset", rep), side = 3, line = 1.5, font = 2, cex = 1, xpd = NA)
  mtext("Fixed M innovation SD", side = 1, line = 3, cex = .85, xpd = NA)
  mtext("Reoptimized concentration", side = 2, line = 3.5, cex = .85, xpd = NA)
}
labels <- sub("concentration/", "", owners, fixed = TRUE)
labels <- sub("fleet1/", "Fleet: ", labels, fixed = TRUE)
labels <- sub("survey1/", "Survey: ", labels, fixed = TRUE)
labels <- sub("_comp", "", labels, fixed = TRUE)
plot.new(); legend("center", legend = c(labels, "True concentration: 200"),
  col = c(colors, "grey65"), lty = c(rep(1, length(owners)), 3), bty = "n", cex = .8)
text(.5, .15, "Solid/circles: saved states\nDashed/triangles: cold states", cex = .8)
dev.off()
