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
using SharedLogRRecruitmentProcess = std::shared_ptr<LogRRecruitmentInterface>;
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

void set_beverton_holt_n_years_(Rcpp::XPtr<SharedBevertonHoltRecruitment> xp,
                                int n_years) {
  (*xp)->n_years = n_years;
}

void set_beverton_holt_process_id_(Rcpp::XPtr<SharedBevertonHoltRecruitment> xp,
                                   int process_id) {
  (*xp)->SetRecruitmentProcessID(process_id);
}

void set_beverton_holt_logit_steep_(
    Rcpp::XPtr<SharedBevertonHoltRecruitment> xp, Rcpp::NumericVector values,
    Rcpp::CharacterVector estimation_status) {
  (*xp)->logit_steep.resize(values.size());
  (*xp)->logit_steep.set_values(values);
  (*xp)->logit_steep.set_estimation_status(estimation_status);
}

void set_beverton_holt_log_rzero_(Rcpp::XPtr<SharedBevertonHoltRecruitment> xp,
                                  Rcpp::NumericVector values,
                                  Rcpp::CharacterVector estimation_status) {
  (*xp)->log_rzero.resize(values.size());
  (*xp)->log_rzero.set_values(values);
  (*xp)->log_rzero.set_estimation_status(estimation_status);
}

void set_beverton_holt_log_devs_(Rcpp::XPtr<SharedBevertonHoltRecruitment> xp,
                                 Rcpp::NumericVector values,
                                 Rcpp::CharacterVector estimation_status) {
  (*xp)->log_devs.resize(values.size());
  (*xp)->log_devs.set_values(values);
  (*xp)->log_devs.set_estimation_status(estimation_status);
}

void set_beverton_holt_log_r_(Rcpp::XPtr<SharedBevertonHoltRecruitment> xp,
                              Rcpp::NumericVector values,
                              Rcpp::CharacterVector estimation_status) {
  (*xp)->log_r.resize(values.size());
  (*xp)->log_r.set_values(values);
  (*xp)->log_r.set_estimation_status(estimation_status);
}

uint32_t get_beverton_holt_recruitment_id_(
    Rcpp::XPtr<SharedBevertonHoltRecruitment> xp) {
  return (*xp)->get_id();
}

uint32_t get_log_devs_recruitment_process_id_(
    Rcpp::XPtr<SharedLogDevsRecruitmentProcess> xp) {
  return (*xp)->get_id();
}

uint32_t get_log_r_recruitment_process_id_(
    Rcpp::XPtr<SharedLogRRecruitmentProcess> xp) {
  return (*xp)->get_id();
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
  Rcpp::function("set_beverton_holt_n_years_", &set_beverton_holt_n_years_);
  Rcpp::function("set_beverton_holt_process_id_",
                 &set_beverton_holt_process_id_);
  Rcpp::function("set_beverton_holt_logit_steep_",
                 &set_beverton_holt_logit_steep_);
  Rcpp::function("set_beverton_holt_log_rzero_", &set_beverton_holt_log_rzero_);
  Rcpp::function("set_beverton_holt_log_devs_", &set_beverton_holt_log_devs_);
  Rcpp::function("set_beverton_holt_log_r_", &set_beverton_holt_log_r_);
  Rcpp::function("get_beverton_holt_recruitment_id_",
                 &get_beverton_holt_recruitment_id_);
  Rcpp::function("beverton_holt_recruitment_to_fims_xptr_",
                 &beverton_holt_recruitment_to_fims_xptr_);
  Rcpp::function("create_log_devs_recruitment_process_",
                 &create_log_devs_recruitment_process_);
  Rcpp::function("get_log_devs_recruitment_process_id_",
                 &get_log_devs_recruitment_process_id_);
  Rcpp::function("log_devs_recruitment_process_to_fims_xptr_",
                 &log_devs_recruitment_process_to_fims_xptr_);
  Rcpp::function("create_log_r_recruitment_process_",
                 &create_log_r_recruitment_process_);
  Rcpp::function("get_log_r_recruitment_process_id_",
                 &get_log_r_recruitment_process_id_);
  Rcpp::function("log_r_recruitment_process_to_fims_xptr_",
                 &log_r_recruitment_process_to_fims_xptr_);
}
