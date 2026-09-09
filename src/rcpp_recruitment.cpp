/**
 * \file rcpp_recruitment.cpp
 * \brief Implementation of Rcpp recruitment interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_recruitment.hpp"
#include <Rcpp.h>
// static id of the RecruitmentInterfaceBase object
uint32_t RecruitmentInterfaceBase::id_g = 1;
namespace {
// Adds this counter to the list clear() rewinds.
IdCounterRegistration recruitment_id_g_registration(
    &RecruitmentInterfaceBase::id_g);
}  // namespace

// ── Short name for the pointer type ─────────────────────────────────────────
// SharedRecruitment is a short name for the pointer type used throughout this
// file. It covers both the stock--recruit relationship and the recruitment
// process modules: evaluate_mean() and evaluate_process() are
// declared on RecruitmentInterfaceBase, so the functions below call them
// without knowing which form it is. Parameters differ between forms, so those
// are asked for by name.
using SharedRecruitment = std::shared_ptr<RecruitmentInterfaceBase>;

// ── Building a module ──────────────────────────────────────────────────────
/**
 * @brief Create a recruitment module of the requested type.
 *
 * @details Allocates the recruitment interface object to heap memory, hands ownership 
 * to a shared_ptr, and wraps that shared_ptr in the XPtr. The 'true' argument 
 * binds the XPtr to R's garbage collector: when the R variable is garbage 
 * collected, 'delete shared_ptr*' runs and the reference count is decremented.
 *
 * @param type One of "BevertonHolt", "log_devs", or "log_r".
 *   The latter two are recruitment process modules, which carry no parameters
 *   of their own and are linked to a stock--recruit module by ID.
 */
Rcpp::XPtr<SharedRecruitment> create_recruitment_(std::string type) {
  SharedRecruitment recruitment_interface;
  switch (RecruitmentTypeFromString(type)) {
    case RecruitmentType::beverton_holt:
      recruitment_interface =
          std::make_shared<BevertonHoltRecruitmentInterface>();
      break;
    case RecruitmentType::log_devs:
      recruitment_interface = std::make_shared<LogDevsRecruitmentInterface>();
      break;
    case RecruitmentType::log_r:
      recruitment_interface = std::make_shared<LogRRecruitmentInterface>();
      break;
  }
  return Rcpp::XPtr<SharedRecruitment>(
      new SharedRecruitment(recruitment_interface), true);
}

// ── Scalar setters ───────────────────────────────────────────────────────────
// process_id is declared on the base, so this works for any recruitment form.
/**
 * @brief Link a recruitment parameterization to the recruitment process module.
 *
 * @param xp The recruitment module.
 * @param process_id The process module's ID, as returned by
 *   get_module_id_() on a process module.
 */
void set_recruitment_process_id_(Rcpp::XPtr<SharedRecruitment> xp,
                                 int process_id) {
  (*xp)->process_id = process_id;
}

/**
 * @brief Read the ID of the process module a recruitment module is linked to.
 *
 * @details The counterpart to set_recruitment_process_id_(). An unset link
 * reads back as the value the module is constructed with, so a caller can
 * tell "not linked" from "linked to module 0".
 *
 * @param xp The recruitment module.
 * @return The linked process module's ID.
 */
int get_recruitment_process_id_(Rcpp::XPtr<SharedRecruitment> xp) {
  return (*xp)->process_id;
}

// n_years sizes log_expected_recruitment, which only the stock--recruit module
// has.
/**
 * @brief Set the number of years this recruitment module spans.
 *
 * @details Sizes the expected-recruitment vector, which only a stock--recruit
 *   module has, so calling this on a process module is an error.
 *
 * @param xp The recruitment module.
 * @param n_years Number of years.
 */
void set_recruitment_n_years_(Rcpp::XPtr<SharedRecruitment> xp, int n_years) {
  std::shared_ptr<BevertonHoltRecruitmentInterface> beverton_holt =
      std::dynamic_pointer_cast<BevertonHoltRecruitmentInterface>(*xp);
  if (!beverton_holt) {
    Rcpp::stop(
        "`n_years` applies only to a stock--recruit module, not to a "
        "recruitment process module.");
  }
  beverton_holt->n_years = n_years;
}

// ── Getters ──────────────────────────────────────────────────────────────────

/**
 * @brief Evaluate expected recruitment.
 *
 * @param xp The recruitment module.
 * @param spawners Spawning biomass.
 * @param phi_0 Unfished spawning biomass per recruit.
 * @return Expected recruitment. Process modules have no stock--recruit
 *   relationship and return 0.
 */
double evaluate_recruitment_mean_(Rcpp::XPtr<SharedRecruitment> xp,
                                  double spawners, double phi_0) {
  return (*xp)->evaluate_mean(spawners, phi_0);
}

/**
 * @brief Evaluate the recruitment process based on the parameterization.
 *
 * @param xp The recruitment process parameterization.
 * @param pos The time step to evaluate at.
 * @return The process value. Stock--recruit modules have no process and return
 *   0.
 */
double evaluate_recruitment_process_(Rcpp::XPtr<SharedRecruitment> xp,
                                     size_t pos) {
  return (*xp)->evaluate_process(pos);
}

// ── Base-class conversion for CreateTMBModel() ───────────────────────────────
/**
 * @brief Return a second pointer to the same object, typed to the common
 *   interface base class.
 *
 * @details This does not create a new object. CreateTMBModel() holds a mixed
 *   list of modules and calls add_to_fims_tmb() on each, which it can only do
 *   through a pointer typed to the class they all share.
 *
 * @param xp The recruitment module.
 * @return A base-class pointer to the same module.
 */
Rcpp::XPtr<SharedBase> recruitment_to_fims_xptr_(
    Rcpp::XPtr<SharedRecruitment> xp) {
  // Every interface class inherits from FIMSRcppInterfaceBase, so this
  // conversion needs no cast: same object, more general pointer type.
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

// ── Invalidation for clear() ─────────────────────────────────────────────────
/**
 * @brief Invalidate a pointer to a recruitment module, releasing this
 * pointer's share of the module.
 *
 * @details Called by the R `clear()` wrapper for every module in the registry.
 * The external pointer is set to NULL in place, so the R variable still holding
 * it reports "external pointer is not valid" on the next use instead of quietly
 * operating on a module that is no longer part of any model. Once both this
 * pointer and the module's base pointer are released, nothing owns the module
 * and its memory is returned immediately rather than at the next garbage
 * collection.
 *
 * Releasing an already-released pointer does nothing, so this is safe to call
 * twice.
 *
 * @param xp The module to invalidate.
 */
void release_recruitment_(Rcpp::XPtr<SharedRecruitment> xp) { xp.release(); }

/**
 * Function to register recruitment classes with the Rcpp module system.
 *
 */
/**
 * @brief Register the recruitment functions with the Rcpp module system.
 *
 * @param m The Rcpp module to register into.
 */
void register_recruitment(Rcpp::Module& m) {
  Rcpp::function("create_recruitment_", &create_recruitment_);
  Rcpp::function("set_recruitment_process_id_", &set_recruitment_process_id_);
  Rcpp::function("get_recruitment_process_id_",
                 &get_recruitment_process_id_);
  Rcpp::function("set_recruitment_n_years_", &set_recruitment_n_years_);
  Rcpp::function("evaluate_recruitment_mean_", &evaluate_recruitment_mean_);
  Rcpp::function("evaluate_recruitment_process_",
                 &evaluate_recruitment_process_);
  Rcpp::function("recruitment_to_fims_xptr_", &recruitment_to_fims_xptr_);
  Rcpp::function("release_recruitment_", &release_recruitment_);
}
