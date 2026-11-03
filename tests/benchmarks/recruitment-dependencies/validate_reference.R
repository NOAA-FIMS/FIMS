# Compare diagnostics-off against a separately built, uninstrumented library.
# Generate both files with ablation.R 30 0 0 0 and the respective R_LIBS paths.
library(Matrix)
args <- commandArgs(TRUE)
reference <- readRDS(args[1])
diagnostic <- readRDS(args[2])
checks <- c(
  initial = isTRUE(all.equal(reference$initial, diagnostic$initial, tolerance = 1e-10)),
  mode = isTRUE(all.equal(reference$opt$par, diagnostic$opt$par, tolerance = 1e-10)),
  hessian = isTRUE(all.equal(reference$hessian, diagnostic$hessian, tolerance = 1e-10)),
  reports = isTRUE(all.equal(reference$report, diagnostic$report, tolerance = 1e-10))
)
print(checks)
stopifnot(all(checks))
write.csv(data.frame(check = names(checks), passed = unname(checks)),
          args[3], row.names = FALSE)
