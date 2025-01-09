library(devtools)
withr::local_options(pkg.build_extra_flags = FALSE)
load_all()
# use FIMS data frame
data_4_model <- FIMSFrame(data1)
fleet1 <- survey1 <- list(
  selectivity = list(form = "LogisticSelectivity"),
  data_distribution = c(
    Index = "DlnormDistribution",
    AgeComp = "DmultinomDistribution"
  )
)
# Create default parameters
default_parameters <- data_4_model |>
  create_default_parameters(fleets = list(fleet1 = fleet1, survey1 = survey1))
fit_no <- default_parameters |>
  initialize_fims(data = data_4_model)

fit <- default_parameters |>
 initialize_fims(data = data_4_model) |>
  fit_fims(optimize = TRUE)
print(fit)
finalize(get_obj(fit), get_opt(fit))
json_output <- get_output()
cat(json_output)
write(json_output, "fims.json")
