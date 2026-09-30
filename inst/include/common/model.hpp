/**
 * @file model.hpp
 * @brief : Loops over model components and returns the negative log-likelihood
 * function.
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

  /**
   * @brief Construct a new Model object.
   *
   */
  Model() {}

  /**
   * @brief Destroy the Model object.
   *
   */
  ~Model() {}
  /**
   * @brief Evaluate. Calculates the joint negative log-likelihood function.
   */
#ifdef TMB_MODEL
  // nullptr outside of a TMB call because TMB owns the objective function.
  ::objective_function<Type> *of = nullptr;
#endif

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
          "Model: fims_information is not set. Please set fims_information "
          "before "
          "calling Evaluate().");
      return jnll;
    }

    // Create vector for reporting out nll components
    fims::Vector<Type> nll_vec(
        this->fims_information->density_components.size(), 0.0);
    // Create a vector of strings with a pre-defined size for the nll component names
    std::vector<std::string> nll_component_names(
        this->fims_information->density_components.size());

    for (m_it = this->fims_information->models_map.begin();
         m_it != this->fims_information->models_map.end(); ++m_it) {
      //(*m_it).second points to the Model module
      std::shared_ptr<fims_popdy::FisheryModelBase<Type>> m = (*m_it).second;
      m->Prepare();
      m->Evaluate();
    }

    // Loop over all density components once, evaluating and categorizing them
    // to preserve reporting order.
    typename fims_info::Information<Type>::density_components_iterator d_it;
    std::vector<std::pair<Type, std::string>> prior_components;
    std::vector<std::pair<Type, std::string>> re_components;
    std::vector<std::pair<Type, std::string>> data_components;

    for (d_it = this->fims_information->density_components.begin();
         d_it != this->fims_information->density_components.end(); ++d_it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*d_it).second;
#ifdef TMB_MODEL
      d->of = this->of;
#endif
      Type nll_val = -d->evaluate();
      jnll += nll_val;

      if (d->input_type == "prior") {
        prior_components.push_back(
            {nll_val, "prior_" + fims::to_string(d->id)});
      } else if (d->input_type == "random_effects") {
        re_components.push_back(
            {nll_val, "random_effects_" + fims::to_string(d->id)});
      } else if (d->input_type == "data") {
        data_components.push_back(
            {nll_val, "data_" + fims::to_string(d->id)});
      }
    }

    FIMS_INFO_LOG("Model: Finished evaluating " +
                  fims::to_string(prior_components.size()) + " priors, " +
                  fims::to_string(re_components.size()) +
                  " random effects, and " +
                  fims::to_string(data_components.size()) +
                  " data likelihoods. The total jnll is: " +
                  fims::to_string(jnll));

    // Assemble the final report vectors in the desired order (priors, re, data)
    int nll_vec_idx = 0;
    auto copy_components =
        [&](const std::vector<std::pair<Type, std::string>>& comps) {
          for (const auto& comp : comps) {
            nll_vec[nll_vec_idx] = comp.first;
            nll_component_names[nll_vec_idx] = comp.second;
            nll_vec_idx++;
          }
        };

    copy_components(prior_components);
    copy_components(re_components);
    copy_components(data_components);

    // report out nll components

#ifdef TMB_MODEL

    vector<Type> nll_components = nll_vec.to_tmb();
    FIMS_REPORT_F(nll_components, this->of);
    FIMS_REPORT_F(nll_component_names, this->of);
    FIMS_REPORT_F(jnll, this->of);

#endif

    // report out model family objects
    for (m_it = this->fims_information->models_map.begin();
         m_it != this->fims_information->models_map.end(); ++m_it) {
      //(*m_it).second points to the Model module
      std::shared_ptr<fims_popdy::FisheryModelBase<Type>> m = (*m_it).second;
      m->of = this->of;  // link to TMB objective function
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
