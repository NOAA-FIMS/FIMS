
library(Rcpp)
library(FIMS)

fims <- Rcpp::Module("fims", PACKAGE= "FIMS")

a <- new(fims$parameter, .1)
a
print(a$value)
print(fims$create_model())
