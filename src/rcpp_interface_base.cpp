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
 * @brief Find a VariableVector or stop with a message naming the module and
 * field.
 */
VariableVector &require_variable_vector(Rcpp::XPtr<SharedBase> xp,
                                  const std::string &name) {
  if (!xp || !(*xp)) {
    Rcpp::stop("Cannot look up '" + name + "' on a null module.");
  }
  VariableVector *variable_vector = (*xp)->get_variable_vector(name);
  if (variable_vector == nullptr) {
    Rcpp::stop("This module has no field named '" + name + "'.");
  }
  return *variable_vector;
}
}  // namespace

/**
 * @brief Set one VariableVector field by name, on any module that has one.
 *
 * @details Resizes the vector, assigns initial values, and sets estimation
 * statuses from a character vector (length 1 is recycled across all elements).
 * Naming a field the module does not have is an error rather than a silent
 * no-op.
 *
 * The estimation status is given per call rather than being a fixed property
 * of the field, which is what lets the same quantity be estimated in one model
 * and assumed known or derived in another.
 *
 * @param xp The module to act on.
 * @param name The field name as used in R.
 * @param values The values to write. The vector is resized to match.
 * @param estimation_status One status name, or one per element.
 */
void set_variable_vector_(Rcpp::XPtr<SharedBase> xp, std::string name,
                    Rcpp::NumericVector values,
                    Rcpp::CharacterVector estimation_status) {
  fill_variable_vector(require_variable_vector(xp, name), values,
                       estimation_status);
}

/**
 * @brief Set one plain numeric vector by name, on any module that has one.
 *
 * @details The counterpart to set_variable_vector_(), for fields declared as
 * fims::Vector<double> rather than VariableVector: observations,
 * uncertainties, ages, empirical weights. These carry no estimation status, so
 * only values are given, and nothing can be estimated from them.
 *
 * @param xp The module to act on.
 * @param name The vector name as used in R.
 * @param values The values to write.
 */
void set_numeric_vector_(Rcpp::XPtr<SharedBase> xp, std::string name,
                 Rcpp::NumericVector values) {
  if (!xp || !(*xp)) {
    Rcpp::stop("Cannot set vector '" + name + "' on a null module.");
  }
  fims::Vector<double> *vector = (*xp)->get_numeric_vector(name);
  if (vector == nullptr) {
    Rcpp::stop("This module has no vector named '" + name + "'.");
  }
  fill_numeric_vector(*vector, values);
}

/**
 * @brief Get a module's own unique ID.
 *
 * @details This is the ID other modules refer to it by: a fleet's selectivity
 * ID, a population's growth ID, the data IDs a distribution is linked to. Not
 * to be confused with get_variable_vector_id_(), which returns the ID of one
 * VariableVector inside a module rather than the module's own.
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
 * @brief Get the id of one VariableVector field by name.
 *
 * @details This is the id that fims_info::Information::variable_map is keyed
 * by, so it is what set_distribution_links_() needs in order to attach a
 * distribution to this field. It works for any VariableVector the module
 * exposes, whether the model reads it as an input or computes it -- a
 * distribution on observed catch names a computed quantity, and a prior on a
 * parameter names an input.
 *
 * @param xp The module to read from.
 * @param name The field name as used in R.
 * @return The id.
 */
uint32_t get_variable_vector_id_(Rcpp::XPtr<SharedBase> xp, std::string name) {
  return require_variable_vector(xp, name).id_m;
}

/**
 * @brief Read one VariableVector field by name, on any module that has one.
 *
 * @details The counterpart to set_variable_vector_(): it returns what that
 * function wrote, so a value can be checked after it is set. Values and
 * statuses come back together because they are set together.
 *
 * The values are the ones given as input. After a model is fitted the
 * estimates live in the model rather than in the module, so this reports
 * starting values, not estimates.
 *
 * @param xp The module to read from.
 * @param name The field name as used in R.
 * @return A list with `values`, the numeric input values, and
 * `estimation_status`, one status name per element.
 */
/**
 * @brief Report whether a module has a VariableVector by this name.
 *
 * @details Lets R decide what to set without a hardcoded list of each module's
 * fields. The parameters tibble carries rows for a module and for the
 * distribution attached to it, and only the module's own rows can be set on
 * it, so the caller needs to be able to ask.
 *
 * @param xp The module to ask.
 * @param name The field name as used in R.
 * @return true if the module has a field by that name.
 */
bool has_variable_vector_(Rcpp::XPtr<SharedBase> xp, std::string name) {
  if (!xp || !(*xp)) {
    Rcpp::stop("Cannot look up '" + name + "' on a null module.");
  }
  return (*xp)->get_variable_vector(name) != nullptr;
}

Rcpp::List get_variable_vector_(Rcpp::XPtr<SharedBase> xp, std::string name) {
  VariableVector &variable_vector = require_variable_vector(xp, name);
  const size_t size = variable_vector.size();

  Rcpp::NumericVector values(size);
  Rcpp::CharacterVector estimation_status(size);
  for (size_t i = 0; i < size; i++) {
    Variable &v = variable_vector.storage_m->at(i);
    values[i] = v.initial_value_m;
    estimation_status[i] = EstimationStatusToString(v.estimation_status_m);
  }

  return Rcpp::List::create(Rcpp::Named("values") = values,
                            Rcpp::Named("estimation_status") =
                                estimation_status);
}

/**
 * @brief Read one plain numeric vector by name, on any module that has one.
 *
 * @details The counterpart to set_numeric_vector_(). These fields carry no
 * estimation status, so only the values are returned.
 *
 * @param xp The module to read from.
 * @param name The field name as used in R.
 * @return The values.
 */
Rcpp::NumericVector get_numeric_vector_(Rcpp::XPtr<SharedBase> xp,
                                        std::string name) {
  if (!xp || !(*xp)) {
    Rcpp::stop("Cannot read '" + name + "' from a null module.");
  }
  fims::Vector<double> *vector = (*xp)->get_numeric_vector(name);
  if (vector == nullptr) {
    Rcpp::stop("This module has no numeric vector named '" + name + "'.");
  }
  Rcpp::NumericVector values(vector->size());
  for (size_t i = 0; i < vector->size(); i++) {
    values[i] = (*vector)[i];
  }
  return values;
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
 * Function to register the shared field accessors with the Rcpp module
 * system.
 */
void register_parameters(Rcpp::Module &m) {
  Rcpp::function("set_variable_vector_", &set_variable_vector_);
  Rcpp::function("set_numeric_vector_", &set_numeric_vector_);
  Rcpp::function("get_module_id_", &get_module_id_);
  Rcpp::function("get_variable_vector_id_", &get_variable_vector_id_);
  Rcpp::function("get_variable_vector_", &get_variable_vector_);
  Rcpp::function("has_variable_vector_", &has_variable_vector_);
  Rcpp::function("get_numeric_vector_", &get_numeric_vector_);
  Rcpp::function("release_base_", &release_base_);
}
