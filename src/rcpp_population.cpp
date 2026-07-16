/**
 * \file rcpp_population.cpp
 * \brief Implementation of Rcpp population interfaces for the FIMS framework.
 */
#include "../inst/include/interface/rcpp/rcpp_objects/rcpp_population.hpp"

// static id of the PopulationInterfaceBase object
uint32_t PopulationInterfaceBase::id_g = 1;
// local id of the PopulationInterfaceBase object map relating the ID of the
// PopulationInterfaceBase to the PopulationInterfaceBase objects
std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>
    PopulationInterfaceBase::live_objects;

#include <Rcpp.h>

// ── XPtr type alias ──────────────────────────────────────────────────────────
using SharedPopulation = std::shared_ptr<PopulationInterface>;

// ── Creator ──────────────────────────────────────────────────────────────────
Rcpp::XPtr<SharedPopulation> create_population_() {
  auto obj = std::make_shared<PopulationInterface>();
  return Rcpp::XPtr<SharedPopulation>(new SharedPopulation(obj), true);
}

// ── Scalar setters ───────────────────────────────────────────────────────────
void set_population_constants_(Rcpp::XPtr<SharedPopulation> xp,
                               int n_years, int n_ages, int n_lengths) {
  (*xp)->n_years   = n_years;
  (*xp)->n_ages    = n_ages;
  (*xp)->n_lengths = n_lengths;
}
void set_population_process_ids_(Rcpp::XPtr<SharedPopulation> xp,
                                 int maturity_id        = -999,
                                 int growth_id          = -999,
                                 int recruitment_id     = -999,
                                 int recruitment_err_id = -999) {
  (*xp)->maturity_id        = maturity_id;
  (*xp)->growth_id          = growth_id;
  (*xp)->recruitment_id     = recruitment_id;
  (*xp)->recruitment_err_id = recruitment_err_id;
}
void set_population_name_(Rcpp::XPtr<SharedPopulation> xp,
                          std::string name) {
  (*xp)->name = name;
}
void add_fleet_to_population_(Rcpp::XPtr<SharedPopulation> xp,
                              uint32_t fleet_id) {
  (*xp)->fleet_ids->insert(fleet_id);
}

// ── VariableVector setters ───────────────────────────────────────────────────
// Each setter resizes the vector, assigns initial values, and sets estimation
// types from a character vector (length 1 is recycled across all elements).
void set_population_log_M_(Rcpp::XPtr<SharedPopulation> xp,
                           Rcpp::NumericVector values,
                           Rcpp::CharacterVector estimation_types) {
  PopulationInterface& pop = **xp;
  pop.log_M.resize(values.size());
  for (int i = 0; i < values.size(); i++) {
    Variable& v = pop.log_M.storage_m->at(i);
    v.initial_value_m = values[i];
    v.estimation_type_m.set(
        Rcpp::as<std::string>(estimation_types[estimation_types.size() == 1 ? 0 : i]));
  }
}

void set_population_log_init_naa_(Rcpp::XPtr<SharedPopulation> xp,
                                  Rcpp::NumericVector values,
                                  Rcpp::CharacterVector estimation_types) {
  PopulationInterface& pop = **xp;
  pop.log_init_naa.resize(values.size());
  for (int i = 0; i < values.size(); i++) {
    Variable& v = pop.log_init_naa.storage_m->at(i);
    v.initial_value_m = values[i];
    v.estimation_type_m.set(
        Rcpp::as<std::string>(estimation_types[estimation_types.size() == 1 ? 0 : i]));
  }
}

void set_population_log_f_multiplier_(Rcpp::XPtr<SharedPopulation> xp,
                                      Rcpp::NumericVector values,
                                      Rcpp::CharacterVector estimation_types) {
  PopulationInterface& pop = **xp;
  pop.log_f_multiplier.resize(values.size());
  for (int i = 0; i < values.size(); i++) {
    Variable& v = pop.log_f_multiplier.storage_m->at(i);
    v.initial_value_m = values[i];
    v.estimation_type_m.set(
        Rcpp::as<std::string>(estimation_types[estimation_types.size() == 1 ? 0 : i]));
  }
}

void set_population_proportion_female_(Rcpp::XPtr<SharedPopulation> xp,
                                       Rcpp::NumericVector values,
                                       Rcpp::CharacterVector estimation_types) {
  PopulationInterface& pop = **xp;
  pop.proportion_female.resize(values.size());
  for (int i = 0; i < values.size(); i++) {
    Variable& v = pop.proportion_female.storage_m->at(i);
    v.initial_value_m = values[i];
    v.estimation_type_m.set(
        Rcpp::as<std::string>(estimation_types[estimation_types.size() == 1 ? 0 : i]));
  }
}

void set_population_ages_(Rcpp::XPtr<SharedPopulation> xp,
                          Rcpp::NumericVector values) {
  (*xp)->ages.resize(values.size());
  for (int i = 0; i < values.size(); i++) {
    (*xp)->ages[i] = values[i];
  }
}

// ── ID getter ────────────────────────────────────────────────────────────────
uint32_t get_population_id_(Rcpp::XPtr<SharedPopulation> xp) {
  return (*xp)->get_id();
}

// ── Base-class conversion for CreateTMBModel() ───────────────────────────────
using SharedBase = std::shared_ptr<FIMSRcppInterfaceBase>;

Rcpp::XPtr<SharedBase> population_to_fims_xptr_(
    Rcpp::XPtr<SharedPopulation> xp) {
  SharedBase base = *xp;  // implicit upcast: shared_ptr<Derived> -> shared_ptr<Base>
  return Rcpp::XPtr<SharedBase>(new SharedBase(base), true);
}

// ── Module registration ───────────────────────────────────────────────────────
void register_population(Rcpp::Module& m) {
  // XPtr creator, setters, and conversion — trailing underscores prevent name
  // collision with the R wrapper functions that own the clean names (Section 4b
  // of the design doc).
  Rcpp::function("create_population_",              &create_population_);
  Rcpp::function("set_population_constants_",        &set_population_constants_);
  Rcpp::function("set_population_process_ids_",      &set_population_process_ids_);
  Rcpp::function("set_population_name_",            &set_population_name_);
  Rcpp::function("add_fleet_to_population_",        &add_fleet_to_population_);
  Rcpp::function("set_population_log_M_",           &set_population_log_M_);
  Rcpp::function("set_population_log_init_naa_",    &set_population_log_init_naa_);
  Rcpp::function("set_population_log_f_multiplier_",&set_population_log_f_multiplier_);
  Rcpp::function("set_population_proportion_female_",
                                                    &set_population_proportion_female_);
  Rcpp::function("set_population_ages_",            &set_population_ages_);
  Rcpp::function("get_population_id_",              &get_population_id_);
  Rcpp::function("population_to_fims_xptr_",        &population_to_fims_xptr_);
}