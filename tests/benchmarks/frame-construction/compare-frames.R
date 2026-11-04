# Usage: compare-frames.R BEFORE_RDS AFTER_RDS OUTPUT_CSV
args <- commandArgs(TRUE)
before <- readRDS(args[1])
after <- readRDS(args[2])
stopifnot(identical(names(before), names(after)))
results <- data.frame(
  case = names(before),
  valid_frame = vapply(before, function(x) methods::is(x$value, "FIMSFrame"), logical(1)),
  identical = vapply(names(before), function(name) {
    identical(before[[name]], after[[name]])
  }, logical(1))
)
print(results, row.names = FALSE)
stopifnot(all(results$identical))
dir.create(dirname(args[3]), recursive = TRUE, showWarnings = FALSE)
write.csv(results, args[3], row.names = FALSE)
