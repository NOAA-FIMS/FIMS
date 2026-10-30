make_mapping_input <- function() {
  data <- FIMS::data_big |>
    dplyr::filter(
      is.na(.data[["timing"]]) |
        .data[["timing"]] <= dplyr::if_else(
          .data[["type"]] == "weight_at_age", 6, 5
        )
    ) |>
    FIMSFrame()
  setup_default_parameters(data) |>
    initialize_fims(data = data)
}

test_that("TMB map names omit fixed parameters and collapse shared levels", {
  parameter_names <- c("alpha", "beta", "gamma", "delta")
  map <- factor(c(1, 1, NA, 2))

  expect_equal(
    FIMS:::tmb_mapped_parameter_names(parameter_names, map),
    c("alpha", "delta")
  )
})

test_that("TMB map names preserve all names without a map", {
  parameter_names <- c("alpha", "beta")
  expect_equal(
    FIMS:::tmb_mapped_parameter_names(parameter_names),
    parameter_names
  )
})

test_that("TMB map names validate the map", {
  expect_error(
    FIMS:::tmb_mapped_parameter_names(c("alpha", "beta"), c(1, 2)),
    "must be a factor"
  )
  expect_error(
    FIMS:::tmb_mapped_parameter_names(c("alpha", "beta"), factor(1)),
    "one entry per FIMS parameter"
  )
})

test_that("TMB mapped values remain aligned in FIMS output", {
  clear()
  on.exit(clear(), add = TRUE)

  selectivities <- lapply(seq_len(2), function(index) {
    selectivity <- methods::new(LogisticSelectivity)
    selectivity$inflection_point[1]$value <- 0.1
    selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
    selectivity$slope[1]$value <- 0.1
    selectivity$slope[1]$estimation_type$set("fixed_effects")
    selectivity
  })
  CreateTMBModel()

  initial_parameters <- get_fixed()
  map <- factor(c(1, 1, NA, 2))
  obj <- TMB::MakeADFun(
    data = list(),
    parameters = list(p = initial_parameters, re = get_random()),
    map = list(p = map),
    random = "re",
    DLL = "FIMS",
    silent = TRUE
  )
  full_names <- names(get_parameter_names(as.list(initial_parameters)))
  names(obj$par) <- FIMS:::tmb_mapped_parameter_names(full_names, map)
  opt <- list(par = unname(obj$par) + c(0.5, 1))
  obj$env$last.par.best <- opt$par
  expanded_parameters <- obj$env$parList(par = obj$env$last.par.best)$p
  FIMS:::restore_tmb_parameters(obj)
  estimates <- FIMS:::reshape_tmb_estimates(
    obj = obj,
    opt = opt,
    parameter_names = names(obj$par)
  )

  expect_length(obj$par, 2L)
  expect_identical(names(obj$par), full_names[c(1, 4)])
  expect_equal(get_fixed(), expanded_parameters)
  expect_equal(
    get_fixed(),
    c(opt$par[1], opt$par[1], initial_parameters[3], opt$par[2])
  )
  expect_equal(estimates$initial, unname(obj$par))
  expect_equal(estimates$estimate, unname(opt$par))

  TMB::FreeADFun(obj)
  rm(obj)
})

for (random_map in list(factor(c(1, NA, 1, 2)), factor(rep(NA, 4)))) {
  test_that(paste("best joint parameters restore", nlevels(random_map), "random levels"), {
    clear()
    on.exit(clear(), add = TRUE)

    selectivities <- lapply(seq_len(4), function(index) {
      selectivity <- methods::new(LogisticSelectivity)
      selectivity$inflection_point[1]$value <- 10
      selectivity$inflection_point[1]$estimation_type$set("fixed_effects")
      selectivity$slope[1]$value <- 0.1
      selectivity$slope[1]$estimation_type$set("random_effects")
      selectivity
    })
    CreateTMBModel()
    initial_fixed <- get_fixed()
    initial_random <- get_random()

    # Cover shared levels, fixed entries, and a fully mapped-off random vector.
    obj <- TMB::MakeADFun(
      data = list(),
      parameters = list(p = initial_fixed, re = initial_random),
      map = list(p = factor(c(1, 1, NA, 2)), re = random_map),
      random = "re",
      DLL = "FIMS",
      silent = TRUE
    )
    # Distinct best and current vectors catch accidental use of last.par.
    best <- obj$env$last.par.best
    best[obj$env$lfixed()] <- c(20, 30)
    best[obj$env$lrandom()] <- seq_len(sum(obj$env$lrandom())) + 0.5
    obj$env$last.par.best <- best
    obj$env$last.par <- best + 10

    expect_no_warning(FIMS:::restore_tmb_parameters(obj))
    expect_equal(get_fixed(), c(20, 20, initial_fixed[3], 30))
    expected_random <- if (all(is.na(random_map))) {
      initial_random
    } else {
      c(1.5, initial_random[2], 1.5, 2.5)
    }
    expect_equal(get_random(), expected_random)

    fit <- FIMSFit(
      input = list(
        parameters = list(p = initial_fixed, re = initial_random),
        map = list(p = factor(c(1, 1, NA, 2)), re = random_map),
        model = list(get_output = function() "{}")
      ),
      obj = obj
    )
    expect_equal(
      unname(fit@number_of_parameters),
      c(2, nlevels(random_map))
    )
    rm(obj, fit)
    gc()
  })
}

test_that("TMB map metadata validates inputs and shared starting values", {
  #' @description Invalid maps and conflicting shared starts fail before fitting.
  clear()
  on.exit(clear(), add = TRUE)
  input <- make_mapping_input()
  input[["map"]] <- list(unknown = factor(1))
  expect_error(fit_fims(input), "named list")
  input[["map"]] <- list(p = seq_along(input[["parameters"]][["p"]]))
  expect_error(fit_fims(input), "must be a factor")
  input[["map"]] <- list(re = factor(1))
  expect_error(fit_fims(input), "one entry per FIMS parameter")
  mapping <- seq_along(input[["parameters"]][["p"]])
  mapping[2] <- mapping[1]
  input[["map"]] <- list(p = factor(mapping))
  input[["parameters"]][["p"]][2] <- input[["parameters"]][["p"]][1] + 1
  expect_error(fit_fims(input), "identical starting values")
})

test_that("optimized fixed maps preserve shared uncertainty and fixed entries", {
  #' @description Reordered fixed maps expand estimates and uncertainty after clear().
  clear()
  on.exit(clear(), add = TRUE)
  input <- make_mapping_input()
  full_names <- names(get_parameter_names(as.list(input[["parameters"]][["p"]])))
  shared <- utils::head(which(grepl("log_Fmort", full_names)), 2)
  fishing <- which(grepl("log_Fmort", full_names))
  mapping <- rep(NA_integer_, length(full_names))
  mapping[fishing] <- seq_along(fishing)
  mapping[shared[2]] <- mapping[shared[1]]
  input[["map"]] <- list(p = factor(mapping, levels = rev(unique(stats::na.omit(mapping)))))
  fit <- fit_fims(
    input, number_of_loops = 0,
    control = list(eval.max = 10000, iter.max = 10000, rel.tol = 1e-12)
  )
  metadata <- get_input(fit)[["tmb_mapping"]][["p"]]
  expect_equal(names(get_obj(fit)[["par"]]), metadata[["reduced_names"]])
  expect_equal(names(get_opt(fit)[["par"]]), metadata[["reduced_names"]])
  expect_equal(get_number_of_parameters(fit)[["fixed_effects"]], length(fishing) - 1L)
  expect_equal(get_fixed()[shared[1]], get_fixed()[shared[2]])
  expect_equal(get_fixed()[1], input[["parameters"]][["p"]][1])
  expanded <- FIMS:::reshape_tmb_estimates(
    obj = get_obj(fit), sdreport = get_sdreport(fit), opt = get_opt(fit),
    parameter_names = metadata[["reduced_names"]],
    precomputed_gradient = get_gradient(fit), mapping = get_input(fit)[["tmb_mapping"]]
  )
  expect_equal(expanded[["uncertainty"]][shared[1]], expanded[["uncertainty"]][shared[2]])
  expect_true(is.finite(expanded[["uncertainty"]][shared[1]]))
  expect_true(is.na(expanded[["uncertainty"]][1]))
  expect_true(is.na(expanded[["gradient"]][shared[2]]))
  output <- get_estimates(fit)
  shared_ids <- as.integer(sub(".*\\.", "", full_names[shared]))
  shared_output <- output |>
    dplyr::filter(.data[["parameter_id"]] %in% shared_ids)
  expect_equal(nrow(shared_output), 2L)
  expect_equal(shared_output[["uncertainty"]][1], shared_output[["uncertainty"]][2])
  clear()
  expect_equal(get_estimates(fit), output)
})

test_that("optimized random maps expand shared values and standard errors", {
  #' @description Random maps preserve shared uncertainty and mapped-off values.
  clear()
  on.exit(clear(), add = TRUE)
  input <- make_mapping_input()
  full_names <- names(get_parameter_names(as.list(input[["parameters"]][["p"]])))
  fishing <- which(grepl("log_Fmort", full_names))
  mapping <- rep(NA_integer_, length(full_names))
  mapping[fishing] <- seq_along(fishing)
  input[["map"]] <- list(p = factor(mapping), re = factor(c(1, NA, 1, 2)))
  fit <- fit_fims(
    input, number_of_loops = 0,
    control = list(eval.max = 10000, iter.max = 10000, rel.tol = 1e-12)
  )
  metadata <- get_input(fit)[["tmb_mapping"]][["re"]]
  expect_equal(get_number_of_parameters(fit)[["random_effects"]], 2L)
  expect_equal(names(get_sdreport(fit)[["par.random"]]), metadata[["reduced_names"]])
  expect_equal(get_random()[1], get_random()[3])
  expect_equal(get_random()[2], input[["parameters"]][["re"]][2])
  output <- get_estimates(fit) |>
    dplyr::filter(.data[["estimation_type"]] == "random_effects")
  expect_equal(nrow(output), 4L)
  expect_equal(output[["uncertainty"]][1], output[["uncertainty"]][3])
  expect_true(is.finite(output[["uncertainty"]][1]))
  expect_true(is.na(output[["uncertainty"]][2]))
  clear()
  expect_equal(
    get_estimates(fit) |> dplyr::filter(.data[["estimation_type"]] == "random_effects"),
    output
  )
})

test_that("unmapped random effects still fit and report uncertainty", {
  #' @description Unmapped models retain finite random-effect standard errors.
  clear()
  on.exit(clear(), add = TRUE)
  fit <- make_mapping_input() |> fit_fims(number_of_loops = 0)
  expect_equal(unname(get_number_of_parameters(fit)), c(24L, 4L))
  output <- get_estimates(fit) |>
    dplyr::filter(.data[["estimation_type"]] == "random_effects")
  expect_true(all(is.finite(output[["uncertainty"]])))
})

test_that("fully mapped-off vectors work without an outer optimizer", {
  #' @description Empty active vectors have defined counts and optimization behavior.
  clear()
  on.exit(clear(), add = TRUE)
  input <- make_mapping_input()
  input[["map"]] <- list(p = factor(rep(NA, length(input[["parameters"]][["p"]]))))
  fit <- fit_fims(input, number_of_loops = 0)
  expect_equal(unname(get_number_of_parameters(fit)), c(0L, 4L))
  expect_equal(get_max_gradient(fit), 0)
  expect_equal(get_fixed(), input[["parameters"]][["p"]])
  input[["map"]][["re"]] <- factor(rep(NA, length(input[["parameters"]][["re"]])))
  expect_error(fit_fims(input), "at least one parameter")
  fit <- fit_fims(input, optimize = FALSE)
  expect_equal(unname(get_number_of_parameters(fit)), c(0L, 0L))
  expect_true(all(is.na(get_estimates(fit)[["uncertainty"]])))
})

test_that("fully mapped-off random effects support fitting without sdreport", {
  #' @description Mapped-off random vectors support optimized and initial fits.
  clear()
  on.exit(clear(), add = TRUE)
  input <- make_mapping_input()
  full_names <- names(get_parameter_names(as.list(input[["parameters"]][["p"]])))
  fishing <- which(grepl("log_Fmort", full_names))
  mapping <- rep(NA_integer_, length(full_names))
  mapping[fishing] <- seq_along(fishing)
  input[["map"]] <- list(
    p = factor(mapping),
    re = factor(rep(NA, length(input[["parameters"]][["re"]])))
  )
  fit <- fit_fims(input, get_sd = FALSE, number_of_loops = 0)
  expect_equal(unname(get_number_of_parameters(fit)), c(length(fishing), 0L))
  expect_equal(get_opt(fit)[["convergence"]], 0L)
  expect_equal(get_random(), input[["parameters"]][["re"]])
  expect_true(all(is.na(get_estimates(fit)[["uncertainty"]])))
  fit_with_sd <- fit_fims(input, number_of_loops = 0)
  expect_length(get_sdreport(fit_with_sd)[["par.random"]], 0L)
  expect_true(all(is.na(
    get_estimates(fit_with_sd) |>
      dplyr::filter(.data[["estimation_type"]] == "random_effects") |>
      dplyr::pull("uncertainty")
  )))
  initial <- fit_fims(input, optimize = FALSE)
  expect_equal(get_fixed(), input[["parameters"]][["p"]])
  expect_length(get_estimates(initial)[["gradient"]], nrow(get_estimates(initial)))
})

test_that("nonoptimized shared maps preserve full initial vectors", {
  #' @description Shared maps use full vectors without optimization or standard errors.
  clear()
  on.exit(clear(), add = TRUE)
  input <- make_mapping_input()
  full_names <- names(get_parameter_names(as.list(input[["parameters"]][["p"]])))
  shared <- utils::head(which(grepl("log_Fmort", full_names)), 2)
  mapping <- seq_along(full_names)
  mapping[shared[2]] <- mapping[shared[1]]
  input[["map"]] <- list(
    p = factor(mapping, levels = c("unused", rev(as.character(unique(mapping))))),
    re = factor(c(1, NA, 1, 2))
  )
  fit <- fit_fims(input, optimize = FALSE)
  expect_equal(get_fixed(), input[["parameters"]][["p"]])
  expect_equal(get_random(), input[["parameters"]][["re"]])
  output <- get_estimates(fit)
  expect_true(all(is.na(output[["uncertainty"]])))
  clear()
  expect_equal(get_estimates(fit), output)
})

test_that("mapped diagnostic fits retain consistent best parameters", {
  #' @description Nonconverged mapped fits restore the parameters returned by the optimizer.
  clear()
  on.exit(clear(), add = TRUE)
  input <- make_mapping_input()
  mapping <- seq_along(input[["parameters"]][["p"]])
  mapping[1] <- NA_integer_
  input[["map"]] <- list(p = factor(mapping))
  expect_warning(
    fit <- fit_fims(
      input, get_sd = FALSE, number_of_loops = 0,
      control = list(eval.max = 1, iter.max = 1)
    ),
    "Optimization failed convergence checks"
  )
  metadata <- get_input(fit)[["tmb_mapping"]][["p"]]
  active <- !is.na(metadata[["index"]])
  expect_equal(
    get_fixed()[active],
    unname(get_opt(fit)[["par"]][metadata[["index"]][active]])
  )
  expect_equal(get_fixed()[1], input[["parameters"]][["p"]][1])
})
