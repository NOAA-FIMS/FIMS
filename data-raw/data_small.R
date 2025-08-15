#' Create a small simulated data set using arbitrary inputs
#'
#' This data set isn't a realistic example like data1 but is much smaller
#' which may be useful for debugging.
#'
#' @details
#'   * Currently has 3 years of data, 3 ages bins, 3 length bins, and 1 fleet.
#'   * Contains landings, indices, age composition, length composition,
#'     weight-at-age, and age-to-length conversion data.
#'   * See also data-raw/data1.R
#'
#' @author Ian G. Taylor

# set dimensions
years <- 1:3
lengths <- c(10, 20, 30)
ages <- 1:3
fleets <- 1

# create landings data (slowly increasing over 3 years)
landings <- tibble::tibble(
    type = "landings",
    name = paste0("fleet", fleets),
    age = NA,
    length = NA,
    datestart = paste0(years, "-01-01"),
    dateend = paste0(years, "-12-31"),
    value = 1000 + 100 * years,
    unit = "mt",
    uncertainty = 0.01
)

# create indices data (slowly decreasing over 3 years)
indices <- tibble::tibble(
    type = "index",
    name = paste0("fleet", fleets),
    age = NA,
    length = NA,
    datestart = paste0(years, "-01-01"),
    dateend = paste0(years, "-12-31"),
    value = 4000 - 100 * years,
    unit = "mt",
    uncertainty = 0.25
)

# age composition data (equal proportions across ages)
agecomps <- tidyr::expand_grid(
    year = years,
    age = ages
) |>
    dplyr::mutate(
        type = "age",
        name = paste0("fleet", fleets),
        age = age,
        length = NA,
        datestart = paste0(year, "-01-01"),
        dateend = paste0(year, "-12-31"),
        value = 1 / length(ages),
        unit = "proportion",
        uncertainty = 60
    ) |>
    dplyr::select(-year)

# length composition data (equal proportions across lengths)
lencomps <- tidyr::expand_grid(
    year = years,
    length = lengths
) |>
    dplyr::mutate(
        type = "length",
        name = paste0("fleet", fleets),
        age = NA,
        length = length,
        datestart = paste0(year, "-01-01"),
        dateend = paste0(year, "-12-31"),
        value = 1 / length(lengths),
        unit = "proportion",
        uncertainty = 120
    ) |>
    dplyr::select(-year)

# weight-at-age data (weight in kg equal to age, converted to mt)
weight_at_age <- tidyr::expand_grid(
    year = years,
    age = ages
) |>
    dplyr::mutate(
        type = "weight-at-age",
        name = paste0("fleet", fleets),
        age = age,
        length = NA,
        datestart = paste0(year, "-01-01"),
        dateend = paste0(year, "-12-31"),
        value = age / 1000,
        unit = "mt",
        uncertainty = NA
    ) |>
    dplyr::select(-year)

# make up an age-to-length conversion matrix if 3 ages and 3 lengths
len_at_age_matrix <- matrix(
    c(1.0, 0.0, 0.0, 0.1, 0.8, 0.1, 0.0, 0.3, 0.7),
    nrow = length(ages),
    byrow = TRUE
)
# if dimensions don't match (if not 3 x 3 as above),
# create simple diagonal matrix instead
if (
    nrow(len_at_age_matrix) != length(ages) ||
        ncol(len_at_age_matrix) != length(lengths)
) {
    len_at_age_matrix <- matrix(
        0,
        nrow = length(ages),
        ncol = length(lengths)
    )
    diag(len_at_age_matrix) <- 1
    # any ages beyond the number of lengths map to the final length bin
    if (length(ages) > length(lengths)) {
        len_at_age_matrix[
            (length(lengths) + 1):length(ages),
            length(lengths)
        ] <- 1
    }
}

# simple function to get values from the age-to-length conversion matrix
# given age and length to use in the mutate() command with expand_grid() below
get_len_at_age_value <- function(age, length) {
    age_idx <- match(age, ages)
    length_idx <- match(length, lengths)
    mapply(function(a, l) len_at_age_matrix[a, l], age_idx, length_idx)
}

# create age-to-length conversion data
age_to_length_conversion <- tidyr::expand_grid(
    year = years,
    age = ages,
    length = lengths
) |>
    dplyr::mutate(
        type = "age-to-length-conversion",
        name = paste0("fleet", fleets),
        age = age,
        length = length,
        datestart = paste0(year, "-01-01"),
        dateend = paste0(year, "-12-31"),
        value = get_len_at_age_value(age, length),
        unit = "mt",
        uncertainty = NA
    ) |>
    # reorder columns to match FIMSFrame format used in other tibbles above
    dplyr::select(
        type,
        name,
        age,
        length,
        datestart,
        dateend,
        value,
        unit,
        uncertainty
    )

# combine all data sources and convert to FIMSFrame
data_small <- rbind(
    landings,
    indices,
    agecomps,
    lencomps,
    weight_at_age,
    age_to_length_conversion
)

# save as .rda file for use in the package
usethis::use_data(data_small, overwrite = TRUE)
rm(list = ls())
