#' Read and configure the FIMS log
#'
#' The native logger stores messages as JSON. Getter functions return JSON
#' strings, except `get_log_path()`, which returns the output file path.
#' Messages remain in memory until `clear()` is called. `write_log()` controls
#' writing on session exit; it does not immediately write a file.
#' `set_log_throw_on_error(TRUE)` makes `log_error()` raise an R error after
#' recording the message. The default is FALSE.
#' @param log_entry A single non-missing character string to record.
#' @param path Output file path.
#' @param write Whether to write the log on exit.
#' @param throw_on_error Whether logged errors stop execution.
#' @return Getters return a character string. Setters and message functions
#'   return NULL invisibly.
#' @name logging
NULL

#' @rdname logging
#' @export
get_log <- function() {
  .Call("fims_call_logging", "get_log", NULL, PACKAGE = "FIMS")
}

#' @rdname logging
#' @export
get_log_errors <- function() {
  .Call("fims_call_logging", "get_log_errors", NULL, PACKAGE = "FIMS")
}

#' @rdname logging
#' @export
get_log_warnings <- function() {
  .Call("fims_call_logging", "get_log_warnings", NULL, PACKAGE = "FIMS")
}

#' @rdname logging
#' @export
get_log_info <- function() {
  .Call("fims_call_logging", "get_log_info", NULL, PACKAGE = "FIMS")
}

#' @rdname logging
#' @export
get_log_path <- function() {
  .Call("fims_call_logging", "get_log_path", NULL, PACKAGE = "FIMS")
}

#' @rdname logging
#' @export
write_log <- function(write) {
  invisible(.Call("fims_call_logging", "write_log", write, PACKAGE = "FIMS"))
}

#' @rdname logging
#' @export
set_log_path <- function(path) {
  invisible(.Call("fims_call_logging", "set_log_path", path, PACKAGE = "FIMS"))
}

#' @rdname logging
#' @export
set_log_throw_on_error <- function(throw_on_error) {
  invisible(.Call("fims_call_logging", "set_log_throw_on_error", throw_on_error, PACKAGE = "FIMS"))
}

#' @rdname logging
#' @export
log_info <- function(log_entry) {
  invisible(.Call("fims_call_logging", "log_info", log_entry, PACKAGE = "FIMS"))
}

#' @rdname logging
#' @export
log_warning <- function(log_entry) {
  invisible(.Call("fims_call_logging", "log_warning", log_entry, PACKAGE = "FIMS"))
}

#' @rdname logging
#' @export
log_error <- function(log_entry) {
  invisible(.Call("fims_call_logging", "log_error", log_entry, PACKAGE = "FIMS"))
}
