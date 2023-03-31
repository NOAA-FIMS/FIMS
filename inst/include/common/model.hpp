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
template <typename T>
class Model {  // may need singleton
 public:
  static std::shared_ptr<Model<T> >
      fims_model; /*!< Create a shared fims_model as a pointer to Model*/
  std::shared_ptr<Information<T> >
      fims_information; /*!< Create a shared fims_information as a pointer to
                           Information*/

#ifdef TMB_MODEL
  ::objective_function<T> *of;
#endif

  // constructor
  virtual ~Model() {}

  /**
   * Returns a single Information object for type T.
   *
   * @return singleton for type T
   */
  static std::shared_ptr<Model<T> > GetInstance() {
    if (Model<T>::fims_model == nullptr) {
      Model<T>::fims_model = std::make_shared<fims::Model<T> >();
      Model<T>::fims_model->fims_information =
          fims::Information<T>::GetInstance();
    }
    return Model<T>::fims_model;
  }

  /**
   * @brief Evaluate. Calculates the joint negative log-likelihood function.
   */
  const T Evaluate() {
    typename fims::Information<T>::population_iterator it;
    for (it = this->fims_information->populations.begin();
         it != this->fims_information->populations.end(); ++it) {
//(*it).second points to the Population module
#ifdef TMB_MODEL
      (*it).second->of = this->of;
#endif
      (*it).second->Prepare();
      (*it).second->Evaluate();
    }

    // nll = negative-log-likelihood (the objective function)
    T nll = 0.0;
    // nll will loop over fleets (Fleet module does not have evaluate function
    // yet) Sum up nlls here in model.hpp or in fleet.hpp? for(jt =
    // this->fims_information->fleets.begin(); jt !=
    // this->fims_information->fleets.end(); ++jt ){ nll +=
    // (*jt).second.Evaluate();
    //}

    return nll;
  }
};

// Create singleton instance of Model class
template <typename T>
std::shared_ptr<Model<T> > Model<T>::fims_model =
    nullptr;  // singleton instance
}  // namespace fims

#endif /* FIMS_COMMON_MODEL_HPP */
