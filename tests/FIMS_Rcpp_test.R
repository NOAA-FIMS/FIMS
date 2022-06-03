
library(Rcpp)
library(FIMS)

fims <- Rcpp::Module("fims", PACKAGE= "FIMS")

a <- new(fims$parameter, .1)
beverton_holt<-new(fims$beverton_holt)
print(a$value)
print(paste0("is random effect = ",a$is_random_effect))
print(fims$create_tmb_model())


logistic_selectivity<-new(fims$logistic_selectivity)
logistic_selectivity$slope$value <- .7
logistic_selectivity$median$value<- 5.0

print(logistic_selectivity$slope$value)
print(logistic_selectivity$get_id())

logistic_selectivity2<-new(fims$logistic_selectivity)
print(logistic_selectivity2$get_id())
