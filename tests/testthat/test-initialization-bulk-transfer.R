test_that("parameter initialization preserves mixed types, order and identities", {
  on.exit(FIMS::clear(), add = TRUE)
  module <- methods::new(FIMS::Population)
  rows <- tibble::tibble(
    label = c("log_M", "log_init_naa", "log_M", "log_M"),
    value = c(-2, 100, -1, -3),
    estimation_type = c("constant", "constant", "fixed_effects", "random_effects")
  )
  FIMS:::set_param_vector("log_M", module, rows, "Population")
  target <- module$log_M
  values <- function() vapply(seq_len(target$size()), function(i) target$get(i - 1)$value, numeric(1))
  types <- function() vapply(seq_len(target$size()), function(i) target$get(i - 1)$estimation_type$get(), character(1))
  ids <- function() vapply(seq_len(target$size()), function(i) target$get(i - 1)$id, numeric(1))
  expect_equal(values(), c(-2, -1, -3))
  expect_equal(types(), c("constant", "fixed_effects", "random_effects"))
  original_ids <- ids()
  vector_id <- target$get_id()
  rows$value <- rows$value + 0.25
  rows$estimation_type[c(1, 3, 4)] <- c("random_effects", "constant", "fixed_effects")
  FIMS:::set_param_vector("log_M", module, rows, "Population")
  expect_identical(target$get_id(), vector_id)
  expect_identical(ids(), original_ids)
  expect_equal(values(), c(-1.75, -0.75, -2.75))
  expect_equal(types(), c("random_effects", "constant", "fixed_effects"))
  # Resizing must preserve surviving identities and reset values/types in order.
  FIMS:::set_param_vector("log_M", module, rows[c(1, 3), ], "Population")
  expect_equal(target$size(), 2)
  expect_identical(ids(), original_ids[1:2])
  expect_equal(values(), c(-1.75, -0.75))
  expect_equal(types(), c("random_effects", "constant"))
  missing <- rows[c(1, 3), ]
  missing$estimation_type[1] <- NA_character_
  FIMS:::set_param_vector("log_M", module, missing, "Population")
  expect_identical(ids(), original_ids[1:2])
  expect_equal(types(), c("NA", "constant"))
  expect_equal(values(), missing$value)
  expect_error(FIMS:::set_param_vector("log_M", module, rows[2, ], "Population"),
               "Missing value or estimation_type")
})
