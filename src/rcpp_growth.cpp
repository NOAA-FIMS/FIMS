/**
 * \file rcpp_growth.cpp
 * \brief Implementation of Rcpp growth interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_growth.hpp"
// static id of the GrowthInterfaceBase object
uint32_t GrowthInterfaceBase::id_g = 1;

#include <Rcpp.h>

// SharedGrowth is the pointer type used throughout this file and allows
// functions to work on the correct child class based on the input string.
using SharedGrowth = std::shared_ptr<GrowthInterfaceBase>;

// ── Building a module ──────────────────────────────────────────────────────
/**
 * @brief Create a growth module of the requested type.
 *
 * @details Allocates the growth interface object to heap memory, hands ownership
 * to a shared_ptr, and wraps that shared_ptr in the XPtr. The 'true' argument 
 * binds the XPtr to R's garbage collector: when the R variable is garbage 
 * collected, 'delete shared_ptr*' runs and the reference count is decremented.
 *
 * @param type Currently only "ewaa", for empirical weight-at-age.
 *
 * @return A pointer to the new growth module.
 */
Rcpp::XPtr<SharedGrowth> create_growth_(std::string type) {
  SharedGrowth growth_interface;
  switch (GrowthTypeFromString(type)) {
    case GrowthType::ewaa:
      growth_interface = std::make_shared<EWAAGrowthInterface>();
      break;
  }
  return Rcpp::XPtr<SharedGrowth>(new SharedGrowth(growth_interface), true);
}

// ── Setters ──────────────────────────────────────────────────────────────────

/**
 * @brief Set the number of years this growth module spans.
 *
 * @details Growth forms without a time dimension report an error rather than
 *   silently ignoring the value.
 *
 * @param xp The growth module.
 * @param n_years Number of years.
 */
void set_growth_n_years_(Rcpp::XPtr<SharedGrowth> xp, int n_years) {
  (*xp)->set_n_years(n_years);
}

// ── Getters ──────────────────────────────────────────────────────────────────

/**
 * @brief Evaluate growth at one age.
 *
 * @details Evaluates the interface object directly, so it can be called before
 *   CreateTMBModel() to check a growth curve.
 *
 * @param xp The growth module.
 * @param age The age to evaluate at.
 * @return Weight at that age.
 */
double evaluate_growth_(Rcpp::XPtr<SharedGrowth> xp, double age) {
  return (*xp)->evaluate(age);
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
 * @param xp The growth module.
 * @return A base-class pointer to the same module.
 */
Rcpp::XPtr<SharedBase> growth_to_fims_xptr_(Rcpp::XPtr<SharedGrowth> xp) {
  // Every interface class inherits from FIMSRcppInterfaceBase, so this
  // conversion needs no cast: same object, more general pointer type.
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

/**
 * Function to register growth classes with the Rcpp module system.
 *
 */
/**
 * @brief Register the growth functions with the Rcpp module system.
 *
 * @param m The Rcpp module to register into.
 */
void register_growth(Rcpp::Module& m) {
  // Rcpp::class_<> registrations removed — users no longer call methods::new().
  Rcpp::function("create_growth_", &create_growth_);
  Rcpp::function("set_growth_n_years_", &set_growth_n_years_);
  Rcpp::function("evaluate_growth_", &evaluate_growth_);
  Rcpp::function("growth_to_fims_xptr_", &growth_to_fims_xptr_);
}
