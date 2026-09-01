#' Access the FIMS log
#'
#' FIMS stores structured log entries in native memory and returns them as a
#' JSON array string. The filtered accessors return only entries at the named
#' severity level.
#'
#' @return A length-one character vector containing a JSON array.
#' @name get_log
#' @export
get_log <- function() {
  .Call("fims_call_log_get", PACKAGE = "FIMS")
}

#' @rdname get_log
#' @export
get_log_errors <- function() {
  .Call("fims_call_log_get_errors", PACKAGE = "FIMS")
}

#' @rdname get_log
#' @export
get_log_warnings <- function() {
  .Call("fims_call_log_get_warnings", PACKAGE = "FIMS")
}

#' @rdname get_log
#' @export
get_log_info <- function() {
  .Call("fims_call_log_get_info", PACKAGE = "FIMS")
}

#' Add an entry to the FIMS log
#'
#' @param message A non-missing character scalar.
#' @return `NULL`, invisibly.
#' @name log_message
#' @export
log_info <- function(message) {
  invisible(.Call("fims_call_log_info", message, PACKAGE = "FIMS"))
}

#' @rdname log_message
#' @export
log_warning <- function(message) {
  invisible(.Call("fims_call_log_warning", message, PACKAGE = "FIMS"))
}

#' @rdname log_message
#' @export
log_error <- function(message) {
  invisible(.Call("fims_call_log_error", message, PACKAGE = "FIMS"))
}

#' Configure native FIMS logging
#'
#' @param write Whether to write the log when the native logger exits.
#' @param path A non-missing character scalar naming the log output file.
#' @param throw_on_error Whether logging an error should immediately raise an
#'   R error.
#' @return `NULL`, invisibly.
#' @name configure_logging
#' @export
write_log <- function(write) {
  invisible(.Call("fims_call_log_write_on_exit", write, PACKAGE = "FIMS"))
}

#' @rdname configure_logging
#' @export
set_log_path <- function(path) {
  invisible(.Call("fims_call_log_set_path", path, PACKAGE = "FIMS"))
}

#' @rdname configure_logging
#' @export
set_log_throw_on_error <- function(throw_on_error) {
  invisible(
    .Call(
      "fims_call_log_set_throw_on_error",
      throw_on_error,
      PACKAGE = "FIMS"
    )
  )
}
