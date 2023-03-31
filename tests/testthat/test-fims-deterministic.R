library(FIMS)
data(package = "FIMS")

test_that("deterministic test of fims", {

## Set-up OM

## Set-up Rcpp modules and fix parameters to "true"
fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
fims$clear()

## Set-up TMB
# Create parameter list from Rcpp modules
parameters <- list(p = fims$get_fixed())
obj <- MakeADFun(data=list(), parameters, DLL="FIMS")
report <- obj$report()
#test output: expect_equal(?om_output$naa, report$naa) #need to add REPORT_F to population for any output to compare



}