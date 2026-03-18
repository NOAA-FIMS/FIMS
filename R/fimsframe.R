# To remove the NOTE
# no visible binding for global variable
utils::globalVariables(c(
  "name", "timing", "value"
))

# Developers: ----

# This file defines the parent class FIMSFrame and its potential children. The
# class is an S4 class with accessors and validators but no setters.
#
# The top of this file contains the declaration of the FIMSFrame class, which
# is the controller of everything. Then the function FIMSFrame() is how objects
# of that class are created, i.e., the constructor, and how users will interact
# with the class the most. When the returned object from that constructor are
# changed, the call to methods::setClass() that defines the class must also be
# changed. The remainder of the file is set up to help you easily augment this
# class. Follow the step-by-step instructions in order or at least know that
# the functions are present in this order:
#
# 1. Add or remove the slot of interest in the call to `methods::setClass()`,
#    e.g., if you are adding a new slot you must declare the slot and the type
#    of object that should be expected in that slot; to remove an object from
#    the FIMSFrame class you must remove the slot here.
# 2. Add an accessor function, e.g., get_*(), to allow users to access the
#    object stored in the new slot; or, remove the accessor function if you
#    remove a slot. Some accessors are also available for data types, e.g.,
#    model_*(), that provide vectorized data to use in a module.
# 3. If we had setter functions for FIMSFrame, you would add or delete the
#    appropriate setter functions next but we do not. Instead, we want users to
#    re-run FIMSFrame() when they make any changes to their data, that way all
#    of the slots will be updated simultaneously. @nathanvaughan-NOAA mentioned
#    during Code club 2024-12-17 that this may be a problem for future use of
#    FIMSFrame objects, especially when doing MSE or simulation when there is a
#    large overhead in running FIMSFrame and you just want to change a small,
#    simple thing in your data and re-run the model. We will cross that bridge
#    later. @msupernaw also informed us about the ability to lock an R object
#    so it cannot be altered. See https://rdrr.io/r/base/bindenv.html.
# 4. Augment the validator functions to ensure that users do not pass
#    incompatible information to FIMSFrame().
# 5. Augment FIMSFrame() to ensure that the slot is created if you are adding a
#    new object or remove the object from the returned object if you are
#    removing a slot.

# TODO: ----

# TODO: make date_formats a local variable
# TODO: document sorting of information in terms of alphabetized fleet order
# TODO: test implement addition of -999
# TODO: validate that all length-age combinations exist in the conversion matrix

# methods::setClass: ----

# Classes are not currently exported, and therefore, do not need documentation.
# See the following link if we do want to document them in the future:
# https://stackoverflow.com/questions/7368262/how-to-properly-document-s4-class-slots-using-roxygen2

methods::setClass(
  Class = "FIMSFrame",
  slots = c(
    data = "tbl_df",
    fleets = "character",
    n_years = "integer",
    ages = "numeric",
    n_ages = "integer",
    lengths = "numeric",
    n_lengths = "integer",
    start_year = "integer",
    end_year = "integer"
  )
)

# methods::setMethod: accessors ----

# Methods for accessing info in the slots using get_*() or model_*()

#' Get a slot in a FIMSFrame object
#'
#' There is an accessor function for each slot in the S4 class `FIMSFrame`,
#' where the function is named `get_*()` and the star can be replaced with the
#' slot name, e.g., [get_data()]. These accessor functions are the preferred
#' way to access objects stored in the available slots.
#'
#' @param x An object returned from [FIMSFrame()].
#' @name get_FIMSFrame
#' @keywords FIMSFrame
NULL

#' @return
#' [get_data()] returns a data frame of the class `tbl_df` containing data for
#' a FIMS model in a long format. The tibble will potentially have the
#' following columns depending if it fits to ages and lengths or just one of
#' them:
#' `r glue::glue_collapse(colnames(data_big), sep = ", ", last = ", and ")`.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric("get_data", function(x) standardGeneric("get_data"))
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_data", "FIMSFrame", function(x) x@data)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_data",
  "data.frame",
  function(x) FIMSFrame(x)@data
)

#' @return
#' [get_fleets()] returns a vector of strings containing the fleet names.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric("get_fleets", function(x) standardGeneric("get_fleets"))
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_fleets", "FIMSFrame", function(x) x@fleets)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_fleets",
  "data.frame",
  function(x) FIMSFrame(x)@fleets
)

#' @return
#' [get_n_fleets()] returns an integer specifying the number of fleets in the
#' model, where fleets is inclusive of both fishing fleets and survey vessels.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric("get_n_fleets", function(x) standardGeneric("get_n_fleets"))
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_n_fleets", "FIMSFrame", function(x) length(x@fleets))
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_n_fleets",
  "data.frame",
  function(x) length(FIMSFrame(x)@fleets)
)

#' @return
#' [get_n_years()] returns an integer specifying the number of years in the
#' model.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric("get_n_years", function(x) standardGeneric("get_n_years"))
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_n_years", "FIMSFrame", function(x) x@n_years)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_n_years",
  "data.frame",
  function(x) FIMSFrame(x)@n_years
)

#' @return
#' [get_start_year()] returns an integer specifying the start year of the
#' model.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric(
  "get_start_year",
  function(x) standardGeneric("get_start_year")
)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_start_year", "FIMSFrame", function(x) x@start_year)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_start_year",
  "data.frame",
  function(x) FIMSFrame(x)@start_year
)

#' @return
#' [get_end_year()] returns an integer specifying the end year of the
#' model.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric("get_end_year", function(x) standardGeneric("get_end_year"))
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_end_year", "FIMSFrame", function(x) x@end_year)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_end_year",
  "data.frame",
  function(x) FIMSFrame(x)@end_year
)

#' @return
#' [get_ages()] returns a vector of age bins used in the model.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric("get_ages", function(x) standardGeneric("get_ages"))
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_ages", "FIMSFrame", function(x) x@ages)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_ages",
  "data.frame",
  function(x) FIMSFrame(x)@ages
)

#' @return
#' [get_n_ages()] returns an integer specifying the number of age bins used in
#' the model.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric("get_n_ages", function(x) standardGeneric("get_n_ages"))
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_n_ages", "FIMSFrame", function(x) x@n_ages)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_n_ages",
  "data.frame",
  function(x) FIMSFrame(x)@n_ages
)

#' @return
#' [get_lengths()] returns a vector of length bins used in the model.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric("get_lengths", function(x) standardGeneric("get_lengths"))
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_lengths", "FIMSFrame", function(x) x@lengths)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_lengths",
  "data.frame",
  function(x) FIMSFrame(x)@lengths
)

#' @return
#' [get_n_lengths()] returns an integer specifying the number of length bins
#' used in the model.
#' @export
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setGeneric(
  "get_n_lengths",
  function(x) standardGeneric("get_n_lengths")
)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod("get_n_lengths", "FIMSFrame", function(x) x@n_lengths)
#' @rdname get_FIMSFrame
#' @keywords FIMSFrame
methods::setMethod(
  "get_n_lengths",
  "data.frame",
  function(x) FIMSFrame(x)@n_lengths
)

#' Get a vector of data to be passed to a FIMS module from a FIMSFrame object
#'
#' There is an accessor function for each data type needed to run a FIMS model.
#' A FIMS model accepts vectors of data and thus each of the `model_*()`
#' functions, where the star can be replaced with the data type separated by
#' underscores, e.g., weight_at_age. These accessor functions are the preferred
#' way to pass data to a FIMS module because the data will have the appropriate
#' indexing.
#'
#' @details
#' `Age_to_length_conversion` data, i.e., the proportion of age "a" that are
#' length "l", are used to convert lengths (input data) to ages (modeled) as
#' a way to fit length data without estimating growth.
#'
#' @inheritParams get_data
#' @param fleet_name A string, or vector of strings, specifying the name of the
#'   fleet(s) of interest that you want landings data for. The strings must
#'   exactly match strings in the column `"name"` of `get_data(x)`.
#' @return
#' All of the `model_*()` functions return vectors of data. Currently, the
#' order of the data is the same order as the data frame because no arranging
#' is done in [FIMSFrame()] and the function just extracts the appropriate
#' column.
#' @name model_
#' @keywords FIMSFrame
NULL

#' @details
#' ## `model_landings()`
#' Returns a numeric vector of landings data (type `"landings"`) for the
#' specified fleet(s).
#'
#' ## `model_index()`
#' Returns a numeric vector of index data (type `"index"`) for the specified
#' fleet(s).
#'
#' ## `model_age_comp()`
#' Returns a numeric vector of age-composition data (type `"age_comp"`) for the
#' specified fleet(s).
#'
#' ## `model_length_comp()`
#' Returns a numeric vector of length-composition data (type `"length_comp"`)
#' for the specified fleet(s). This accessor requires that age-to-length
#' conversion data are present in the `FIMSFrame` object.
#'
#' ## `model_weight_at_age()`
#' Returns a numeric vector of weight-at-age values (type `"weight_at_age"`),
#' ordered by age and year (plus one additional year) and using `-999` to
#' indicate missing values. When multiple fleets are present, values are
#' averaged across fleets.
#'
#' ## `model_age_to_length_conversion()`
#' Returns a numeric vector of age-to-length conversion values (type
#' `"age_to_length_conversion"`) for the specified fleet(s). Values are ordered
#' by age and length in the order defined by [FIMSFrame()].
#'
#' @export
#' @rdname model_
#' @keywords FIMSFrame
methods::setGeneric(
  "model_landings",
  function(x, fleet_name) standardGeneric("model_landings")
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_landings", "FIMSFrame",
  function(x, fleet_name) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "landings",
      .data[["name"]] %in% fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_landings",
  "data.frame",
  function(x, fleet_name) model_landings(FIMSFrame(x), fleet_name)
)

#' @export
#' @rdname model_
#' @keywords FIMSFrame
methods::setGeneric(
  "model_index",
  function(x, fleet_name) standardGeneric("model_index")
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_index", "FIMSFrame",
  function(x, fleet_name) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "index",
      .data[["name"]] %in% fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_index",
  "data.frame",
  function(x, fleet_name) model_index(FIMSFrame(x), fleet_name)
)

#' @export
#' @rdname model_
#' @keywords FIMSFrame
methods::setGeneric(
  "model_age_comp",
  function(x, fleet_name) standardGeneric("model_age_comp")
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_age_comp", "FIMSFrame",
  function(x, fleet_name) {
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "age_comp",
      .data[["name"]] %in% fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_age_comp",
  "data.frame",
  function(x, fleet_name) model_age_comp(FIMSFrame(x), fleet_name)
)

#' @export
#' @rdname model_
#' @keywords FIMSFrame
methods::setGeneric(
  "model_length_comp",
  function(x, fleet_name) standardGeneric("model_length_comp")
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_length_comp",
  "FIMSFrame",
  function(x, fleet_name) {
    conversion_data <- dplyr::filter(
      .data = x@data,
      .data[["type"]] == "age_to_length_conversion"
    )
    if (NROW(conversion_data) == 0) {
      cli::cli_abort(c(
        "There are no {.var age_to_length_conversion} data present, therefore
        you cannot fit to {.var length_comp} data."
      ))
    }
    dplyr::filter(
      .data = x@data,
      .data[["type"]] == "length_comp",
      .data[["name"]] %in% fleet_name
    ) |>
      dplyr::pull(.data[["value"]])
  }
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_length_comp",
  "data.frame",
  function(x, fleet_name) model_length_comp(FIMSFrame(x), fleet_name)
)

#' @export
#' @rdname model_
#' @keywords FIMSFrame
methods::setGeneric(
  "model_weight_at_age",
  function(x) standardGeneric("model_weight_at_age")
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_weight_at_age",
  "FIMSFrame",
  function(x) {
    model_data <- dplyr::filter(
      .data = as.data.frame(x@data),
      .data[["type"]] == "weight_at_age"
    )
    if (NROW(model_data) == 0) {
      cli::cli_abort(
        message = "No weight_at_age data found in FIMSFrame object."
      )
    }
    fleet_names <- unique(model_data[["name"]])
    if (length(fleet_names) > 1) {
      cli::cli_warn(c(
        "x" = "Multiple fleets found in weight_at_age data.",
        "i" = "{.fn model_weight_at_age} will average values across fleets."
      ))
      model_data <- dplyr::group_by(
        .data = model_data,
        .data[["timing"]],
        .data[["age"]]
      ) |>
        dplyr::mutate(
          value = ifelse(.data[["value"]] == -999, NA, .data[["value"]])
        ) |>
        dplyr::summarize(
          value = mean(.data[["value"]], na.rm = TRUE)
        ) |>
        dplyr::mutate(
          value = ifelse(is.nan(.data[["value"]]), -999, .data[["value"]])
        )
    }
    # Create time-series vector if only available by age
    n_rows <- NROW(dplyr::filter(model_data, value != -999))
    n_rows_needed <- get_n_ages(x) * (get_n_years(x) + 1)
    if (n_rows < n_rows_needed) {
      if (n_rows == get_n_ages(x)) {
        model_data <- dplyr::bind_rows(
          replicate(
            # Adds a year for terminal year + 1 because to calculate
            # spawning biomass after fishing in terminal year
            get_n_years(x) + 1,
            dplyr::filter(model_data, value != -999),
            simplify = FALSE
          )
        )
      } else {
        cli::cli_abort(
          "Too few rows of weight_at_age data found, you need at least
          {n_rows_needed}, one for every year and age combination."
        )
      }
    }
    model_data |>
      dplyr::pull(.data[["value"]])
  }
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_weight_at_age",
  "data.frame",
  function(x) {
    model_weight_at_age(FIMSFrame(x))
  }
)

#' @export
#' @rdname model_
#' @keywords FIMSFrame
methods::setGeneric(
  "model_age_to_length_conversion",
  function(x, fleet_name) standardGeneric("model_age_to_length_conversion")
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_age_to_length_conversion",
  "FIMSFrame",
  function(x, fleet_name) {
    if ("length" %in% colnames(x@data)) {
      if (!"age" %in% colnames(x@data)) {
        cli::cli_abort("The age column is not present in your data.")
      }
      dplyr::filter(
        .data = as.data.frame(x@data),
        .data[["type"]] == "age_to_length_conversion",
        .data[["name"]] %in% fleet_name
      ) |>
        dplyr::group_by(.data[["age"]], .data[["length"]]) |>
        dplyr::summarize(
          mean_value = mean(as.numeric(.data[["value"]]), na.rm = TRUE)
        ) |>
        dplyr::pull(as.numeric(.data[["mean_value"]]))
    } else {
      cli::cli_abort(
        "The length column is not present in your data."
      )
    }
  }
)
#' @rdname model_
#' @keywords FIMSFrame
methods::setMethod(
  "model_age_to_length_conversion",
  "data.frame",
  function(x, fleet_name) {
    model_age_to_length_conversion(
      FIMSFrame(x),
      fleet_name
    )
  }
)

# methods::setMethod: initialize ----

# Not currently using methods::setMethod(f = "initialize")
# because @kellijohnson-NOAA did not quite understand how they actually work.

# methods::setMethod: plot ----
#' Plot a `FIMSFrame` object
#'
#' Use `ggplot2::geom_point()` to plot summary information stored in the data slot
#' of the `FIMSFrame` class.
#'
#' @param x A `FIMSFrame` object.
#' @param y Unused (inherited from R base).
#' @param ... Unused (inherited from R base).
#'
#' @return
#' A \pkg{ggplot2} object is returned that uses [stockplotr::theme_noaa()].
#' There will be one panel per input type with fleet-specific information
#' denoted using colors.
#' @examples
#' \dontrun{
#' data("data_big", package = "FIMS")
#' data_4_model <- FIMSFrame(data_big)
#' plot(data_4_model)
#' }
#'
#' @export
#' @method plot FIMSFrame
#' @rdname plot
#' @aliases plot,FIMSFrame,missing-method
#' @exportMethod plot
setGeneric("plot", function(x, y, ...) {
  standardGeneric("plot")
})
methods::setMethod(
  f = "plot",
  signature = c(x = "FIMSFrame", y = "missing"),
  definition = function(x, y, ...) {
    data_for_plot <- get_data(x) |>
      dplyr::mutate(
        type = gsub("_", " ", type)
      ) |>
      dplyr::group_by(name, timing, type) |>
      dplyr::filter(value != -999) |>
      dplyr::summarize(
        no = dplyr::n()
      ) |>
      dplyr::filter(no > 0)
    ggplot2::ggplot(
      data = data_for_plot,
      mapping = ggplot2::aes(
        x = timing,
        y = name,
        col = name
      )
    ) +
      # Using Set3 b/c it is the palette with the largest number of colors
      # and not {nmfspalette} b/c didn't want to depend on GitHub package
      ggplot2::scale_color_brewer(palette = "Set3") +
      ggplot2::facet_wrap(
        "type",
        scales = "free_y",
        labeller = ggplot2::label_wrap_gen(width = 10),
        ncol=1
      ) +
      ggplot2::geom_point(alpha = 0.8) +
      ggplot2::xlab("Timing") +
      ggplot2::ylab("") +
      ggplot2::theme(
        axis.text.x = ggplot2::element_text(angle = 15),
        axis.text.y = ggplot2::element_blank(),
        axis.ticks.y = ggplot2::element_blank()
      ) +
      ggplot2::labs(col = "Fleet") +
      stockplotr::theme_noaa()
  }
)

# methods::setMethod: show ----

methods::setMethod(
  f = "show",
  signature = "FIMSFrame",
  definition = function(object) {
    message("tbl_df of class '", class(object), "'")
    dat_types <- unique(object@data[[which(colnames(object@data) == "type")]])
    message("with the following 'types': ", paste0(dat_types, collapse = ", "))
    snames <- slotNames(object)
    ordinnames <- !snames %in% c(
      "data",
      ".S3Class",
      "row.names",
      "names"
    )
    print(utils::head(object@data))
    cat("additional slots include the following:")
    for (nm in snames[ordinnames]) {
      cat(nm, ":\n", sep = "")
      print(slot(object, nm))
    }
  }
)

is.FIMSFrame <- function(x) {
  inherits(x, "FIMSFrame")
}

# methods::setValidity ----

methods::setValidity(
  Class = "FIMSFrame",
  method = function(object) {
    errors <- character()

    if (NROW(object@data) == 0) {
      errors <- c(errors, "data must have at least one row")
    }

    # FIMS models currently cannot run without weight_at_age data
    weight_at_age_data <- dplyr::filter(object@data, type == "weight_at_age")
    if (NROW(weight_at_age_data) == 0) {
      errors <- c(errors, "data must contain data of the type weight_at_age")
    }

    errors <- c(errors, validate_data_colnames(object@data))

    # Check the format for acceptable variants of the ideal numeric
    if (!all(is.numeric(object@data[["timing"]]))) {
      errors <- c(errors, "timing must be in numeric format")
    }
    if (!all(as.integer(object@data[["timing"]]) -
      object@data[["timing"]] == 0)) {
      errors <- c(errors, "timing can only handle years right now")
    }

    # TODO: Add checks for other slots
    # Add validity check for types
    present_types <- unique(object@data[["type"]])

    # Issues warning if there are any unrecognized types
    unknown_types <- sort(setdiff(present_types, fims_input_types))
    if (length(unknown_types) > 0) {
      cli::cli_warn(c(
        "!" = "Data contains unexpected type(s): {unknown_types}",
        "i" = "Allowed types are: {fims_input_types}",
        "i" = "Model will run but check that data types are correct."
      ))
    }

    # Ensure composition data sum to 1.0 per group if units are proportions
    for (present_type in grep("_comp", present_types, value = TRUE)) {
      test <- object@data |>
        dplyr::filter(type == present_type, value != -999) |>
        dplyr::group_by(name, timing, .drop = FALSE) |>
        dplyr::group_map(.keep = TRUE, \(.x, .y) {
          validate_composition_data(.x)
        })
      if (sum(unlist(test)) > 0) {
        cli::cli_abort(
          "The above errors were found in your {present_type}."
        )
      }
    }
  }
)

validate_data_colnames <- function(data) {
  the_column_names <- colnames(data)
  errors <- character()
  if (!"type" %in% the_column_names) {
    errors <- c(errors, "data must contain 'type'")
  }
  if (!"name" %in% the_column_names) {
    errors <- c(errors, "data must contain 'name'")
  }
  if (!"timing" %in% the_column_names) {
    errors <- c(errors, "data must contain 'timing'")
  }
  if (!"value" %in% the_column_names) {
    errors <- c(errors, "data must contain 'value'")
  }
  if (!"unit" %in% the_column_names) {
    errors <- c(errors, "data must contain 'unit'")
  }
  if (!any(c("age", "length") %in% the_column_names)) {
    errors <- c(errors, "data must contain 'ages' and/or 'lengths'")
  }
  return(errors)
}

validate_composition_data <- function(data) {
  composition_type <- pretty_type(unique(data[["type"]]))
  if (all(data[["value"]] == -999)) {
    return(0)
  }
  groupings <- names(data)[
    sapply(
      data,
      function(x) dplyr::n_distinct(x, na.rm = FALSE) == 1 & !all(is.na(x))
    )
  ]
  grouping_message <- glue::glue("{groupings} = {data[1, groupings]}")
  names(grouping_message) <- rep("*", length(grouping_message))
  units <- unique(data[["unit"]])
  errors <- vector()
  if (length(units) != 1) {
    errors <- c(
      errors,
      "x" = "There should only be one unit per grouping, units are {units}."
    )
  }
  sum_of_value <- sum(data[["value"]])
  if (all(units == "proportion") && abs(sum_of_value - 1.0) > 1e-8) {
    errors <- c(
      errors,
      "x" = "The sum is equal to {sum_of_value}, not 1.0."
    )
  }
  if (length(errors) > 0) {
    cli::cli_bullets(c(
      " " = "Group-level information for {composition_type} errors",
      grouping_message,
      "!" = "Errors are as follows:",
      errors
    ))
    return(1)
  } else {
    return(0)
  }
}

# Constructors ----

# All constructors in this file are documented in 1 roxygen file via @rdname.

#' Class constructors for `FIMSFrame` and associated child classes
#'
#' All constructor functions take a single input and build an object specific
#' to the needs of each model type within \pkg{FIMS}. `FIMSFrame` is the parent
#' class. Future, associated child classes will have the additional slots
#' needed for different types of models.
#'
#' @details
#' ## data
#' The input data are both sorted and expanded before returning them in the
#' data slot.
#' ### Sorting
#' It is important that the order of the rows in the data are correct but it is
#' not expected that the user will do this. Instead, the returned data are
#' sorted using [dplyr::arrange()] before placing them in the data slot. Data
#' are first sorted by data type, placing all `weight_at_age` data next to
#' other `weight_at_age` data and all landings data next to landings data.
#' Thus, `age_comp` data will come first because their type is "age" and "a" is
#' first in the alphabet. All other types will follow according to their order
#' in the alphabet.
#' Next, within each type, data are organized by fleet. So, `age_comp`
#' information for fleet1 will come before survey1. Next, all data within type
#' and fleet are arranged by timing, e.g., by year. That is the end of the
#' sorting for time series data like landings and indices. The biological data
#' are further sorted by bin. Thus, `age_comp` information will be arranged as
#' follows:
#'
#' | type     | name     | timing  | age  | value  |
#' |:-------- |:--------:|:-------:|:----:|-------:|
#' | age_comp | fleet1   | 2022    | 1    | 0.3    |
#' | age_comp | fleet1   | 2022    | 2    | 0.7    |
#' | age_comp | fleet1   | 2023    | 1    | 0.5    |
#'
#' `length_comp` data are sorted the same way but by length bin instead of
#' by age bin. It becomes more complicated for the `age_to_length_conversion`
#' data, which are sorted by type, name, timing, age, and then length. So, a
#' full set of length, e.g., length 10, length 20, length 30, etc., is placed
#' together for a given age. After that age, another entire set of length
#' information will be provided for that next age. Once the year is complete
#' for a given fleet then the next year will begin.
#'
#' @rdname FIMSFrame
#'
#' @param data A `data.frame` that contains the necessary columns to construct
#'   a `FIMSFrame-class` object. Currently, those columns are
#'   `r glue::glue_collapse(colnames(data_big), sep = ", ", last = ", and ")`.
#'   See the `data_big` object in FIMS, e.g., `data(data_big, package = "FIMS")`.
#'
#' @return
#' An object of the S4 class `FIMSFrame` class, or one of its child classes, is
#' validated and then returned. All objects will at a minimum have a slot
#' called `data` to store the input data frame. Additional slots are dependent
#' on the child class. Use [methods::showClass()] to see all available slots.
#' @export
#' @keywords FIMSFrame
FIMSFrame <- function(data) {
  errors <- validate_data_colnames(data)
  if (length(errors) > 0) {
    stop(
      "Check the columns of your data, the following are missing:\n",
      paste(errors, sep = "\n", collapse = "\n")
    )
  }
  if (NROW(data) == 0) {
    cli::cli_abort(
      "{.var data} has 0 rows of data and cannot be used to make a FIMSFrame."
    )
  }

  # Get the earliest and latest year formatted as integers
  data_to_use_4_timing <- dplyr::filter(
    data,
    type != "weight_at_age"
  ) |>
    dplyr::pull(timing)
  start_year <- as.integer(floor(min(data_to_use_4_timing, na.rm = TRUE)))
  end_year <- as.integer(floor(max(data_to_use_4_timing, na.rm = TRUE)))
  n_years <- as.integer(end_year - start_year + 1)
  years <- start_year:end_year

  # Get the fleets represented in the data
  fleets <- unique(data[["name"]])
  n_fleets <- length(fleets)

  if ("age" %in% colnames(data)) {
    if (all(is.na(data[["age"]]))) {
      cli::cli_abort(
        message = "The `age` column exists in `data` but they are all `NA`."
      )
    } else {
      # Forced to use annual age bins because the model has an annual time step
      # FUTURE: allow for different age bins rather than 1 year increment
      ages <- min(data[["age"]], na.rm = TRUE):max(data[["age"]], na.rm = TRUE)
    }
  } else {
    ages <- numeric()
  }
  n_ages <- length(ages)

  if ("length" %in% colnames(data)) {
    if (all(is.na(data[["length"]]))) {
      lengths <- numeric()
    } else {
      lengths <- sort(unique(data[["length"]]))
      lengths <- lengths[!is.na(lengths)]
    }
  } else {
    lengths <- numeric()
  }
  n_lengths <- length(lengths)

  # Work on filling in missing data with -999 and arrange in the correct
  # order so that getting information out with model_*() are correct.
  formatted_data <- tibble::as_tibble(data)
  missing_time_series <- create_missing_data(
    data = formatted_data,
    timings = years
  )
  if ("age" %in% colnames(formatted_data)) {
    missing_ages <- create_missing_data(
      data = formatted_data,
      bins = ages,
      timings = years,
      column = age,
      types = c("weight_at_age", "age_comp")
    )
  } else {
    missing_ages <- missing_time_series[0, ]
  }
  if ("length" %in% colnames(formatted_data)) {
    missing_lengths <- create_missing_data(
      data = formatted_data,
      bins = lengths,
      timings = years,
      column = length,
      types = "length_comp"
    )
  } else {
    missing_lengths <- missing_time_series[0, ]
  }
  if ("age_to_length_conversion" %in% formatted_data[["type"]]) {
    if (!"age" %in% colnames(data)) {
      cli::cli_abort(
        "age is a required column if you have age_to_length_conversion data."
      )
    }
    if (!"length" %in% colnames(data)) {
      cli::cli_abort(
        "length is a required column if you have age_to_length_conversion data."
      )
    }
    # Must do this by hand because it is across two dimensions
    temp_age_to_length_data <- formatted_data |>
      dplyr::group_by(type, name)
    missing_age_to_length <- temp_age_to_length_data |>
      dplyr::group_by(type, name) |>
      dplyr::filter(type %in% "age_to_length_conversion") |>
      tidyr::expand(unit, timing = years, age = ages, length = lengths) |>
      dplyr::anti_join(
        y = dplyr::select(
          temp_age_to_length_data,
          type, name, unit, timing, age, length
        ),
        by = dplyr::join_by(type, name, unit, timing, age, length)
      ) |>
      dplyr::mutate(
        value = 0
      ) |>
      dplyr::ungroup()
  } else {
    missing_age_to_length <- missing_time_series[0, ]
  }
  missing_data <- dplyr::bind_rows(
    missing_time_series,
    missing_ages,
    missing_lengths,
    missing_age_to_length
  )
  sort_order <- intersect(
    c("name", "type", "timing", "age", "length"),
    colnames(formatted_data)
  )
  complete_data <- dplyr::full_join(
    formatted_data,
    missing_data,
    by = colnames(missing_data)
  ) |>
    dplyr::arrange(!!!rlang::parse_exprs(sort_order))

  # Fill the empty data frames with data extracted from the data file
  out <- methods::new("FIMSFrame",
    data = complete_data,
    fleets = fleets,
    n_years = n_years,
    start_year = start_year,
    end_year = end_year,
    ages = ages,
    n_ages = n_ages,
    lengths = lengths,
    n_lengths = n_lengths
  )
  return(out)
}

# Unexported functions ----
create_missing_data <- function(
  data,
  bins,
  timings,
  column,
  types = c("landings", "index")
) {
  use_this_data <- data |>
    dplyr::group_by(type, name)
  out_data <- if (missing(bins)) {
    # This only pertains to annual data without bins
    use_this_data |>
      dplyr::filter(type %in% types) |>
      tidyr::expand(unit, timing = timings) |>
      dplyr::anti_join(
        y = dplyr::select(use_this_data, type, name, unit, timing),
        by = dplyr::join_by(type, name, unit, timing)
      )
  } else {
    use_this_data |>
      dplyr::group_by(type, name) |>
      dplyr::filter(type %in% types) |>
      tidyr::expand(unit, timing = timings, {{ column }} := bins) |>
      dplyr::anti_join(
        y = dplyr::select(use_this_data, type, name, unit, timing, {{ column }}),
        by = dplyr::join_by(type, name, unit, timing, {{ column }})
      )
  }
  out_data |>
    dplyr::mutate(
      value = -999
    ) |>
    dplyr::ungroup()
}

pretty_type <- function(x) {
  gsub("comp", "composition", x) |>
    gsub(pattern = "_", replacement = " ")
}
