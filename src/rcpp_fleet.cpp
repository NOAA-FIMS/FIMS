/**
 * \file rcpp_fleet.cpp
 * \brief Implementation of Rcpp fleet interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_fleet.hpp"

#include <Rcpp.h>
// static id of the FleetInterfaceBase object
uint32_t FleetInterfaceBase::id_g = 1;
namespace {
// Adds this counter to the list clear() rewinds.
IdCounterRegistration fleet_id_g_registration(
    &FleetInterfaceBase::id_g);
}  // namespace

// ── Short name for the pointer type ────────────────────────────────────────
using SharedFleet = std::shared_ptr<FleetInterface>;

// ── Building a module ──────────────────────────────────────────────────────
/**
 * @brief Create a fleet module.
 *
 * @details Allocates the fleet interface object to heap memory, hands ownership
 * to a shared_ptr, and wraps that shared_ptr in the XPtr. The 'true' argument 
 * binds the XPtr to R's garbage collector: when the R variable is garbage 
 * collected, 'delete shared_ptr*' runs and the reference count is decremented.
 *
 * @return A pointer to the new fleet.
 */
Rcpp::XPtr<SharedFleet> create_fleet_() {
  auto fleet_interface = std::make_shared<FleetInterface>();
  return Rcpp::XPtr<SharedFleet>(new SharedFleet(fleet_interface), true);
}

// ── Scalar setters ───────────────────────────────────────────────────────────
/**
 * @brief Set the dimensions of the fleet's data.
 *
 * @param xp The fleet module.
 * @param n_years Number of years.
 * @param n_ages Number of age bins.
 * @param n_lengths Number of length bins, or 0 if the fleet has no length data.
 */
void set_fleet_constants_(Rcpp::XPtr<SharedFleet> xp, int n_years, int n_ages,
                          int n_lengths) {
  (*xp)->n_years = n_years;
  (*xp)->n_ages = n_ages;
  (*xp)->n_lengths = n_lengths;
}

/**
 * @brief Set the fleet's name, used to label it in the output.
 *
 * @param xp The fleet module.
 * @param name The name to record.
 */
void set_fleet_name_(Rcpp::XPtr<SharedFleet> xp, std::string name) {
  (*xp)->SetName(name);
}

/**
 * @brief Set the units the fleet's observations are measured in.
 *
 * @param xp The fleet module.
 * @param observed_catch_units Either "weight" or "numbers".
 * @param observed_index_units Either "weight" or "numbers".
 */
void set_fleet_units_(Rcpp::XPtr<SharedFleet> xp,
                      std::string observed_catch_units,
                      std::string observed_index_units) {
  (*xp)->observed_catch_units = observed_catch_units;
  (*xp)->observed_index_units = observed_index_units;
}

// ── Module links ─────────────────────────────────────────────────────────────
// Selectivity and the observed data modules are referenced by ID only. The IDs
// are what fims_popdy::Fleet consumes, and the reporting path is handed those
// modules directly rather than reaching them through the fleet.
/**
 * @brief Link the fleet to a selectivity module.
 *
 * @param xp The fleet module.
 * @param selectivity_id The selectivity module's ID, as returned by
 *   get_module_id_().
 */
void set_fleet_selectivity_id_(Rcpp::XPtr<SharedFleet> xp, int selectivity_id) {
  (*xp)->SetSelectivityID(selectivity_id);
}

/**
 * @brief Link the fleet to the data modules holding its observations.
 *
 * @details IDs come from get_module_id_() on each data module. All four are
 *   set together so a fleet's
 *   data links are described in one call.
 *
 * @param xp The fleet module.
 * @param agecomp_data_id Age-composition data ID, or -999 if the fleet has
 *   none.
 * @param lengthcomp_data_id Length-composition data ID, or -999.
 * @param index_data_id Index data ID, or -999.
 * @param catch_data_id Catch data ID, or -999.
 */
void set_fleet_observed_data_ids_(Rcpp::XPtr<SharedFleet> xp,
                                  int agecomp_data_id = -999,
                                  int lengthcomp_data_id = -999,
                                  int index_data_id = -999,
                                  int catch_data_id = -999) {
  (*xp)->SetObservedAgeCompDataID(agecomp_data_id);
  (*xp)->SetObservedLengthCompDataID(lengthcomp_data_id);
  (*xp)->SetObservedIndexDataID(index_data_id);
  (*xp)->SetObservedCatchDataID(catch_data_id);
}

// ── VariableVector setters ───────────────────────────────────────────────────

// ── Getters ──────────────────────────────────────────────────────────────────

/**
 * @brief Get the fleet's name.
 *
 * @param xp The fleet module.
 * @return The name.
 */
std::string get_fleet_name_(Rcpp::XPtr<SharedFleet> xp) {
  return (*xp)->GetName();
}

/**
 * @brief Get the ID of the fleet's selectivity module.
 *
 * @param xp The fleet module.
 * @return The ID, or -999 if none is linked.
 */
int get_fleet_selectivity_id_(Rcpp::XPtr<SharedFleet> xp) {
  return (*xp)->GetSelectivityID();
}

/**
 * @brief Get the IDs of the data modules holding this fleet's observations.
 *
 * @details The mirror of set_fleet_observed_data_ids_(): all four are returned
 *   together, named, so a fleet's data links can be read in one call. The names
 *   match the "data_ids" entries in the JSON output.
 *
 * @param xp The fleet module.
 * @return A named integer vector with elements "agecomp", "lengthcomp",
 *   "index", and "catch". An entry is -999 when no data module of that kind is
 *   linked.
 */
Rcpp::IntegerVector get_fleet_observed_data_ids_(Rcpp::XPtr<SharedFleet> xp) {
  return Rcpp::IntegerVector::create(
      Rcpp::Named("agecomp") = (*xp)->GetObservedAgeCompDataID(),
      Rcpp::Named("lengthcomp") = (*xp)->GetObservedLengthCompDataID(),
      Rcpp::Named("index") = (*xp)->GetObservedIndexDataID(),
      Rcpp::Named("catch") = (*xp)->GetObservedCatchDataID());
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
 * @param xp The fleet module.
 * @return A base-class pointer to the same module.
 */
Rcpp::XPtr<SharedBase> fleet_to_fims_xptr_(Rcpp::XPtr<SharedFleet> xp) {
  // Every interface class inherits from FIMSRcppInterfaceBase, so this
  // conversion needs no cast: same object, more general pointer type.
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

// ── Invalidation for clear() ─────────────────────────────────────────────────
/**
 * @brief Invalidate a pointer to a fleet module, releasing this pointer's
 * share of the module.
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
void release_fleet_(Rcpp::XPtr<SharedFleet> xp) { xp.release(); }

/**
 * Function to register fleet classes with the Rcpp module system.
 *
 */
/**
 * @brief Register the fleet functions with the Rcpp module system.
 *
 * @param m The Rcpp module to register into.
 */
void register_fleet(Rcpp::Module& m) {
  Rcpp::function("create_fleet_", &create_fleet_);
  Rcpp::function("set_fleet_constants_", &set_fleet_constants_);
  Rcpp::function("set_fleet_name_", &set_fleet_name_);
  Rcpp::function("set_fleet_units_", &set_fleet_units_);
  Rcpp::function("set_fleet_selectivity_id_", &set_fleet_selectivity_id_);
  Rcpp::function("set_fleet_observed_data_ids_",
                 &set_fleet_observed_data_ids_);
  Rcpp::function("get_fleet_name_", &get_fleet_name_);
  Rcpp::function("get_fleet_selectivity_id_", &get_fleet_selectivity_id_);
  Rcpp::function("get_fleet_observed_data_ids_",
                 &get_fleet_observed_data_ids_);
  Rcpp::function("fleet_to_fims_xptr_", &fleet_to_fims_xptr_);
  Rcpp::function("release_fleet_", &release_fleet_);
}
