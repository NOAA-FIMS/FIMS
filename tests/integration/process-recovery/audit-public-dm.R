# Repository root: completed output directory; optional "regenerate" reruns generation.
args <- commandArgs(trailingOnly = TRUE); stopifnot(length(args) %in% 1:2)
x <- readRDS(file.path(args[[1]], "result.rds"))
a <- x$tables$attempts; r <- x$tables$reports; m <- x$tables$metrics; p <- x$tables$points
stopifnot(nrow(r) == 20L, nrow(m) == 60L, nrow(x$tables$checks) == 10L,
  sum(a$stage == "initial") == 80L, length(x$inputs) == 10L)
groups <- split(a, interaction(a$replicate, a$scenario, a$case, drop = TRUE))
for (z in groups) {
  stopifnot(sum(z$selected) == 1L, sum(z$stage == "initial") == 4L)
  if (any(z$stage != "initial")) stopifnot(!any(z$optimizer_valid[z$stage == "initial"]),
    identical(sort(z$stage[z$stage != "initial"]), c("refine_original", "refine_tight")))
}
for (field in c("source_md5", "production_md5")) {
  hashes <- x$configuration[[field]]
  stopifnot(identical(unname(hashes), unname(tools::md5sum(names(hashes)))))
}
for (rep in seq_len(x$configuration$replicates)) {
  moderate <- x$inputs[[paste(rep, "moderate", sep = "/")]]
  strong <- x$inputs[[paste(rep, "strong", sep = "/")]]
  stopifnot(identical(moderate$truth, strong$truth))
  idx <- moderate$data$type %in% c("catch", "index")
  stopifnot(identical(moderate$data$observed[idx], strong$data$observed[idx]))
}
consistent <- r$status == "consistent_local_report"
stopifnot(all(abs(r$objective_difference[consistent]) < 1e-5),
  all(abs(r$joint_reference_difference[consistent]) < 1e-5), all(abs(r$after_difference[consistent]) < 1e-5))
stopifnot(nrow(x$moments$bins) == 420L, nrow(x$moments$covariance) == 24L,
  nrow(x$moments$generator_differences) == 210L, all(x$moments$bins$n == x$configuration$moment_draws))
cat("Counts, adaptive rule, source/production hashes, paired truth/scalars and report objectives: PASS\n")
cat("Attempts:", nrow(a), "valid:", sum(a$optimizer_valid), "; selected valid:",sum(a$selected & a$optimizer_valid),"/20\n")
print(x$summaries$fit_summary)
print(subset(x$summaries$state_summary,process == "M" & stratum == "eligible" & metric %in% c("coverage", "rmse")))
print(subset(x$summaries$paired_summary,process == "M"))
for (name in c("bins", "generator_differences")) for (metric in c("mean_z", "variance_z")) {
  z <- x$moments[[name]][[metric]]; valid <- is.finite(z)
  cat(name, metric, ": informative",sum(valid),"/",length(z),"max abs",max(abs(z[valid])),"over 6",sum(abs(z[valid]) > 6),"\n")
}
c <- x$moments$covariance
cat("Selected covariance residual maximum MCSE units:", max(abs((c$estimate - c$expected) / c$mcse)), "\n")
paired_covariance <- merge(c[c$generator == "public",], c[c$generator == "reference",],
  by = c("concentration","fleet","type","bin1","bin2"), suffixes = c("_public","_reference"))
paired_covariance$difference <- paired_covariance$estimate_public - paired_covariance$estimate_reference
paired_covariance$mcse <- sqrt(paired_covariance$mcse_public^2 + paired_covariance$mcse_reference^2)
paired_covariance$z <- paired_covariance$difference / paired_covariance$mcse
write.csv(paired_covariance,file.path(args[[1L]],"covariance-differences.csv"),row.names=FALSE)
cat("Public/reference covariance-difference maximum MCSE units:",max(abs(paired_covariance$z)),"\n")


if (length(args) == 2L) {
  stopifnot(args[[2L]] == "regenerate")
  e <- new.env(parent = asNamespace("FIMS"))
  for (file in c("benchmark.R","sensitivity.R","information.R","calibration.R","robustness.R",
    "mode-handoff.R","dm-validation.R","dm-profiles.R","public-dm.R"))
    sys.source(file.path("tests/integration/process-recovery",file),e)
  fixture <- e$public_dm_fixture(x$configuration$years)
  for (rep in seq_len(x$configuration$replicates)) for (kappa in c(2000,200)) {
    scenario <- if (kappa == 2000) "moderate" else "strong"
    regenerated <- e$public_dm_roundtrip(e$public_dm_generate(fixture,x$configuration$seed+rep-1L,kappa))
    original <- x$inputs[[paste(rep,scenario,sep="/")]]
    stopifnot(identical(regenerated$truth,original$truth),
      identical(FIMS::get_data(regenerated$data),original$data))
  }
  cat("Complete input/truth regeneration for all ten datasets: PASS; RNG kind:",paste(RNGkind(),collapse=", "),"\n")
}
