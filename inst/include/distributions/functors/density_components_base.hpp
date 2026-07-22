
/**
 * @file density_components_base.hpp
 * @brief Declares the DensityComponentBase class, which is the base class for
 * all distribution functors.
 * @details This header defines two core templates used by concrete
 * distribution functors (e.g., NormalLPDF, LogNormalLPDF, MultinomialLPMF):
 * DistributionElementObject, which stores and resolves observed/expected
 * inputs, and DensityComponentBase, which adds shared bookkeeping for
 * log-likelihood contributions and simulation/reporting behavior.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef DENSITY_COMPONENT_BASE_HPP
#define DENSITY_COMPONENT_BASE_HPP

#include "../../common/variable_object.hpp"
#include "../../common/model_object.hpp"
#include "../../interface/interface.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/fims_math.hpp"

namespace fims_distributions {


enum class Density_Function {
    NORMAL,
    LOGNORMAL,
    MULTINOMIAL
};

enum class Likelihood_Kind {
    DATA,
    RANDOM_EFFECT,
    PRIOR,
    PENALTY
};

/** @brief Base class for all module_name functors.
 *
 * @tparam Type The type of the module_name functor.
 *
 */
template <typename Type>
struct DensityComponentBase : public fims_model_object::FIMSObject<Type> {
  // id_g is the ID of the instance of the DensityComponentBase class.
  // this is like a memory tracker.
  // Assigning each one its own ID is a way to keep track of
  // all the instances of the DensityComponentBase class.
  /**
   * @brief Global unique identifier for distribution modules.
   */
  static uint32_t id_g;

  /**
   * @brief Local id of this likelihood component.
   *
   */
  Type id;

  /**
   * @brief Total log probability density contribution of the distribution.
   *
   */
  Type lpdf;

  /**
   * @brief Vector storing observation-level log-likelihood contributions.
   */
  fims::Vector<Type> lpdf_vec;

  /**
   * @brief Boolean; if true, one-step-ahead (OSA) residuals are calculated.
   */
  bool osa_flag = false;

  /**
   * @brief Boolean; if true, data are simulated from the distribution.
   */
  bool simulate_flag = false;

  /**
   * @brief Classification of the input pathway for this distribution object.
   * Options used by accessor methods are, "data", "random_effects", 
   *"prior", and "penalty".
   */
  Likelihood_Kind likelihood_type;

  /**
   * @brief Classification of the input pathway for this distribution object.
   * Options used by accessor methods are, "prior", "random_effects", and
   *"data".
   */
  Density_Function distribution_type;

  /** @brief Fixed observed values for the distribution function. */
  fims::Vector<Type> observed_values;
  
   /** @brief Vector of pointers to observed value ids. */
  fims::Vector<size_t> observed_subvector;
  
   /** @brief Vector of pointers to observed value ids. */
  std::shared_ptr<fims::Vector<Type>> observed_pointer;

  /**
   * @brief Unique ID for variable map that points to a fims::Vector used to 
   * specify the observed values for likelihood component.
   */
  std::vector<uint32_t> observed_key;

  /** @brief Fixed expected values for the distribution function. */
  fims::Vector<Type> expected_values;
  
  /** @brief Vector of pointers to expected value ids. */
  fims::Vector<size_t> expected_subvector;
  
  /** @brief Vector of pointers to expected value ids. */
  std::shared_ptr<fims::Vector<Type>> expected_pointer;

  /**
   * @brief Unique ID for variable map that points to a fims::Vector used to 
   * specify the expected values for likelihood component.
   */
  std::vector<uint32_t> expected_key;

  /** @brief Fixed uncertainty values for the distribution function. */
  fims::Vector<Type> uncertainty_values;
  
   /** @brief Vector of pointers to uncertainty value ids. */
  fims::Vector<size_t> uncertainty_subvector;
  
   /** @brief Vector of pointers to uncertainty value ids. */
  std::shared_ptr<fims::Vector<Type>> uncertainty_pointer;
  
  /**
   * @brief Unique ID for variable map that points to a fims::Vector used to 
   * specify the uncertainty values for likelihood component.
   */
  std::vector<uint32_t> uncertainty_key;

  /** @brief Vector of lambda values to weight nll contributions. */
  fims::Vector<Type> lambda_values;

  /** @brief Vector of dimension sizes to structure the distribution. */
  fims::Vector<size_t> dims;

  /** @brief NA value for specifiying missing values. */
  Type na_value = static_cast<Type>(-999); 

  /**
   * @brief Retrieve one observed value.
   * @param i Index into the active observed source, e.g., vector or pointer.
   * @return Reference to the selected observed value.
   */
  inline Type& get_observed(size_t i) {
    if(observed_subvector.size() != 0){
      return (*observed_pointer).get_force_scalar(observed_subvector.get_force_scalar(i));
    }else{
      return (*observed_pointer).get_force_scalar(i);
    }
  }

  /**
   * @brief Update one observed value.
   * @param i Index into the active observed source, e.g., vector or pointer.
   */
  inline void set_observed(size_t i, Type new_value) {
    if(observed_subvector.size() != 0){
      (*observed_pointer).get_force_scalar(observed_subvector.get_force_scalar(i)) = new_value;
    }else{
      (*observed_pointer).get_force_scalar(i) = new_value;
    }
  }

  /**
   * @brief Update multiple observed values.
   * @param i Index into the active observed source, e.g., vector or pointer.
   */
  inline void set_observed(std::vector<size_t> i, fims::Vector<Type> new_value) {
    for(size_t j = 0; j < i.size(); j++){
      if(observed_subvector.size() != 0){
        (*observed_pointer).get_force_scalar(observed_subvector.get_force_scalar(i[j])) = new_value.get_force_scalar(j);
      }else{
        (*observed_pointer).get_force_scalar(i[j]) = new_value.get_force_scalar(j);
      }
    }
  }

  /**
   * @brief Retrieve one expected value.
   * @param i Index into the active expected source, e.g., vector or pointer.
   * @return Reference to the selected expected value.
   */
  inline Type& get_expected(size_t i) {
    if(expected_subvector.size() != 0){
      return (*expected_pointer).get_force_scalar(expected_subvector.get_force_scalar(i));
    }else{
      return (*expected_pointer).get_force_scalar(i);
    }
  }

  /**
   * @brief Retrieve one uncertainty value.
   * @param i Index into the active uncertainty source, e.g., vector or pointer.
   * @return Reference to the selected uncertainty value.
   */
  inline Type& get_uncertainty(size_t i) {
    if(uncertainty_subvector.size() != 0){
      return (*uncertainty_pointer).get_force_scalar(uncertainty_subvector.get_force_scalar(i));
    }else{
      return (*uncertainty_pointer).get_force_scalar(i);
    }
  }

  /**
   * @brief Retrieve one lambda value.
   * @param i Index into the active lambda vector.
   * @return Reference to the selected lambda value.
   */
  inline Type& get_lambda(size_t i) {
    return lambda_values.get_force_scalar(i);
  }
  
  /**
   * @brief Get length of the active observed input vector.
   * @return Size of the observed input.
   */
  inline size_t get_n_observed() {
    if(observed_subvector.size() != 0){
      return observed_subvector.size();
    }else{
      return (*observed_pointer).size();
    }
  }
  
  /**
   * @brief Get length of the active expected input vector.
   * @return Size of the expected input.
   */
  inline size_t get_n_expected() {
    if(expected_subvector.size() != 0){
      return expected_subvector.size();
    }else{
      return (*expected_pointer).size();
    }
  }
  
  /**
   * @brief Get length of the active uncertainty input vector.
   * @return Size of the uncertainty input.
   */
  inline size_t get_n_uncertainty() {
    if(uncertainty_subvector.size() != 0){
      return uncertainty_subvector.size();
    }else{
      return (*uncertainty_pointer).size();
    }
  }
  
  /**
   * @brief Get length of the active lambda input vector.
   * @return Size of the lambda input.
   */
  inline size_t get_n_lambda() {
    return lambda_values.size();
  }

  /**
   * @brief Check the sizes of the active input vectors.
   * @return matching size of all input vectors.
   */
  inline size_t check_input_sizes() {
    
    if(this->dims.size() != 0) {
      if(this->dims.size() != 2) {
        throw std::invalid_argument(
          "Input size mismatch. The dims vector is of size " +
          std::to_string(this->dims.size()) + " but should be of size 2.");
      }
      if(this->get_n_observed() > 1 && this->get_n_observed() != this->dims[0] * this->dims[1]) {
        throw std::invalid_argument(
          "Input size mismatch. The observed vector is of size " +
          std::to_string(this->get_n_observed()) + " but should be of size " +
          std::to_string(this->dims[0] * this->dims[1]) + " based on the dims vector.");
      }
      if(this->get_n_expected() > 1 && this->get_n_expected() != this->dims[0] * this->dims[1]) {
        throw std::invalid_argument(
          "Input size mismatch. The expected vector is of size " +
          std::to_string(this->get_n_expected()) + " but should be of size " +
          std::to_string(this->dims[0] * this->dims[1]) + " based on the dims vector.");
      }
      if(this->get_n_uncertainty() > 1 && this->get_n_uncertainty() != this->dims[0] * this->dims[1]) {
        throw std::invalid_argument(
          "Input size mismatch. The uncertainty vector is of size " +
          std::to_string(this->get_n_uncertainty()) + " but should be of size " +
          std::to_string(this->dims[0] * this->dims[1]) + " based on the dims vector.");
      }
      if(this->get_n_lambda() > 1 && this->get_n_lambda() != this->dims[0]) {
        throw std::invalid_argument(
          "Input size mismatch. The lambda vector is of size " +
          std::to_string(this->get_n_lambda()) + " but should be of size " +
          std::to_string(this->dims[0]) + " based on the dims vector ("+
          std::to_string(this->dims[0]) +","+
          std::to_string(this->dims[1]) +").");
      } 
        
      return dims[0] * dims[1];
      
    }else{
      // First throw an error if observed and expected vectors are both greater 
      // than 1 and not the same size
      if (this->get_n_observed() > 1 && this->get_n_expected() > 1 &&
        this->get_n_observed() != this->get_n_expected()) {
        throw std::invalid_argument(
            "Input size mismatch. The observed vector is of size " +
            std::to_string(this->get_n_observed()) + " and the expected vector is of size " +
            std::to_string(this->get_n_expected()));
      }

      // Then throw an error if uncertainty vector is greater than 1 and not the 
      // same size as expected
      if(this->get_n_uncertainty() > 1 && this->get_n_expected() > 1 &&
        this->get_n_uncertainty() != this->get_n_expected()) {
        throw std::invalid_argument(
            "Input size mismatch. The uncertainty vector is of size " +
            std::to_string(this->get_n_uncertainty()) + " and the expected vector is of size " +
            std::to_string(this->get_n_expected()));
      }

      // Then throw an error if uncertainty vector is greater than 1 and not the 
      // same size as observed
      if(this->get_n_uncertainty() > 1 && this->get_n_observed() > 1 &&
        this->get_n_uncertainty() != this->get_n_observed()) {
        throw std::invalid_argument(
            "Input size mismatch. The uncertainty vector is of size " +
            std::to_string(this->get_n_uncertainty()) + " and the observed vector is of size " +
            std::to_string(this->get_n_observed()));
      }

      // Then throw an error if lambda vector is greater than 1 and not the 
      // same size as observed
      if(this->get_n_lambda() > 1 && this->get_n_observed() > 1 &&
        this->get_n_lambda() != this->get_n_observed()) {
        throw std::invalid_argument(
            "Input size mismatch. The lambda vector is of size " +
            std::to_string(this->get_n_lambda()) + " and the observed vector is of size " +
            std::to_string(this->get_n_observed()));
      }

      // Then throw an error if lambda vector is greater than 1 and not the 
      // same size as expected
      if(this->get_n_lambda() > 1 && this->get_n_expected() > 1 &&
        this->get_n_lambda() != this->get_n_expected()) {
        throw std::invalid_argument(
            "Input size mismatch. The lambda vector is of size " +
            std::to_string(this->get_n_lambda()) + " and the expected vector is of size " +
            std::to_string(this->get_n_expected()));
      }

      // Then throw an error if lambda vector is greater than 1 and not the 
      // same size as uncertainty
      if(this->get_n_lambda() > 1 && this->get_n_uncertainty() > 1 &&
        this->get_n_lambda() != this->get_n_uncertainty()) {
        throw std::invalid_argument(
            "Input size mismatch. The lambda vector is of size " +
            std::to_string(this->get_n_lambda()) + " and the uncertainty vector is of size " +
            std::to_string(this->get_n_uncertainty()));
      }

      // Finally return the largest vector size among the active inputs for use 
      // in lpdf calculations. This allows for recycling of inputs when one 
      // vector is of length 1.
      if(this->get_n_observed() >= this->get_n_expected() && this->get_n_observed() >= this->get_n_uncertainty() && this->get_n_observed() >= this->get_n_lambda()) {
        return this->get_n_observed();
      }else if(this->get_n_expected() >= this->get_n_observed() && this->get_n_expected() >= this->get_n_uncertainty() && this->get_n_expected() >= this->get_n_lambda()) {
        return this->get_n_expected();
      }else if(this->get_n_uncertainty() >= this->get_n_observed() && this->get_n_uncertainty() >= this->get_n_expected() && this->get_n_uncertainty() >= this->get_n_lambda()) {
        return this->get_n_uncertainty();
      }else {
        return this->get_n_lambda();
      }
    }
  }

#ifdef TMB_MODEL
  /**
   * @brief Pointer to the TMB objective function.
   */
  ::objective_function<Type>* of;
#endif

  /**
   * @brief Constructor, which initializes default prior pointer state and ID.
   */
  DensityComponentBase() {
    this->id = DensityComponentBase::id_g++;
    this->register_self(this->id);
    this->observed_pointer = std::make_shared<fims::Vector<Type>>(this->observed_values);
    this->expected_pointer = std::make_shared<fims::Vector<Type>>(this->expected_values);
    this->uncertainty_pointer = std::make_shared<fims::Vector<Type>>(this->uncertainty_values);
  }

  virtual ~DensityComponentBase() {}
  
  /**
   * @brief Evaluate the distribution-specific log-likelihood contribution.
   * @return Total log-likelihood contribution for the active inputs.
   */
  virtual const Type evaluate() {
    
    // extract evaluation vector size based on inputs 
    size_t n_evals = this->check_input_sizes();

    switch (this->distribution_type) {
      case Density_Function::LOGNORMAL:
        if (this->dims.size() != 0) {
          throw std::invalid_argument(
              "Dimensions specified for lognormal distribution, but lognormal distribution is not multivariate. Remove dimensions to evaluate.");
        }
        break;
      case Density_Function::NORMAL:
        if (this->dims.size() != 0) {
          throw std::invalid_argument(
              "Dimensions specified for normal distribution, but normal distribution is not multivariate. Remove dimensions to evaluate.");
        }
        break;
      case Density_Function::MULTINOMIAL:
        if (this->dims.size() != 2) {
          throw std::invalid_argument(
              "No dimensions specified for multinomial distribution");
        }
        break;
      default:
        throw std::invalid_argument(
            "Distribution type not recognized in evaluate() method.");
    }
    
    if(this->dims.size() == 0) {
      this->dims.resize(2);
      // could add an OSA residuls check here to see if dims should be set to 
      // 1 x n_evals or n_evals x 1
      this->dims[0] = 1;
      this->dims[1] = n_evals;
    }

    // setup vector for recording the log probability density function values
    this->lpdf_vec.resize(dims[0] * dims[1]);

    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(),
              static_cast<Type>(0));
    
    this->lpdf = static_cast<Type>(0);

    size_t lpdf_vec_idx = 0; /**< index for lpdf_vec vector */
    
    // Create new observed_values and prob vectors
    fims::Vector<Type> values_vector_observed;
    fims::Vector<Type> values_vector_expected;
    fims::Vector<Type> values_vector_uncertainty;
    std::vector<size_t> values_vector_index;
    fims::Vector<Type> values_vector_lambda;

    bool containsNA;
    size_t idx;
    for (size_t i = 0; i < dims[0]; i++) {

      values_vector_observed.reserve(dims[1]);
      values_vector_expected.reserve(dims[1]);
      values_vector_uncertainty.reserve(dims[1]);
      values_vector_index.reserve(dims[1]);
      values_vector_lambda.reserve(dims[1]);

      containsNA = false;

      for (size_t j = 0; j < dims[1]; j++) {
        
        idx = (i * dims[1]) + j;
        
        if (this->get_observed(idx) != this->na_value &&
              this->get_expected(idx) != this->na_value &&
              this->get_uncertainty(idx) != this->na_value) {
          if(this->distribution_type == Density_Function::LOGNORMAL){
            values_vector_observed.push_back(fims_math::log(this->get_observed(idx)));
          }else{
            values_vector_observed.push_back(static_cast<Type>(this->get_observed(idx)));
          }
          values_vector_expected.push_back(static_cast<Type>(this->get_expected(idx))); 
          if(this->distribution_type == Density_Function::NORMAL || 
            this->distribution_type == Density_Function::LOGNORMAL) {
            values_vector_uncertainty.push_back(
              fims_math::exp(static_cast<Type>(this->get_uncertainty(idx))));
          }
          values_vector_index.push_back(static_cast<size_t>(idx));
          values_vector_lambda.push_back(static_cast<Type>(this->get_lambda(idx)));
        }else{
          containsNA = true;
        }
      }

#ifdef TMB_MODEL
      if (this->simulate_flag) {
        switch(this->distribution_type) {
          case Density_Function::LOGNORMAL:
            FIMS_SIMULATE_F(this->of) { // preprocessor definition in 
                                        // interface.hpp this simulates data 
                                        // that is mean biased
              for(size_t j = 0; j < values_vector_index.size(); j++) {
                this->set_observed(values_vector_index[j],
                fims_math::exp(rnorm(values_vector_expected[j], 
                values_vector_uncertainty[j])));
              }
              //this->set_observed(values_vector_index,
              //fims_math::exp(rnorm(values_vector_expected, 
              //values_vector_uncertainty)));
            }
            break;
          case Density_Function::NORMAL:
            FIMS_SIMULATE_F(this->of) {
              for(size_t j = 0; j < values_vector_index.size(); j++) {
                this->set_observed(values_vector_index[j],
                rnorm(values_vector_expected[j], 
                values_vector_uncertainty[j]));
              }
              //this->set_observed(values_vector_index,
              //rnorm(values_vector_expected,values_vector_uncertainty));
            }
            break;
          case Density_Function::MULTINOMIAL:
            FIMS_SIMULATE_F(this->of) { // preprocessor definition in 
                                        // interface.hpp this simulates data 
                                        // that is mean biased
              // this->set_observed(values_vector_index,
              // rmultinom(values_vector_expected));
            }
            break;
          default:
            throw std::invalid_argument(
                "Distribution type not recognized in evaluate() method.");
        }
      }else{
        switch(this->distribution_type) {
          case Density_Function::LOGNORMAL:
            // See Deroba and Miller, 2016
            // (https://doi.org/10.1016/j.fishres.2015.12.002) for the use of
            // lognormal constant for bias correction.
            for (size_t j = 0; j < values_vector_index.size(); j++) {
              this->lpdf_vec[values_vector_index[j]] = values_vector_lambda[j] *( dnorm(values_vector_observed[j], 
                  values_vector_expected[j],
                  values_vector_uncertainty[j], true)-
                  values_vector_observed[j]);
              this->lpdf += this->lpdf_vec[values_vector_index[j]];
            }
            break;
          case Density_Function::NORMAL:
           for (size_t j = 0; j < values_vector_index.size(); j++) {
              this->lpdf_vec[values_vector_index[j]] = values_vector_lambda[j] * (
              dnorm(values_vector_observed[j], 
                    values_vector_expected[j],
                    values_vector_uncertainty[j], true));
              this->lpdf += this->lpdf_vec[values_vector_index[j]];
            }
            //this->lpdf_vec[values_vector_index] = values_vector_lambda * (
            //dnorm(values_vector_observed.to_tmb(), 
            //      values_vector_expected.to_tmb(),
            //      values_vector_uncertainty.to_tmb(), true));
            break;
          case Density_Function::MULTINOMIAL:
            if(!containsNA) {
              this->lpdf_vec[values_vector_index[0]] =  
              (dmultinom(values_vector_observed.to_tmb(),
                         values_vector_expected.to_tmb(), true));

              for (size_t j = 1; j < values_vector_index.size(); j++) {
                this->lpdf_vec[values_vector_index[j]] = values_vector_lambda[j] * 
                this->lpdf_vec[values_vector_index[0]];
              }

              this->lpdf_vec[values_vector_index[0]] = values_vector_lambda[0] * 
              this->lpdf_vec[values_vector_index[0]];
              this->lpdf += this->lpdf_vec[values_vector_index[0]];
            }
            break;
          default:
            throw std::invalid_argument(
                "Distribution type not recognized in evaluate() method.");
        }
      }
#endif    
      values_vector_observed.resize(0);
      values_vector_expected.resize(0);
      values_vector_uncertainty.resize(0);
      values_vector_index.resize(0);
      values_vector_lambda.resize(0);
    }
    
    return (this->lpdf);
  }
};

/** @brief Default id of the singleton distribution class
 */
template <typename Type>
uint32_t DensityComponentBase<Type>::id_g = 0;

}  // namespace fims_distributions

#endif /* DENSITY_COMPONENT_BASE_HPP */
