/**
 * \file rcpp_selectivity.cpp
 * \brief Implementation of Rcpp selectivity interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_selectivity.hpp"

// static id of the SelectivityInterfaceBase object
uint32_t SelectivityInterfaceBase::id_g = 1;
namespace {
// Adds this counter to the list clear() rewinds.
IdCounterRegistration selectivity_id_g_registration(
    &SelectivityInterfaceBase::id_g);
}  // namespace

#include <Rcpp.h>

// ── Short name for the pointer type ─────────────────────────────────────────
// SharedSelectivity is a short name for the pointer type used throughout this
// file. create_selectivity_() hands back a pointer typed to
// SelectivityInterfaceBase whichever form it built. The forms carry different
// parameters, so the setter asks for one by name instead of naming a member.
using SharedSelectivity = std::shared_ptr<SelectivityInterfaceBase>;

// ── Building a module ──────────────────────────────────────────────────────
/**
 * @brief Create a selectivity module of the requested type.
 *
 * @details Allocates the selectivity interface object to heap memory, hands ownership 
 * to a shared_ptr, and wraps that shared_ptr in the XPtr. The 'true' argument 
 * binds the XPtr to R's garbage collector: when the R variable is garbage 
 * collected, 'delete shared_ptr*' runs and the reference count is decremented.
 *
 * @param type Either "logistic" or "double_logistic".
 *
 * @return A pointer to the new selectivity module.
 */
Rcpp::XPtr<SharedSelectivity> create_selectivity_(std::string type) {
  SharedSelectivity selectivity_interface;
  switch (SelectivityTypeFromString(type)) {
    case SelectivityType::logistic:
      selectivity_interface = std::make_shared<LogisticSelectivityInterface>();
      break;
    case SelectivityType::double_logistic:
      selectivity_interface =
          std::make_shared<DoubleLogisticSelectivityInterface>();
      break;
  }
  return Rcpp::XPtr<SharedSelectivity>(
      new SharedSelectivity(selectivity_interface), true);
}

/**
 * @brief Evaluate selectivity at one point.
 *
 * @details Evaluates the interface object directly, so it can be called before
 *   CreateTMBModel() to check a selectivity curve.
 *
 * @param xp The selectivity module.
 * @param x The independent variable, usually age or length.
 * @return Selectivity at that point.
 */
double evaluate_selectivity_(Rcpp::XPtr<SharedSelectivity> xp, double x) {
  return (*xp)->evaluate(x);
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
 * @param xp The selectivity module.
 * @return A base-class pointer to the same module.
 */
Rcpp::XPtr<SharedBase> selectivity_to_fims_xptr_(
    Rcpp::XPtr<SharedSelectivity> xp) {
  // Every interface class inherits from FIMSRcppInterfaceBase, so this
  // conversion needs no cast: same object, more general pointer type.
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

// ── Invalidation for clear() ─────────────────────────────────────────────────
/**
 * @brief Invalidate a pointer to a selectivity module, releasing this
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
void release_selectivity_(Rcpp::XPtr<SharedSelectivity> xp) { xp.release(); }

/**
 * Function to register selectivity classes with the Rcpp module system.
 *
 */
/**
 * @brief Register the selectivity functions with the Rcpp module system.
 *
 * @param m The Rcpp module to register into.
 */
void register_selectivity(Rcpp::Module& m) {
  Rcpp::function("create_selectivity_", &create_selectivity_);
  Rcpp::function("evaluate_selectivity_", &evaluate_selectivity_);
  Rcpp::function("selectivity_to_fims_xptr_", &selectivity_to_fims_xptr_);
  Rcpp::function("release_selectivity_", &release_selectivity_);
}
