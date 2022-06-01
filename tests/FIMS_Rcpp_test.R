
library(Rcpp)
library(FIMS)

fims <- Rcpp::Module("fims", PACKAGE= "FIMS")

a <- new(fims$parameter, .1)
beverton_holt<-new(fims$beverton_holt)
print(a$value)
print(paste0("is random effect = ",a$is_random_effect))
print(fims$create_tmb_model())
