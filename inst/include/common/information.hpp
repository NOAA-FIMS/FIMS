/**
 * @file information.hpp
 * @brief Code to store all objects that are created in FIMS because FIMS uses
 * integer representation. Code loops over all model components and sets them
 * up based on unique identifiers.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */

#ifndef FIMS_COMMON_INFORMATION_HPP
#define FIMS_COMMON_INFORMATION_HPP

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "../distributions/kernels/distribution_kernels.hpp"
#include "../distributions/distributions.hpp"
#include "../likelihood/likelihood.hpp"
#include "../models/functors/fishery_model_base.hpp"
#include "../population_dynamics/fleet/fleet.hpp"
#include "../population_dynamics/growth/growth.hpp"
#include "../population_dynamics/population/population.hpp"
#include "../population_dynamics/recruitment/recruitment.hpp"
#include "../population_dynamics/selectivity/selectivity.hpp"
#include "def.hpp"
#include "fims_vector.hpp"
#include "model_object.hpp"

namespace fims_info {

/**
 * @brief Stores FIMS model information and creates model. Contains all objects
 * and data pre-model construction
 */
template <typename Type>
class Information {
 public:
  size_t n_years = 0; /**< number of years >*/
  size_t n_ages = 0;  /**< number of ages>*/
  bool use_likelihood_terms =
      false; /**< opt-in flag for mirrored likelihood evaluation >*/

  static std::shared_ptr<Information<Type>>
      fims_information;          /**< singleton instance >*/
  std::vector<Type*> parameters; /**< list of all estimated parameters >*/
  std::vector<Type*>
      random_effects_parameters; /**< list of all random effects parameters >*/
  std::vector<Type*>
      fixed_effects_parameters; /**< list of all fixed effects parameters >*/
  std::vector<std::string> parameter_names; /**< list of all parameter names
                                               estimated in the model */
  std::vector<std::string>
      random_effects_names; /**< list of all random effects names estimated in
                               the model */

  // data objects
  std::map<uint32_t, std::shared_ptr<fims_data_object::DataObject<Type>>>
      data_objects; /**< map that holds data objects >*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_data_object::DataObject<Type>>>::iterator
      data_iterator; /**< iterator for the data objects */

  // life history modules
  std::map<uint32_t, std::shared_ptr<fims_popdy::RecruitmentBase<Type>>>
      recruitment_models; /**<hash map to link each object to its shared
                           location in memory*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_popdy::RecruitmentBase<Type>>>::iterator
      recruitment_models_iterator;
  /**< iterator for recruitment objects>*/

  std::map<uint32_t, std::shared_ptr<fims_popdy::RecruitmentBase<Type>>>
      recruitment_process_models; /**<hash map to link each object to its shared
                           location in memory*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_popdy::RecruitmentBase<Type>>>::iterator
      recruitment_process_iterator;
  /**< iterator for recruitment process objects>*/

  std::map<uint32_t, std::shared_ptr<fims_popdy::SelectivityBase<Type>>>
      selectivity_models; /**<hash map to link each object to its shared
                           location in memory*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_popdy::SelectivityBase<Type>>>::iterator
      selectivity_models_iterator;
  /**< iterator for selectivity objects>*/

  std::map<uint32_t, std::shared_ptr<fims_popdy::GrowthBase<Type>>>
      growth_models; /**<hash map to link each object to its shared location in
                      memory*/
  typedef
      typename std::map<uint32_t,
                        std::shared_ptr<fims_popdy::GrowthBase<Type>>>::iterator
          growth_models_iterator;
  /**< iterator for growth objects>*/

  std::map<uint32_t, std::shared_ptr<fims_popdy::MaturityBase<Type>>>
      maturity_models; /**<hash map to link each object to its shared location
                        in memory*/
  typedef typename std::map<
      uint32_t, std::shared_ptr<fims_popdy::MaturityBase<Type>>>::iterator
      maturity_models_iterator;
  /**< iterator for maturity objects>*/

  // fleet modules
  std::map<uint32_t, std::shared_ptr<fims_popdy::Fleet<Type>>>
      fleets; /**<hash map to link each object to its shared location in
               memory*/
  typedef typename std::map<uint32_t,
                            std::shared_ptr<fims_popdy::Fleet<Type>>>::iterator
      fleet_iterator;
  /**< iterator for fleet objects>*/

  // populations
  std::map<uint32_t, std::shared_ptr<fims_popdy::Population<Type>>>
      populations; /**<hash map to link each object to its shared location in
                    memory*/
  typedef
      typename std::map<uint32_t,
                        std::shared_ptr<fims_popdy::Population<Type>>>::iterator
          population_iterator;
  /**< iterator for population objects>*/

  // distributions
  std::map<uint32_t,
           std::shared_ptr<fims_distributions::DensityComponentBase<Type>>>
      density_components; /**<hash map to link each object to its shared
                            location in memory*/
  typedef typename std::map<
      uint32_t,
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>>>::iterator
      density_components_iterator;
  /**< iterator for distribution objects>*/

  // likelihood terms
  std::vector<std::shared_ptr<fims_likelihood::LikelihoodTerm<Type>>>
      likelihood_terms; /**< composable likelihood contribution objects >*/
  typedef typename std::vector<
      std::shared_ptr<fims_likelihood::LikelihoodTerm<Type>>>::iterator
      likelihood_terms_iterator;
  /**< iterator for likelihood terms>*/

  std::unordered_map<uint32_t,
                     std::shared_ptr<fims_popdy::FisheryModelBase<Type>>>
      models_map; /**<hash map of fishery models, e.g., CAA, GMACS, Spatial,
                     etc*/
  typedef typename std::unordered_map<
      uint32_t, std::shared_ptr<fims_popdy::FisheryModelBase<Type>>>::iterator
      model_map_iterator; /**< iterator for variable map>*/

  std::unordered_map<uint32_t, fims::Vector<Type>*>
      variable_map; /**<hash map to link a parameter, derived value, or
                      observation to its shared location in memory */
  typedef typename std::unordered_map<uint32_t, fims::Vector<Type>*>::iterator
      variable_map_iterator; /**< iterator for variable map>*/

  Information() {}

  virtual ~Information() {}

  /**
   * @brief Clears all containers.
   *
   */
  void Clear() {
    this->data_objects.clear();
    this->populations.clear();
    this->fixed_effects_parameters.clear();
    this->fleets.clear();
    this->growth_models.clear();
    this->maturity_models.clear();
    this->parameter_names.clear();
    this->parameters.clear();
    this->random_effects_names.clear();
    this->random_effects_parameters.clear();
    this->recruitment_models.clear();
    this->recruitment_process_models.clear();
    this->selectivity_models.clear();
    this->models_map.clear();
    this->likelihood_terms.clear();
    this->n_years = 0;
    this->n_ages = 0;
    this->use_likelihood_terms = false;

    for (density_components_iterator it = density_components.begin();
         it != density_components.end(); ++it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*it).second;
      if ((d->priors)[0] != NULL) {
        d->priors.clear();
      }
      if (d->data_expected_values != NULL) {
        d->data_expected_values->clear();
      }
      if (d->re != NULL) {
        d->re->clear();
      }
      if (d->re_expected_values != NULL) {
        d->re_expected_values->clear();
      }
    }
    this->density_components.clear();
  }

  /**
   * @brief Get a summary string of the Information object state.
   *
   * @details Returns a string containing the sizes and states of all major
   * containers and model components in the Information object. Useful for
   * debugging and diagnostics.
   *
   * @return std::string summary of the Information object state.
   */
  std::string State() {
    std::stringstream ss;
    ss << "Information object State:\n";
    ss << "data_objects: " << this->data_objects.clear();
    ss << "populations: " << this->populations.size() << std::endl;
    ss << "fixed_effects_parameters: " << this->fixed_effects_parameters.size()
       << std::endl;
    ss << "fleets: " << this->fleets.size() << std::endl;
    ss << "growth_models: " << this->growth_models.size() << std::endl;
    ss << "maturity_models: " << this->maturity_models.size() << std::endl;
    ss << "parameter_names: " << this->parameter_names.size() << std::endl;
    ss << "parameters: " << this->parameters.size() << std::endl;
    ss << "random_effects_names: " << this->random_effects_names.size()
       << std::endl;
    ss << "random_effects_parameters: "
       << this->random_effects_parameters.size() << std::endl;
    ss << "recruitment_models: " << this->recruitment_models.size()
       << std::endl;
    ss << "recruitment_process_models: "
       << this->recruitment_process_models.size() << std::endl;
    ss << "selectivity_models: " << this->selectivity_models.size()
       << std::endl;
    ss << "models_map: " << this->models_map.size() << std::endl;
    ss << "n_years: " << this->n_years << std::endl;
    ss << "n_ages: " << this->n_ages << std::endl;
    ss << "density_components: " << this->density_components.size()
       << std::endl;
    ss << "likelihood_terms: " << this->likelihood_terms.size() << std::endl;
    ss << "use_likelihood_terms: " << this->use_likelihood_terms << std::endl;
    return ss.str();
  }

  /**
   * @brief Returns a singleton Information object for type T.
   *
   * @return singleton for type T
   */
  static std::shared_ptr<Information<Type>> GetInstance() {
    if (Information<Type>::fims_information == nullptr) {
      Information<Type>::fims_information =
          std::make_shared<fims_info::Information<Type>>();
    }
    return Information<Type>::fims_information;
  }

  /**
   * @brief Register a parameter as estimable.
   *
   * @param p parameter
   */
  void RegisterParameter(Type& p) {
    this->fixed_effects_parameters.push_back(&p);
  }

  /**
   * @brief Register a random effect as estimable.
   *
   * @param re random effect
   */
  void RegisterRandomEffect(Type& re) {
    this->random_effects_parameters.push_back(&re);
  }

  /**
   * @brief Register a parameter name.
   *
   * @param p_name parameter name
   */
  void RegisterParameterName(std::string p_name) {
    this->parameter_names.push_back(p_name);
  }

  /**
   * @brief Register a random effects name.
   *
   * @param re_name random effects name
   */
  void RegisterRandomEffectName(std::string re_name) {
    this->random_effects_names.push_back(re_name);
  }

  /**
   * @brief Remove likelihood terms of a specific semantic type.
   *
   * @param type likelihood term type to remove
   */
  void RemoveLikelihoodTerms(fims_likelihood::LikelihoodTermType type) {
    this->likelihood_terms.erase(
        std::remove_if(this->likelihood_terms.begin(),
                       this->likelihood_terms.end(),
                       [type](const std::shared_ptr<
                              fims_likelihood::LikelihoodTerm<Type>>& term) {
                         return term->type == type;
                       }),
        this->likelihood_terms.end());
  }

  /**
   * @brief Evaluate all likelihood terms of a specific semantic type.
   *
   * @param type likelihood term type to evaluate
   * @return Sum of log-density contributions for matching terms.
   */
  Type EvaluateLikelihoodTerms(fims_likelihood::LikelihoodTermType type) {
    Type log_density_sum = static_cast<Type>(0);
    for (likelihood_terms_iterator it = this->likelihood_terms.begin();
         it != this->likelihood_terms.end(); ++it) {
      if ((*it)->type == type) {
        log_density_sum += (*it)->evaluate();
      }
    }
    return log_density_sum;
  }

  /**
   * @brief Evaluate all likelihood terms.
   *
   * @return Sum of log-density contributions across all terms.
   */
  Type EvaluateLikelihoodTerms() {
    Type log_density_sum = static_cast<Type>(0);
    for (likelihood_terms_iterator it = this->likelihood_terms.begin();
         it != this->likelihood_terms.end(); ++it) {
      log_density_sum += (*it)->evaluate();
    }
    return log_density_sum;
  }

  /**
   * @brief Evaluate negative log-likelihood for terms of a specific type.
   *
   * @param type likelihood term type to evaluate
   * @return Negative sum of log-density contributions for matching terms.
   */
  Type EvaluateNegativeLogLikelihoodTerms(
      fims_likelihood::LikelihoodTermType type) {
    return -this->EvaluateLikelihoodTerms(type);
  }

  /**
   * @brief Evaluate negative log-likelihood for all likelihood terms.
   *
   * @return Negative sum of log-density contributions across all terms.
   */
  Type EvaluateNegativeLogLikelihoodTerms() {
    return -this->EvaluateLikelihoodTerms();
  }

  /**
   * @brief Find a likelihood term by legacy density component source ID.
   *
   * @param source_id ID of the mirrored legacy density component.
   * @return Matching likelihood term, or nullptr if none exists.
   */
  std::shared_ptr<fims_likelihood::LikelihoodTerm<Type>> FindLikelihoodTerm(
      uint32_t source_id) {
    for (likelihood_terms_iterator it = this->likelihood_terms.begin();
         it != this->likelihood_terms.end(); ++it) {
      if ((*it)->source_id == source_id) {
        return (*it);
      }
    }
    return nullptr;
  }

  /**
   * @brief Find a likelihood term by source ID and semantic type.
   *
   * @param source_id ID of the mirrored legacy density component.
   * @param type likelihood term type to match
   * @return Matching likelihood term, or nullptr if none exists.
   */
  std::shared_ptr<fims_likelihood::LikelihoodTerm<Type>> FindLikelihoodTerm(
      uint32_t source_id, fims_likelihood::LikelihoodTermType type) {
    for (likelihood_terms_iterator it = this->likelihood_terms.begin();
         it != this->likelihood_terms.end(); ++it) {
      if ((*it)->source_id == source_id && (*it)->type == type) {
        return (*it);
      }
    }
    return nullptr;
  }

  /**
   * @brief Evaluate one mirrored likelihood term by source ID and type.
   *
   * @param source_id ID of the mirrored legacy density component.
   * @param type likelihood term type to match
   * @return Log-density contribution for the matching term.
   * @throws std::runtime_error if no matching likelihood term exists.
   */
  Type EvaluateLikelihoodTerm(uint32_t source_id,
                              fims_likelihood::LikelihoodTermType type) {
    std::shared_ptr<fims_likelihood::LikelihoodTerm<Type>> term =
        this->FindLikelihoodTerm(source_id, type);
    if (term == nullptr) {
      throw std::runtime_error(
          "No likelihood term found for source_id " +
          fims::to_string(source_id) + ".");
    }
    return term->evaluate();
  }

  /**
   * @brief Evaluate negative log-likelihood for one mirrored term.
   *
   * @param source_id ID of the mirrored legacy density component.
   * @param type likelihood term type to match
   * @return Negative log-density contribution for the matching term.
   * @throws std::runtime_error if no matching likelihood term exists.
   */
  Type EvaluateNegativeLogLikelihoodTerm(
      uint32_t source_id, fims_likelihood::LikelihoodTermType type) {
    return -this->EvaluateLikelihoodTerm(source_id, type);
  }

  /**
   * @brief Add a mirrored likelihood term.
   *
   * @param type likelihood term type
   * @param name term name
   * @param source_id ID of the mirrored legacy density component
   * @param x observed/target values
   * @param location expected/location values
   * @param scale scale values
   * @param log_density distribution log-density function
   * @param include optional predicate for skipping entries
   */
  void AddLikelihoodTerm(
      fims_likelihood::LikelihoodTermType type, const std::string& name,
      uint32_t source_id, fims_likelihood::ValueRef<Type> x,
      fims_likelihood::ValueRef<Type> location,
      fims_likelihood::ValueRef<Type> scale,
      typename fims_likelihood::LikelihoodTerm<Type>::LogDensityFunction
          log_density,
      typename fims_likelihood::LikelihoodTerm<Type>::IncludeFunction include =
          nullptr) {
    this->likelihood_terms.push_back(fims_likelihood::make_likelihood_term(
        type, name, x, location, scale, log_density, include, source_id));
  }

  /**
   * @brief Add a mirrored row-wise likelihood term.
   *
   * @param type likelihood term type
   * @param name term name
   * @param source_id ID of the mirrored legacy density component
   * @param x observed/target values
   * @param location expected/location values
   * @param row_size number of entries per row-wise density contribution
   * @param log_density row-wise distribution log-density function
   * @param include optional predicate for skipping entries
   */
  void AddLikelihoodTerm(
      fims_likelihood::LikelihoodTermType type, const std::string& name,
      uint32_t source_id, fims_likelihood::ValueRef<Type> x,
      fims_likelihood::ValueRef<Type> location, size_t row_size,
      typename fims_likelihood::LikelihoodTerm<Type>::VectorLogDensityFunction
          log_density,
      typename fims_likelihood::LikelihoodTerm<Type>::IncludeFunction include =
          nullptr) {
    this->likelihood_terms.push_back(fims_likelihood::make_likelihood_term(
        type, name, x, location, row_size, log_density, include, source_id));
  }

  /**
   * @brief Build a ValueRef for a density component's observed/target values.
   */
  fims_likelihood::ValueRef<Type> ObservedValueRef(
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d) {
    return fims_likelihood::ValueRef<Type>(
        [d](size_t i) -> Type { return d->get_observed(i); }, d->get_n_x());
  }

  /**
   * @brief Build a ValueRef for a density component's expected/location values.
   */
  fims_likelihood::ValueRef<Type> ExpectedValueRef(
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d) {
    return fims_likelihood::ValueRef<Type>(
        [d](size_t i) -> Type { return d->get_expected(i); },
        d->get_n_expected());
  }

  /**
   * @brief Build a scale ValueRef from a normal distribution's log_sd vector.
   */
  fims_likelihood::ValueRef<Type> ScaleValueRef(
      std::shared_ptr<fims_distributions::NormalLPDF<Type>> normal) {
    return fims_likelihood::ValueRef<Type>(
        [normal](size_t i) -> Type {
          return fims_math::exp(normal->log_sd.get_force_scalar(i));
        },
        normal->log_sd.size());
  }

  /**
   * @brief Build a scale ValueRef from a lognormal distribution's log_sd vector.
   */
  fims_likelihood::ValueRef<Type> ScaleValueRef(
      std::shared_ptr<fims_distributions::LogNormalLPDF<Type>> lognormal) {
    return fims_likelihood::ValueRef<Type>(
        [lognormal](size_t i) -> Type {
          return fims_math::exp(lognormal->log_sd.get_force_scalar(i));
        },
        lognormal->log_sd.size());
  }

  /**
   * @brief Build an include predicate that skips legacy data NA values.
   */
  typename fims_likelihood::LikelihoodTerm<Type>::IncludeFunction
  DataIncludePredicate(
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d) {
    return [d](size_t i) -> bool {
      return d->get_observed(i) != d->data_observed_values->na_value;
    };
  }

  /**
   * @brief Try to mirror a legacy normal density component.
   *
   * @return true if a normal likelihood term was added.
   */
  bool TryAddNormalLikelihoodTerm(
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d,
      fims_likelihood::LikelihoodTermType type, const std::string& name,
      fims_likelihood::ValueRef<Type> x,
      fims_likelihood::ValueRef<Type> location,
      typename fims_likelihood::LikelihoodTerm<Type>::IncludeFunction include =
          nullptr) {
    std::shared_ptr<fims_distributions::NormalLPDF<Type>> normal =
        std::dynamic_pointer_cast<fims_distributions::NormalLPDF<Type>>(d);
    if (!normal || normal->log_sd.size() == 0) {
      return false;
    }

    this->AddLikelihoodTerm(
        type, name, d->id, x, location, this->ScaleValueRef(normal),
        fims_distributions::kernels::Normal<Type>::log_density, include);
    return true;
  }

  /**
   * @brief Try to mirror a legacy lognormal density component.
   *
   * @return true if a lognormal likelihood term was added.
   */
  bool TryAddLogNormalLikelihoodTerm(
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d,
      fims_likelihood::LikelihoodTermType type, const std::string& name,
      fims_likelihood::ValueRef<Type> x,
      fims_likelihood::ValueRef<Type> location,
      typename fims_likelihood::LikelihoodTerm<Type>::LogDensityFunction
          log_density,
      typename fims_likelihood::LikelihoodTerm<Type>::IncludeFunction include =
          nullptr) {
    std::shared_ptr<fims_distributions::LogNormalLPDF<Type>> lognormal =
        std::dynamic_pointer_cast<fims_distributions::LogNormalLPDF<Type>>(d);
    if (!lognormal || lognormal->log_sd.size() == 0) {
      return false;
    }

    this->AddLikelihoodTerm(type, name, d->id, x, location,
                            this->ScaleValueRef(lognormal), log_density,
                            include);
    return true;
  }

  /**
   * @brief Try to mirror a legacy multinomial data density component.
   *
   * @return true if a multinomial likelihood term was added.
   */
  bool TryAddMultinomialDataLikelihoodTerm(
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d,
      const std::string& name, fims_likelihood::ValueRef<Type> x,
      fims_likelihood::ValueRef<Type> location,
      typename fims_likelihood::LikelihoodTerm<Type>::IncludeFunction include =
          nullptr) {
    std::shared_ptr<fims_distributions::MultinomialLPMF<Type>> multinomial =
        std::dynamic_pointer_cast<fims_distributions::MultinomialLPMF<Type>>(d);
    if (!multinomial) {
      return false;
    }

    size_t row_size = 0;
    if (multinomial->dims.size() == 2) {
      row_size = multinomial->dims[1];
    } else if (multinomial->data_observed_values != NULL &&
               multinomial->data_observed_values->dimensions >= 2) {
      row_size = multinomial->data_observed_values->jmax;
    }

    if (row_size == 0 || x.size() % row_size != 0 ||
        location.size() % row_size != 0) {
      return false;
    }

    this->AddLikelihoodTerm(
        fims_likelihood::LikelihoodTermType::Data, name, d->id, x, location,
        row_size, fims_distributions::kernels::Multinomial<Type>::log_density,
        include);
    return true;
  }

  /**
   * @brief Mirror legacy prior density components into likelihood terms.
   *
   * @details This builds the new composable representation without changing
   * model evaluation. Only complete normal and lognormal prior distributions
   * are mirrored.
   */
  void SetupPriorLikelihoodTerms() {
    this->RemoveLikelihoodTerms(fims_likelihood::LikelihoodTermType::Prior);

    for (density_components_iterator it = density_components.begin();
         it != density_components.end(); ++it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*it).second;
      if (d->input_type != "prior" || d->get_n_x() == 0 ||
          d->get_n_expected() == 0 || d->priors.size() == 0 ||
          d->priors[0] == NULL) {
        continue;
      }

      fims_likelihood::ValueRef<Type> x = this->ObservedValueRef(d);
      fims_likelihood::ValueRef<Type> location = this->ExpectedValueRef(d);

      if (this->TryAddNormalLikelihoodTerm(
              d, fims_likelihood::LikelihoodTermType::Prior,
              "normal_prior." + fims::to_string(d->id), x, location)) {
        continue;
      }

      this->TryAddLogNormalLikelihoodTerm(
          d, fims_likelihood::LikelihoodTermType::Prior,
          "lognormal_prior." + fims::to_string(d->id), x, location,
          fims_distributions::kernels::LogNormal<Type>::log_density);
    }
  }

  /**
   * @brief Mirror legacy random-effect density components into likelihood
   * terms.
   *
   * @details This builds the new composable representation without changing
   * model evaluation. Only complete normal and lognormal random-effect
   * distributions are mirrored.
   */
  void SetupRandomEffectLikelihoodTerms() {
    this->RemoveLikelihoodTerms(
        fims_likelihood::LikelihoodTermType::RandomEffect);

    for (density_components_iterator it = density_components.begin();
         it != density_components.end(); ++it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*it).second;
      if (d->input_type != "random_effects" || d->re == NULL ||
          d->re_expected_values == NULL || d->get_n_x() == 0 ||
          d->get_n_expected() == 0) {
        continue;
      }

      fims_likelihood::ValueRef<Type> x = this->ObservedValueRef(d);
      fims_likelihood::ValueRef<Type> location = this->ExpectedValueRef(d);

      if (this->TryAddNormalLikelihoodTerm(
              d, fims_likelihood::LikelihoodTermType::RandomEffect,
              "normal_random_effect." + fims::to_string(d->id), x,
              location)) {
        continue;
      }

      this->TryAddLogNormalLikelihoodTerm(
          d, fims_likelihood::LikelihoodTermType::RandomEffect,
          "lognormal_random_effect." + fims::to_string(d->id), x, location,
          fims_distributions::kernels::LogNormal<
              Type>::log_density_log_scale);
    }
  }

  /**
   * @brief Mirror legacy data density components into likelihood terms.
   *
   * @details This builds the new composable representation without changing
   * model evaluation. Only complete normal and lognormal data distributions
   * are mirrored.
   */
  void SetupDataLikelihoodTerms() {
    this->RemoveLikelihoodTerms(fims_likelihood::LikelihoodTermType::Data);

    for (density_components_iterator it = density_components.begin();
         it != density_components.end(); ++it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*it).second;
      if (d->input_type != "data" || d->data_observed_values == NULL ||
          d->data_expected_values == NULL || d->get_n_x() == 0 ||
          d->get_n_expected() == 0) {
        continue;
      }

      fims_likelihood::ValueRef<Type> x = this->ObservedValueRef(d);
      fims_likelihood::ValueRef<Type> location = this->ExpectedValueRef(d);
      typename fims_likelihood::LikelihoodTerm<Type>::IncludeFunction include =
          this->DataIncludePredicate(d);

      if (this->TryAddNormalLikelihoodTerm(
              d, fims_likelihood::LikelihoodTermType::Data,
              "normal_data." + fims::to_string(d->id), x, location,
              include)) {
        continue;
      }

      this->TryAddLogNormalLikelihoodTerm(
          d, fims_likelihood::LikelihoodTermType::Data,
          "lognormal_data." + fims::to_string(d->id), x, location,
          fims_distributions::kernels::LogNormal<Type>::log_density, include);

      this->TryAddMultinomialDataLikelihoodTerm(
          d, "multinomial_data." + fims::to_string(d->id), x, location,
          include);
    }
  }

  /**
   * @brief Loop over distributions and set links to distribution x value if
   * distribution is a prior type.
   */
  void SetupPriors() {
    for (density_components_iterator it = density_components.begin();
         it != density_components.end(); ++it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*it).second;
      if (d->input_type == "prior") {
        FIMS_INFO_LOG("Setup prior for distribution " + fims::to_string(d->id));
        variable_map_iterator vmit;
        FIMS_INFO_LOG("Link prior from distribution " + fims::to_string(d->id) +
                      " to parameter " + fims::to_string(d->key[0]));
        d->priors.resize(d->key.size());
        for (size_t i = 0; i < d->key.size(); i++) {
          FIMS_INFO_LOG("Link prior from distribution " +
                        fims::to_string(d->id) + " to parameter " +
                        fims::to_string(d->key[0]));
          vmit = this->variable_map.find(d->key[i]);
          d->priors[i] = (*vmit).second;
        }
        FIMS_INFO_LOG("Prior size for distribution " + fims::to_string(d->id) +
                      "is: " + fims::to_string(d->observed_values.size()));
      }
    }
    this->SetupPriorLikelihoodTerms();
  }

  /**
   * @brief Loop over distributions and set links to distribution x value if
   * distribution is a random effects type.
   */
  void SetupRandomEffects() {
    for (density_components_iterator it = this->density_components.begin();
         it != this->density_components.end(); ++it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*it).second;
      if (d->input_type == "random_effects") {
        FIMS_INFO_LOG("Setup random effects for distribution " +
                      fims::to_string(d->id));
        variable_map_iterator vmit;
        FIMS_INFO_LOG("Link random effects from distribution " +
                      fims::to_string(d->id) + " to derived value " +
                      fims::to_string(d->key[0]));
        vmit = this->variable_map.find(d->key[0]);
        d->re = (*vmit).second;
        if (d->key.size() == 2) {
          vmit = this->variable_map.find(d->key[1]);
          d->re_expected_values = (*vmit).second;
        } else {
          d->re_expected_values = &d->expected_values;
        }
        FIMS_INFO_LOG("Random effect size for distribution " +
                      fims::to_string(d->id) +
                      " is: " + fims::to_string(d->observed_values.size()));
      }
    }
    this->SetupRandomEffectLikelihoodTerms();
  }

  /**
   * @brief Loop over distributions and set links to distribution expected value
   * if distribution is a data type.
   */
  void SetupData() {
    for (density_components_iterator it = this->density_components.begin();
         it != this->density_components.end(); ++it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*it).second;
      if (d->input_type == "data") {
        FIMS_INFO_LOG("Setup expected value for data distribution " +
                      fims::to_string(d->id));
        variable_map_iterator vmit;
        FIMS_INFO_LOG("Link expected value from distribution " +
                      fims::to_string(d->id) + " to derived value " +
                      fims::to_string(d->key[0]));
        vmit = this->variable_map.find(d->key[0]);
        d->data_expected_values = (*vmit).second;
        FIMS_INFO_LOG(
            "Expected value size for distribution " + fims::to_string(d->id) +
            " is: " + fims::to_string((*d->data_expected_values).size()));
      }
    }
    this->SetupDataLikelihoodTerms();
  }

  /**
   * @brief Set pointers to landings data in the fleet module.
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   * @param f shared pointer to fleet module
   */
  void SetFleetLandingsData(bool& valid_model,
                            std::shared_ptr<fims_popdy::Fleet<Type>> f) {
    if (f->fleet_observed_landings_data_id_m != static_cast<Type>(-999)) {
      uint32_t observed_landings_id =
          static_cast<uint32_t>(f->fleet_observed_landings_data_id_m);
      data_iterator it = this->data_objects.find(observed_landings_id);
      if (it != this->data_objects.end()) {
        f->observed_landings_data = (*it).second;
        FIMS_INFO_LOG("Landings data for fleet " + fims::to_string(f->id) +
                      " successfully set to " +
                      fims::to_string(f->observed_landings_data->at(1)));
      } else {
        valid_model = false;
        FIMS_ERROR_LOG("Expected landings data not defined for fleet " +
                       fims::to_string(f->id) + ", index " +
                       fims::to_string(observed_landings_id));
      }
    }
  }

  /**
   * @brief Set pointers to index data in the fleet module.
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   * @param f shared pointer to fleet module
   */
  void SetFleetIndexData(bool& valid_model,
                         std::shared_ptr<fims_popdy::Fleet<Type>> f) {
    if (f->fleet_observed_index_data_id_m != static_cast<Type>(-999)) {
      uint32_t observed_index_id =
          static_cast<uint32_t>(f->fleet_observed_index_data_id_m);
      data_iterator it = this->data_objects.find(observed_index_id);
      if (it != this->data_objects.end()) {
        f->observed_index_data = (*it).second;
        FIMS_INFO_LOG("Index data for fleet " + fims::to_string(f->id) +
                      " successfully set to " +
                      fims::to_string(f->observed_index_data->at(1)));
      } else {
        valid_model = false;
        FIMS_ERROR_LOG("Expected index data not defined for fleet " +
                       fims::to_string(f->id) + ", index " +
                       fims::to_string(observed_index_id));
      }
    }
  }

  /**
   * @brief Set pointers to age composition data in the fleet module.
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   * @param f shared pointer to fleet module
   */
  void SetAgeCompositionData(bool& valid_model,
                             std::shared_ptr<fims_popdy::Fleet<Type>> f) {
    if (f->fleet_observed_agecomp_data_id_m != static_cast<Type>(-999)) {
      uint32_t observed_agecomp_id =
          static_cast<uint32_t>(f->fleet_observed_agecomp_data_id_m);
      data_iterator it = this->data_objects.find(observed_agecomp_id);
      if (it != this->data_objects.end()) {
        f->observed_agecomp_data = (*it).second;
        FIMS_INFO_LOG("Observed input age-composition data for fleet " +
                      fims::to_string(f->id) + " successfully set to " +
                      fims::to_string(f->observed_agecomp_data->at(1)));
      } else {
        valid_model = false;
        FIMS_ERROR_LOG(
            "Expected age-composition observations not defined for fleet " +
            fims::to_string(f->id));
      }
    }
  }

  /**
   * @brief Set pointers to length composition data in the fleet module.
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   * @param f shared pointer to fleet module
   */
  void SetLengthCompositionData(bool& valid_model,
                                std::shared_ptr<fims_popdy::Fleet<Type>> f) {
    if (f->fleet_observed_lengthcomp_data_id_m != static_cast<Type>(-999)) {
      uint32_t observed_lengthcomp_id =
          static_cast<uint32_t>(f->fleet_observed_lengthcomp_data_id_m);
      data_iterator it = this->data_objects.find(observed_lengthcomp_id);
      if (it != this->data_objects.end()) {
        f->observed_lengthcomp_data = (*it).second;
        FIMS_INFO_LOG("Observed input length-composition data for fleet " +
                      fims::to_string(f->id) + " successfully set to " +
                      fims::to_string(f->observed_lengthcomp_data->at(1)));
      } else {
        valid_model = false;
        FIMS_ERROR_LOG(
            "Expected length-composition observations not defined for fleet " +
            fims::to_string(f->id));
      }
    }
  }

  /**
   * @brief Set pointers to the selectivity module referenced in the fleet
   * module.
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   * @param f shared pointer to fleet module
   */
  void SetFleetSelectivityModel(bool& valid_model,
                                std::shared_ptr<fims_popdy::Fleet<Type>> f) {
    if (f->fleet_selectivity_id_m != static_cast<Type>(-999)) {
      uint32_t sel_id = static_cast<uint32_t>(
          f->fleet_selectivity_id_m);  // cast as unsigned integer
      selectivity_models_iterator it = this->selectivity_models.find(
          sel_id);  // if find, set it, otherwise invalid

      if (it != this->selectivity_models.end()) {
        f->selectivity = (*it).second;  // elements in container held in pair
        FIMS_INFO_LOG("Selectivity model " +
                      fims::to_string(f->fleet_selectivity_id_m) +
                      " successfully set to fleet " + fims::to_string(f->id));
      } else {
        valid_model = false;
        FIMS_ERROR_LOG("Expected selectivity pattern not defined for fleet " +
                       fims::to_string(f->id) + ", selectivity pattern " +
                       fims::to_string(sel_id));
      }
    } else {
      FIMS_WARNING_LOG("Warning: No selectivity pattern defined for fleet " +
                       fims::to_string(f->id) +
                       ". FIMS requires selectivity be defined for all fleets "
                       "when running a catch at age model.");
    }
  }

  /**
   * @brief Set pointers to the recruitment module referenced in the population
   * module.
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   * @param p shared pointer to population module
   */
  void SetRecruitment(bool& valid_model,
                      std::shared_ptr<fims_popdy::Population<Type>> p) {
    if (p->recruitment_id != static_cast<Type>(-999)) {
      uint32_t recruitment_uint = static_cast<uint32_t>(p->recruitment_id);

      recruitment_models_iterator it =
          this->recruitment_models.find(recruitment_uint);

      if (it != this->recruitment_models.end()) {
        p->recruitment = (*it).second;  // recruitment defined in population.hpp
        FIMS_INFO_LOG("Recruitment model " + fims::to_string(recruitment_uint) +
                      " successfully set to population " +
                      fims::to_string(p->id));
      } else {
        valid_model = false;
        FIMS_ERROR_LOG(
            "Expected recruitment function not defined for "
            "population " +
            fims::to_string(p->id) + ", recruitment function " +
            fims::to_string(recruitment_uint));
      }
    } else {
      FIMS_WARNING_LOG(
          "No recruitment function defined for population " +
          fims::to_string(p->id) +
          ". FIMS requires recruitment functions be defined for all "
          "populations when running a catch at age model.");
    }
  }

  /**
   * @brief Set pointers to the recruitment process module referenced in the
   * population module.
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   * @param p shared pointer to population module
   */
  void SetRecruitmentProcess(bool& valid_model,
                             std::shared_ptr<fims_popdy::Population<Type>> p) {
    std::shared_ptr<fims_popdy::RecruitmentBase<Type>> r = p->recruitment;
    // if recruitment is defined
    if (r) {
      if (r->process_id != static_cast<Type>(-999)) {
        uint32_t process_uint = static_cast<uint32_t>(r->process_id);
        recruitment_process_iterator it =
            this->recruitment_process_models.find(process_uint);

        if (it != this->recruitment_process_models.end()) {
          r->process = (*it).second;  // recruitment process
          FIMS_INFO_LOG(
              "Recruitment Process model " + fims::to_string(process_uint) +
              " successfully set to population " + fims::to_string(p->id));
          (*it).second->recruitment = r;
        } else {
          valid_model = false;
          FIMS_ERROR_LOG(
              "Expected recruitment process function not defined for "
              "population " +
              fims::to_string(p->id) + ", recruitment process function " +
              fims::to_string(process_uint));
        }
      } else {
        FIMS_WARNING_LOG(
            "No recruitment process function defined for population " +
            fims::to_string(p->id) +
            ". FIMS requires recruitment process functions be defined for all "
            "recruitments when running a catch at age model.");
      }
    }
  }

  /**
   * @brief Set pointers to the growth module referenced in the population
   * module.
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   * @param p shared pointer to population module
   */
  void SetGrowth(bool& valid_model,
                 std::shared_ptr<fims_popdy::Population<Type>> p) {
    if (p->growth_id != static_cast<Type>(-999)) {
      uint32_t growth_uint = static_cast<uint32_t>(p->growth_id);
      growth_models_iterator it = this->growth_models.find(
          growth_uint);  // growth_models is specified in information.hpp
      // and used in rcpp
      // at the head of information.hpp; are the
      // dimensions of ages defined in rcpp or where?
      if (it != this->growth_models.end()) {
        p->growth =
            (*it).second;  // growth defined in population.hpp (the object
        // is called p, growth is within p)
        FIMS_INFO_LOG("Growth model " + fims::to_string(growth_uint) +
                      " successfully set to population " +
                      fims::to_string(p->id));
      } else {
        valid_model = false;
        FIMS_ERROR_LOG("Expected growth function not defined for population " +
                       fims::to_string(p->id) + ", growth function " +
                       fims::to_string(growth_uint));
      }
    } else {
      FIMS_WARNING_LOG("Growth function undefined for population " +
                       fims::to_string(p->id) +
                       ". FIMS requires growth functions be defined for all "
                       "populations when running a catch at age model.");
    }
  }

  /**
   * @brief Set pointers to the maturity module referenced in the population
   * module.
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   * @param p shared pointer to population module
   */
  void SetMaturity(bool& valid_model,
                   std::shared_ptr<fims_popdy::Population<Type>> p) {
    if (p->maturity_id != static_cast<Type>(-999)) {
      uint32_t maturity_uint = static_cast<uint32_t>(p->maturity_id);
      maturity_models_iterator it = this->maturity_models.find(
          maturity_uint);  // >maturity_models is specified in
      // information.hpp and used in rcpp
      if (it != this->maturity_models.end()) {
        p->maturity = (*it).second;  // >maturity defined in population.hpp
        FIMS_INFO_LOG("Maturity model " + fims::to_string(maturity_uint) +
                      " successfully set to population " +
                      fims::to_string(p->id));
      } else {
        valid_model = false;
        FIMS_ERROR_LOG(
            "Expected maturity function not defined for population " +
            fims::to_string(p->id) + ", maturity function " +
            fims::to_string(maturity_uint));
      }
    } else {
      FIMS_WARNING_LOG("Maturity function undefined for population " +
                       fims::to_string(p->id) +
                       ". FIMS requires maturity functions be defined for all "
                       "populations when running a catch at age model.");
    }
  }

  /**
   * @brief Loop over all fleets and set pointers to fleet objects
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   */
  void CreateFleetObjects(bool& valid_model) {
    for (fleet_iterator it = this->fleets.begin(); it != this->fleets.end();
         ++it) {
      std::shared_ptr<fims_popdy::Fleet<Type>> f = (*it).second;
      FIMS_INFO_LOG("Initializing fleet " + fims::to_string(f->id));

      SetFleetLandingsData(valid_model, f);

      SetFleetIndexData(valid_model, f);

      SetAgeCompositionData(valid_model, f);

      SetLengthCompositionData(valid_model, f);

      SetFleetSelectivityModel(valid_model, f);
    }
  }

  /**
   * @brief Loop over all density components and set pointers to data objects
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   */
  void SetDataObjects(bool& valid_model) {
    for (density_components_iterator it = this->density_components.begin();
         it != this->density_components.end(); ++it) {
      std::shared_ptr<fims_distributions::DensityComponentBase<Type>> d =
          (*it).second;

      // set data objects if distribution is a data type
      if (d->input_type == "data") {
        if (d->observed_data_id_m != static_cast<Type>(-999)) {
          uint32_t observed_data_id =
              static_cast<uint32_t>(d->observed_data_id_m);
          data_iterator it = this->data_objects.find(observed_data_id);

          if (it != this->data_objects.end()) {
            d->data_observed_values = (*it).second;
            FIMS_INFO_LOG("Observed data " + fims::to_string(observed_data_id) +
                          " successfully set to density component " +
                          fims::to_string(d->id));
          } else {
            valid_model = false;
            FIMS_ERROR_LOG(
                "Expected data observations not defined for density "
                "component " +
                fims::to_string(d->id) + ", observed data " +
                fims::to_string(observed_data_id));
          }
        } else {
          valid_model = false;
          FIMS_ERROR_LOG("No data input for density component" +
                         fims::to_string(d->id));
        }
      }
    }
  }

  /**
   * @brief Loop over all populations and set pointers to population objects
   *
   * @param &valid_model reference to true/false boolean indicating whether
   * model is valid.
   */
  void CreatePopulationObjects(bool& valid_model) {
    for (population_iterator it = this->populations.begin();
         it != this->populations.end(); ++it) {
      std::shared_ptr<fims_popdy::Population<Type>> p = (*it).second;

      FIMS_INFO_LOG("Initializing population " + fims::to_string(p->id));
      // check if population has fleets
      typename std::set<uint32_t>::iterator fleet_ids_it;

      for (fleet_ids_it = p->fleet_ids.begin();
           fleet_ids_it != p->fleet_ids.end(); ++fleet_ids_it) {
        // error check and set population elements
        // check me - add another fleet iterator to push information from
        //  for (fleet_iterator it = this->fleets.begin(); it !=
        //  this->fleets.end();
        //        ++it) {

        fleet_iterator it = this->fleets.find(*fleet_ids_it);

        if (it != this->fleets.end()) {
          // Initialize fleet object
          std::shared_ptr<fims_popdy::Fleet<Type>> f = (*it).second;
          // population to the individual fleets This is to pass catch at age
          // from population to fleets?
          // any shared member in p (population is pushed into fleets)
          p->fleets.push_back(f);
        } else {
          valid_model = false;
          FIMS_ERROR_LOG("Fleet \"" + fims::to_string(*fleet_ids_it) +
                         "\" undefined, not found for Population \"" +
                         fims::to_string(p->id) + "\". ");
        }
      }

      // set information dimensions
      this->n_years = std::max(this->n_years, p->n_years);
      this->n_ages = std::max(this->n_ages, p->n_ages);

      SetRecruitment(valid_model, p);

      SetRecruitmentProcess(valid_model, p);

      SetGrowth(valid_model, p);

      SetMaturity(valid_model, p);
    }
  }

  /**
   * @brief Loop over all models and set pointers to population objects
   */
  void CreateModelingObjects(bool& valid_model) {
    for (model_map_iterator it = this->models_map.begin();
         it != this->models_map.end(); ++it) {
      std::shared_ptr<fims_popdy::FisheryModelBase<Type>>& model = (*it).second;
      std::set<uint32_t>::iterator jt;

      for (jt = model->population_ids.begin();
           jt != model->population_ids.end(); ++jt) {
        population_iterator pt = this->populations.find((*jt));

        if (pt != this->populations.end()) {
          std::shared_ptr<fims_popdy::Population<Type>> p = (*pt).second;
          model->populations.push_back(p);
          for (size_t i = 0; i < p->fleets.size(); i++) {
            uint32_t local_fleet_id = p->fleets[i]->GetId();
            model->fleets[local_fleet_id] = p->fleets[i];
            FIMS_INFO_LOG(std::string("Linked fleet id ") +
                          fims::to_string(local_fleet_id) +
                          std::string(" into model id ") +
                          fims::to_string(model->GetId()));
          }
        } else {
          valid_model = false;
          FIMS_ERROR_LOG("No population object defined for model " +
                         fims::to_string(model->GetId()));
        }
      }
      model->Initialize();
    }
  }

  /**
   * @brief Create the generalized stock assessment model that will evaluate the
   * objective function. Does error checking to make sure the program has
   * all necessary components for the model and that they're in the right
   * dimensions. This sets up pointers to all memory objects and initializes
   * fleet and population objects.
   *
   * @return True if valid model, False if invalid model, check fims.log for
   * errors.
   */
  bool CreateModel() {
    FIMS_INFO_LOG("Creating model and checking for required components...");
    bool valid_model = true;

    CreateFleetObjects(valid_model);

    SetDataObjects(valid_model);

    CreatePopulationObjects(valid_model);

    CreateModelingObjects(valid_model);

    // setup priors, random effect, and data density components
    SetupPriors();
    SetupRandomEffects();
    SetupData();

    if (valid_model) {
      FIMS_INFO_LOG("Model successfully created.");
    } else {
      FIMS_ERROR_LOG("Model creation failed.");
    }

    return valid_model;
  }

  /**
   * @brief Get the Nages object
   *
   * @return size_t
   */
  size_t GetNages() const { return n_ages; }

  /**
   * @brief Set the Nages object
   *
   * @param n_ages
   */
  void SetNages(size_t n_ages) { this->n_ages = n_ages; }

  /**
   * @brief Get the n_years object
   *
   * @return size_t
   */
  size_t GetNyears() const { return n_years; }

  /**
   * @brief Set the n_years object
   *
   * @param n_years
   */
  void SetNyears(size_t n_years) { this->n_years = n_years; }

  /**
   * @brief Get the Parameters object
   *
   * @return std::vector<Type*>&
   */
  std::vector<Type*>& GetParameters() { return parameters; }

  /**
   * @brief Get the Fixed Effects Parameters object
   *
   * @return std::vector<Type*>&
   */
  std::vector<Type*>& GetFixedEffectsParameters() {
    return fixed_effects_parameters;
  }

  /**
   * @brief Get the Random Effects Parameters object
   *
   * @return std::vector<Type*>&
   */
  std::vector<Type*>& GetRandomEffectsParameters() {
    return random_effects_parameters;
  }

  /**
   * @brief Checks to make sure all required modules are present for specified
   * model
   *
   * @return True if valid model, False if invalid model, check fims.log for
   * errors.
   */
  bool CheckModel() {
    bool valid_model = true;
    for (model_map_iterator it = this->models_map.begin();
         it != this->models_map.end(); ++it) {
      std::shared_ptr<fims_popdy::FisheryModelBase<Type>>& model = (*it).second;
      std::set<uint32_t>::iterator jt;

      for (jt = model->population_ids.begin();
           jt != model->population_ids.end(); ++jt) {
        population_iterator pt = this->populations.find((*jt));

        if (pt != this->populations.end()) {
          std::shared_ptr<fims_popdy::Population<Type>> p = (*pt).second;

          if (model->model_type_m == "caa") {
            typename std::set<uint32_t>::iterator fleet_ids_it;
            for (fleet_ids_it = p->fleet_ids.begin();
                 fleet_ids_it != p->fleet_ids.end(); ++fleet_ids_it) {
              fleet_iterator it = this->fleets.find(*fleet_ids_it);

              if (it != this->fleets.end()) {
                // Initialize fleet object
                std::shared_ptr<fims_popdy::Fleet<Type>> f = (*it).second;

                if (f->fleet_selectivity_id_m == static_cast<Type>(-999)) {
                  valid_model = false;
                  FIMS_ERROR_LOG(
                      "No selectivity pattern defined for fleet " +
                      fims::to_string(f->id) +
                      ". FIMS requires selectivity be defined for all fleets "
                      "when running a catch at age model.");
                }
              }
            }

            if (p->recruitment_id == static_cast<Type>(-999)) {
              valid_model = false;
              FIMS_ERROR_LOG(
                  "No recruitment function defined for population " +
                  fims::to_string(p->id) +
                  ". FIMS requires recruitment functions be defined for all "
                  "populations when running a catch at age model.");
            }

            std::shared_ptr<fims_popdy::RecruitmentBase<Type>> r =
                p->recruitment;
            r = p->recruitment;
            if (r->process_id == static_cast<Type>(-999)) {
              valid_model = false;
              FIMS_ERROR_LOG(
                  "No recruitment process function defined for population " +
                  fims::to_string(p->id) +
                  ". FIMS requires recruitment process functions be defined "
                  "for all "
                  "recruitments when running a catch at age model.");
            }

            if (p->growth_id == static_cast<Type>(-999)) {
              valid_model = false;
              FIMS_ERROR_LOG(
                  "No growth function defined for population " +
                  fims::to_string(p->id) +
                  ". FIMS requires growth functions be defined for all "
                  "populations when running a catch at age model.");
            }

            if (p->maturity_id == static_cast<Type>(-999)) {
              valid_model = false;

              FIMS_WARNING_LOG(
                  "No maturity function defined for population " +
                  fims::to_string(p->id) +
                  ". FIMS requires maturity functions be defined for all "
                  "populations when running a catch at age model.");
            }
          }
        }
      }
    }
    return valid_model;
  }
};

template <typename Type>
std::shared_ptr<Information<Type>> Information<Type>::fims_information =
    nullptr;  // singleton instance

}  // namespace fims_info

#endif /* FIMS_COMMON_INFORMATION_HPP */
