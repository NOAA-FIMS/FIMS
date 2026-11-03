# Usage: retention.R full|compact COUNT OUTPUT
args <- commandArgs(TRUE)
mode <- args[1]
n <- as.integer(args[2])
library(FIMS)
options(rlib_message_verbosity="quiet")
data("data_big", package="FIMS")
frame <- FIMSFrame(data_big)
parameters <- setup_default_parameters(data=frame)
parameters$estimation_type <- "constant"
parameters$estimation_type[parameters$label == "log_rzero"] <- "fixed_effects"
parameters$estimation_type[parameters$label == "log_devs"] <- "random_effects"
input <- initialize_fims(parameters, data=frame)
keys <- c("spawning_biomass", "biomass", "expected_recruitment",
          "catch_expected", "index_expected")
results <- vector("list", n)
for (i in seq_len(n)) {
  fit <- fit_fims(input, number_of_loops=0, get_sd=TRUE, uncertainty_reports=keys)
  results[[i]] <- if (mode == "compact") compact_fims(fit, reports=keys) else fit
  rm(fit)
  invisible(gc())
}
extract <- function(x) {
  if (mode == "full") {
    list(opt=get_opt(x), estimates=get_estimates(x),
         report=get_report(x)[names(get_report(x)) %in% keys])
  } else x[c("opt", "estimates", "report")]
}
# Validate one retained fit; compare full/compact across fresh processes.
check <- extract(results[[1]])
saveRDS(list(mode=mode, count=n, check=check), args[3])
cat("Retained", n, mode, "results\n")
