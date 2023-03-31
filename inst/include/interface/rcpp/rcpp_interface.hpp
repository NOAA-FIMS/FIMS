/*
 * File:   rcpp_interface.hpp
 *
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file for reuse
 * information.
 *
 *
 */
#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP

#include "rcpp_objects/rcpp_data.hpp"
#include "rcpp_objects/rcpp_fleet.hpp"
#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_maturity.hpp"
#include "rcpp_objects/rcpp_natural_mortality.hpp"
#include "rcpp_objects/rcpp_nll.hpp"
#include "rcpp_objects/rcpp_population.hpp"
#include "rcpp_objects/rcpp_recruitment.hpp"
#include "rcpp_objects/rcpp_selectivity.hpp"
#include "rcpp_objects/rcpp_tmb_distribution.hpp"

/**
 * @brief Create the TMB model object and add interface objects to it.
 */
bool CreateTMBModel() {
  for (size_t i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
       i++) {
    FIMSRcppInterfaceBase::fims_interface_objects[i]->add_to_fims_tmb();
  }

  // base model
  std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  d0->CreateModel();

  // first-order derivative
  std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
      fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();
  d1->CreateModel();

  // second-order derivative
  std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
      fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();
  d2->CreateModel();

  // third-order derivative
  std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
      fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();
  d3->CreateModel();

  return true;
}

Rcpp::NumericVector get_fixed_parameters_vector() {
  // base model
  std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (int i = 0; i < d0->fixed_effects_parameters.size(); i++) {
    p.push_back(*d0->fixed_effects_parameters[i]);
  }

  return p;
}

Rcpp::NumericVector get_random_parameters_vector() {
  // base model
  std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

  Rcpp::NumericVector p;

  for (int i = 0; i < d0->random_effects_parameters.size(); i++) {
    p.push_back(*d0->random_effects_parameters[i]);
  }

  return p;
}

/**
 * Clears the vector of independent variables.
 */
void clear() {
  FIMSRcppInterfaceBase::fims_interface_objects.clear();

  std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
      fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  d0->fixed_effects_parameters.clear();
  d0->random_effects_parameters.clear();

  LogisticSelectivityInterface::id_g = 1;
  LogisticSelectivityInterface::selectivity_objects.clear();

  BevertonHoltRecruitmentInterface::id_g = 1;
  BevertonHoltRecruitmentInterface::live_objects.clear();
}

RCPP_EXPOSED_CLASS(Parameter)
RCPP_MODULE(fims) {
  Rcpp::function("CreateTMBModel", &CreateTMBModel);
  Rcpp::function("get_fixed", &get_fixed_parameters_vector);
  Rcpp::function("get_random", &get_random_parameters_vector);
  Rcpp::function("clear", clear);

  Rcpp::class_<Parameter>("Parameter")
      .constructor()
      .constructor<double>()
      .constructor<Parameter>()
      .field("value", &Parameter::value)
      .field("min", &Parameter::min)
      .field("max", &Parameter::max)
      .field("is_random_effect", &Parameter::is_random_effect)
      .field("estimated", &Parameter::estimated);

  Rcpp::class_<BevertonHoltRecruitmentInterface>("BevertonHoltRecruitment")
      .constructor()
      .field("steep", &BevertonHoltRecruitmentInterface::steep)
      .field("rzero", &BevertonHoltRecruitmentInterface::rzero)
      .method("get_id", &BevertonHoltRecruitmentInterface::get_id)
      .method("evaluate", &BevertonHoltRecruitmentInterface::evaluate);

  Rcpp::class_<RecruitmentNLLInterface>("RecruitmentNLL")
      .constructor()
      .field("log_sigma_recruit", &RecruitmentNLLInterface::log_sigma_recruit)
      .field("recruitment_devs", &RecruitmentNLLInterface::recruit_deviations)
      .field("recruitment_bias_adj",
             &RecruitmentNLLInterface::recruit_bias_adjustment)
      .field("do_bias_correction",
             &RecruitmentNLLInterface::use_recruit_bias_adjustment)
      .field("estimate_recruit_deviations",
             &RecruitmentNLLInterface::estimate_recruit_deviations)
      .method("evaluate", &RecruitmentNLLInterface::evaluate_nll);

  Rcpp::class_<FleetInterface>("Fleet")
      .constructor()
      .field("log_q", &FleetInterface::log_q)
      .field("log_Fmort", &FleetInterface::log_Fmort)
      .field("nages", &FleetInterface::nages)
      .field("nyears", &FleetInterface::nyears)
      .field("estimate_F", &FleetInterface::estimate_F)
      .field("estimate_q", &FleetInterface::estimate_q)
      .field("random_q", &FleetInterface::random_q)
      .field("random_F", &FleetInterface::random_F)
      .method("SetAgeCompLikelihood", &FleetInterface::SetAgeCompLikelihood)
      .method("SetIndexLikelihood", &FleetInterface::SetIndexLikelihood)
      .method("SetObservedAgeCompData", &FleetInterface::SetObservedAgeCompData)
      .method("SetObservedIndexData", &FleetInterface::SetObservedIndexData)
      .method("SetSelectivity", &FleetInterface::SetSelectivity);

  Rcpp::class_<DataInterface>("Data").constructor().field(
      "observed_data", &DataInterface::observed_data);

  Rcpp::class_<AgeCompDataInterface>("AgeComp").constructor<int, int>().field(
      "age_comp_data", &AgeCompDataInterface::age_comp_data);

  Rcpp::class_<IndexDataInterface>("Index").constructor<int>().field(
      "index_data", &IndexDataInterface::index_data);

  Rcpp::class_<PopulationInterface>("Population")
      .constructor()
      .method("get_id", &PopulationInterface::get_id)
      .field("nages", &PopulationInterface::nages)
      .field("nfleets", &PopulationInterface::nfleets)
      .field("nseasons", &PopulationInterface::nseasons)
      .field("nyears", &PopulationInterface::nyears)
      .field("log_M", &PopulationInterface::log_M)
      .field("log_init_naa", &PopulationInterface::log_init_naa)
      .field("prop_female", &PopulationInterface::prop_female)
      .method("evaluate", &PopulationInterface::evaluate);

  Rcpp::class_<DnormDistributionsInterface>("TMBDnormDistribution")
      .constructor()
      .method("get_id", &DnormDistributionsInterface::get_id)
      .method("evaluate", &DnormDistributionsInterface::evaluate)
      .field("x", &DnormDistributionsInterface::x)
      .field("mean", &DnormDistributionsInterface::mean)
      .field("sd", &DnormDistributionsInterface::sd);

  Rcpp::class_<LogisticMaturityInterface>("LogisticMaturity")
      .constructor()
      .field("median", &LogisticMaturityInterface::median)
      .field("slope", &LogisticMaturityInterface::slope)
      .method("get_id", &LogisticMaturityInterface::get_id)
      .method("evaluate", &LogisticMaturityInterface::evaluate);

  Rcpp::class_<LogisticSelectivityInterface>("LogisticSelectivity")
      .constructor()
      .field("median", &LogisticSelectivityInterface::median)
      .field("slope", &LogisticSelectivityInterface::slope)
      .method("get_id", &LogisticSelectivityInterface::get_id)
      .method("evaluate", &LogisticSelectivityInterface::evaluate);

  Rcpp::class_<DoubleLogisticSelectivityInterface>("DoubleLogisticSelectivity")
      .constructor()
      .field("median_asc", &DoubleLogisticSelectivityInterface::median_asc)
      .field("slope_asc", &DoubleLogisticSelectivityInterface::slope_asc)
      .field("median_desc", &DoubleLogisticSelectivityInterface::median_desc)
      .field("slope_desc", &DoubleLogisticSelectivityInterface::slope_desc)
      .method("get_id", &DoubleLogisticSelectivityInterface::get_id)
      .method("evaluate", &DoubleLogisticSelectivityInterface::evaluate);

  Rcpp::class_<EWAAGrowthInterface>("EWAAgrowth")
      .constructor()
      .field("ages", &EWAAGrowthInterface::ages)
      .field("weights", &EWAAGrowthInterface::weights)
      .method("evaluate", &EWAAGrowthInterface::evaluate);

  Rcpp::class_<DlnormDistributionsInterface>("TMBDlnormDistribution")
      .constructor()
      .method("get_id", &DlnormDistributionsInterface::get_id)
      .method("evaluate", &DlnormDistributionsInterface::evaluate)
      .field("x", &DlnormDistributionsInterface::x)
      .field("meanlog", &DlnormDistributionsInterface::meanlog)
      .field("sdlog", &DlnormDistributionsInterface::sdlog)
      .field("do_bias_correction",
             &DlnormDistributionsInterface::do_bias_correction);

  Rcpp::class_<DmultinomDistributionsInterface>("TMBDmultinomDistribution")
      .constructor()
      .method("evaluate", &DmultinomDistributionsInterface::evaluate)
      .method("get_id", &DmultinomDistributionsInterface::get_id)
      .field("x", &DmultinomDistributionsInterface::x)
      .field("p", &DmultinomDistributionsInterface::p);
}

#endif /* RCPP_INTERFACE_HPP */
