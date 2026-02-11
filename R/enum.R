Selectivity <- local({
  e <- new.env(parent = emptyenv())
  e$LOGISTIC    <- 0L
  e$DOUBLE_LOGISTIC <- 1L
  e$RANDOM_WALK <- 2L
  lockEnvironment(e, bindings = TRUE)
  e
})