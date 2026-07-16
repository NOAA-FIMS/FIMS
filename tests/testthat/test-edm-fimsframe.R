test_that("FIMSFrame has an empty edm_embeddings slot by default", {
  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  embs <- get_edm_embeddings(data_4_model)
  expect_type(embs, "list")
  expect_length(embs, 0)
})

test_that("model_edm_matrix() errors when no embeddings exist", {
  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  expect_error(
    model_edm_matrix(data_4_model, "nonexistent"),
    regexp = "No embedding named"
  )
})

test_that("create_edm_embedding() validates bad inputs", {
  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  # Not a FIMSFrame
  expect_error(
    create_edm_embedding(list(), "index", "survey1", E = 3, tau = 1),
    regexp = "FIMSFrame"
  )

  # Bad series_type type
  expect_error(
    create_edm_embedding(data_4_model, 123, "survey1", E = 3, tau = 1),
    regexp = "series_type"
  )

  # Bad E (0)
  expect_error(
    create_edm_embedding(data_4_model, "index", "survey1", E = 0, tau = 1),
    regexp = "E"
  )

  # Bad E (non-integer 3.5)
  expect_error(
    create_edm_embedding(data_4_model, "index", "survey1", E = 3.5, tau = 1),
    regexp = "E"
  )

  # Bad tau (-1)
  expect_error(
    create_edm_embedding(data_4_model, "index", "survey1", E = 3, tau = -1),
    regexp = "tau"
  )

  # Bad tau (non-integer 1.2)
  expect_error(
    create_edm_embedding(data_4_model, "index", "survey1", E = 3, tau = 1.2),
    regexp = "tau"
  )

  # Non-existent series
  expect_error(
    create_edm_embedding(data_4_model, "index", "no_such_fleet", E = 3, tau = 1),
    regexp = "No data found"
  )
})

test_that("create_edm_embedding() stores correct metadata and matrix dimensions", {
  skip_if_not(
    tryCatch({ clear(); TRUE }, error = function(e) FALSE),
    "FIMS module not available"
  )

  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  # Identify an available fleet / type combination
  fleet_nm <- "survey1"
  data_4_model_embedded <- create_edm_embedding(
    data_4_model,
    series_type = "index",
    series_name = fleet_nm,
    E = 3L,
    tau = 1L,
    drop_missing = TRUE
  )

  # Slot is now populated
  embs <- get_edm_embeddings(data_4_model_embedded)
  expect_length(embs, 1)

  key <- names(embs)[1]
  emb <- embs[[key]]

  expect_identical(emb[["E"]], 3L)
  expect_identical(emb[["tau"]], 1L)
  expect_true(emb[["drop_missing"]])
  expect_identical(emb[["n_cols"]], 3L)
  expect_true(emb[["n_rows"]] > 0)
  expect_length(emb[["embedded_values"]], emb[["n_rows"]] * emb[["n_cols"]])
  expect_length(emb[["target_values"]], emb[["n_rows"]])
  clear()
})

test_that("model_edm_matrix() returns a matrix of the correct shape", {
  skip_if_not(
    tryCatch({ clear(); TRUE }, error = function(e) FALSE),
    "FIMS module not available"
  )

  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  fleet_nm <- "survey1"
  data_4_model_embedded <- create_edm_embedding(
    data_4_model,
    series_type = "index",
    series_name = fleet_nm,
    E = 3L,
    tau = 1L
  )

  key <- names(get_edm_embeddings(data_4_model_embedded))[1]
  mat <- model_edm_matrix(data_4_model_embedded, key)

  emb <- get_edm_embeddings(data_4_model_embedded)[[key]]
  expect_true(is.matrix(mat))
  expect_equal(nrow(mat), emb[["n_rows"]])
  expect_equal(ncol(mat), 3L)
  clear()
})

test_that("multiple embeddings can be stored in a single FIMSFrame", {
  skip_if_not(
    tryCatch({ clear(); TRUE }, error = function(e) FALSE),
    "FIMS module not available"
  )

  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  fleet_nm <- "survey1"
  ff <- data_4_model |>
    create_edm_embedding("index", fleet_nm, E = 2L, tau = 1L) |>
    create_edm_embedding("index", fleet_nm, E = 3L, tau = 2L)

  expect_length(get_edm_embeddings(ff), 2)
  clear()
})
test_that("create_edm_embedding() stores uncertainty fields when uncertainty_name is provided", {
  skip_if_not(
    tryCatch({ clear(); TRUE }, error = function(e) FALSE),
    "FIMS module not available"
  )

  # Build a minimal FIMSFrame-compatible dataset that includes both a value
  # series ("survey1") and a matching uncertainty series ("survey1_sd").
  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  # Grab the existing survey1 index rows and fabricate a matching sd series
  base_data <- get_data(data_4_model) |>
    dplyr::filter(.data[["type"]] == "index",
                  .data[["fleet"]] == "survey1") |>
    dplyr::arrange(.data[["timing"]])

  sd_rows <- base_data |>
    dplyr::mutate(
      fleet = "survey1_sd",
      value = 0.1 * .data[["value"]]
    )

  extended_data <- dplyr::bind_rows(get_data(data_4_model), sd_rows)
  data_4_model2 <- FIMSFrame(extended_data)

  ff <- create_edm_embedding(
    data_4_model2,
    series_type     = "index",
    series_name     = "survey1",
    E               = 3L,
    tau             = 1L,
    drop_missing    = TRUE,
    uncertainty_name = "survey1_sd"
  )

  emb <- get_edm_embeddings(ff)[[1]]

  #' @description Test that embedded_uncertainty is present and correctly sized when uncertainty_name is given.
  expect_length(emb[["embedded_uncertainty"]], emb[["n_rows"]] * emb[["n_cols"]])
  #' @description Test that target_uncertainty is present and correctly sized when uncertainty_name is given.
  expect_length(emb[["target_uncertainty"]], emb[["n_rows"]])

  # target_uncertainty should equal 0.1 * target_values (proportional)
  #' @description Test that target_uncertainty values are proportional to target_values.
  expect_equal(emb[["target_uncertainty"]], 0.1 * emb[["target_values"]],
               tolerance = 1e-10)

  clear()
})

test_that("create_edm_embedding() does not store uncertainty fields when uncertainty_name is NULL", {
  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  skip_if_not(
    tryCatch({ clear(); TRUE }, error = function(e) FALSE),
    "FIMS module not available"
  )

  ff <- create_edm_embedding(
    data_4_model,
    series_type  = "index",
    series_name  = "survey1",
    E            = 3L,
    tau          = 1L,
    drop_missing = TRUE
  )

  emb <- get_edm_embeddings(ff)[[1]]

  #' @description Test that embedded_uncertainty is absent from the embedding list when no uncertainty_name is supplied.
  expect_false("embedded_uncertainty" %in% names(emb))
  #' @description Test that target_uncertainty is absent from the embedding list when no uncertainty_name is supplied.
  expect_false("target_uncertainty" %in% names(emb))

  clear()
})
