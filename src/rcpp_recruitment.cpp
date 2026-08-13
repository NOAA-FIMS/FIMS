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

using SharedBevertonHoltRecruitment =
    std::shared_ptr<BevertonHoltRecruitmentInterface>;
using SharedLogDevsRecruitmentProcess =
    std::shared_ptr<LogDevsRecruitmentInterface>;
using SharedLogRRecruitmentProcess =
    std::shared_ptr<LogRRecruitmentInterface>;
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedBevertonHoltRecruitment> create_beverton_holt_recruitment_() {
  auto obj = std::make_shared<BevertonHoltRecruitmentInterface>();
  return Rcpp::XPtr<SharedBevertonHoltRecruitment>(
      new SharedBevertonHoltRecruitment(obj), true);
}

Rcpp::XPtr<SharedLogDevsRecruitmentProcess>
create_log_devs_recruitment_process_() {
  auto obj = std::make_shared<LogDevsRecruitmentInterface>();
  return Rcpp::XPtr<SharedLogDevsRecruitmentProcess>(
      new SharedLogDevsRecruitmentProcess(obj), true);
}

Rcpp::XPtr<SharedLogRRecruitmentProcess> create_log_r_recruitment_process_() {
  auto obj = std::make_shared<LogRRecruitmentInterface>();
  return Rcpp::XPtr<SharedLogRRecruitmentProcess>(
      new SharedLogRRecruitmentProcess(obj), true);
}

Rcpp::XPtr<SharedBase> beverton_holt_recruitment_to_fims_xptr_(
    Rcpp::XPtr<SharedBevertonHoltRecruitment> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

Rcpp::XPtr<SharedBase> log_devs_recruitment_process_to_fims_xptr_(
    Rcpp::XPtr<SharedLogDevsRecruitmentProcess> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

Rcpp::XPtr<SharedBase> log_r_recruitment_process_to_fims_xptr_(
    Rcpp::XPtr<SharedLogRRecruitmentProcess> xp) {
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

/**
 * Function to register recruitment classes with the Rcpp module system.
 *
 */
void register_recruitment(Rcpp::Module& m) {
  Rcpp::function("create_beverton_holt_recruitment_",
                 &create_beverton_holt_recruitment_);
  Rcpp::function("beverton_holt_recruitment_to_fims_xptr_",
                 &beverton_holt_recruitment_to_fims_xptr_);
  Rcpp::function("create_log_devs_recruitment_process_",
                 &create_log_devs_recruitment_process_);
  Rcpp::function("log_devs_recruitment_process_to_fims_xptr_",
                 &log_devs_recruitment_process_to_fims_xptr_);
  Rcpp::function("create_log_r_recruitment_process_",
                 &create_log_r_recruitment_process_);
  Rcpp::function("log_r_recruitment_process_to_fims_xptr_",
                 &log_r_recruitment_process_to_fims_xptr_);

  Rcpp::class_<BevertonHoltRecruitmentInterface>(
      "BevertonHoltRecruitment",
      "See "
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
      "LogDevsRecruitmentProcess",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLogDevsRecruitmentInterface.html.")
      .constructor()
      .method("get_id", &LogDevsRecruitmentInterface::get_id)
      .method("evaluate_process",
              &LogDevsRecruitmentInterface::evaluate_process);

  Rcpp::class_<LogRRecruitmentInterface>(
      "LogRRecruitmentProcess",
      "See "
      "https://noaa-fims.github.io/FIMS/doxygen/"
      "classLogRRecruitmentInterface.html.")
      .constructor()
      .method("get_id", &LogRRecruitmentInterface::get_id)
      .method("evaluate_process", &LogRRecruitmentInterface::evaluate_process);
}
