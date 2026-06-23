
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

#include "../../common/data_object.hpp"
#include "../../common/model_object.hpp"
#include "../../interface/interface.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/fims_math.hpp"

namespace fims_distributions {

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
   * Options used by accessor methods are, "prior", "random_effects", and
   *"data".
   */
  std::string input_type;

  /**
   * @brief Likelihood distribution type for this distribution object.
   * Current options are "normal", "lognormal", and "multinomial".
   */
  std::string distribution_type;

  /** @brief Fixed observed values for the distribution function. */
  fims::Vector<Type> observed_values;
  
   /** @brief Vector of pointers to observed value ids. */
  fims::Vector<Type> observed_subvector = NULL;
  
   /** @brief Vector of pointers to observed value ids. */
  std::shared_ptr<fims::Vector<Type>> observed_pointer = &this->observed_values;

  /**
   * @brief Unique ID for variable map that points to a fims::Vector used to 
   * specify the observed values for likelihood component.
   */
  std::vector<uint32_t> observed_key = NULL;

  /** @brief Fixed expected values for the distribution function. */
  fims::Vector<Type> expected_values;
  
  /** @brief Vector of pointers to expected value ids. */
  fims::Vector<Type> expected_subvector = NULL;
  
  /** @brief Vector of pointers to expected value ids. */
  std::shared_ptr<fims::Vector<Type>> expected_pointer = &this->expected_values;

  /**
   * @brief Unique ID for variable map that points to a fims::Vector used to 
   * specify the expected values for likelihood component.
   */
  std::vector<uint32_t> expected_key = NULL;

  /** @brief Fixed uncertainty values for the distribution function. */
  fims::Vector<Type> uncertainty_values;
  
   /** @brief Vector of pointers to uncertainty value ids. */
  fims::Vector<Type> uncertainty_subvector = NULL;
  
   /** @brief Vector of pointers to uncertainty value ids. */
  std::shared_ptr<fims::Vector<Type>> uncertainty_pointer = &this->uncertainty_values;
  
  /**
   * @brief Unique ID for variable map that points to a fims::Vector used to 
   * specify the uncertainty values for likelihood component.
   */
  std::vector<uint32_t> uncertainty_key = NULL;

  /** @brief Vector of lambda values to weight nll contributions. */
  fims::Vector<Type> lambda_values = 1;

  /** @brief Vector of dimension sizes to structure the distribution. */
  fims::Vector<Type> dims = NULL;

  /** @brief NA value for specifiying missing values. */
  Type na_value = static_cast<Type>(-999); 

  /**
   * @brief Retrieve one observed value.
   * @param i Index into the active observed source, e.g., vector or pointer.
   * @return Reference to the selected observed value.
   * @throws std::runtime_error If input_type is "prior" and priors is empty.
   */
  inline Type& get_observed(size_t i) {
    if(observed_subvector != NULL){
      return (*observed_pointer).get_force_scalar(observed_subvector.get_force_scalar(i));
    }else{
      return (*observed_pointer).get_force_scalar(i);
    }
  }

  /**
   * @brief Update one observed value.
   * @param i Index into the active observed source, e.g., vector or pointer.
   * @throws std::runtime_error If input_type is "prior" and priors is empty.
   */
  inline void set_observed(size_t i, Type new_value) {
    if(observed_subvector != NULL){
      (*observed_pointer).get_force_scalar(observed_subvector.get_force_scalar(i)) = new_value;
    }else{
      (*observed_pointer).get_force_scalar(i) = new_value;
    }
  }

  /**
   * @brief Retrieve one expected value.
   * @param i Index into the active expected source, e.g., vector or pointer.
   * @return Reference to the selected expected value.
   */
  inline Type& get_expected(size_t i) {
    if(expected_subvector != NULL){
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
    if(uncertainty_subvector != NULL){
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
    if(observed_subvector != NULL){
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
    if(expected_subvector != NULL){
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
    if(uncertainty_subvector != NULL){
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
    
    if(this->dims != NULL) {
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
            std::to_string(n_observed) + " and the expected vector is of size " +
            std::to_string(n_expected));
      }

      // Then throw an error if uncertainty vector is greater than 1 and not the 
      // same size as expected
      if(this->get_n_uncertainty() > 1 && this->get_n_expected() > 1 &&
        this->get_n_uncertainty() != this->get_n_expected()) {
        throw std::invalid_argument(
            "Input size mismatch. The uncertainty vector is of size " +
            std::to_string(n_uncertainty) + " and the expected vector is of size " +
            std::to_string(n_expected));
      }

      // Then throw an error if uncertainty vector is greater than 1 and not the 
      // same size as observed
      if(this->get_n_uncertainty() > 1 && this->get_n_observed() > 1 &&
        this->get_n_uncertainty() != this->get_n_observed()) {
        throw std::invalid_argument(
            "Input size mismatch. The uncertainty vector is of size " +
            std::to_string(n_uncertainty) + " and the observed vector is of size " +
            std::to_string(n_observed));
      }

      // Then throw an error if lambda vector is greater than 1 and not the 
      // same size as observed
      if(this->get_n_lambda() > 1 && this->get_n_observed() > 1 &&
        this->get_n_lambda() != this->get_n_observed()) {
        throw std::invalid_argument(
            "Input size mismatch. The lambda vector is of size " +
            std::to_string(n_lambda) + " and the observed vector is of size " +
            std::to_string(n_observed));
      }

      // Then throw an error if lambda vector is greater than 1 and not the 
      // same size as expected
      if(this->get_n_lambda() > 1 && this->get_n_expected() > 1 &&
        this->get_n_lambda() != this->get_n_expected()) {
        throw std::invalid_argument(
            "Input size mismatch. The lambda vector is of size " +
            std::to_string(n_lambda) + " and the expected vector is of size " +
            std::to_string(n_expected));
      }

      // Then throw an error if lambda vector is greater than 1 and not the 
      // same size as uncertainty
      if(this->get_n_lambda() > 1 && this->get_n_uncertainty() > 1 &&
        this->get_n_lambda() != this->get_n_uncertainty()) {
        throw std::invalid_argument(
            "Input size mismatch. The lambda vector is of size " +
            std::to_string(n_lambda) + " and the uncertainty vector is of size " +
            std::to_string(n_uncertainty));
      }

      // Finally return the largest vector size among the active inputs for use 
      // in lpdf calculations. This allows for recycling of inputs when one 
      // vector is of length 1.
      if(this->get_n_observed() >= this->get_n_expected() && this->get_n_observed() >= this->get_n_uncertainty()) {
        return this->get_n_observed();
      }else if(this->get_n_expected() >= this->get_n_observed() && this->get_n_expected() >= this->get_n_uncertainty()) {
        return this->get_n_expected();
      }else {
        return this->get_n_uncertainty();
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
  }

  virtual ~DensityComponentBase() {}
  /**
   * @brief Evaluate the distribution-specific log-likelihood contribution.
   * @return Total log-likelihood contribution for the active inputs.
   */
  virtual const Type evaluate() = 0;
};

/** @brief Default id of the singleton distribution class
 */
template <typename Type>
uint32_t DensityComponentBase<Type>::id_g = 0;

}  // namespace fims_distributions

#endif /* DENSITY_COMPONENT_BASE_HPP */
