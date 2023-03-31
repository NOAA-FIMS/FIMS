# tests for input objects
data(package = "FIMS")
age_frame <- FIMS::FIMSFrameAge(data_mile1)
fims_frame <- FIMSFrame(data_mile1)

nyears <- age_frame@nyrs
nages <- max(age_frame@ages)

fleet_names_agecomp <- dplyr::filter(
  .data = as.data.frame(age_frame@data),
  type == "age"
) %>%
  dplyr::distinct(name) %>%
  dplyr::pull(name)
nagecomp <- length(fleet_names_agecomp)

fleet_names_index <- dplyr::filter(
  .data = as.data.frame(age_frame@data),
  type == "index"
) %>%
  dplyr::distinct(name) %>%
  dplyr::pull(name)
nindex <- length(fleet_names_index)

test_that("Can add index data to model", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  indexdat <- vector(mode = "list", length = nindex)
  names(indexdat) <- fleet_names_index

  for (index_i in 1:nindex) {
    index <- fims$Index
    indexdat[[fleet_names_index[index_i]]] <- new(index, nyears)
    expect_silent(indexdat[[fleet_names_index[index_i]]] <-
      m_index(age_frame, fleet_names_index[index_i]))
  }

  fims$clear()
})

test_that("Can add agecomp data to model", {
  fims <- Rcpp::Module("fims", PACKAGE = "FIMS")

  agecompdat <- vector(mode = "list", length = nagecomp)
  names(agecompdat) <- fleet_names_agecomp

  for (fleet_f in 1:nagecomp) {
    agecompdat[[fleet_names_agecomp[fleet_f]]] <- new(fims$AgeComp, nyears, nages)
    expect_silent(agecompdat[[fleet_names_agecomp[fleet_f]]]$age_comp_data <-
      m_agecomp(age_frame, fleet_names_agecomp[fleet_f]))
  }

  fims$clear()
})
