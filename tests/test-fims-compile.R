withr::local_options(pkg.build_extra_flags = FALSE)
devtools::load_all()

devtools::install_version("TMB", version = "1.8.0", repos = "http://cran.us.r-project.org")

#run from command line: Rscript tests/test-fims-compile.R >output.txt 2>&1

TMB::gdbsource("tests/testthat/test-fims-estimation-bug.R", interactive = TRUE)
