script <- sub("^--file=", "", grep("^--file=", commandArgs(), value = TRUE)[1])
source(file.path(dirname(script), "common.R"))
args <- commandArgs(TRUE)
years <- as.integer(args[1])
centered <- as.logical(as.integer(args[2]))
no_plus <- as.logical(as.integer(args[3]))
no_feedback <- as.logical(as.integer(args[4]))
output <- args[5]
set_diagnostic(centered, no_plus, no_feedback)
context <- make_input(years, centered)
start <- proc.time()
obj <- make_objective(context)
build <- proc.time() - start
initial <- list(par = obj$par, value = obj$fn(obj$par),
                gradient = obj$gr(obj$par), report = obj$report(obj$par))
start <- proc.time()
opt <- nlminb(obj$par, obj$fn, obj$gr,
              control = list(iter.max = 1000, eval.max = 2000,
                             rel.tol = 1e-11))
inner <- proc.time() - start
start <- proc.time()
h <- obj$env$spHess(opt$par)
first_hessian <- proc.time() - start
entries <- summary(h)
n <- nrow(h)
chol <- tryCatch(factor_fresh(h), error = function(e) NULL)
results <- data.frame(
  years, ages = context$ages, centered, plus_group = !no_plus,
  stock_recruit_feedback = !no_feedback,
  random_effects = n, structural_entries = length(h@x),
  triangle_entries = n * (n + 1) / 2,
  density = length(h@x) / (n * (n + 1) / 2),
  bandwidth = max(abs(entries$i - entries$j)),
  build_cpu_s = sum(build[c("user.self", "sys.self")]),
  inner_cpu_s = sum(inner[c("user.self", "sys.self")]),
  first_hessian_cpu_s = sum(first_hessian[c("user.self", "sys.self")]),
  hessian_cpu_s = time_calls(hessian_probe(obj, opt$par), 20L),
  factor_cpu_s = if (is.null(chol)) NA_real_ else
    time_calls(function() factor_fresh(h), 100L),
  joint_value = opt$objective, convergence = opt$convergence,
  max_gradient = max(abs(obj$gr(opt$par))),
  hessian_positive_definite = !is.null(chol)
)
write.csv(results, paste0(output, ".csv"), row.names = FALSE)
saveRDS(list(results = results, initial = initial, opt = opt,
             hessian = h, report = obj$report(opt$par)), paste0(output, ".rds"))
print(results)
clear()
