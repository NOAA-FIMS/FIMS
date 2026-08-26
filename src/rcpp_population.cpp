/**
 * \file rcpp_population.cpp
 * \brief Implementation of Rcpp population interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp"

// static id of the PopulationInterfaceBase object
uint32_t PopulationInterfaceBase::id_g = 1;

#include <Rcpp.h>

// SharedPopulation is the pointer type used throughout this file to reference 
// the population.
using SharedPopulation = std::shared_ptr<PopulationInterface>;
// SharedFleetPopulation is the pointer type used throughout this file to work on 
// the linked fleets associated with this population.
using SharedFleetPopulation = std::shared_ptr<FleetInterface>;

// ── Building a module ──────────────────────────────────────────────────────
/**
 * @brief Create a population module.
 *
 * @details Allocates the population interface object to heap memory, hands ownership 
 * to a shared_ptr, and wraps that shared_ptr in the XPtr. The 'true' argument 
 * binds the XPtr to R's garbage collector: when the R variable is garbage 
 * collected, 'delete shared_ptr*' runs and the reference count is decremented.
 *
 * @return A pointer to the new population.
 */
Rcpp::XPtr<SharedPopulation> create_population_() {
  auto population_interface = std::make_shared<PopulationInterface>();
  return Rcpp::XPtr<SharedPopulation>(
      new SharedPopulation(population_interface), true);
}

// ── Scalar setters ───────────────────────────────────────────────────────────
/**
 * @brief Set the dimensions of the population.
 *
 * @param xp The population module.
 * @param n_years Number of years.
 * @param n_ages Number of age bins.
 * @param n_lengths Number of length bins, or 0 if the model is age-based.
 */
void set_population_constants_(Rcpp::XPtr<SharedPopulation> xp,
                               int n_years, int n_ages, int n_lengths) {
  (*xp)->n_years   = n_years;
  (*xp)->n_ages    = n_ages;
  (*xp)->n_lengths = n_lengths;
}
/**
 * @brief Set the population's name, used to label it in the output.
 *
 * @param xp The population module.
 * @param name The name to record.
 */
void set_population_name_(Rcpp::XPtr<SharedPopulation> xp,
                          std::string name) {
  (*xp)->name = name;
}

// ── Module links ─────────────────────────────────────────────────────────────
// Growth, maturity, and recruitment are referenced by ID only: the population
// passes the ID to fims_popdy::Population, and the reporting path is handed
// those modules directly rather than reaching them through the population.
/**
 * @brief Link the population to its growth, maturity, and recruitment modules.
 *
 * @param xp The population module.
 * @param maturity_id The maturity module's ID, or -999 for none.
 * @param growth_id The growth module's ID, or -999 for none.
 * @param recruitment_id The stock--recruit module's ID, or -999 for none.
 * @param recruitment_err_id The recruitment process module's ID, or -999.
 */
void set_population_process_ids_(Rcpp::XPtr<SharedPopulation> xp,
                                 int maturity_id = -999,
                                 int growth_id = -999,
                                 int recruitment_id = -999,
                                 int recruitment_err_id = -999) {
  (*xp)->maturity_id        = maturity_id;
  (*xp)->growth_id          = growth_id;
  (*xp)->recruitment_id     = recruitment_id;
  (*xp)->recruitment_err_id = recruitment_err_id;
}

// Fleets are the exception: the model walks the populations to reach every
// fleet it needs to size derived quantities for, so the population holds
// direct links as well as the IDs. SetFleets() rebuilds both together, and
// replaces rather than appends so a population can be given a different set.
/**
 * @brief Set the fleets operating on this population, replacing any already
 * set.
 *
 * @param xp The population module.
 * @param fleets A list of fleet XPtrs. A fleet listed twice is recorded once
 *   and a warning is issued.
 */
void set_population_fleets_(Rcpp::XPtr<SharedPopulation> xp,
                            Rcpp::List fleets) {
  std::vector<std::shared_ptr<FleetInterface>> fleet_interfaces;
  fleet_interfaces.reserve(fleets.size());
  for (int i = 0; i < fleets.size(); i++) {
    Rcpp::XPtr<SharedFleetPopulation> fleet =
        Rcpp::as<Rcpp::XPtr<SharedFleetPopulation>>(fleets[i]);
    fleet_interfaces.push_back(*fleet);
  }
  (*xp)->SetFleets(fleet_interfaces);
}

// ── Getters ──────────────────────────────────────────────────────────────────

/**
 * @brief Get the population's name.
 *
 * @param xp The population module.
 * @return The name.
 */
std::string get_population_name_(Rcpp::XPtr<SharedPopulation> xp) {
  return (*xp)->GetName();
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
 * @param xp The population module.
 * @return A base-class pointer to the same module.
 */
Rcpp::XPtr<SharedBase> population_to_fims_xptr_(
    Rcpp::XPtr<SharedPopulation> xp) {
  // Every interface class inherits from FIMSRcppInterfaceBase, so this
  // conversion needs no cast: same object, more general pointer type.
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

// ── Module registration ──────────────────────────────────────────────────────
/**
 * @brief Register the population functions with the Rcpp module system.
 *
 * @param m The Rcpp module to register into.
 */
void register_population(Rcpp::Module& m) {
  Rcpp::function("create_population_",               &create_population_);
  Rcpp::function("set_population_constants_",   &set_population_constants_);
  Rcpp::function("set_population_name_",             &set_population_name_);
  Rcpp::function("set_population_process_ids_", &set_population_process_ids_);
  Rcpp::function("set_population_fleets_",           &set_population_fleets_);
  Rcpp::function("get_population_name_",             &get_population_name_);
  Rcpp::function("population_to_fims_xptr_",         &population_to_fims_xptr_);
}
