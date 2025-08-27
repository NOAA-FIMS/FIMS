/**
 * @file model.hpp
 * @brief TODO: provide a brief description.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
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
  static std::shared_ptr<Model<Type>>
      fims_model; /**< Create a shared fims_model as a pointer to Model*/
  std::shared_ptr<fims_info::Information<Type>>
      fims_information; /**< Create a shared fims_information as a pointer to
                         Information*/

#ifdef TMB_MODEL
  bool do_tmb_reporting = true;
  ::objective_function<Type> *of;
#endif

  // constructor

  virtual ~Model() {}

  /**
   * Returns a single Information object for type Type.
   *
   * @return singleton for type Type
   */
  static std::shared_ptr<Model<Type>> GetInstance() {
    if (Model<Type>::fims_model == nullptr) {
      Model<Type>::fims_model = std::make_shared<fims_model::Model<Type>>();
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
    Type jnll = static_cast<Type>(0.0);
    typename fims_info::Information<Type>::model_map_iterator m_it;
    // Check if fims_information is set
    if (this->fims_information == nullptr) {
      FIMS_ERROR_LOG(
          "fims_information is not set. Please set fims_information before "
          "calling Evaluate().");
      return jnll;
    }
    
     // Create vector for reporting out nll components
    fims::Vector<Type> nll_vec(
          this->fims_information->density_components.size(), 0.0);

    for (m_it = this->fims_information->models_map.begin();
         m_it != this->fims_information->models_map.end(); ++m_it) {
      //(*m_it).second points to the Model module
      std::shared_ptr<fims_popdy::FisheryModelBase<Type>> m = (*m_it).second;
      m->of = this->of;  // link to TMB objective function
      m->Prepare();
    }
    
    
    // Loop over densities and evaluate joint negative log densities for priors
    typename fims_info::Information<Type>::density_components_iterator d_it;
    int nll_vec_idx = 0;
    size_t n_priors = 0;
    FIMS_INFO_LOG("Begin evaluating prior densities.")
    for (d_it = this->fims_information->density_components.begin();
         d_it != this->fims_information->density_components.end(); ++d_it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*d_it).second;
#ifdef TMB_MODEL
      d->of = this->of;
#endif
        if (d->input_type == "prior") {
          nll_vec[nll_vec_idx] = -d->evaluate();
          jnll += nll_vec[nll_vec_idx];
          n_priors += 1;
          nll_vec_idx += 1;
        }
      }
    FIMS_INFO_LOG(
        "Model: Finished evaluating prior distributions. The jnll after "
        "evaluating " +
          fims::to_string(n_priors) + " priors is: " + fims::to_string(jnll));
    
    
    // Loop over densities and evaluate joint negative log-likelihoods for
    // random effects
    size_t n_random_effects = 0;
    for (d_it = this->fims_information->density_components.begin();
         d_it != this->fims_information->density_components.end(); ++d_it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*d_it).second;
#ifdef TMB_MODEL
      d->of = this->of;
#endif
      if (d->input_type == "random_effects") {
        nll_vec[nll_vec_idx] = -d->evaluate();
        jnll += nll_vec[nll_vec_idx];
        n_random_effects += 1;
        nll_vec_idx += 1;
      }
    }
    FIMS_INFO_LOG(
        "Model: Finished evaluating random effect distributions. The jnll "
        "after evaluating priors and " +
        fims::to_string(n_random_effects) +
        " random_effects is: " + fims::to_string(jnll));
    
    //evaluate model
    for (m_it = this->fims_information->models_map.begin();
         m_it != this->fims_information->models_map.end(); ++m_it) {
      //(*m_it).second points to the Model module
      std::shared_ptr<fims_popdy::FisheryModelBase<Type>> m = (*m_it).second;
      m->of = this->of;  // link to TMB objective function
      m->Evaluate();
    }

    this->fims_information->SetupData();
    // Loop over and evaluate data joint negative log-likelihoods
    int n_data = 0;
    for (d_it = this->fims_information->density_components.begin();
         d_it != this->fims_information->density_components.end(); ++d_it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*d_it).second;
#ifdef TMB_MODEL
      d->of = this->of;
      // d->keep = this->keep;
#endif
      if (d->input_type == "data") {
        nll_vec[nll_vec_idx] = -d->evaluate();
        jnll += nll_vec[nll_vec_idx];
        n_data += 1;
        nll_vec_idx += 1;
      }
    }

// report out nll components
#ifdef TMB_MODEL
    vector<Type> nll_components = nll_vec;
    FIMS_REPORT_F(nll_components, this->of);
    FIMS_REPORT_F(jnll, this->of);
#endif

    // report out model family objects
    for (m_it = this->fims_information->models_map.begin();
         m_it != this->fims_information->models_map.end(); ++m_it) {
      //(*m_it).second points to the Model module
      std::shared_ptr<fims_popdy::FisheryModelBase<Type>> m = (*m_it).second;
      m->Report();
    }

    return jnll;
  }
};

// Create singleton instance of Model class
template <typename Type>
std::shared_ptr<Model<Type>> Model<Type>::fims_model =
    nullptr;  // singleton instance
}  // namespace fims_model

#endif /* FIMS_COMMON_MODEL_HPP */
