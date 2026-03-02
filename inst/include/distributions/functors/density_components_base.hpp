
/**
 * @file density_components_base.hpp
 * @brief Declares the DensityComponentBase class, which is the base class for
 * all distribution functors.
 * @details Defines guards for distributions module outline to define the
 * density_components_base hpp file if not already defined.
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
  static uint32_t
      id_g; /**< global unique identifier for distribution modules */

  // Input type is used to categorize likelihood contributions to order
  // calculations to ensure simulation and one-step-ahead residuals are calculated correctly.
  std::string input_type; /**< string classifies the type of the negative
                             log-likelihood; options are: "priors",
                             "random_effects", "data", and "process" */
  
  // values fields are use to store user specified values that may be constant
  // or estimated parameters.
  std::vector<fims::Vector<Type>> observed_values; /**< Observed values of distribution 
  function*/
  std::vector<fims::Vector<Type>> expected_values; /**< expected values of distribution 
  function*/
  std::vector<fims::Vector<Type>> uncertainty_values; /**< Uncertainty values of distribution 
  function*/
  
  // pointer fields are used to allow for the distribution functors to point to
  // internal model objects such as data objects or derived quantities. If a 
  // pointer is not specified it will point to the input values vectors above.
  std::vector<fims::Vector<Type>*> observed_pointer = NULL; /**< pointer to process observed 
  values*/
  std::vector<fims::Vector<Type>*> expected_pointer = NULL; /**< pointer to process expected 
  values*/
  std::vector<fims::Vector<Type>*> uncertainty_pointer = NULL; /**< pointer to process 
  uncertainty values*/
  
  // key fields are used to allow the user to input an ID key to a parameter or
  // derived quantity vector in the model that the distribution functor will 
  // point to. TODO: This needs to be updated from uint32_t to a type that
  // allows for string or enum keys as well to identify derived quantities.
  std::vector<uint32_t>
      observed_key; /**< unique id for variable map that points to 
      observed_pointer fims::Vector */
  std::vector<uint32_t>
      expected_key; /**< unique id for variable map that points to 
      expected_pointer fims::Vector */
  std::vector<uint32_t>
      uncertainty_key; /**< unique id for variable map that points to 
      uncertainty_pointer fims::Vector */
  
  // index fields are used to subset the observed, expected, and uncertainty 
  // values to specify which values are included in the distribution likelihood 
  // calculations. Will assume full vector is included if index vectors are not 
  // specified.
  std::vector<fims::Vector<Type>> observed_index; /**< Index of which observed values to 
  include in density likelihood calculations*/
  std::vector<fims::Vector<Type>> expected_index; /**< Index of which expected values to 
  include in density likelihood calculations*/
  std::vector<fims::Vector<Type>> uncertainty_index; /**< Index of which uncertainty values 
  to include in density likelihood calculations*/
  
  // lambda is used to weight the likelihood contributions of each observation 
  // in the distribution calculations. If lambda is not specified, all 
  // observations will be given a weight of 1. This is used for cases where the 
  // user may want to downweight certain observations/data sources in the 
  // likelihood calculations to explore model sensitivity.
  fims::Vector<Type> lambda = NULL; /**< process lambda likelihood weights*/

  fims::Vector<Type> lpdf_vec; /**< vector to record observation level negative
                                  log-likelihood values */

  Type lpdf = static_cast<Type>(0.0); /**< total log probability density
                                         contribution of the distribution */

  bool osa_flag = false; /**< Boolean; if true, osa residuals are calculated */
  
  bool simulate_flag =
      false; /**< Boolean; if true, data are simulated from the distribution */


  int observed_data_id_m = -999; /*!< id of observed data component*/
  fims::Vector<Type> expected_mean; /**< the expected mean of the distribution, 
  overrides expected values */
  std::string use_mean = fims::to_string("no"); /**< should expected_mean
                                          be used over expected values */
  std::shared_ptr<fims_data_object::DataObject<Type>>
      data_observed_values; /**< observed data*/
  fims::Vector<Type>* re = NULL; /**< pointer to random effects vector*/
  fims::Vector<Type>* re_expected_values =
      NULL; /**< expected value of random effects*/
  fims::Vector<Type>* data_expected_values = NULL; /**< expected value of data*/
  std::vector<fims::Vector<Type>*>
      priors; /**< vector of pointers where each points to a prior parameter */
  std::vector<uint32_t>
      key; /**< unique id for variable map that points to a fims::Vector */

#ifdef TMB_MODEL
  ::objective_function<Type>* of; /**< Pointer to the TMB objective function */
#endif

  /** 
   * @brief Constructor.
   */
  DensityComponentBase() {
    // initialize the priors vector with a size of 1 and set the first element
    // to NULL
    this->priors.resize(1);
    this->priors[0] = NULL;
    this->id = DensityComponentBase::id_g++;
  }

  virtual ~DensityComponentBase() {}
  /**
   * @brief Generic probability density function. Calculates the pdf at the
   * independent variable value.
   */
  virtual const Type evaluate() = 0;

  /**
   * Retrieve element from observed data set, random effect, or prior.
   * @param i index referencing vector or pointer
   * @return the reference to the value of the vector or pointer at position i
   */
  inline Type& get_observed(size_t i) {
    if (this->input_type == "data") {
      return data_observed_values->at(i);
    }
    if (this->input_type == "random_effects") {
      return (*re)[i];
    }
    if (this->input_type == "prior") {
      if (priors.size() == 0) {
        throw std::runtime_error("No priors defined for this distribution.");
      } else if (priors.size() == 1) {
        return (*(priors[0]))[i];
      } else if (priors.size() > 1) {
        return (*(priors[i]))[0];
      }
    }
    return observed_values[i];
  }

  /**
   * Retrieve element from observed data set, random effect, or prior.
   * @param i index referencing row
   * @param j index referencing column
   * @return the reference to the row and column at position i, j
   */
  inline Type& get_observed(size_t i, size_t j) {
    if (this->input_type == "data") {
      return data_observed_values->at(i, j);
    }
    if (this->input_type == "random_effects") {
      return (*re)[i, j];
    }
    if (this->input_type == "prior") {
      return (*(priors[i, j]))[0];
    }
    return observed_values[i];
  }

  /**
   * Retrieve expected element given data, random effect, or prior.
   * @param i index referencing vector or pointer
   * @return the reference to the value of the vector or pointer at position i
   */
  inline Type& get_expected(size_t i) {
    if (this->input_type == "data") {
      return (*data_expected_values)[i];
    } else if (this->use_mean == "yes") {
      return this->expected_mean.get_force_scalar(i);
    } else if (this->input_type == "random_effects") {
      return (*re_expected_values)[i];
    } else {
      return this->expected_values.get_force_scalar(i);
    }
  }

  /**
   * Retrieve expected element size given data, random effect, or prior.
   * @return The size of the element.
   */
  inline size_t get_n_x() {
    if (this->input_type == "data") {
      return this->data_observed_values->data.size();
    }
    if (this->input_type == "random_effects") {
      return (*re).size();
    }
    if (this->input_type == "prior") {
      return this->expected_values.size();
    }
    return observed_values.size();
  }

  /**
   * Retrieve expected value element size given data, random effect, or prior.
   * @return The size of the element.
   */
  inline size_t get_n_expected() {
    if (this->input_type == "data") {
      return (*data_expected_values).size();
    }
    if (this->input_type == "random_effects") {
      return (*re_expected_values).size();
    }
    if (this->input_type == "prior") {
      return this->expected_values.size();
    }
    return observed_values.size();
  }
};

/** @brief Default id of the singleton distribution class
 */
template <typename Type>
uint32_t DensityComponentBase<Type>::id_g = 0;

}  // namespace fims_distributions

#endif /* DENSITY_COMPONENT_BASE_HPP */
