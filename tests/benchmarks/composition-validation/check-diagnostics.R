# Usage: check-diagnostics.R OUTPUT_RDS (run with each isolated library).
args <- commandArgs(TRUE)
library(FIMS)
options(cli.num_colors = 1, cli.width = 80, width = 80)
x <- tibble::tibble(type = "age_comp", fleet = "survey", timing = 3,
  unit = "proportion", observed = c(0.25, 0.75), age = 1:2,
  metadata = "constant", missing_metadata = NA_character_)
bad_sum <- x; bad_sum$observed <- c(0.2, 0.7)
bad_units <- x; bad_units$unit <- c("number", "proportion")
counts <- x; counts$unit <- "number"; counts$observed <- c(25, 75)
missing <- x; missing$observed <- -999
missing_unit <- x; missing_unit$unit <- NA_character_
cases <- list(valid = x, bad_sum = bad_sum, bad_units = bad_units,
              counts = counts, missing = missing, missing_unit = missing_unit,
              empty = x[0, ])
results <- lapply(cases, function(data) {
  messages <- capture.output(
    value <- tryCatch(FIMS:::validate_composition_data(data),
                      error = function(e) list(error = conditionMessage(e))),
    type = "message"
  )
  list(value = value, messages = messages)
})
saveRDS(results, args[1])
