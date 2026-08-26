/**
 * \file rcpp_models.cpp
 * \brief Implementation of Rcpp fishery model interfaces for the FIMS
 * framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_models.hpp"
// static id of the FisheryModelInterfaceBase object
uint32_t FisheryModelInterfaceBase::id_g = 1;

#include <Rcpp.h>

// SharedFisheryModel is the pointer type used throughout this file and allows
// functions to work on the correct child class based on the input string.
using SharedFisheryModel = std::shared_ptr<FisheryModelInterfaceBase>;
// SharedPopulation is the pointer type used throughout this file to work on 
// the linked populations associated with this fishery model.
using SharedPopulationModel = std::shared_ptr<PopulationInterface>;

// ── Building a module ──────────────────────────────────────────────────────
/**
 * @brief Create a fishery model of the requested type.
 *
 * @details Allocates the model interface object to heap memory, hands ownership 
 * to a shared_ptr, and wraps that shared_ptr in the XPtr. The 'true' argument 
 * binds the XPtr to R's garbage collector: when the R variable is garbage 
 * collected, 'delete shared_ptr*' runs and the reference count is decremented.
 *
 * @param type Currently only "catch_at_age".
 *
 * @return A pointer to the new model.
 */
Rcpp::XPtr<SharedFisheryModel> create_fishery_model_(std::string type) {
  SharedFisheryModel model_interface;
  switch (FisheryModelTypeFromString(type)) {
    case FisheryModelType::catch_at_age:
      model_interface = std::make_shared<CatchAtAgeInterface>();
      break;
  }
  return Rcpp::XPtr<SharedFisheryModel>(
      new SharedFisheryModel(model_interface), true);
}

// ── Population link ──────────────────────────────────────────────────────────
// Replaces rather than appends, so a model can be given a different set of
// populations without rebuilding it.
/**
 * @brief Set the populations this model operates on, replacing any already set.
 *
 * @param xp The fishery model.
 * @param populations A list of population XPtrs. A population listed twice is
 *   recorded once and a warning is issued.
 */
void set_model_populations_(Rcpp::XPtr<SharedFisheryModel> xp,
                            Rcpp::List populations) {
  std::vector<std::shared_ptr<PopulationInterface>> population_interfaces;
  population_interfaces.reserve(populations.size());
  for (int i = 0; i < populations.size(); i++) {
    Rcpp::XPtr<SharedPopulationModel> population =
        Rcpp::as<Rcpp::XPtr<SharedPopulationModel>>(populations[i]);
    population_interfaces.push_back(*population);
  }
  (*xp)->SetPopulations(population_interfaces);
}

// ── Reporting control ────────────────────────────────────────────────────────
/**
 * @brief Enable or disable reporting for the model.
 *
 * @param xp The fishery model.
 * @param report true to report derived quantities, false to skip them.
 */
void do_model_reporting_(Rcpp::XPtr<SharedFisheryModel> xp, bool report) {
  (*xp)->DoReporting(report);
}

/**
 * @brief Report whether reporting is enabled for the model.
 *
 * @param xp The fishery model.
 * @return true if reporting is on, false otherwise or if the model type does
 *   not support it.
 */
bool is_model_reporting_(Rcpp::XPtr<SharedFisheryModel> xp) {
  return (*xp)->IsReporting();
}

// ── Getters ──────────────────────────────────────────────────────────────────

/**
 * @brief Serialize the model to JSON.
 *
 * @details The model reaches its populations and their fleets through its own
 * links. Everything else it reports on -- growth, recruitment, maturity,
 * selectivity, the observed data modules, and the density components -- is
 * referenced only by integer id, so those modules are handed in.
 *
 * @param report_modules A list of base XPtrs, as produced by the
 * `*_to_fims_xptr_()` functions. This is the same list the R-side registry
 * builds for CreateTMBModel(); entries that are not reportable modules are
 * ignored.
 */
std::string get_model_output_(Rcpp::XPtr<SharedFisheryModel> xp,
                              Rcpp::List report_modules) {
  std::vector<SharedBase> modules;
  modules.reserve(report_modules.size());
  for (int i = 0; i < report_modules.size(); i++) {
    Rcpp::XPtr<SharedBase> module =
        Rcpp::as<Rcpp::XPtr<SharedBase>>(report_modules[i]);
    modules.push_back(*module);
  }
  return (*xp)->to_json(modules);
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
 * @param xp The fishery model.
 * @return A base-class pointer to the same module.
 */
Rcpp::XPtr<SharedBase> model_to_fims_xptr_(Rcpp::XPtr<SharedFisheryModel> xp) {
  // Every interface class inherits from FIMSRcppInterfaceBase, so this
  // conversion needs no cast: same object, more general pointer type.
  SharedBase base = *xp;
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

/**
 * Function to register fishery model classes with the Rcpp module system.
 *
 */
/**
 * @brief Register the fishery model functions with the Rcpp module system.
 *
 * @param m The Rcpp module to register into.
 */
void register_fishery_models(Rcpp::Module& m) {
  // Rcpp::class_<> registrations removed — users no longer call methods::new().
  Rcpp::function("create_fishery_model_", &create_fishery_model_);
  Rcpp::function("set_model_populations_", &set_model_populations_);
  Rcpp::function("get_model_output_", &get_model_output_);
  Rcpp::function("do_model_reporting_", &do_model_reporting_);
  Rcpp::function("is_model_reporting_", &is_model_reporting_);
  Rcpp::function("model_to_fims_xptr_", &model_to_fims_xptr_);
}
