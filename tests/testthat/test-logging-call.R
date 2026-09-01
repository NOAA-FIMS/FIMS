test_that("native log accessors return valid filtered JSON", {
  native_clear()
  log_info("native info")
  log_warning("native warning")
  log_error("native error")

  all_entries <- jsonlite::fromJSON(get_log())
  info_entries <- jsonlite::fromJSON(get_log_info())
  warning_entries <- jsonlite::fromJSON(get_log_warnings())
  error_entries <- jsonlite::fromJSON(get_log_errors())

  expect_equal(all_entries$level, c("info", "warning", "error"))
  expect_equal(info_entries$message, "native info")
  expect_equal(warning_entries$message, "native warning")
  expect_equal(error_entries$message, "native error")
})

test_that("native logging validates scalar inputs", {
  native_clear()
  expect_error(log_info(c("one", "two")), "one non-missing string")
  expect_error(log_warning(NA_character_), "one non-missing string")
  expect_error(write_log(NA), "TRUE or FALSE")
  expect_error(set_log_path(character()), "one non-missing string")
  expect_error(set_log_throw_on_error(1), "TRUE or FALSE")
})

test_that("throw-on-error is translated to an R error", {
  native_clear()
  set_log_throw_on_error(TRUE)
  on.exit(set_log_throw_on_error(FALSE), add = TRUE)

  expect_error(log_error("stop from native logger"), "stop from native logger")
  expect_equal(jsonlite::fromJSON(get_log_errors())$message, "stop from native logger")
})
