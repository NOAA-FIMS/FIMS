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

namespace fims {

/**
 * @brief Model class. FIMS objective function.
 */
template <typename Type>
class Model {  // may need singleton
 public:
  static std::shared_ptr<Model<Type> >
      fims_model; /*!< Create a shared fims_model as a pointer to Model*/
  std::shared_ptr<Information<Type> >
      fims_information; /*!< Create a shared fims_information as a pointer to
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
      Model<Type>::fims_model = std::make_shared<fims::Model<Type> >();
      Model<Type>::fims_model->fims_information =
          fims::Information<Type>::GetInstance();
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
    // Loop over populations, evaluate, and sum up the recruitment likelihood
    // component
    typename fims::Information<Type>::population_iterator it;
    for (it = this->fims_information->populations.begin();
         it != this->fims_information->populations.end(); ++it) {
      //(*it).second points to the Population module
      FIMS_LOG << "inside pop loop" << std::endl;
      // Prepare recruitment
      (*it).second->recruitment->Prepare();
      FIMS_LOG << "recruitment prepare works" << std::endl;
// link to TMB objective function
#ifdef TMB_MODEL
      (*it).second->of = this->of;
#endif
      // Evaluate population
      (*it).second->Evaluate();
      // Recrtuiment negative log-likelihood
      rec_nll += (*it).second->recruitment->evaluate_nll();
      FIMS_LOG << "rec nll: " << rec_nll << std::endl;
    }

    typename fims::Information<Type>::fleet_iterator jt;
    for (jt = this->fims_information->fleets.begin();
         jt != this->fims_information->fleets.end(); ++jt) {
#ifdef TMB_MODEL
      (*jt).second->of = this->of;
#endif
      age_comp_nll += (*jt).second->evaluate_age_comp_nll();
      FIMS_LOG << "survey and fleet age comp nll sum: " << age_comp_nll
               << std::endl;
      index_nll += (*jt).second->evaluate_index_nll();
      FIMS_LOG << "survey and fleet index nll sum: " << index_nll << std::endl;
      if ((*jt).second->is_survey == false) {
#ifdef TMB_MODEL
        (*jt).second->of = this->of;
#endif
        (*jt).second->ReportFleet();
      }
    }

    jnll = rec_nll + age_comp_nll + index_nll;

    return jnll;
  }
};

// Create singleton instance of Model class
template <typename Type>
std::shared_ptr<Model<Type> > Model<Type>::fims_model =
    nullptr;  // singleton instance
}  // namespace fims

#endif /* FIMS_COMMON_MODEL_HPP */
