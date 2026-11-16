# Reproduce one strongest-scenario fit and compare cold/warm latent-mode rebuilds.
# Rscript tests/integration/process-recovery/diagnose-robustness.R OUTPUT_DIRECTORY
suppressPackageStartupMessages(library(FIMS))
for (file in c("benchmark.R", "sensitivity.R", "information.R", "calibration.R", "robustness.R"))
  source(file.path("tests/integration/process-recovery", file))
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) == 1L)
output <- args[[1L]]
result <- readRDS(file.path(output, "result.rds"))
selected <- subset(result$tables$attempts, scenario == "strong" & replicate == 1 & selected)
stopifnot(nrow(selected) == 1L, selected$valid)
fixture <- recovery_fixture(result$configuration$years)
raw <- result$operating_models[[1L]]
operating <- list(data = FIMSFrame(raw$data$strong), truth = raw$truth)
setup <- sensitivity_objective(fixture, operating, "normal", selected$start_sd)
obj <- setup$obj
opt <- nlminb(obj$par, obj$fn, obj$gr, lower = log(.001), upper = 0,
  control = list(iter.max = 300, eval.max = 600))
reference <- as.numeric(obj$fn(opt$par))
reference_joint <- obj$env$last.par
reference_joint_nll <- as.numeric(obj$env$f(reference_joint, order = 0))
full <- obj$env$parList(par = reference_joint)
sigma <- exp(full$p[setup$positions])
# Rebuild fresh native models with identical covariance values. Only the random
# effect initial vector differs; neither rebuild reoptimizes outer covariance.
rebuild <- function(warm) {
  input <- recovery_fit_input(fixture, operating,
    data.frame(processes = "R_F_M", initialization = "normal", sd_mode = "estimated"))$input
  input$parameters$p[] <- full$p
  if (warm) input$parameters$re[] <- full$re
  rebuilt <- TMB::MakeADFun(data = list(), parameters = input$parameters, random = "re", DLL = "FIMS", silent = TRUE)
  value <- as.numeric(rebuilt$fn(rebuilt$par))
  joint <- rebuilt$env$last.par
  gradient <- as.numeric(rebuilt$env$f(joint, order = 1))[rebuilt$env$random]
  data.frame(initialization = if (warm) "saved inner mode" else "cold default",
    objective = value, delta_from_reference = value - reference,
    max_inner_gradient = max(abs(gradient)),
    joint_nll_at_reference = as.numeric(rebuilt$env$f(reference_joint, order = 0)),
    joint_reference_difference = as.numeric(rebuilt$env$f(reference_joint, order = 0)) - reference_joint_nll)
}
diagnostic <- rbind(data.frame(initialization = "reproduced covariance fit", objective = reference,
  delta_from_reference = 0, max_inner_gradient = NA_real_, joint_nll_at_reference = reference_joint_nll,
  joint_reference_difference = 0), rebuild(FALSE), rebuild(TRUE))
diagnostic$selected_original_objective <- selected$objective
write.csv(diagnostic, file.path(output, "mode-rebuild-diagnostic.csv"), row.names = FALSE)
print(diagnostic)
