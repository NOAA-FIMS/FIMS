args <- commandArgs(TRUE)
input <- args[1]
output <- args[2]
dir.create(output, recursive = TRUE, showWarnings = FALSE)
runs <- jsonlite::fromJSON(file.path(input, "runs.json"))
runs <- runs[runs$phase != "uncertainty" | runs$years == 30, ]
stopifnot(nrow(runs) == 28L, all(runs$exit_code == 0))
checks <- list()
for (years in unique(runs$years)) for (phase in unique(runs$phase)) {
  subset <- runs[runs$years == years & runs$phase == phase, ]
  if (nrow(subset) == 0L) next
  reference <- NULL
  for (i in seq_len(nrow(subset))) {
    label <- subset$label[i]
    obj <- readRDS(file.path(input, paste0(label, ".rds")))
    rows <- runs$label == label
    runs$recording_seconds[rows] <- obj$recording[["elapsed"]]
    runs$phase_seconds[rows] <- obj$phase_time[["elapsed"]]
    obj[c("session", "recording", "phase_time")] <- NULL
    if (is.null(reference)) reference <- obj
    identical_result <- isTRUE(all.equal(obj, reference, tolerance = 1e-10))
    checks[[label]] <- data.frame(label, passed = identical_result)
    if (!identical_result) print(all.equal(obj, reference, tolerance = 1e-10))
    stopifnot(identical_result)
  }
}
summary <- aggregate(cbind(peak_rss_mib, recording_seconds, phase_seconds) ~
                       years + phase + variant, data = runs, FUN = median)
write.csv(runs, file.path(output, "runs.csv"), row.names = FALSE)
write.csv(summary, file.path(output, "summary.csv"), row.names = FALSE)
write.csv(do.call(rbind, checks), file.path(output, "validation.csv"), row.names = FALSE)
print(summary)
keys <- unique(rownames(readRDS(file.path(input,
  "30-uncertainty-1-before.rds"))$uncertainty))
payload <- do.call(rbind, lapply(unique(runs$years), function(years) {
  report <- readRDS(file.path(input,
    sprintf("%d-report-1-before.rds", years)))$reports[[1]]
  sizes <- vapply(report[keys], function(x) length(unlist(x)), integer(1))
  data.frame(years, quantities = length(keys), elements = sum(sizes),
             before_retained_copy_bytes = 2 * sum(sizes) * 8,
             after_largest_buffer_bytes = max(sizes) * 8)
}))
write.csv(payload, file.path(output, "double-payload.csv"), row.names = FALSE)
print(payload)
