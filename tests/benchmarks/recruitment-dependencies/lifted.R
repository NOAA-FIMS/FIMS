# Exact deterministic-state lifting experiment on the full FIMS objective.
# This validates algebra and measures component costs; it is not a new fitter.
script <- sub("^--file=", "", grep("^--file=", commandArgs(), value = TRUE)[1])
source(file.path(dirname(script), "common.R"))
args <- commandArgs(TRUE)
years <- as.integer(args[1])
output <- args[2]
set_diagnostic(centered = TRUE)
context <- make_input(years, centered = TRUE)
reduced <- make_objective(context)
opt <- nlminb(reduced$par, reduced$fn, reduced$gr,
              control = list(iter.max = 1000, eval.max = 2000, rel.tol = 1e-12))
r <- opt$par
# Polish the conditional mode so the determinant comparison is at a stationary
# point, without relying on the optimizer's relative objective stopping rule.
for (iteration in seq_len(8)) {
  gradient <- reduced$gr(r)
  if (max(abs(gradient)) < 1e-7) break
  h <- reduced$env$spHess(r)
  step <- as.numeric(solve(h, gradient))
  candidate <- r - step
  if (reduced$fn(candidate) > reduced$fn(r) + 1e-8) break
  r <- candidate
}
value <- reduced$fn(r)
gradient <- reduced$gr(r)
h <- reduced$env$spHess(r)
report <- reduced$report(r)
n <- length(r)
stopifnot(n == years - 1L)
a <- context$ages
naa <- matrix(report$numbers_at_age[[1]], ncol = a, byrow = TRUE)
z <- matrix(report$mortality_Z[[1]], ncol = a, byrow = TRUE)
states <- naa[2:years, a] / context$rzero

# C(r,z)=0 is the scaled deterministic plus-group transition. All mortality
# and other fixed effects are held constant in this experiment.
cz <- sparseMatrix(i = c(seq_len(n), 2:n),
                   j = c(seq_len(n), 1:(n - 1L)),
                   x = c(rep(1, n), -exp(-z[2:n, a])), dims = c(n, n))
rows <- if (n >= a) a:n else integer(0)
cr <- sparseMatrix(i = rows, j = rows - a + 1L,
                   x = -naa[rows, a - 1L] * exp(-z[rows, a - 1L]) / context$rzero,
                   dims = c(n, n))
j <- cbind(cr, cz)
point <- c(r, states)
set_diagnostic(centered = TRUE, lifted = TRUE)
lifted <- make_objective(context, states)
lifted_value <- lifted$fn(point)
lifted_gradient <- lifted$gr(point)
lifted_report <- lifted$report(point)
constraints <- lifted_report$diagnostic_constraints
report_error <- relative_error(unlist(lifted_report[names(report)]), unlist(report))
lambda <- as.numeric(solve(t(cz), -lifted_gradient[n + seq_len(n)]))
lagrangian <- make_objective(context, states, lambda)
lagrangian_gradient <- lagrangian$gr(point)
first_w_start <- proc.time()
w <- lagrangian$env$spHess(point)
first_w_cpu <- sum((proc.time() - first_w_start)[c("user.self", "sys.self")])

# Independently check the analytic constraint Jacobian with an AD
# vector-Jacobian product (nonzero deterministic weights in every row).
weights <- sin(seq_len(n))
constraint_objective <- make_objective(context, states, weights,
                                       constraints_only = TRUE)
jacobian_error <- relative_error(constraint_objective$gr(point),
                                 as.numeric(t(j) %*% weights))

# Sparse indefinite system; eliminating states and multipliers recovers the
# original dense recruitment Hessian. The Lagrangian curvature term matters.
k <- rbind(cbind(w, t(j)), cbind(j, Matrix(0, n, n, sparse = TRUE)))
k <- as(k, "generalMatrix")
permutation <- as.vector(rbind(seq_len(n), n + seq_len(n), 2L * n + seq_len(n)))
kp <- k[permutation, permutation]
fresh_lu <- function() {
  kp@factors <- list()
  Matrix::lu(kp)
}
lu <- fresh_lu()
rhs <- cos(seq_len(n))
full_rhs <- c(rhs, rep(0, 2L * n))
k_solution <- as.numeric(solve(k, full_rhs))[seq_len(n)]
reduced_solution <- as.numeric(solve(h, rhs))
solve_error <- relative_error(k_solution, reduced_solution)
logdet_h <- as.numeric(determinant(h, logarithm = TRUE)$modulus)
logdet_k <- as.numeric(determinant(k, logarithm = TRUE)$modulus)

# Explicit dense Schur-complement verification only at small sizes. It is
# deliberately excluded from component timing; a sparse solver would not form it.
hessian_error <- NA_real_
if (n <= 119L) {
  tangent <- rbind(Diagonal(n), -solve(cz, cr))
  h_from_lift <- t(tangent) %*% w %*% tangent
  hessian_error <- relative_error(as.matrix(h_from_lift), as.matrix(h))
}
errors <- c(
  constraint = max(abs(constraints)),
  value = relative_error(lifted_value, value),
  reports = report_error,
  gradient = relative_error(lagrangian_gradient[seq_len(n)], gradient),
  adjoint = max(abs(lagrangian_gradient[n + seq_len(n)])),
  jacobian = jacobian_error, solve = solve_error,
  logdet = abs(logdet_k - logdet_h), hessian = hessian_error
)
print(errors)
stopifnot(all(errors[!is.na(errors)] < 1e-5))

# Set flags to match each recorded model when timings trigger AD setup.
set_diagnostic(centered = TRUE)
reduced_hessian_time <- time_calls(hessian_probe(reduced, r), 20L)
set_diagnostic(centered = TRUE, lifted = TRUE)
lifted_hessian_time <- time_calls(hessian_probe(lagrangian, point), 20L)
results <- data.frame(
  years, ages = a, recruitment_effects = n, augmented_dimension = nrow(k),
  reduced_hessian_entries = length(h@x),
  lifted_hessian_entries = length(w@x), kkt_entries = length(k@x),
  lu_entries = length(lu@L@x) + length(lu@U@x),
  first_lifted_hessian_cpu_s = first_w_cpu,
  reduced_hessian_cpu_s = reduced_hessian_time,
  lifted_hessian_cpu_s = lifted_hessian_time,
  reduced_factor_cpu_s = time_calls(function() factor_fresh(h), 100L),
  lifted_factor_cpu_s = time_calls(fresh_lu, 100L),
  max_gradient = max(abs(gradient)), convergence = opt$convergence,
  max_constraint = errors[["constraint"]],
  hessian_relative_error = hessian_error,
  solve_relative_error = solve_error, logdet_absolute_error = errors[["logdet"]]
)
write.csv(results, paste0(output, ".csv"), row.names = FALSE)
saveRDS(list(results = results, errors = errors, reduced_hessian = h,
             lifted_hessian = w, kkt = k, r = r, states = states, lambda = lambda),
        paste0(output, ".rds"))
print(results)
clear()
