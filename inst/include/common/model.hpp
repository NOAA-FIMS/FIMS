/*
 * File:   model.hpp
 *
 * Author: Matthew Supernaw, Andrea Havron
 * National Oceanic and Atmospheric Administration
 * National Marine Fisheries Service
 * Email: matthew.supernaw@noaa.gov, andrea.havron@noaa.gov
 *
 * Created on September 30, 2021, 1:08 PM
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */
#ifndef FIMS_COMMON_MODEL_HPP
#define FIMS_COMMON_MODEL_HPP

#include <future>
#include <memory>

#include "information.hpp"

namespace fims_model {

/**
 * @brief Model class. FIMS objective function.
 */
template <typename Type>
class Model {  // may need singleton
 public:
  static std::shared_ptr<Model<Type> >
      fims_model; /**< Create a shared fims_model as a pointer to Model*/
  std::shared_ptr<fims_info::Information<Type> >
      fims_information; /**< Create a shared fims_information as a pointer to
                           Information*/

#ifdef TMB_MODEL
  ::objective_function<Type> *of;
#endif

  // constructor
  virtual ~Model() {}

  /**
   * Returns a single Information object for type Type.
   *
   * @return singleton for type Type
   */
  static std::shared_ptr<Model<Type> > GetInstance() {
    if (Model<Type>::fims_model == nullptr) {
      Model<Type>::fims_model = std::make_shared<fims_model::Model<Type> >();
      Model<Type>::fims_model->fims_information =
          fims_info::Information<Type>::GetInstance();
    }
    return Model<Type>::fims_model;
  }

  /**
   * @brief Evaluate. Calculates the joint negative log-likelihood function.
   */
  const Type Evaluate() {
    // jnll = negative-log-likelihood (the objective function)
    Type jnll = 0.0;
    Type rec_nll = 0.0;       // recrutiment nll
    Type age_comp_nll = 0.0;  // age composition nll
    Type index_nll = 0.0;     // survey and fishery cacth nll

    int n_fleets = fims_information->fleets.size();
    int n_pops = fims_information->populations.size();

// Create vector lists to store output for reporting
#ifdef TMB_MODEL
    // vector< vector<Type> > creates a nested vector structure where
    // each vector can be a different dimension. Does not work with ADREPORT
    // fleets
    vector<vector<Type> > exp_index(n_fleets);
    vector<vector<Type> > exp_catch(n_fleets);
    vector<vector<Type> > cnaa(n_fleets);
    vector<vector<Type> > cwaa(n_fleets);
    vector<vector<Type> > F_mort(n_fleets);
    // populations
    vector<vector<Type> > naa(n_pops);
    vector<vector<Type> > ssb(n_pops);
    vector<vector<Type> > biomass(n_pops);
    vector<vector<Type> > log_recruit_dev(n_pops);
    vector<vector<Type> > recruitment(n_pops);
    vector<vector<Type> > M(n_pops);

#endif

    // Loop over populations, evaluate, and sum up the recruitment likelihood
    // component

    typename fims_info::Information<Type>::population_iterator it;
    MODEL_LOG << "Evaluating expected values and summing recruitment nlls for "
              << this->fims_information->populations.size() << " populations."
              << std::endl;

    for (it = this->fims_information->populations.begin();
         it != this->fims_information->populations.end(); ++it) {
      //(*it).second points to the Population module
      MODEL_LOG << "Setting up pointer to population " << (*it).second->id
                << "." << std::endl;
      // Prepare recruitment
      (*it).second->recruitment->Prepare();
      MODEL_LOG << "Recruitment for population successfully prepared"
                << std::endl;
// link to TMB objective function
#ifdef TMB_MODEL
      (*it).second->of = this->of;
#endif
      // Evaluate population
      (*it).second->Evaluate();
      // Recrtuiment negative log-likelihood
      rec_nll += (*it).second->recruitment->evaluate_nll();
      MODEL_LOG << "Recruitment negative log-likelihood is: " << rec_nll
                << std::endl;
    }
    MODEL_LOG << "All populations successfully evaluated." << std::endl;

    // Loop over fleets/surveys, and sum up age comp and index nlls

    typename fims_info::Information<Type>::fleet_iterator jt;
    MODEL_LOG << "Evaluating expected values and summing nlls for "
              << this->fims_information->fleets.size() << " fleets."
              << std::endl;

    for (jt = this->fims_information->fleets.begin();
         jt != this->fims_information->fleets.end(); ++jt) {
      //(*jt).second points to each individual Fleet module
#ifdef TMB_MODEL
      (*jt).second->of = this->of;
#endif
      MODEL_LOG << "Setting up pointer to fleet " << (*jt).second->id << "."
                << std::endl;
      age_comp_nll += (*jt).second->evaluate_age_comp_nll();
      MODEL_LOG << "Sum of survey and age comp negative log-likelihood is: "
                << age_comp_nll << std::endl;
      index_nll += (*jt).second->evaluate_index_nll();
    }
    MODEL_LOG << "All fleets successfully evaluated." << std::endl;
    // Loop over populations and fleets/surveys and fill in reporting

    // initiate population index for structuring report out objects
    int pop_idx = 0;
    for (it = this->fims_information->populations.begin();
         it != this->fims_information->populations.end(); ++it) {
#ifdef TMB_MODEL
      naa(pop_idx) = vector<Type>((*it).second->numbers_at_age);
      ssb(pop_idx) = vector<Type>((*it).second->spawning_biomass);
      log_recruit_dev(pop_idx) =
          vector<Type>((*it).second->recruitment->log_recruit_devs);
      recruitment(pop_idx) = vector<Type>((*it).second->expected_recruitment);
      biomass(pop_idx) = vector<Type>((*it).second->biomass);
      M(pop_idx) = vector<Type>((*it).second->M);
#endif
      pop_idx += 1;
    }

    // initiate fleet index for structuring report out objects
    int fleet_idx = 0;
    for (jt = this->fims_information->fleets.begin();
         jt != this->fims_information->fleets.end(); ++jt) {
#ifdef TMB_MODEL
      exp_index(fleet_idx) = (*jt).second->expected_index;
      exp_catch(fleet_idx) = (*jt).second->expected_catch;
      F_mort(fleet_idx) = (*jt).second->Fmort;
      cnaa(fleet_idx) = (*jt).second->catch_numbers_at_age;
      cwaa(fleet_idx) = (*jt).second->catch_weight_at_age;
#endif
      fleet_idx += 1;
    }

    jnll = rec_nll + age_comp_nll + index_nll;

// Reporting
#ifdef TMB_MODEL
    FIMS_REPORT_F(rec_nll, of);
    FIMS_REPORT_F(age_comp_nll, of);
    FIMS_REPORT_F(index_nll, of);
    FIMS_REPORT_F(jnll, of);
    FIMS_REPORT_F(naa, of);
    FIMS_REPORT_F(ssb, of);
    FIMS_REPORT_F(log_recruit_dev, of);
    FIMS_REPORT_F(recruitment, of);
    FIMS_REPORT_F(biomass, of);
    FIMS_REPORT_F(M, of);
    FIMS_REPORT_F(exp_index, of);
    FIMS_REPORT_F(exp_catch, of);
    FIMS_REPORT_F(F_mort, of);
    FIMS_REPORT_F(cnaa, of);
    FIMS_REPORT_F(cwaa, of);

    /*ADREPORT using ADREPORTvector defined in
     * inst/include/interface/interface.hpp:
     * function collapses the nested vector into a single vector
     */
    vector<Type> NAA = ADREPORTvector(naa);
    vector<Type> Biomass = ADREPORTvector(biomass);
    vector<Type> SSB = ADREPORTvector(ssb);
    vector<Type> LogRecDev = ADREPORTvector(log_recruit_dev);
    vector<Type> FMort = ADREPORTvector(F_mort);
    vector<Type> ExpectedIndex = ADREPORTvector(exp_index);
    vector<Type> CNAA = ADREPORTvector(cnaa);

    ADREPORT_F(NAA, of);
    ADREPORT_F(Biomass, of);
    ADREPORT_F(SSB, of);
    ADREPORT_F(LogRecDev, of);
    ADREPORT_F(FMort, of);
    ADREPORT_F(ExpectedIndex, of);
    ADREPORT_F(CNAA, of);
#endif

    return jnll;
  }
};

// Create singleton instance of Model class
template <typename Type>
std::shared_ptr<Model<Type> > Model<Type>::fims_model =
    nullptr;  // singleton instance
}  // namespace fims_model

#endif /* FIMS_COMMON_MODEL_HPP */
