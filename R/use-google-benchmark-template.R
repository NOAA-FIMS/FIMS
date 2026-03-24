#' Create tests/google_benchmark/benchmark_*.cpp and register it in CMakeLists.txt
#'
#' This helper function generates a Google Benchmark C++ template file for a
#' given workload and appends lines to `tests/google_benchmark/CMakeLists.txt`
#' to register the benchmark as a separate executable.
#'
#' @param name A string representing the name for the C++ benchmark file and
#'   CMake executable target. Use a short, descriptive identifier with
#'   underscores (e.g. `"population_CatchNumbersAtAge"`). The generated file
#'   will be `tests/google_benchmark/benchmark_<name>.cpp` and the CMake
#'   target will be `benchmark_<name>`. The name must be a valid C++
#'   identifier (letters, numbers, underscores only).
#'
#' @return
#' If successful, this function invisibly returns `TRUE` to allow for the
#' chaining of commands. If the function is unsuccessful, an error message is
#' returned.
#'
#' Messages from the usethis package and cli indicate where the new file was
#' created and that the benchmark was registered in
#' `tests/google_benchmark/CMakeLists.txt`.
#'
#' @examples
#' \dontrun{
#' # Create a new benchmark file and register it in CMake:
#' FIMS:::use_google_benchmark_template(name = "population_CatchNumbersAtAge")
#' }
#'
#' @keywords developer
use_google_benchmark_template <- function(name = "population_ModuleName_Workload") {
  bm_dir <- file.path("tests", "google_benchmark")
  cmakelist_path <- file.path(bm_dir, "CMakeLists.txt")

  if (!file.exists(cmakelist_path)) {
    cli::cli_abort(
      c("{.file {cmakelist_path}} does not exist.",
        "i" = "Please ensure that the CMakeLists.txt file is present in the
        {.file tests/google_benchmark} directory."
      )
    )
  }

  # Validate name: non-empty and valid C++ identifier (letters, numbers, underscores)
  if (!nzchar(name)) {
    cli::cli_abort(
      c("Invalid {.var name}.",
        "i" = "{.var name} must be a non-empty string."
      )
    )
  }
  if (!grepl("^[A-Za-z][A-Za-z0-9_]*$", name)) {
    cli::cli_abort(
      c("Invalid {.var name} format.",
        "i" = "Use a valid C++ identifier (letters, numbers, underscores only).",
        "x" = "Received name: {.val {name}}."
      )
    )
  }

  path <- file.path(bm_dir, paste0("benchmark_", name, ".cpp"))
  if (file.exists(path)) {
    cli::cli_abort("{.file {path}} already exists.")
  }

  usethis::use_template(
    template = "google_benchmark_template.cpp",
    save_as = path,
    data = list(benchmark_name = name),
    package = "FIMS"
  )

  CON <- file(cmakelist_path, "a")
  on.exit(close(CON), add = TRUE)
  cmake_block <- glue::glue("
    # benchmark_{name}.cpp
    add_executable(benchmark_{name}
      benchmark_{name}.cpp
    )

    target_link_libraries(benchmark_{name}
      benchmark::benchmark_main
      fims_test
      GTest::gtest
    )")
  writeLines(c("\n", cmake_block), CON)
  cli::cli_alert_success(
    "Registering benchmark {.val benchmark_{name}} in {.file {cmakelist_path}}."
  )

  invisible(TRUE)
}
