args <- commandArgs(TRUE)
input <- args[1]
output <- args[2]
dir.create(output, recursive = TRUE, showWarnings = FALSE)
runs <- jsonlite::fromJSON(file.path(input, "runs.json"))
stopifnot(nrow(runs) == 8L, all(runs$exit_code == 0L))
validation <- list()
selected <- c("spawning_biomass", "biomass", "expected_recruitment",
              "catch_expected", "index_expected")
for (years in unique(runs$years)) {
  reference <- readRDS(file.path(input, sprintf("%d-1-all.rds", years)))
  for (i in which(runs$years == years)) {
    result <- readRDS(file.path(input, paste0(runs$label[i], ".rds")))
    expected <- reference$uncertainty
    if (runs$variant[i] == "selected") {
      expected <- expected[rownames(expected) %in% selected, , drop = FALSE]
    }
    checks <- c(
      objective = isTRUE(all.equal(result$value, reference$value, tolerance=1e-10)),
      gradient = isTRUE(all.equal(result$gradient, reference$gradient, tolerance=1e-10)),
      ordinary_reports = isTRUE(all.equal(result$report, reference$report, tolerance=1e-10)),
      uncertainty = isTRUE(all.equal(result$uncertainty, expected, tolerance=1e-8)),
      parameter_covariance = isTRUE(all.equal(result$parameter_covariance,
                                             reference$parameter_covariance, tolerance=1e-10))
    )
    stopifnot(all(checks))
    validation[[runs$label[i]]] <- data.frame(run=runs$label[i], check=names(checks), passed=checks)
    runs$uncertainty_seconds[i] <- result$time[["elapsed"]]
    runs$reported_values[i] <- nrow(result$uncertainty)
  }
}
summary <- aggregate(cbind(peak_rss_mib, uncertainty_seconds, reported_values) ~
                       years + variant, data=runs, FUN=median)
write.csv(runs, file.path(output,"runs.csv"),row.names=FALSE)
write.csv(summary, file.path(output,"summary.csv"),row.names=FALSE)
write.csv(do.call(rbind,validation),file.path(output,"validation.csv"),row.names=FALSE)
print(summary)
