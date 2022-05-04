# Use code from model comparison project to make a database

#based on https://github.com/Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison#usage


# Set up OM ---------------------------------------------------------------


maindir <- file.path(".")

om_sim_num <- 1 # total number of iterations per case
keep_sim_num <- 1 # number of kept iterations per case
figure_number <- 1 # number of individual iteration to plot

seed_num <- 9924

## Life-history parameters
year <- 1:30
ages <- 1:12 # Age structure of the popn

initial_equilibrium_F <- TRUE
median_R0 <- 1000000 # Average annual unfished recruitment (scales the popn)
median_h <- 0.75 # Steepness of the Beverton-Holt spawner-recruit relationship.
mean_R0 <- NULL
mean_h <- NULL
SRmodel <- 1 # 1=Beverton-Holt; 2=Ricker
M <- 0.2 # Age-invariant natural mortality

Linf <- 800 # Asymptotic average length
K <- 0.18 # Growth coefficient
a0 <- -1.36 # Theoretical age at size 0
a.lw <- 0.000000025 # Length-weight coefficient
b.lw <- 3.0 # Length-weight exponent
A50.mat <- 2.25 # Age at 50% maturity
slope.mat <- 3 # Slope of maturity ogive
pattern.mat <- 1 # Simple logistic maturity
female.proportion <- 0.5 # Sex ratio

## Fleet settings
fleet_num <- 1

# CV of landings for OM
cv.L <- list()
cv.L$fleet1 <- 0.005

# Input CV of landings for EMs
input.cv.L <- list()
input.cv.L$fleet1 <- 0.01

# Annual sample size (nfish) of age comp samples
n.L <- list()
n.L$fleet1 <- 200

# Define fleet selectivity
sel_fleet <- list()

sel_fleet$fleet1$pattern <- 1
sel_fleet$fleet1$A50.sel1 <- 2
sel_fleet$fleet1$slope.sel1 <- 1

## Survey settings
survey_num <- 1

# CV of surveys for OM
cv.survey <- list()
cv.survey$survey1 <- 0.1

# Input CV of surveys for EMs
input.cv.survey <- list()
input.cv.survey$survey1 <- 0.2

# Annual sample size (nfish) of age comp samples
n.survey <- list()
n.survey$survey1 <- 200

# Define survey selectivity
sel_survey <- list()

sel_survey$survey1$pattern <- 1
sel_survey$survey1$A50.sel1 <- 1.5
sel_survey$survey1$slope.sel1 <- 2

## Other settings
logf_sd <- 0.2
f_dev_change <- FALSE
f_pattern <- 1
start_val <- 0.01
middle_val <- NULL
end_val <- 0.39
f_val <- NULL
start_year <- 1
middle_year <- NULL

logR_sd <- 0.4
r_dev_change <- TRUE

om_bias_cor <- FALSE
bias_cor_method <- "none" # Options: "none", "median_unbiased", and "mean_unbiased"
em_bias_cor <- FALSE

# Case 1 : null case -----------------------------------------------------------

null_case_input <- ASSAMC::save_initial_input(base_case = TRUE, case_name = "C1")

## Run OM
ASSAMC::run_om(input_list = null_case_input, show_iter_num = T)


# Case 2 : change logR_sd from 0.4 to 0.6 --------------------------------------

# updated_input <- save_initial_input(
#   base_case = FALSE,
#   input_list = null_case_input,
#   case_name = "C2",
#   logR_sd = 0.6
# )
# run_om(input_list = updated_input, show_iter_num = F)

# modfy the generated data set ------

rm(list = ls()) # to get rid of all the other obj cluttering workspace
load("C1/output/OM/OM1.RData")

# landings - assume fishery occurs continuously over the whole year.

landings_data <- data.frame(
    type = "landings", # cor catch?
    name = names(om_output$L.mt)[1],
    age = NA, # The inputs are not by age in this case, but there is a by age option.
    year = om_input$year, #keep this column if we add timing start and end?
    timingstart = 0,
    timingend = 1, #how to indicate end of the year?
    value = om_output$L.mt[[1]], # note only 1 fleet in this case tho
    unit = "mt", # metric tons
    family = paste0("ilnorm(CV=", om_input$cv.L[[1]], ")")
)

# survey trend (index of abundance) - assume occurs instantaneously at the beginning of the year.

trend_data <- data.frame(
    type = "trend",
    name = names(om_output$survey_index)[1],
    age = NA, # The inputs are not by age in this case.
    year = om_input$year, # may want to add fractional components to this to indicate timing.
    timingstart = 0,
    timingend = 0,
    value = om_output$survey_index[[1]],
    unit = "numbers", # I think?
    family = paste0("ilnorm(CV=", om_input$cv.survey[[1]], ")") # Is this ok?
)

# survey comp data
matrix_convert_comp_data <- as.data.frame(om_output$survey_age_comp[[1]])
colnames(matrix_convert_comp_data)  <- om_input$ages
matrix_convert_comp_data$year <- om_input$year
age_comp_data <- tidyr::pivot_longer(matrix_convert_comp_data, 
  cols = om_input$ages, 
  names_to = "age",
  values_to = "value")

age_comp_data$name <- names(om_output$survey_age_comp)[1]
age_comp_data$type <- "agecomp" # or rather agefrequency???
age_comp_data$unit <- "numbers"
age_comp_data$family <- paste0("imultinom(size=", om_input$n.survey[[1]], ")") # I think size is consistent with r function??
age_comp_data$timingstart <- 0
age_comp_data$timingend <- 0

# order the same as trend data.
age_comp_data <- 
  age_comp_data[,c("type", "name", "age", "year", "timingstart", "timingend", "value", "unit", "family" )]

# From the paper it sounds like there should be agecomp for the fishery, but I don't
# see this in the om_output.

# bind together into a single data frame

data_df <- rbind(landings_data, trend_data, age_comp_data)
data_df <- type.convert(data_df, as.is = TRUE)
write.csv(data_df, 
  file.path("C1", "output", "OM", "FIMS_input_data.csv"),
  row.names = FALSE)

# check csv can be read into r well ----
test_read <- read.csv(file.path("C1", "output", "OM", "FIMS_input_data.csv"))

# TODO: need any adjustments to timing?