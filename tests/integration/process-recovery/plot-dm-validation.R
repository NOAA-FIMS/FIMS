# From repository root: input/result directory and optional output PNG.
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) >= 1L)
directory <- args[[1L]]
output <- if (length(args) >= 2L) args[[2L]] else file.path(directory, "dm-validation.png")
metrics <- read.csv(file.path(directory, "metrics.csv"))
points <- read.csv(file.path(directory, "points.csv"))
scenarios <- c("multinomial", "moderate", "strong")
cases <- c("multinomial", "known_dm", "estimated_dm")
colors <- c("#333333", "#0072B2", "#D55E00")
png(output, width = 1800, height = 1200, res = 160)
par(mfrow = c(2, 2), mar = c(4.2, 4.3, 3, 1))
for (metric in c("coverage", "rmse")) {
  x <- metrics[metrics$process == "M" & metrics$eligible, ]
  ylim <- if (metric == "coverage") c(0, 1) else c(0, max(x[[metric]], na.rm = TRUE) * 1.05)
  plot(NA, xlim = c(.7, 3.3), ylim = ylim, xaxt = "n", xlab = "Generating composition errors",
    ylab = if (metric == "coverage") "Pointwise log-M coverage" else "Log-M RMSE",
    main = "Eligible warm reports; dots are datasets")
  axis(1, at = 1:3, labels = c("Multinomial", "DM 2000", "DM 200"))
  if (metric == "coverage") abline(h = .95, lty = 2, col = "grey60")
  for (i in seq_along(cases)) for (j in seq_along(scenarios)) {
    z <- x[x$case == cases[i] & x$scenario == scenarios[j], ]
    if (nrow(z)) points(rep(j + (i - 2) * .08, nrow(z)), z[[metric]], pch = 16, col = colors[i])
  }
  if (metric == "coverage") legend("bottomleft", legend = c("Multinomial", "Known DM", "Estimated DM"), col = colors, pch = 16, bty = "n", cex = .8)
}
x <- points[points$case == "estimated_dm" & grepl("concentration/", points$parameter) & points$eligible, ]
plot(NA, xlim = c(.7, 3.3), ylim = c(10, 1e5), log = "y", xaxt = "n", xlab = "Generating composition errors",
  ylab = "Estimated concentration (log scale)", main = "Four separate composition concentrations / dataset")
axis(1, at = 1:3, labels = c("Multinomial", "DM 2000", "DM 200"))
for (j in seq_along(scenarios)) {
  z <- x[x$scenario == scenarios[j], ]
  if (nrow(z)) points(rep(j, nrow(z)), z$estimate, pch = ifelse(z$boundary, 4, 16), col = colors[3])
}
points(2:3, c(2000, 200), pch = 18, cex = 1.5, col = colors[2])
abline(h = 1e5, lty = 3)
legend("bottomleft", legend = c("Estimate", "Bound estimate", "Finite truth"), col = c(colors[3], colors[3], colors[2]), pch = c(16, 4, 18), bty = "n", cex = .8)
x <- points[points$parameter == "M" & points$eligible, ]
plot(NA, xlim = c(.7, 3.3), ylim = c(.001, 1), log = "y", xaxt = "n", xlab = "Generating composition errors",
  ylab = "Estimated M innovation SD (log scale)", main = "Local state reports can coexist with variance boundaries")
axis(1, at = 1:3, labels = c("Multinomial", "DM 2000", "DM 200")); abline(h = .05, lty = 2)
for (i in seq_along(cases)) for (j in seq_along(scenarios)) {
  z <- x[x$case == cases[i] & x$scenario == scenarios[j], ]
  if (nrow(z)) points(rep(j + (i - 2) * .08, nrow(z)), z$estimate,
    pch = ifelse(z$boundary, 4, 16), col = colors[i])
}
dev.off()
