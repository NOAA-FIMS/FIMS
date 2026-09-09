/**
 * \file rcpp_maturity.cpp
 * \brief Implementation of Rcpp maturity interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_maturity.hpp"
// static id of the MaturityInterfaceBase object
uint32_t MaturityInterfaceBase::id_g = 1;
namespace {
// Adds this counter to the list clear() rewinds.
IdCounterRegistration maturity_id_g_registration(
    &MaturityInterfaceBase::id_g);
}  // namespace

#include <Rcpp.h>

// SharedMaturity is the pointer type used throughout this file and allows
// functions to work on the correct child class based on the input string.
using SharedMaturity = std::shared_ptr<MaturityInterfaceBase>;

// ── Building a module ──────────────────────────────────────────────────────
/**
 * @brief Create a maturity module of the requested type.
 *
 * @details Allocates the maturity interface object to heap memory, hands ownership 
 * to a shared_ptr, and wraps that shared_ptr in the XPtr. The 'true' argument 
 * binds the XPtr to R's garbage collector: when the R variable is garbage 
 * collected, 'delete shared_ptr*' runs and the reference count is decremented.
 *
 * @param type Currently only "Logistic".
 *
 * @return A pointer to the new maturity module.
 */
Rcpp::XPtr<SharedMaturity> create_maturity_(std::string type) {
  SharedMaturity maturity_interface;
  switch (MaturityTypeFromString(type)) {
    case MaturityType::logistic:
      maturity_interface = std::make_shared<LogisticMaturityInterface>();
      break;
  }
  return Rcpp::XPtr<SharedMaturity>(new SharedMaturity(maturity_interface),
                                    true);
}

/**
 * @brief Evaluate maturity at one point.
 *
 * @details Evaluates the interface object directly, so it can be called before
 *   CreateTMBModel() to check a maturity curve.
 *
 * @param xp The maturity module.
 * @param x The independent variable, usually age or length.
 * @return Proportion mature at that point.
 */
double evaluate_maturity_(Rcpp::XPtr<SharedMaturity> xp, double x) {
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
 * @param xp The maturity module.
 * @return A base-class pointer to the same module.
 */
Rcpp::XPtr<SharedBase> maturity_to_fims_xptr_(Rcpp::XPtr<SharedMaturity> xp) {
  // Every interface class inherits from FIMSRcppInterfaceBase, so this
  // conversion needs no cast: same object, more general pointer type.
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

// ── Invalidation for clear() ─────────────────────────────────────────────────
/**
 * @brief Invalidate a pointer to a maturity module, releasing this
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
void release_maturity_(Rcpp::XPtr<SharedMaturity> xp) { xp.release(); }

/**
 * Function to register maturity classes with the Rcpp module system.
 *
 */
/**
 * @brief Register the maturity functions with the Rcpp module system.
 *
 * @param m The Rcpp module to register into.
 */
void register_maturity(Rcpp::Module& m) {
  Rcpp::function("create_maturity_", &create_maturity_);
  Rcpp::function("evaluate_maturity_", &evaluate_maturity_);
  Rcpp::function("maturity_to_fims_xptr_", &maturity_to_fims_xptr_);
  Rcpp::function("release_maturity_", &release_maturity_);
}
