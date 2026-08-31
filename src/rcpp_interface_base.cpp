/**
 * \file rcpp_interface_base.cpp
 * \brief Implementation of the base class for Rcpp interfaces in the FIMS
 * framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_interface_base.hpp"

#include <Rcpp.h>

/**
 * @brief The unique ID for the variable map that points to a fims::Vector.
 */
uint32_t Variable::id_g = 0;

uint32_t VariableVector::id_g = 0;

namespace {
// Adds these counters to the list clear() rewinds.
IdCounterRegistration variable_id_g_registration(&Variable::id_g);
IdCounterRegistration variable_vector_id_g_registration(
    &VariableVector::id_g);
}  // namespace

// ── Parameters, by name, for any module ──────────────────────────────────────
// These take an XPtr typed to FIMSRcppInterfaceBase, so one of each serves
// every module family. The R wrapper already holds that pointer: it is the same
// one it puts in the registry for CreateTMBModel().

namespace {
/**
 * @brief Find a parameter or stop with a message naming the module and field.
 */
VariableVector &require_parameter(Rcpp::XPtr<SharedBase> xp,
                                  const std::string &name) {
  if (!xp || !(*xp)) {
    Rcpp::stop("Cannot look up parameter '" + name + "' on a null module.");
  }
  VariableVector *parameter = (*xp)->get_parameter(name);
  if (parameter == nullptr) {
    Rcpp::stop("This module has no parameter named '" + name + "'.");
  }
  return *parameter;
}
}  // namespace

/**
 * @brief Set one parameter vector by name, on any module that has one.
 *
 * @details Resizes the vector, assigns initial values, and sets estimation
 * statuses from a character vector (length 1 is recycled across all elements).
 * Naming a parameter the module does not have is an error rather than a silent
 * no-op.
 */
void set_parameter_(Rcpp::XPtr<SharedBase> xp, std::string name,
                    Rcpp::NumericVector values,
                    Rcpp::CharacterVector estimation_status) {
  set_variable_vector(require_parameter(xp, name), values, estimation_status);
}

/**
 * @brief Set one fixed numeric vector by name, on any module that has one.
 *
 * @details The counterpart to set_parameter_(), for vectors that hold plain
 * numbers rather than estimable parameters: observations, uncertainties, ages,
 * empirical weights. These carry no estimation status, so only values are
 * given. The vector is resized to match.
 *
 * @param xp The module to act on.
 * @param name The vector name as used in R.
 * @param values The values to write.
 */
void set_vector_(Rcpp::XPtr<SharedBase> xp, std::string name,
                 Rcpp::NumericVector values) {
  if (!xp || !(*xp)) {
    Rcpp::stop("Cannot set vector '" + name + "' on a null module.");
  }
  fims::Vector<double> *vector = (*xp)->get_vector(name);
  if (vector == nullptr) {
    Rcpp::stop("This module has no vector named '" + name + "'.");
  }
  set_real_vector(*vector, values);
}

/**
 * @brief Get a module's own unique ID.
 *
 * @details This is the ID other modules refer to it by: a fleet's selectivity
 * ID, a population's growth ID, the data IDs a distribution is linked to. Not
 * to be confused with get_parameter_id_(), which returns the ID of one
 * parameter vector inside a module rather than the module's own.
 *
 * @param xp The module to read.
 * @return The module's unique ID.
 */
uint32_t get_module_id_(Rcpp::XPtr<SharedBase> xp) {
  if (!xp || !(*xp)) {
    Rcpp::stop("Cannot read the id of a null module.");
  }
  return (*xp)->get_id();
}

/**
 * @brief Get the id of one parameter vector by name.
 *
 * @details This is the id that fims_info::Information::variable_map is keyed
 * by, so it is what set_distribution_links_() needs in order to attach a
 * distribution to this parameter.
 */
uint32_t get_parameter_id_(Rcpp::XPtr<SharedBase> xp, std::string name) {
  return require_parameter(xp, name).id_m;
}

// ── Invalidation for clear() ─────────────────────────────────────────────────
/**
 * @brief Invalidate a module's base pointer, releasing its share of the
 * module.
 *
 * @details The companion to the per-family `release_*_()` functions. Every
 * module is reachable through two pointers, one typed to its family and one
 * typed to this base class, and each owns a share of the module. The R
 * `clear()` wrapper releases both, which invalidates the module for any later
 * use and returns its memory immediately.
 *
 * A separate function per family is needed for the other pointer because
 * releasing deletes through the pointer's own type, and the family types are
 * unrelated to this one.
 *
 * @param xp The module to invalidate.
 */
void release_base_(Rcpp::XPtr<SharedBase> xp) { xp.release(); }

/**
 * Function to register the shared parameter accessors with the Rcpp module
 * system.
 */
void register_parameters(Rcpp::Module &m) {
  Rcpp::function("set_parameter_", &set_parameter_);
  Rcpp::function("set_vector_", &set_vector_);
  Rcpp::function("get_module_id_", &get_module_id_);
  Rcpp::function("get_parameter_id_", &get_parameter_id_);
  Rcpp::function("release_base_", &release_base_);
}
