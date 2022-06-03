
library(Rcpp)
library(FIMS)

fims <- Rcpp::Module("fims", PACKAGE= "FIMS")

a <- new(fims$Parameter, .1)
beverton_holt<-new(fims$BevertonHoltRecruitment)
print(a$value)
print(paste0("is random effect = ",a$is_random_effect))
print(fims$CreateTMBModel())


logistic_selectivity<-new(fims$LogisticSelectivity)
logistic_selectivity$slope$value <- .7
logistic_selectivity$median$value<- 5.0

print(logistic_selectivity$slope$value)
print(logistic_selectivity$get_id())

logistic_selectivity2<-new(fims$LogisticSelectivity)
print(logistic_selectivity2$get_id())
