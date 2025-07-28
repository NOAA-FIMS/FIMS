
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

/**
 * Container to hold density components including pointers to density inputs.
 */
template <typename Type>
struct DistributionElementObject {
  std::string input_type; /**< string classifies the type of the negative
                             log-likelihood; options are: "extended_prior", "priors",
                             "random_effects", and "data" */
  std::shared_ptr<fims_data_object::DataObject<Type>>
      observed_values; /**< observed data*/
  fims::Vector<Type>
      expected_values;           /**< expected value of distribution function*/
  fims::Vector<Type>* re = NULL; /**< pointer to random effects vector*/
  fims::Vector<Type>* re_expected_values =
      NULL; /**< expected value of random effects*/
  std::vector<fims::Vector<Type>*>
      priors; /**< vector of pointers where each points to a prior parameter */
  std::vector<fims::Vector<Type>*>
      prior_observed = NULL; /**< vector of pointers where 
      each points to a parameter representing the observed value */
  std::vector<fims::Vector<Type>*>
      prior_expected = NULL; /**< vector of pointers where 
      each points to a parameter representing the expected value */
  std::vector<fims::Vector<Type>>
      prior_observed_index = NULL; /**< vector of vectors indexing the values
      to use from the observed prior vectors */
  std::vector<fims::Vector<Type>>
      prior_expected_index = NULL; /**< vector of vectors indexing the values
      to use from the expected prior vectors */
  fims::Vector<Type> x; /**< input value of distribution function for priors or
                           random effects*/
  // std::shared_ptr<DistributionElementObject<Type>> expected; /**< expected
  // value of distribution function */

  /**
   * Retrieve element from observed data set, random effect, or prior.
   * @param i index referencing vector or pointer
   * @return the reference to the value of the vector or pointer at position i
   */
  inline Type& get_observed(size_t i) {
    if (this->input_type == "data") {
      return observed_values->at(i);
    }
    if (this->input_type == "random_effects") {
      return (*re)[i];
    }
    if (this->input_type == "prior") {
      if(priors.size() == 0) {
        throw std::runtime_error("No priors defined.");
      }
      if(priors.size() == 1) {
        return (*(priors[0]))[i];
      }
      if(priors.size() > 1) {
        return (*(priors[i]))[0];
      }
    }
    if (this->input_type == "extended_prior") {
      if(prior_observed.size() == 0) {
        if(prior_observed_index.size() == 0) {
          return observed_values.get_force_scalar(i);
        }
        if(prior_observed_index.size() == 1) {
          return observed_values.get_force_scalar(prior_observed_index[0][i]);
        }
        if(prior_observed_index.size() > 1) {
          return observed_values.get_force_scalar(prior_observed_index[i][0]);
        }
      }
      if(prior_observed.size() == 1) {
        if(prior_observed_index.size() == 0) {
          return (*(prior_observed[0])).get_force_scalar(i);
        }
        if(prior_observed_index.size() == 1) {
          return (*(prior_observed[0])).get_force_scalar(prior_observed_index[0][i]);
        }
        if(prior_observed_index.size() > 1) {
          return (*(prior_observed[0])).get_force_scalar(prior_observed_index[i][0]);
        }
      }
      if(prior_observed.size() > 1) {
        if(prior_observed_index.size() == 0) {
          return (*(prior_observed[i]))->at(0);
        }
        if(prior_observed_index.size() == 1) {
          return (*(prior_observed[prior_observed_index[0][i]]))->at(0);
        }
        if(prior_observed_index.size() > 1) {
          return (*(prior_observed[prior_observed_index[i][0]]))->at(0);
        }
      }
    }
    return x[i];
  }

  /**
   * Retrieve element from observed data set, random effect, or prior.
   * @param i index referencing row
   * @param j index referencing column
   * @return the reference to the row and column at position i, j
   */
  inline Type& get_observed(size_t i, size_t j) {
    if (this->input_type == "data") {
      return observed_values->at(i, j);
    }
    if (this->input_type == "random_effects") {
      return (*re)[i, j]; //TODO: This is wrong but need to figure out how to handle
                          // 2D random effects properly. Either:
                          // a) a single pointer to a matrix
                          // b) a vector pointers to vectors
                          // c) a 2D array or dimension folder vector of pointers 
    }
    if (this->input_type == "prior") {
      if(priors.size() == 1) {
        return (*(priors[0]))[i, j];
      }
      if(priors.size() > 1) {
        return (*(priors[i]))[j];
      }
    }
    return x[i];
  }

  /**
   * Retrieve expected element given data, random effect, or prior.
   * @param i index referencing vector or pointer
   * @return the reference to the value of the vector or pointer at position i
   */
  inline Type& get_expected(size_t i) {
    if (this->input_type == "random_effects") {
      return (*re_expected_values)[i];
    } else if (this->input_type == "extended_prior") {
      if(prior_expected.size() == 0) {
        if(prior_expected_index.size() == 0) {
          return expected_values.get_force_scalar(i);
        }
        if(prior_expected_index.size() == 1) {
          return expected_values.get_force_scalar(prior_expected_index[0][i]);
        }
        if(prior_expected_index.size() > 1) {
          return expected_values.get_force_scalar(prior_expected_index[i][0]);
        }
      }
      if(prior_expected.size() == 1) {
        if(prior_expected_index.size() == 0) {
          return (*(prior_expected[0])).get_force_scalar(i);
        }
        if(prior_expected_index.size() == 1) {
          return (*(prior_expected[0])).get_force_scalar(prior_expected_index[0][i]);
        }
        if(prior_expected_index.size() > 1) {
          return (*(prior_expected[0])).get_force_scalar(prior_expected_index[i][0]);
        }
      }
      if(prior_expected.size() > 1) {
        if(prior_expected_index.size() == 0) {
          return (*(prior_expected[i]))->at(0);
        }
        if(prior_expected_index.size() == 1) {
          return (*(prior_expected[prior_expected_index[0][i]]))->at(0);
        }
        if(prior_expected_index.size() > 1) {
          return (*(prior_expected[prior_expected_index[i][0]]))->at(0);
        }
      }
    }else {
      return this->expected_values.get_force_scalar(i);
    }
  }

  /**
   * Retrieve expected element size given data, random effect, or prior.
   * @return The size of the element.
   */
  inline size_t get_n_x() {
    if (this->input_type == "data") {
      return this->observed_values->data.size();
    }
    if (this->input_type == "random_effects") {
      return this->expected_values.size();
    }
    if (this->input_type == "prior") {
      return this->expected_values.size();
    }
    if (this->input_type == "extended_prior") {
      if(prior_expected.size() > 1) {
        return prior_expected.size();
      }else if(prior_expected.size() == 1) {
        return (*(prior_expected[0])).size();
      } else if(prior_expected.size() == 0) {
        return this->expected_values.size();
      }
    }
    return x.size();
  }
};

/** @brief Base class for all module_name functors.
 *
 * @tparam Type The type of the module_name functor.
 *
 */
template <typename Type>
struct DensityComponentBase : public fims_model_object::FIMSObject<Type>,
                              public DistributionElementObject<Type> {
  // id_g is the ID of the instance of the DensityComponentBase class.
  // this is like a memory tracker.
  // Assigning each one its own ID is a way to keep track of
  // all the instances of the DensityComponentBase class.
  static uint32_t
      id_g; /**< global unique identifier for distribution modules */
  int observed_data_id_m = -999; /*!< id of observed data component*/
  fims::Vector<Type> lpdf_vec; /**< vector to record observation level negative
                                  log-likelihood values */
  fims::Vector<Type> report_lpdf_vec; /**< vector to record observation level
                                         negative log-likelihood values */
  bool osa_flag = false; /**< Boolean; if true, osa residuals are calculated */
  bool simulate_flag =
      false; /**< Boolean; if true, data are simulated from the distribution */
  std::vector<uint32_t>
      key; /**< unique id for variable map that points to a fims::Vector */

#ifdef TMB_MODEL
  ::objective_function<Type>* of; /**< Pointer to the TMB objective function */
#endif

  /** @brief Constructor.
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
};

/** @brief Default id of the singleton distribution class
 */
template <typename Type>
uint32_t DensityComponentBase<Type>::id_g = 0;

}  // namespace fims_distributions

#endif /* DENSITY_COMPONENT_BASE_HPP */
