# tests for input objects
library(dplyr)
library(FIMS)
data(package = "FIMS")
age_frame <- FIMSFrameAge(data_mile1)
fims_frame <- FIMSFrame(data_mile1)

test_that("Can add index data to model",{
  index = filter(age_frame@data, type=="index") %>%
  select(value)
  
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  Index <- fims$Index
  indexdat <- new(Index, length(index))
  indexdat$index_data = index$value
})

test_that("Can add agecomp data to model",{
  agecomp1 = dplyr::filter(age_frame@data, type=="age", 
  name == "fleet1") %>%
  select(value)

  agecomp2 = dplyr::filter(age_frame@data, type=="age", 
  name == "survey1") %>%
  select(value)

  nages = length(age_frame@ages)-1
  nyears = length(agecomp2$value)/nages
  
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")
  agecompdat <- new(fims$AgeComp, nyears, nages)
  agecompdat$age_comp_data = agecomp1$value
  agecompdat2 <- new (fims$AgeComp, nyears, nages) 
  agecompdat2$age_comp_data = agecomp2$value

})
