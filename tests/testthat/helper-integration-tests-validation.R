#' Validate FIMS Model Output
#'
#' This function validates the output from the FIMS
#' against the known OM values.
#' It performs various checks to ensure that the estimates provided by the FIMS
#' are within acceptable tolerance compared to the operating model values.
#'
#' @param report A list containing the results of the TMB model report. This
#' includes the estimated recruitment numbers and other relevant metrics.
#' @param sdr A list containing the standard deviation report from the TMB model.
#' @param sdr_report A matrix containing the summary of the standard deviation report.
#' @param om_input A list containing the operating model inputs, such as years,
#' ages, and other parameters.
#' @param om_output A list containing the operating model outputs, including metrics
#' such as numbers at age, biomass, spawning biomass, fishing mortality, and survey indices.
#' @param em_input A list containing the estimation model inputs, including observed
#' catches, survey indices, and other relevant data.
#' @param use_fimsfit Logical; if `TRUE`, validates using `fit_fims()` results.
#'
#' @return None. The function uses `testthat` functions to perform validations.
#' It ensures that the output is within the expected range of error based on
#' standard deviations provided.
#'
#' @examples
#' # Assume `result` is a list of outputs obtained from running `setup_and_run_FIMS()`.
#' # The `result` list contains components such as `report`, `sdr_report`, and `obj`.
#'
#' validate_fims(
#'   report = result$report,
#'   sdr = TMB::sdreport(result$obj),
#'   sdr_report = result$sdr_report,
#'   om_input = om_input_list[[1]],
#'   om_output = om_output_list[[1]],
#'   em_input = em_input_list[[1]],
#'   use_fimsfit = FALSE
#' )
validate_fims <- function(
    report,
    sdr,
    sdr_report,
    om_input,
    om_output,
    em_input,
    use_fimsfit = FALSE) {
  if (use_fimsfit == TRUE) {
    # Numbers at age
    # Estimates and SE for NAA
    sdr_naa <- sdr_report[which(sdr_report[["name"]] == "NAA"), c("value", "se")]
    naa_are <- rep(0, length(c(t(om_output[["N.age"]]))))
    for (i in 1:length(c(t(om_output[["N.age"]])))) {
      naa_are[i] <- abs(sdr_naa[i, "value"] - c(t(om_output[["N.age"]]))[i])
    }
    # Expect 95% of absolute error to be within 2*SE of NAA
    expect_lte(
      sum(unlist(naa_are) > qnorm(.975) * sdr_naa[1:length(c(t(om_output[["N.age"]]))), "se"]),
      0.05 * length(c(t(om_output[["N.age"]])))
    )

    # Biomass
    sdr_biomass <- sdr_report[which(sdr_report[["name"]] == "Biomass"), c("value", "se")]
    biomass_are <- rep(0, length(om_output[["biomass.mt"]]))
    for (i in 1:length(om_output[["biomass.mt"]])) {
      biomass_are[i] <- abs(sdr_biomass[i, "value"] - om_output[["biomass.mt"]][i]) # / om_output[["biomass.mt"]][i]
      # expect_lte(biomass_are[i], 0.15)
    }
    expect_lte(
      sum(unlist(biomass_are) > qnorm(.975) * sdr_biomass[1:length(om_output[["biomass.mt"]]), "se"]),
      0.05 * length(om_output[["biomass.mt"]])
    )

    # Spawning biomass
    sdr_sb <- sdr_report[which(sdr_report[["name"]] == "SSB"), c("value", "se")]
    sb_are <- rep(0, length(om_output[["SSB"]]))
    for (i in 1:length(om_output[["SSB"]])) {
      sb_are[i] <- abs(sdr_sb[i, "value"] - om_output[["SSB"]][i]) # / om_output[["SSB"]][i]
      # expect_lte(sb_are[i], 0.15)
    }
    expect_lte(
      sum(unlist(sb_are) > qnorm(.975) * sdr_sb[1:length(om_output[["SSB"]]), "se"]),
      0.05 * length(om_output[["SSB"]])
    )

    # Recruitment
    fims_naa <- matrix(report[["naa"]][[1]][1:(om_input[["nyr"]] * om_input[["nages"]])],
                       nrow = om_input[["nyr"]], byrow = TRUE
    )
    sdr_naa1_vec <- sdr_report[which(sdr_report[["name"]] == "NAA"), "se"]
    sdr_naa1 <- sdr_naa1_vec[seq(1, om_input[["nyr"]] * om_input[["nages"]], by = om_input[["nages"]]), "se"]
    fims_naa1_are <- rep(0, om_input[["nyr"]])
    for (i in 1:om_input[["nyr"]]) {
      fims_naa1_are[i] <- abs(fims_naa[i, 1] - om_output[["N.age"]][i, 1]) # /
      # om_output[["N.age"]][i, 1]
      # expect_lte(fims_naa1_are[i], 0.25)
    }
    expect_lte(
      sum(fims_naa1_are > qnorm(.975) * sdr_naa1[1:length(om_output[["SSB"]]), "se"]),
      0.05 * length(om_output[["SSB"]])
    )

    expect_equal(
      fims_naa[, 1],
      report[["recruitment"]][[1]][1:om_input[["nyr"]]]
    )

    # recruitment log deviations
    # the initial value of om_input[["logR.resid"]] is dropped from the model
    sdr_rdev <- sdr_report[which(sdr_report[["name"]] == "LogRecDev"), c("value", "se")]
    rdev_are <- rep(0, length(om_input[["logR.resid"]]) - 1)

    for (i in 1:(length(report[["log_recruit_dev"]][[1]]) - 1)) {
      rdev_are[i] <- abs(report[["log_recruit_dev"]][[1]][i] - om_input[["logR.resid"]][i + 1]) # /
      #   exp(om_input[["logR.resid"]][i])
      # expect_lte(rdev_are[i], 1) # 1
    }
    expect_lte(
      sum(rdev_are > qnorm(.975) * sdr_rdev[1:length(om_input[["logR.resid"]]) - 1, 2]),
      0.05 * length(om_input[["logR.resid"]])
    )

    # F (needs to be updated when std.error is available)
    sdr_F <- sdr_report[which(sdr_report[["name"]] == "FMort"), c("value", "se")]
    f_are <- rep(0, length(om_output[["f"]]))
    for (i in 1:length(om_output[["f"]])) {
      f_are[i] <- abs(sdr_F[i, 1] - om_output[["f"]][i])
    }
    # Expect 95% of absolute error to be within 2*SE of Fmort
    expect_lte(
      sum(unlist(f_are) > qnorm(.975) * sdr_F[1:length(om_output[["f"]]), 2]),
      0.05 * length(om_output[["f"]])
    )

    # Expected fishery catch and survey index
    fims_index <- sdr_report[which(sdr_report[["name"]] == "ExpectedIndex"), c("value", "se")]
    fims_catch <- fims_index[1:om_input[["nyr"]], ]
    fims_survey <- fims_index[(om_input[["nyr"]] + 1):(om_input[["nyr"]] * 2), ]

    # Expected fishery catch - om_output
    catch_are <- rep(0, length(om_output[["L.mt"]][["fleet1"]]))
    for (i in 1:length(om_output[["L.mt"]][["fleet1"]])) {
      catch_are[i] <- abs(fims_catch[i, 1] - om_output[["L.mt"]][["fleet1"]][i])
    }
    # Expect 95% of absolute error to be within 2*SE of fishery catch
    expect_lte(
      sum(unlist(catch_are) > qnorm(.975) * fims_catch[, 2]),
      0.05 * length(om_output[["L.mt"]][["fleet1"]])
    )

    # Expected fishery catch - em_input
    catch_are <- rep(0, length(em_input[["L.obs"]][["fleet1"]]))
    for (i in 1:length(em_input[["L.obs"]][["fleet1"]])) {
      catch_are[i] <- abs(fims_catch[i, 1] - em_input[["L.obs"]][["fleet1"]][i])
    }
    # Expect 95% of absolute error to be within 2*SE of fishery catch
    expect_lte(
      sum(unlist(catch_are) > qnorm(.975) * fims_catch[, 2]),
      0.05 * length(em_input[["L.obs"]][["fleet1"]])
    )


    # Expected fishery catch number at age
    sdr_cnaa <- sdr_report[which(sdr_report[["name"]] == "CNAA"), c("value", "se")]
    cnaa_are <- rep(0, length(c(t(om_output[["L.age"]][["fleet1"]]))))
    for (i in 1:length(c(t(om_output[["L.age"]][["fleet1"]])))) {
      cnaa_are[i] <- abs(sdr_cnaa[i, 1] - c(t(om_output[["L.age"]][["fleet1"]]))[i])
    }
    # Expect 95% of absolute error to be within 2*SE of CNAA
    expect_lte(
      sum(unlist(cnaa_are) > qnorm(.975) * sdr_cnaa[, 2]),
      0.05 * length(c(t(om_output[["L.age"]][["fleet1"]])))
    )

    # Expected survey index - om_output
    index_are <- rep(0, length(om_output[["survey_index_biomass"]][["survey1"]]))
    for (i in 1:length(om_output[["survey_index_biomass"]][["survey1"]])) {
      index_are[i] <- abs(fims_survey[i, 1] - om_output[["survey_index_biomass"]][["survey1"]][i])
    }
    # Expect 95% of absolute error to be within 2*SE of survey index
    expect_lte(
      sum(unlist(index_are) > qnorm(.975) * fims_survey[, 2]),
      0.05 * length(om_output[["survey_index_biomass"]][["survey1"]])
    )

    # Expected survey index - em_input
    index_are <- rep(0, length(em_input[["surveyB.obs"]][["survey1"]]))
    for (i in 1:length(em_input[["surveyB.obs"]][["survey1"]])) {
      index_are[i] <- abs(fims_survey[i, 1] - em_input[["surveyB.obs"]][["survey1"]][i])
    }
    # # Expect 95% of absolute error to be within 2*SE of survey index
    # expect_lte(
    #   sum(index_are > qnorm(.975) * fims_survey[, 2]),
    #   0.05 * length(em_input[["surveyB.obs"]][["survey1"]])
    # )

    for (i in 1:length(em_input[["surveyB.obs"]][["survey1"]])) {
      expect_lte(abs(fims_survey[i, 1] - em_input[["surveyB.obs"]][["survey1"]][i]) /
                   em_input[["surveyB.obs"]][["survey1"]][i], 0.25)
    }

    # Expected survey number at age
    # for (i in 1:length(c(t(om_output[["survey_age_comp"]][["survey1"]])))){
    #   expect_lte(abs(report[["cnaa"]][i,2] - c(t(om_output[["survey_age_comp"]][["survey1"]]))[i])/
    #                c(t(om_output[["survey_age_comp"]][["survey1"]]))[i], 0.001)
    # }

    # Expected catch number at age in proportion
    # fims_cnaa <- matrix(report[["cnaa"]][1:(om_input[["nyr"]]*om_input[["nages"]]), 2],
    #                     nrow = om_input[["nyr"]], byrow = TRUE)
    # fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)
    #
    # for (i in 1:length(c(t(em_input[["survey.age.obs"]])))){
    #   expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input[["L.age.obs"]][["fleet1"]]))[i])/
    #                c(t(em_input[["L.age.obs"]][["fleet1"]]))[i], 0.15)
    # }
  } else {
    # Numbers at age
    # Estimates and SE for NAA
    sdr_naa <- sdr_report[which(rownames(sdr_report) == "NAA"), ]
    naa_are <- rep(0, length(c(t(om_output[["N.age"]]))))
    for (i in 1:length(c(t(om_output[["N.age"]])))) {
      naa_are[i] <- abs(sdr_naa[i, 1] - c(t(om_output[["N.age"]]))[i])
    }
    # Expect 95% of absolute error to be within 2*SE of NAA
    expect_lte(
      sum(naa_are > qnorm(.975) * sdr_naa[1:length(c(t(om_output[["N.age"]]))), 2]),
      0.05 * length(c(t(om_output[["N.age"]])))
    )

    # Biomass
    sdr_biomass <- sdr_report[which(rownames(sdr_report) == "Biomass"), ]
    biomass_are <- rep(0, length(om_output[["biomass.mt"]]))
    for (i in 1:length(om_output[["biomass.mt"]])) {
      biomass_are[i] <- abs(sdr_biomass[i, 1] - om_output[["biomass.mt"]][i]) # / om_output[["biomass.mt"]][i]
      # expect_lte(biomass_are[i], 0.15)
    }
    expect_lte(
      sum(biomass_are > qnorm(.975) * sdr_biomass[1:length(om_output[["biomass.mt"]]), 2]),
      0.05 * length(om_output[["biomass.mt"]])
    )

    # Spawning biomass
    sdr_sb <- sdr_report[which(rownames(sdr_report) == "SSB"), ]
    sb_are <- rep(0, length(om_output[["SSB"]]))
    for (i in 1:length(om_output[["SSB"]])) {
      sb_are[i] <- abs(sdr_sb[i, 1] - om_output[["SSB"]][i]) # / om_output[["SSB"]][i]
      # expect_lte(sb_are[i], 0.15)
    }
    expect_lte(
      sum(sb_are > qnorm(.975) * sdr_sb[1:length(om_output[["SSB"]]), 2]),
      0.05 * length(om_output[["SSB"]])
    )

    # Recruitment
    fims_naa <- matrix(report[["naa"]][[1]][1:(om_input[["nyr"]] * om_input[["nages"]])],
                       nrow = om_input[["nyr"]], byrow = TRUE
    )
    sdr_naa1_vec <- sdr_report[which(rownames(sdr_report) == "NAA"), 2]
    sdr_naa1 <- sdr_naa1_vec[seq(1, om_input[["nyr"]] * om_input[["nages"]], by = om_input[["nages"]])]
    fims_naa1_are <- rep(0, om_input[["nyr"]])
    for (i in 1:om_input[["nyr"]]) {
      fims_naa1_are[i] <- abs(fims_naa[i, 1] - om_output[["N.age"]][i, 1]) # /
      # om_output[["N.age"]][i, 1]
      # expect_lte(fims_naa1_are[i], 0.25)
    }
    expect_lte(
      sum(fims_naa1_are > qnorm(.975) * sdr_naa1[1:length(om_output[["SSB"]])]),
      0.05 * length(om_output[["SSB"]])
    )

    expect_equal(
      fims_naa[, 1],
      report[["recruitment"]][[1]][1:om_input[["nyr"]]]
    )

    # recruitment log deviations
    # the initial value of om_input[["logR.resid"]] is dropped from the model
    sdr_rdev <- sdr_report[which(rownames(sdr_report) == "LogRecDev"), ]
    rdev_are <- rep(0, length(om_input[["logR.resid"]]) - 1)

    for (i in 1:(length(report[["log_recruit_dev"]][[1]]) - 1)) {
      rdev_are[i] <- abs(report[["log_recruit_dev"]][[1]][i] - om_input[["logR.resid"]][i + 1]) # /
      #   exp(om_input[["logR.resid"]][i])
      # expect_lte(rdev_are[i], 1) # 1
    }
    expect_lte(
      sum(rdev_are > qnorm(.975) * sdr_rdev[1:length(om_input[["logR.resid"]]) - 1, 2]),
      0.05 * length(om_input[["logR.resid"]])
    )

    # F (needs to be updated when std.error is available)
    sdr_F <- sdr_report[which(rownames(sdr_report) == "FMort"), ]
    f_are <- rep(0, length(om_output[["f"]]))
    for (i in 1:length(om_output[["f"]])) {
      f_are[i] <- abs(sdr_F[i, 1] - om_output[["f"]][i])
    }
    # Expect 95% of absolute error to be within 2*SE of Fmort
    expect_lte(
      sum(f_are > qnorm(.975) * sdr_F[1:length(om_output[["f"]]), 2]),
      0.05 * length(om_output[["f"]])
    )

    # Expected fishery catch and survey index
    fims_index <- sdr_report[which(rownames(sdr_report) == "ExpectedIndex"), ]
    fims_catch <- fims_index[1:om_input[["nyr"]], ]
    fims_survey <- fims_index[(om_input[["nyr"]] + 1):(om_input[["nyr"]] * 2), ]

    # Expected fishery catch - om_output
    catch_are <- rep(0, length(om_output[["L.mt"]][["fleet1"]]))
    for (i in 1:length(om_output[["L.mt"]][["fleet1"]])) {
      catch_are[i] <- abs(fims_catch[i, 1] - om_output[["L.mt"]][["fleet1"]][i])
    }
    # Expect 95% of absolute error to be within 2*SE of fishery catch
    expect_lte(
      sum(catch_are > qnorm(.975) * fims_catch[, 2]),
      0.05 * length(om_output[["L.mt"]][["fleet1"]])
    )

    # Expected fishery catch - em_input
    catch_are <- rep(0, length(em_input[["L.obs"]][["fleet1"]]))
    for (i in 1:length(em_input[["L.obs"]][["fleet1"]])) {
      catch_are[i] <- abs(fims_catch[i, 1] - em_input[["L.obs"]][["fleet1"]][i])
    }
    # Expect 95% of absolute error to be within 2*SE of fishery catch
    expect_lte(
      sum(catch_are > qnorm(.975) * fims_catch[, 2]),
      0.05 * length(em_input[["L.obs"]][["fleet1"]])
    )


    # Expected fishery catch number at age
    sdr_cnaa <- sdr_report[which(rownames(sdr_report) == "CNAA"), ]
    cnaa_are <- rep(0, length(c(t(om_output[["L.age"]][["fleet1"]]))))
    for (i in 1:length(c(t(om_output[["L.age"]][["fleet1"]])))) {
      cnaa_are[i] <- abs(sdr_cnaa[i, 1] - c(t(om_output[["L.age"]][["fleet1"]]))[i])
    }
    # Expect 95% of absolute error to be within 2*SE of CNAA
    expect_lte(
      sum(cnaa_are > qnorm(.975) * sdr_cnaa[, 2]),
      0.05 * length(c(t(om_output[["L.age"]][["fleet1"]])))
    )

    # Expected survey index - om_output
    index_are <- rep(0, length(om_output[["survey_index_biomass"]][["survey1"]]))
    for (i in 1:length(om_output[["survey_index_biomass"]][["survey1"]])) {
      index_are[i] <- abs(fims_survey[i, 1] - om_output[["survey_index_biomass"]][["survey1"]][i])
    }
    # Expect 95% of absolute error to be within 2*SE of survey index
    expect_lte(
      sum(index_are > qnorm(.975) * fims_survey[, 2]),
      0.05 * length(om_output[["survey_index_biomass"]][["survey1"]])
    )

    # Expected survey index - em_input
    index_are <- rep(0, length(em_input[["surveyB.obs"]][["survey1"]]))
    for (i in 1:length(em_input[["surveyB.obs"]][["survey1"]])) {
      index_are[i] <- abs(fims_survey[i, 1] - em_input[["surveyB.obs"]][["survey1"]][i])
    }
    # # Expect 95% of absolute error to be within 2*SE of survey index
    # expect_lte(
    #   sum(index_are > qnorm(.975) * fims_survey[, 2]),
    #   0.05 * length(em_input[["surveyB.obs"]][["survey1"]])
    # )

    for (i in 1:length(em_input[["surveyB.obs"]][["survey1"]])) {
      expect_lte(abs(fims_survey[i, 1] - em_input[["surveyB.obs"]][["survey1"]][i]) /
                   em_input[["surveyB.obs"]][["survey1"]][i], 0.25)
    }

    # Expected survey number at age
    # for (i in 1:length(c(t(om_output[["survey_age_comp"]][["survey1"]])))){
    #   expect_lte(abs(report[["cnaa"]][i,2] - c(t(om_output[["survey_age_comp"]][["survey1"]]))[i])/
    #                c(t(om_output[["survey_age_comp"]][["survey1"]]))[i], 0.001)
    # }

    # Expected catch number at age in proportion
    # fims_cnaa <- matrix(report[["cnaa"]][1:(om_input[["nyr"]]*om_input[["nages"]]), 2],
    #                     nrow = om_input[["nyr"]], byrow = TRUE)
    # fims_cnaa_proportion <- fims_cnaa/rowSums(fims_cnaa)
    #
    # for (i in 1:length(c(t(em_input[["survey.age.obs"]])))){
    #   expect_lte(abs(c(t(fims_cnaa_proportion))[i] - c(t(em_input[["L.age.obs"]][["fleet1"]]))[i])/
    #                c(t(em_input[["L.age.obs"]][["fleet1"]]))[i], 0.15)
    # }
  }
}
