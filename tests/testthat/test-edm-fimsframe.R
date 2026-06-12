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
    tryCatch({ fims::clear(); TRUE }, error = function(e) FALSE),
    "FIMS module not available"
  )

  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  # Identify an available fleet / type combination
  fleet_nm <- get_fleets(data_4_model)[1]
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
  expect_length(emb[["values"]], emb[["n_rows"]] * emb[["n_cols"]])
  expect_length(emb[["target_indices"]], emb[["n_rows"]])
  fims::clear()
})

test_that("model_edm_matrix() returns a matrix of the correct shape", {
  skip_if_not(
    tryCatch({ fims::clear(); TRUE }, error = function(e) FALSE),
    "FIMS module not available"
  )

  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  fleet_nm <- get_fleets(data_4_model)[1]
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
  fims::clear()
})

test_that("multiple embeddings can be stored in a single FIMSFrame", {
  skip_if_not(
    tryCatch({ fims::clear(); TRUE }, error = function(e) FALSE),
    "FIMS module not available"
  )

  data("data_big", package = "FIMS")
  data_4_model <- FIMSFrame(data_big)

  fleet_nm <- get_fleets(data_4_model)[1]
  ff <- data_4_model |>
    create_edm_embedding("index", fleet_nm, E = 2L, tau = 1L) |>
    create_edm_embedding("index", fleet_nm, E = 3L, tau = 2L)

  expect_length(get_edm_embeddings(ff), 2)
  fims::clear()
})
