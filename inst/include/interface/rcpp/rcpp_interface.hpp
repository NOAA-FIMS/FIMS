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

#include "rcpp_objects/rcpp_fishing_mortality.hpp"
#include "rcpp_objects/rcpp_fleet.hpp"
#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_maturity.hpp"
#include "rcpp_objects/rcpp_natural_mortality.hpp"
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
  std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
      fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
  d0->CreateModel();

  // first-order derivative
  std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
      fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();
  d1->CreateModel();

  // second-order derivative
  std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
      fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();
  d2->CreateModel();

  // third-order derivative
  std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
      fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();
  d3->CreateModel();

  return true;
}

RCPP_EXPOSED_CLASS(Parameter)
RCPP_MODULE(fims) {
  Rcpp::function("CreateTMBModel", &CreateTMBModel);

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
      .field("phizero", &BevertonHoltRecruitmentInterface::phizero)
      .method("get_id", &BevertonHoltRecruitmentInterface::get_id);

  Rcpp::class_<LogisticSelectivityInterface>("LogisticSelectivity")
      .constructor()
      .field("median", &LogisticSelectivityInterface::median)
      .field("slope", &LogisticSelectivityInterface::slope)
      .method("get_id", &LogisticSelectivityInterface::get_id);

  Rcpp::class_<FleetInterface>("Fleet")
      .constructor()
      .method("SetAgeCompLikelihood", &FleetInterface::SetAgeCompLikelihood)
      .method("SetIndexLikelihood", &FleetInterface::SetIndexLikelihood)
      .method("SetObservedAgeCompData", &FleetInterface::SetObservedAgeCompData)
      .method("SetObservedIndexData", &FleetInterface::SetObservedIndexData)
      .method("SetSelectivity", &FleetInterface::SetSelectivity);

  Rcpp::class_<DnormDistributionsInterface>("TMBDnormDistribution")
      .constructor()
      .method("get_id", &DnormDistributionsInterface::get_id)
      .method("evaluate", &DnormDistributionsInterface::evaluate)
      .field("x", &DnormDistributionsInterface::x)
      .field("mean", &DnormDistributionsInterface::mean)
      .field("sd", &DnormDistributionsInterface::sd);

  Rcpp::class_<EWAAGrowthInterface>("EWAAgrowth")
      .constructor()
      .field("ages", &EWAAGrowthInterface::ages)
      .field("weights", &EWAAGrowthInterface::weights)
      .method("evaluate", &EWAAGrowthInterface::evaluate);

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
      .field("sdlog", &DlnormDistributionsInterface::sdlog);

  Rcpp::class_<DmultinomDistributionsInterface>("TMBDmultinomDistribution")
      .constructor()
      .method("evaluate", &DmultinomDistributionsInterface::evaluate)
      .method("get_id", &DmultinomDistributionsInterface::get_id)
      .field("x", &DmultinomDistributionsInterface::x)
      .field("p", &DmultinomDistributionsInterface::p);
}

#endif /* RCPP_INTERFACE_HPP */
