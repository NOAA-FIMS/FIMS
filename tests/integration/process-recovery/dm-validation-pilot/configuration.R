list(replicates = 5L, years = 18L, seed = 4301L, design = structure(list(
    scenario = c("multinomial", "moderate", "strong"), concentration = c(Inf,
    2000, 200), sample_size = c(2000L, 2000L, 2000L), variance_inflation = c(1,
    1.99900049975012, 10.9452736318408), effective_size = c(2000,
    1000.5, 182.727272727273)), class = "data.frame", row.names = c(NA,
-3L)), starts = structure(list(start_sd = c(0.05, 0.2, 0.05,
0.2), shifted = c(FALSE, FALSE, TRUE, TRUE)), out.attrs = list(
    dim = c(start_sd = 2L, shifted = 2L), dimnames = list(start_sd = c("start_sd=0.05",
    "start_sd=0.20"), shifted = c("shifted=FALSE", "shifted=TRUE"
    ))), class = "data.frame", row.names = c(NA, -4L)), estimated_concentration_start = 2000,
    concentration_bounds = c(0.1, 1e+05), process_sd_bounds = c(0.001,
    1), true_process_sd = c(R = 0.2, F = 0.15, M = 0.05), latent_shift = ".15 * sin(index), all random states",
    concentration_ownership = "independent per fleet/composition type, constant across years",
    interval_scope = "local fitted log-states, not forecasts or derived quantities",
    source_md5 = c(`tests/integration/process-recovery/benchmark.R` = "26a70c45fbc65d1e280b475922e32451",
    `tests/integration/process-recovery/sensitivity.R` = "04c9b34937d2f73ec7704ad14f073562",
    `tests/integration/process-recovery/information.R` = "a2a257b45ce6380f62da919ac0027873",
    `tests/integration/process-recovery/calibration.R` = "26842dc66e3159084ccdfb4bb11c9e66",
    `tests/integration/process-recovery/robustness.R` = "62d4bc7976127ee96bb314d27c8a85da",
    `tests/integration/process-recovery/mode-handoff.R` = "34b1799f5ee95d6edc3fe4531bd78a20",
    `tests/integration/process-recovery/dm-validation.R` = "3ce597f40833fb82d285bf4feca02a38",
    `tests/integration/process-recovery/run-dm-validation.R` = "a7f64786971abc4f2338c9480e8fa27e"
    ), production_md5 = c(`inst/include/distributions/functors/dirichlet_multinomial_lpmf.hpp` = "48df2c5feeddf3fd0a040c65e79d7991",
    `inst/include/interface/rcpp/rcpp_objects/rcpp_distribution.hpp` = "ab2b900751ba59ad431798c6fc58bf23",
    `R/composition_likelihood.R` = "de9aa0d3a75a9164adac3144925ab305",
    `R/initialize_modules.R` = "e8c27c77f00c0081ecf85bff73e1cc6c"
    ), git_head = "4d504e5445f457a1fd3f5288ae5a0823b9bb3689",
    session = c("R version 4.4.3 (2025-02-28)", "Platform: aarch64-apple-darwin20",
    "Running under: macOS Sequoia 15.7.7", "", "Matrix products: default",
    "BLAS:   /Library/Frameworks/R.framework/Versions/4.4-arm64/Resources/lib/libRblas.0.dylib ",
    "LAPACK: /Library/Frameworks/R.framework/Versions/4.4-arm64/Resources/lib/libRlapack.dylib;  LAPACK version 3.12.0",
    "", "locale:", "[1] C.UTF-8/C.UTF-8/C.UTF-8/C/C.UTF-8/C.UTF-8",
    "", "time zone: America/New_York", "tzcode source: internal",
    "", "attached base packages:", "[1] stats     graphics  grDevices utils     datasets  methods   base     ",
    "", "other attached packages:", "[1] FIMS_0.11.0.9000", "",
    "loaded via a namespace (and not attached):", " [1] compiler_4.4.3    Matrix_1.7-2      cli_3.6.5         generics_0.1.4   ",
    " [5] tools_4.4.3       TMB_1.9.20        Rcpp_1.1.1        codetools_0.2-20 ",
    " [9] grid_4.4.3        rlang_1.1.7       stockplotr_0.14.0 lattice_0.22-6   "
    ))
