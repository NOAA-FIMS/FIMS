/**
 * \file rcpp_recruitment.cpp
 * \brief Implementation of Rcpp recruitment interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_recruitment.hpp"
#include <Rcpp.h>
// static id of the RecruitmentInterfaceBase object
uint32_t RecruitmentInterfaceBase::id_g = 1;
// local id of the RecruitmentInterfaceBase object map relating the ID of the
// RecruitmentInterfaceBase to the RecruitmentInterfaceBase objects
std::map<uint32_t, std::shared_ptr<RecruitmentInterfaceBase>>
    RecruitmentInterfaceBase::live_objects;

/**
 * Function to register recruitment classes with the Rcpp module system.
 *
 */
void register_recruitment(Rcpp::Module &m) {

  Rcpp::class_<BevertonHoltRecruitmentInterface>(
      "BevertonHoltRecruitment", "See "
                                 "https://noaa-fims.github.io/FIMS/doxygen/"
                                 "classBevertonHoltRecruitmentInterface.html.")
      .constructor()
      .field("logit_steep", &BevertonHoltRecruitmentInterface::logit_steep)
      .field("log_rzero", &BevertonHoltRecruitmentInterface::log_rzero)
      .field("log_devs", &BevertonHoltRecruitmentInterface::log_devs)
      .field("log_r", &BevertonHoltRecruitmentInterface::log_r)
      .field("log_expected_recruitment",
             &BevertonHoltRecruitmentInterface::log_expected_recruitment)
      .field("n_years", &BevertonHoltRecruitmentInterface::n_years)
      .method("get_id", &BevertonHoltRecruitmentInterface::get_id)
      .method("SetRecruitmentProcessID",
              &BevertonHoltRecruitmentInterface::SetRecruitmentProcessID)
      .method("evaluate_mean",
              &BevertonHoltRecruitmentInterface::evaluate_mean);

  Rcpp::class_<LogDevsRecruitmentInterface>(
      "LogDevsRecruitmentProcess", "See "
                                   "https://noaa-fims.github.io/FIMS/doxygen/"
                                   "classLogDevsRecruitmentInterface.html.")
      .constructor()
      .method("get_id", &LogDevsRecruitmentInterface::get_id)
      .method("evaluate_process",
              &LogDevsRecruitmentInterface::evaluate_process);

  Rcpp::class_<LogRRecruitmentInterface>(
      "LogRRecruitmentProcess", "See "
                                "https://noaa-fims.github.io/FIMS/doxygen/"
                                "classLogRRecruitmentInterface.html.")
      .constructor()
      .method("get_id", &LogRRecruitmentInterface::get_id)
      .method("evaluate_process", &LogRRecruitmentInterface::evaluate_process);
}
