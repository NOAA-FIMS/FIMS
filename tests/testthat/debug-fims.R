#Run from Rstudio - can't get it to work from command line or VSCode
TMB::gdbsource("tests/testthat/test-fims-deterministic.R", interactive = TRUE)
TMB::precompile()
