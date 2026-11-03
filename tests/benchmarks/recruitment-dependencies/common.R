library(FIMS)
library(Matrix)
options(rlib_message_verbosity = "quiet")

set_diagnostic <- function(centered = FALSE, no_plus = FALSE,
                           no_feedback = FALSE, lifted = FALSE) {
  Sys.setenv(
    FIMS_DIAG_CENTERED = as.integer(centered),
    FIMS_DIAG_NO_PLUS = as.integer(no_plus),
    FIMS_DIAG_NO_FEEDBACK = as.integer(no_feedback),
    FIMS_DIAG_LIFT_PLUS = as.integer(lifted)
  )
}

# Repeat the observed 30-year time series for scaling experiments. These are
# synthetic timing workloads, not additional observations for an assessment.
make_input <- function(years, centered = FALSE) {
  data("data_big", package = "FIMS", envir = environment())
  fixed <- data_big[is.na(data_big$timing), ]
  observed <- data_big[!is.na(data_big$timing) & data_big$type != "weight_at_age", ]
  weight <- data_big[data_big$type == "weight_at_age", ]
  observations <- lapply(seq_len(years), function(y) {
    x <- observed[observed$timing == (y - 1L) %% 30L + 1L, ]
    x$timing <- y
    x
  })
  weights <- lapply(seq_len(years + 1L), function(y) {
    source_year <- if (y <= 31L) y else (y - 1L) %% 30L + 1L
    x <- weight[weight$timing == source_year, ]
    x$timing <- y
    x
  })
  frame <- FIMSFrame(do.call(rbind, c(list(fixed), observations, weights)))
  parameters <- setup_default_parameters(data = frame)
  if (centered) parameters$value[parameters$label == "log_devs"] <- log(1e6)
  input <- initialize_fims(parameters, data = frame)
  # Hold all fixed effects constant: isolate the recruitment Hessian and inner
  # problem, rather than conflate this experiment with an outer model fit.
  input$map$p <- factor(rep(NA, length(input$parameters$p)))
  stopifnot(length(input$parameters$re) == years - 1L)
  input$parameters$diagnostic_plus_states <- numeric(0)
  list(input = input, ages = get_n_ages(frame), years = years,
       rzero = exp(parameters$value[parameters$label == "log_rzero"]))
}

make_objective <- function(context, states = numeric(0),
                           lambda = rep(0, length(states)),
                           constraints_only = FALSE) {
  parameters <- context$input$parameters
  parameters$diagnostic_plus_states <- states
  raw <- TMB::MakeADFun(
    data = list(diagnostic_plus_lambda = lambda,
                diagnostic_constraints_only = as.integer(constraints_only)),
    parameters = parameters, map = context$input$map,
    random = c("re", if (length(states)) "diagnostic_plus_states"),
    DLL = "FIMS", silent = TRUE
  )
  # Mark latent coordinates random so TMB constructs its sparse AD Hessian,
  # but call the joint objective directly (no Laplace/inner solver here).
  list(par = raw$env$par,
       fn = function(x) raw$env$f(x, order = 0),
       gr = function(x) as.numeric(raw$env$f(x, order = 1)),
       report = raw$report, env = raw$env)
}

time_calls <- function(f, n = 50L) {
  f()
  n <- min(n, 2L)
  batch <- function(n) {
    start <- proc.time()
    for (i in seq_len(n)) f()
    delta <- proc.time() - start
    unname(sum(delta[c("user.self", "sys.self")]))
  }
  while (batch(n) < 0.05 && n < 50000L) n <- n * 2L
  times <- replicate(3L, {
    # CPU time avoids wall-clock changes/suspended intervals on this host.
    batch(n) / n
  })
  median(times)
}

hessian_probe <- function(obj, point) {
  counter <- 0L
  function() {
    counter <<- counter + 1L
    x <- point
    x[1] <- x[1] + (counter %% 2L) * 1e-7
    obj$env$spHess(x)
  }
}

factor_fresh <- function(h) {
  h@factors <- list()
  Matrix::Cholesky(h, LDL = FALSE, perm = TRUE, super = FALSE)
}

relative_error <- function(actual, expected) {
  max(abs(actual - expected)) / max(1, max(abs(expected)))
}
