
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

/**
 * @brief Container that stores input vectors and pointers used by distribution
 * functors.
 * @details Provides a common access layer to retrieve observed and expected
 * values across "data", "random_effects", and "prior" pathways.
 */
template <typename Type>
struct DistributionElementObject {
  /**
   * @brief Classification of the input pathway for this distribution object.
   * Options used by accessor methods are, "prior", "random_effects", and
   * "data".
   */
  std::string input_type;

  /** @brief Observed data. */
  std::shared_ptr<fims_data_object::DataObject<Type>> observed_values;

  /** @brief Expected value vector for prior-based pathways. */
  fims::Vector<Type> expected_values;

  /** @brief Pointer to random effects vector. */
  fims::Vector<Type>* re = NULL;

  /** @brief Expected value vector for random-effects pathways. */
  fims::Vector<Type>* re_expected_values = NULL;

  /** @brief Expected value vector for data pathways. */
  fims::Vector<Type>* data_expected_values = NULL;

  /** @brief Vector of pointers where each entry points to a prior parameter. */
  std::vector<fims::Vector<Type>*> priors;

  /**
   * @brief Input value of distribution function for priors or random effects.
   */
  fims::Vector<Type> x;

  /**
   * @brief The expected mean of the distribution; overrides expected values.
   */
  fims::Vector<Type> expected_mean;

  /**
   * @brief If "yes", `expected_mean` is used instead of `expected_values`. The
   * default is "no" leading to the use of `expected_values`.
   */
  std::string use_mean = fims::to_string("no");

  // std::shared_ptr<DistributionElementObject<Type>> expected;
  // // Expected value of distribution function.

  /**
   * @brief Retrieve one observed value based on `input_type`.
   * @param i Index into the active observed source, e.g., vector or pointer.
   * @return Reference to the selected observed value.
   * @throws std::runtime_error If input_type is "prior" and priors is empty.
   */
  inline Type& get_observed(size_t i) {
    if (this->input_type == "data") {
      return observed_values->at(i);
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
    return x[i];
  }

  /**
   * @brief Retrieve one observed matrix-like value based on `input_type`.
   * @param i Row index.
   * @param j Column index.
   * @return Reference to the selected observed value.
   */
  inline Type& get_observed(size_t i, size_t j) {
    if (this->input_type == "data") {
      return observed_values->at(i, j);
    }
    if (this->input_type == "random_effects") {
      return (*re)[i, j];
    }
    if (this->input_type == "prior") {
      return (*(priors[i, j]))[0];
    }
    return x[i];
  }

  /**
   * @brief Retrieve one expected value based on `input_type` and `use_mean`.
   * @param i Index into the active expected source, e.g., vector or pointer.
   * @return Reference to the selected expected value.
   * @details If `use_mean == "yes"`, `expected_mean` overrides other expected
   * vectors and is accessed via scalar/vector semantics.
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
   * @brief Get length of the active observed input vector.
   * @return Size of the observed input under the current `input_type`.
   */
  inline size_t get_n_x() {
    if (this->input_type == "data") {
      return this->observed_values->data.size();
    }
    if (this->input_type == "random_effects") {
      return (*re).size();
    }
    if (this->input_type == "prior") {
      return this->expected_values.size();
    }
    return x.size();
  }

  /**
   * @brief Get length of the active expected input vector.
   * @return Size of the expected input under the current `input_type`.
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
    return x.size();
  }
};

/**
 * @brief Base class for concrete distribution functors used in likelihood
 * evaluations.
 * @tparam Type Scalar type used by the model (e.g., double or AD type).
 */
template <typename Type>
struct DensityComponentBase : public fims_model_object::FIMSObject<Type>,
                              public DistributionElementObject<Type> {
  // id_g is the ID of the instance of the DensityComponentBase class.
  // this is like a memory tracker.
  // Assigning each one its own ID is a way to keep track of
  // all the instances of the DensityComponentBase class.
  /**
   * @brief Global unique identifier for distribution modules.
   */
  static uint32_t id_g;

  /**
   * @brief ID of observed data component.
   */
  int observed_data_id_m = -999;

  /**
   * @brief Vector storing observation-level log-likelihood contributions.
   */
  fims::Vector<Type> lpdf_vec;

  /**
   * @brief Vector mirrored for report output at the observation level.
   */
  fims::Vector<Type> report_lpdf_vec;

  /**
   * @brief Boolean; if true, one-step-ahead (OSA) residuals are calculated.
   */
  bool osa_flag = false;

  /**
   * @brief Boolean; if true, data are simulated from the distribution.
   */
  bool simulate_flag = false;

  /**
   * @brief Unique ID for variable map that points to a fims::Vector.
   */
  std::vector<uint32_t> key;

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
    // initialize the priors vector with a size of 1 and set the first element
    // to NULL
    this->priors.resize(1);
    this->priors[0] = NULL;
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
