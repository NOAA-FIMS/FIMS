# Run in each isolated installation; compare the resulting RDS files exactly.
args <- commandArgs(TRUE)
library(FIMS)
data("data_big", package = "FIMS")
x <- data_big
remove_rows <- function(type, year = 5) {
  x[!(x$type %in% type & !is.na(x$timing) & x$timing == year), ]
}
metadata <- x
metadata$note <- rep(c("first", NA_character_), length.out = nrow(x))
metadata$batch <- factor(rep(c("a", "b"), length.out = nrow(x)))
factors <- x
for (name in c("type", "fleet", "unit")) factors[[name]] <- factor(factors[[name]])
missing_unit <- x
missing_unit$unit[missing_unit$type == "index"] <- NA_character_
annual <- x[x$type %in% c("catch", "index"), ]
annual$age <- annual$length <- NULL
no_age <- x[!x$type %in% c("length_comp", "age_to_length_conversion"), ]
no_age$age <- NULL
no_length <- x[!x$type %in% c("length", "age_to_length_conversion"), ]
no_length$length <- NULL
cases <- list(
  complete = x,
  missing_index_year = remove_rows("index"),
  missing_age_year = remove_rows("age_comp"),
  missing_length_year = remove_rows("length_comp"),
  missing_all_compositions = remove_rows(c("age_comp", "length_comp")),
  duplicate_annual = rbind(x, x[which(x$type == "catch")[1], ]),
  missing_unit = missing_unit,
  metadata = metadata,
  factors = factors,
  annual_only = annual,
  no_age_column = no_age,
  no_length_column = no_length,
  incomplete_age_year = x[-which(x$type == "age_comp")[1], ],
  incomplete_length_year = x[-which(x$type == "length_comp")[1], ]
)
results <- lapply(cases, function(data) {
  warnings <- character()
  value <- withCallingHandlers(
    tryCatch(FIMSFrame(data), error = function(e) list(error = conditionMessage(e))),
    warning = function(w) {
      warnings <<- c(warnings, conditionMessage(w))
      invokeRestart("muffleWarning")
    }
  )
  list(value = value, warnings = warnings)
})
saveRDS(results, args[1])
