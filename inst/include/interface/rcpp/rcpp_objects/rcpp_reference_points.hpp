/**
 * @file rcpp_reference_points.hpp
 * @brief R list interface for post-fit reference point calculations.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_REFERENCE_POINTS_HPP
#define FIMS_INTERFACE_RCPP_REFERENCE_POINTS_HPP

#include <Rcpp.h>
#include "../../../population_dynamics/reference_points/reference_points.hpp"

/** @brief Convert a numeric R vector to the per-recruit vector type. */
inline fims::Vector<double> ReferencePointVector(SEXP values) {
  return fims::Vector<double>(Rcpp::as<std::vector<double>>(values));
}

/** @brief Copy a FIMS vector into an R numeric vector. */
inline Rcpp::NumericVector ReferencePointVectorToR(
    const fims::Vector<double>& values) {
  Rcpp::NumericVector result(values.size());
  for (size_t i = 0; i < values.size(); ++i) result[i] = values[i];
  return result;
}

/** @brief Serialize a biological snapshot, with one list per fleet. */
inline Rcpp::List ReferencePointInputsToR(
    const fims_popdy::ReferencePointInputs<double>& inputs) {
  Rcpp::List fleets;
  for (const auto& fleet : inputs.fleets) {
    fleets.push_back(Rcpp::List::create(
        Rcpp::Named("share") = fleet.share,
        Rcpp::Named("selectivity") = ReferencePointVectorToR(fleet.selectivity),
        Rcpp::Named("weight") = ReferencePointVectorToR(fleet.weight),
        Rcpp::Named("include_in_msy") = fleet.include_in_msy));
  }
  return Rcpp::List::create(
      Rcpp::Named("natural_mortality") =
          ReferencePointVectorToR(inputs.natural_mortality),
      Rcpp::Named("weight") = ReferencePointVectorToR(inputs.weight),
      Rcpp::Named("maturity") = ReferencePointVectorToR(inputs.maturity),
      Rcpp::Named("proportion_female") =
          ReferencePointVectorToR(inputs.proportion_female),
      Rcpp::Named("fleets") = fleets);
}

/** @brief Calculate SPR and optional Beverton--Holt MSY from a reusable
 * snapshot. */
inline Rcpp::List CalculateReferencePointsR(
    Rcpp::List snapshot, Rcpp::NumericVector targets, bool include_msy,
    double max_f, double tolerance, int max_iterations, int grid_intervals) {
  if (max_iterations <= 0 || grid_intervals < 2) {
    Rcpp::stop(
        "max_iterations must be positive and grid_intervals at least two");
  }
  fims_popdy::ReferencePointInputs<double> inputs;
  inputs.natural_mortality =
      ReferencePointVector(snapshot["natural_mortality"]);
  inputs.weight = ReferencePointVector(snapshot["weight"]);
  inputs.maturity = ReferencePointVector(snapshot["maturity"]);
  inputs.proportion_female =
      ReferencePointVector(snapshot["proportion_female"]);
  Rcpp::List fleets = snapshot["fleets"];
  for (SEXP item : fleets) {
    Rcpp::List fleet(item);
    bool include_in_msy = true;
    if (fleet.containsElementNamed("include_in_msy")) {
      SEXP inclusion = fleet["include_in_msy"];
      if (TYPEOF(inclusion) != LGLSXP || Rf_xlength(inclusion) != 1 ||
          LOGICAL(inclusion)[0] == NA_LOGICAL) {
        Rcpp::stop("Each fleet's include_in_msy must be TRUE or FALSE");
      }
      include_in_msy = LOGICAL(inclusion)[0];
    }
    inputs.fleets.push_back({Rcpp::as<double>(fleet["share"]),
                             ReferencePointVector(fleet["selectivity"]),
                             ReferencePointVector(fleet["weight"]),
                             include_in_msy});
  }
  const auto unfished = fims_popdy::CalculatePerRecruit(inputs, 0.0);
  Rcpp::NumericVector f(targets.size()), spr(targets.size()),
      residual(targets.size());
  Rcpp::IntegerVector iterations(targets.size());
  Rcpp::CharacterVector status(targets.size());
  fims_popdy::SPROptions options;
  options.max_f = max_f;
  options.tolerance = tolerance;
  options.max_iterations = max_iterations;
  for (R_xlen_t i = 0; i < targets.size(); ++i) {
    const auto result = fims_popdy::CalculateSPR(inputs, targets[i], options);
    f[i] = result.fishing_mortality;
    spr[i] = result.spr;
    residual[i] = result.residual;
    iterations[i] = result.iterations;
    switch (result.status) {
      case fims_popdy::SPRStatus::converged:
        status[i] = "converged";
        break;
      case fims_popdy::SPRStatus::not_bracketed:
        status[i] = "not_bracketed";
        break;
      default:
        status[i] = "iteration_limit";
    }
  }
  Rcpp::List output = Rcpp::List::create(
      Rcpp::Named("inputs") = snapshot,
      Rcpp::Named("unfished_per_recruit") = Rcpp::List::create(
          Rcpp::Named("biomass") = unfished.biomass,
          Rcpp::Named("spawning_biomass") = unfished.spawning_biomass),
      Rcpp::Named("spr") = Rcpp::DataFrame::create(
          Rcpp::Named("target") = targets, Rcpp::Named("fishing_mortality") = f,
          Rcpp::Named("spr") = spr, Rcpp::Named("residual") = residual,
          Rcpp::Named("iterations") = iterations,
          Rcpp::Named("status") = status),
      Rcpp::Named("msy") = R_NilValue);
  if (include_msy) {
    if (!snapshot.containsElementNamed("recruitment") ||
        Rf_isNull(snapshot["recruitment"])) {
      Rcpp::stop(
          "MSY requires Beverton-Holt recruitment; use msy = FALSE for SPR "
          "only");
    }
    Rcpp::List recruitment = snapshot["recruitment"];
    if (Rcpp::as<std::string>(recruitment["type"]) != "beverton_holt") {
      Rcpp::stop("MSY currently supports only Beverton-Holt recruitment");
    }
    fims_popdy::ReferencePointRecruitment sr{
        Rcpp::as<double>(recruitment["rzero"]),
        Rcpp::as<double>(recruitment["steepness"]),
        Rcpp::as<double>(recruitment["phi0"])};
    fims_popdy::MSYOptions msy_options;
    msy_options.max_f = max_f;
    msy_options.tolerance = tolerance;
    msy_options.max_iterations = max_iterations;
    msy_options.grid_intervals = grid_intervals;
    const auto result = fims_popdy::CalculateMSY(inputs, sr, msy_options);
    std::string msy_status;
    switch (result.status) {
      case fims_popdy::MSYStatus::converged:
        msy_status = "converged";
        break;
      case fims_popdy::MSYStatus::upper_bound:
        msy_status = "upper_bound";
        break;
      case fims_popdy::MSYStatus::no_positive_yield:
        msy_status = "no_positive_yield";
        break;
      default:
        msy_status = "iteration_limit";
    }
    output["msy"] = Rcpp::List::create(
        Rcpp::Named("fishing_mortality") = result.fishing_mortality,
        // Preserve the legacy yield field as total catch from all fleets.
        Rcpp::Named("yield") = result.equilibrium.yield,
        Rcpp::Named("total_yield") = result.equilibrium.yield,
        Rcpp::Named("objective_yield") = result.equilibrium.objective_yield,
        Rcpp::Named("biomass") = result.equilibrium.biomass,
        Rcpp::Named("spawning_biomass") = result.equilibrium.spawning_biomass,
        Rcpp::Named("recruitment") = result.equilibrium.recruitment,
        Rcpp::Named("fleet_yield") =
            ReferencePointVectorToR(result.equilibrium.fleet_yield),
        Rcpp::Named("collapsed") = result.equilibrium.collapsed,
        Rcpp::Named("iterations") = result.iterations,
        Rcpp::Named("status") = msy_status);
  }
  return output;
}
#endif
