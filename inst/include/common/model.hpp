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

namespace fims
{

/**
 * @brief Model class. FIMS objective function.
 */
template <typename T> class Model
{ // may need singleton
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
  virtual ~Model () {}

  /**
   * Returns a single Information object for type T.
   *
   * @return singleton for type T
   */
  static std::shared_ptr<Model<T> >
  GetInstance ()
  {
    if (Model<T>::fims_model == nullptr)
      {
        Model<T>::fims_model = std::make_shared<fims::Model<T> > ();
        Model<T>::fims_model->fims_information
            = fims::Information<T>::GetInstance ();
      }
    return Model<T>::fims_model;
  }

  /**
   * @brief Evaluate. Calculates the joint negative log-likelihood function.
   */
  const T
  Evaluate ()
  {
    // jnll = negative-log-likelihood (the objective function)
    T jnll = 0.0;
    T rec_nll = 0.0;      // recrutiment nll
    T age_comp_nll = 0.0; // age composition nll
    T index_nll = 0.0;    // survey and fishery cacth nll
    // Loop over populations, evaluate, and sum up the recruitment likelihood
    // component
    typename fims::Information<T>::population_iterator it;
    for (it = this->fims_information->populations.begin ();
         it != this->fims_information->populations.end (); ++it)
      {
        //(*it).second points to the Population module
        FIMS_LOG << "inside pop loop" << std::endl;
        // Prepare recruitment
        (*it).second->recruitment->Prepare ();
        FIMS_LOG << "recruitment prepare works" << std::endl;
// link to TMB objective function
#ifdef TMB_MODEL
        (*it).second->of = this->of;
#endif
        // Evaluate population
        (*it).second->Evaluate ();
        // Recrtuiment negative log-likelihood
        rec_nll += (*it).second->recruitment->evaluate_nll ();
        FIMS_LOG << "rec nll: " << rec_nll << std::endl;
      }

    typename fims::Information<T>::fleet_iterator jt;
    for (jt = this->fims_information->fleets.begin ();
         jt != this->fims_information->fleets.end (); ++jt)
      {
#ifdef TMB_MODEL
        (*jt).second->of = this->of;
#endif
        age_comp_nll += (*jt).second->evaluate_age_comp_nll ();
        FIMS_LOG << "survey and fleet age comp nll sum: " << age_comp_nll
                 << std::endl;
        index_nll += (*jt).second->evaluate_index_nll ();
        FIMS_LOG << "survey and fleet index nll sum: " << index_nll
                 << std::endl;
        if ((*jt).second->is_survey == false)
          {
#ifdef TMB_MODEL
            (*jt).second->of = this->of;
#endif
            (*jt).second->ReportFleet ();
          }
      }

    jnll = rec_nll + age_comp_nll + index_nll;

    return jnll;
  }
};

// Create singleton instance of Model class
template <typename T>
std::shared_ptr<Model<T> > Model<T>::fims_model
    = nullptr; // singleton instance
} // namespace fims

#endif /* FIMS_COMMON_MODEL_HPP */
