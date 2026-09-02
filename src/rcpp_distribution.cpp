/**
 * \file rcpp_distribution.cpp
 * \brief Implementation of Rcpp distribution interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_distribution.hpp"

// static id of the DistributionsInterfaceBase object
uint32_t DistributionsInterfaceBase::id_g = 1;
namespace {
// Adds this counter to the list clear() rewinds.
IdCounterRegistration distribution_id_g_registration(
    &DistributionsInterfaceBase::id_g);
}  // namespace

#include <Rcpp.h>

// SharedDistribution is the pointer type used throughout this file and allows
// functions to work on the correct child class based on the input string.
// Parameters differ between distributions, so those are asked for by name.
using SharedDistribution = std::shared_ptr<DistributionsInterfaceBase>;

// ── Building a module ──────────────────────────────────────────────────────
/**
 * @brief Create a distribution module of the requested type.
 *
 * @details Allocates the distribution interface object to heap memory, hands
 * ownership to a shared_ptr, and wraps that shared_ptr in the XPtr. The 'true'
 * argument binds the XPtr to R's garbage collector: when the R variable is
 * garbage collected, 'delete shared_ptr*' runs and the reference count is
 * decremented.
 *
 * @param type One of "Dnorm", "Dlnorm", or "Dmultinom".
 *
 * @return A pointer to the new distribution.
 */
Rcpp::XPtr<SharedDistribution> create_distribution_(std::string type) {
  SharedDistribution distribution_interface;
  switch (DistributionTypeFromString(type)) {
    case DistributionType::kDnorm:
      distribution_interface = std::make_shared<DnormDistributionsInterface>();
      break;
    case DistributionType::kDlnorm:
      distribution_interface = std::make_shared<DlnormDistributionsInterface>();
      break;
    case DistributionType::kDmultinom:
      distribution_interface =
          std::make_shared<DmultinomDistributionsInterface>();
      break;
  }
  return Rcpp::XPtr<SharedDistribution>(
      new SharedDistribution(distribution_interface), true);
}

// ── Link setters ─────────────────────────────────────────────────────────────
// A distribution reaches the quantities it acts on by ID, through
// fims_info::Information::variable_map, rather than by holding a pointer to
// them. The three setters below therefore pass IDs rather than XPtrs:
// set_distribution_links_(), set_distribution_observed_data_(), and
// set_distribution_fixed_mean_().
//
// The methods they call -- set_distribution_links(), set_observed_data(), and
// set_distribution_mean() -- are all declared on DistributionsInterfaceBase,
// so none of the three needs to know which distribution it was handed.

/**
 * @brief Link a distribution to the parameters, derived values, or observed
 * data it acts on.
 *
 * @param xp The distribution module.
 * @param input_type Whether the distribution applies to a prior, a random
 *   effect, or data. Determines which expected values the distribution reads
 *   during evaluation.
 * @param ids The variable_map IDs of the quantities being linked, as returned
 *   by get_variable_vector_id_().
 * @return true if the distribution accepted the links, false otherwise.
 */
bool set_distribution_links_(Rcpp::XPtr<SharedDistribution> xp,
                             std::string input_type, Rcpp::IntegerVector ids) {
  return (*xp)->set_distribution_links(input_type, ids);
}

/**
 * @brief Set the observed data module this distribution is evaluated against.
 *
 * @param xp The distribution module.
 * @param observed_data_id The ID of the data module, as returned by
 *   get_module_id_().
 * @return true if the distribution accepted the ID, false otherwise.
 */
bool set_distribution_observed_data_(Rcpp::XPtr<SharedDistribution> xp,
                                     int observed_data_id) {
  return (*xp)->set_observed_data(observed_data_id);
}

/**
 * @brief Set a fixed expected mean, rather than taking the expected value from
 * a linked model quantity.
 *
 * @details Only some distributions support this. The base-class behavior is to
 * decline, so a distribution that has no fixed mean returns false rather than
 * raising an error.
 *
 * @param xp The distribution module.
 * @param input_value The value to use as the expected mean.
 * @return true if the distribution accepted the mean, false if it does not
 *   support one.
 */
bool set_distribution_fixed_mean_(Rcpp::XPtr<SharedDistribution> xp,
                                  double input_value) {
  return (*xp)->set_distribution_mean(input_value);
}

// ── Multinomial-only settings ────────────────────────────────────────────────
// `dims` is a plain numeric vector rather than a parameter vector, and
// `notes` is a
// string, so neither goes through get_variable_vector().

/**
 * @brief Attach a free-text note, carried through to the JSON output.
 *
 * @param xp The distribution module. Must be a multinomial; anything else is an
 *   error.
 * @param note The text to record.
 */
void set_distribution_note_(Rcpp::XPtr<SharedDistribution> xp,
                            std::string note) {
  std::shared_ptr<DmultinomDistributionsInterface> dmultinom =
      std::dynamic_pointer_cast<DmultinomDistributionsInterface>(*xp);
  if (!dmultinom) {
    Rcpp::stop("`note` applies only to the multinomial distribution.");
  }
  dmultinom->set_note(note);
}

/**
 * @brief Evaluate the distribution at its current values.
 *
 * @details Evaluates the interface object directly rather than through the
 *   assembled model, so it can be called before CreateTMBModel().
 *
 * @param xp The distribution module.
 * @return The log probability density or mass.
 */
double evaluate_distribution_(Rcpp::XPtr<SharedDistribution> xp) {
  return (*xp)->evaluate();
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
 * @param xp The distribution module.
 * @return A base-class pointer to the same module.
 */
Rcpp::XPtr<SharedBase> distribution_to_fims_xptr_(
    Rcpp::XPtr<SharedDistribution> xp) {
  // Every interface class inherits from FIMSRcppInterfaceBase, so this
  // conversion needs no cast: same object, more general pointer type.
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

// ── Invalidation for clear() ─────────────────────────────────────────────────
/**
 * @brief Invalidate a pointer to a distribution module, releasing this
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
void release_distribution_(Rcpp::XPtr<SharedDistribution> xp) { xp.release(); }

/**
 * Function to register distribution classes with the Rcpp module system.
 *
 */
/**
 * @brief Register the distribution functions with the Rcpp module system.
 *
 * @param m The Rcpp module to register into.
 */
void register_distributions(Rcpp::Module& m) {
  Rcpp::function("create_distribution_", &create_distribution_);
  Rcpp::function("set_distribution_links_", &set_distribution_links_);
  Rcpp::function("set_distribution_observed_data_",
                 &set_distribution_observed_data_);
  Rcpp::function("set_distribution_fixed_mean_", &set_distribution_fixed_mean_);
  Rcpp::function("set_distribution_note_", &set_distribution_note_);
  Rcpp::function("evaluate_distribution_", &evaluate_distribution_);
  Rcpp::function("distribution_to_fims_xptr_", &distribution_to_fims_xptr_);
  Rcpp::function("release_distribution_", &release_distribution_);
}
