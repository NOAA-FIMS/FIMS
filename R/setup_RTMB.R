setup_RTMB <- function() {
  if (!requireNamespace("RTMB", quietly = TRUE)) {
    stop("The RTMB package is required but not installed. Please install it using install.packages('RTMB').")
  }
  library(RTMB)

  # Only call if the symbol is available
  dll <- getLoadedDLLs()[["FIMS"]]
  if (!is.null(dll) && "_rtmb_set_shared_pointers" %in% names(getDLLRegisteredRoutines(dll)[[".Call"]])) {
    invisible(.Call("_rtmb_set_shared_pointers", PACKAGE = "FIMS"))
    message("RTMB setup completed successfully.")
  } else {
    warning("This version of FIMS was built without RTMB functionality.")
  }
}