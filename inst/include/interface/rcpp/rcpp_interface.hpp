/*
 * File:   rcpp_interface.hpp
 *
 * Author: Matthew Supernaw
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov
 *
 * Created on May 31, 2022 at 12:04 PM
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 *
 * This software is a "United States Government Work" under the terms of the
 * United States Copyright Act.  It was written as part of the author's official
 * duties as a United States Government employee and thus cannot be copyrighted.
 * This software is freely available to the public for use. The National Oceanic
 * And Atmospheric Administration and the U.S. Government have not placed any
 * restriction on its use or reproduction.  Although all reasonable efforts have
 * been taken to ensure the accuracy and reliability of the software and data,
 * the National Oceanic And Atmospheric Administration and the U.S. Government
 * do not and cannot warrant the performance or results that may be obtained by
 * using this  software or data. The National Oceanic And Atmospheric
 * Administration and the U.S. Government disclaim all warranties, express or
 * implied, including warranties of performance, merchantability or fitness
 * for any particular purpose.
 *
 * Please cite the author(s) in any work or product based on this material.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_INTERFACE_HPP
#define FIMS_INTERFACE_RCPP_INTERFACE_HPP

#include "rcpp_objects/rcpp_fishing_mortality.hpp"
#include "rcpp_objects/rcpp_fleet.hpp"
#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_tmb_dnorm_likelihood.hpp"
#include "rcpp_objects/rcpp_maturity.hpp"
#include "rcpp_objects/rcpp_natural_mortality.hpp"
#include "rcpp_objects/rcpp_population.hpp"
#include "rcpp_objects/rcpp_recruitment.hpp"
#include "rcpp_objects/rcpp_selectivity.hpp"

/**
 *
 */
bool CreateTMBModel() {
  for (int i = 0; i < FIMSRcppInterfaceBase::fims_interface_objects.size();
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

   Rcpp::class_<DnormLikelihoodsInterface>("TMBDnormLikelihood")
    .constructor()
    .method("get_id",  &DnormLikelihoodsInterface::get_id)
    //.method("evaluate",  &DnormLikelihoodsInterface::evaluate<T>)
    .field("x", &DnormLikelihoodsInterface::x)
    .field("mean", &DnormLikelihoodsInterface::mean)
    .field("sd", &DnormLikelihoodsInterface::sd);
}

#endif /* RCPP_INTERFACE_HPP */