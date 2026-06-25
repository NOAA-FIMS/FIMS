
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
 * @brief Classifies the role of a distribution component within the joint
 * likelihood. This determines which inputs are active and controls simulation
 * order in model.hpp (priors first, then random effects, then data).
 */
enum class Distribution_Kind {
  PRIOR,
  RANDOM_EFFECT,
  DATA,
  PENALTY
};

/**
 * @brief Converts a string input_type (from the R/Rcpp interface) to a
 * Distribution_Kind enum value.
 * @param s One of "prior", "random_effects", "data", or "penalty".
 * @throws std::invalid_argument if the string is not recognised.
 */
inline Distribution_Kind distribution_kind_from_string(const std::string& s) {
  if (s == "prior") return Distribution_Kind::PRIOR;
  if (s == "random_effects") return Distribution_Kind::RANDOM_EFFECT;
  if (s == "data") return Distribution_Kind::DATA;
  if (s == "penalty") return Distribution_Kind::PENALTY;
  throw std::invalid_argument(
      "distribution_kind_from_string: unrecognised input_type \"" + s +
      "\". Valid values are \"prior\", \"random_effects\", \"data\", "
      "\"penalty\".");
}

/** @brief Base class for all module_name functors.
 *
 * @tparam Type The type of the module_name functor.
 *
 */
template <typename Type>
struct DensityComponentBase : public fims_model_object::FIMSObject<Type> {
  /**
   * @brief Classification of the role of this distribution component within
   * the joint likelihood. Set via set_distribution_links() in the Rcpp
   * interface using distribution_kind_from_string().
   */
  Distribution_Kind distribution_type = Distribution_Kind::DATA;

  // ---------------------------------------------------------------------------
  // Observed-side storage and unified pointer
  // ---------------------------------------------------------------------------

  /** @brief Local storage for observed values (non-data pathways). */
  fims::Vector<Type> observed_values;

  /**
   * @brief Observed data object; retained for DATA pathways only (NA
   * checking, 2-D access, and simulation write-backs to the data buffer).
   */
  std::shared_ptr<fims_data_object::DataObject<Type>> data_observed_values;

  /**
   * @brief Unified observed-value pointer. Set by SetupDistributions():
   *   - DATA:          &data_observed_values->data
   *   - RANDOM_EFFECT: variable_map[key[0]]
   *   - PRIOR (single key): variable_map[key[0]]
   *   - PRIOR (multi key):  not used; use_priors_vec == true instead
   * Defaults to &observed_values so functors are safe before setup.
   */
  fims::Vector<Type>* observed_ptr = nullptr;

  /**
   * @brief Optional index-remapping vector (from PR #1572). When non-empty,
   * get_observed(i) returns (*observed_ptr)[observed_subvector[i]] instead of
   * (*observed_ptr)[i]. Useful for selecting a subset of elements from a
   * larger registered vector.
   */
  fims::Vector<size_t> observed_subvector;

  /**
   * @brief When true, get_observed(i) returns (*(priors[i]))[0], i.e. the
   * first element of the i-th prior parameter vector. Used only for multi-key
   * PRIOR distributions. Set by SetupDistributions().
   */
  bool use_priors_vec = false;

  /**
   * @brief Vector of pointers for multi-key PRIOR distributions; each entry
   * points to a separate parameter vector. Only populated when
   * use_priors_vec == true.
   */
  std::vector<fims::Vector<Type>*> priors;

  // ---------------------------------------------------------------------------
  // Expected-side storage and unified pointer
  // ---------------------------------------------------------------------------

  /** @brief Local storage for expected values. */
  fims::Vector<Type> expected_values;

  /**
   * @brief Optional fixed or estimated mean; used when use_expected_mean is
   * true. Registered in variable_map so it can be estimated by TMB.
   */
  fims::Vector<Type> expected_mean;

  /**
   * @brief When true, expected_ptr is set to &expected_mean by
   * SetupDistributions(). Set from the Rcpp interface via set_distribution_mean().
   */
  bool use_expected_mean = false;

  /**
   * @brief Unified expected-value pointer. Set by SetupDistributions():
   *   - DATA (normal case):   variable_map[key[0]]
   *   - DATA (use_expected_mean): &expected_mean
   *   - RANDOM_EFFECT (2 keys): variable_map[key[1]]
   *   - RANDOM_EFFECT (1 key):  &expected_values
   *   - PRIOR:                  &expected_values (or &expected_mean)
   * Defaults to &expected_values.
   */
  fims::Vector<Type>* expected_ptr = nullptr;

  // ---------------------------------------------------------------------------
  // Unified accessors — all three are now branchless or near-branchless
  // ---------------------------------------------------------------------------

  /**
   * @brief Return the i-th observed value via the unified observed_ptr.
   * @details Three paths (in priority order):
   *   1. Multi-key PRIOR (use_priors_vec): returns (*(priors[i]))[0].
   *   2. Subvector remapping: returns (*observed_ptr)[observed_subvector[i]].
   *   3. Direct: returns (*observed_ptr)[i].
   */
  inline Type& get_observed(size_t i) {
    if (this->use_priors_vec) {
      return (*(this->priors[i]))[0];
    }
    if (this->observed_subvector.size() > 0) {
      return (*this->observed_ptr)[this->observed_subvector[i]];
    }
    return (*this->observed_ptr)[i];
  }

  /**
   * @brief Return the (i, j) observed value for 2-D data (e.g. multinomial).
   * @details Only meaningful for DATA distribution_type.
   */
  inline Type& get_observed(size_t i, size_t j) {
    if (this->distribution_type == Distribution_Kind::DATA) {
      return data_observed_values->at(i, j);
    }
    return (*this->observed_ptr)[i * j + j];
  }

  /**
   * @brief Return the i-th expected value via the unified expected_ptr.
   * @details Fully branchless: expected_ptr is set at setup time by
   * SetupDistributions() to whichever source is appropriate for the role.
   */
  inline Type& get_expected(size_t i) {
    return this->expected_ptr->get_force_scalar(i);
  }

  /**
   * @brief Get the number of observed values for the active input.
   */
  inline size_t get_n_x() {
    if (this->distribution_type == Distribution_Kind::DATA) {
      return this->data_observed_values->data.size();
    }
    if (this->use_priors_vec) {
      return this->priors.size();
    }
    if (this->observed_subvector.size() > 0) {
      return this->observed_subvector.size();
    }
    return this->observed_ptr->size();
  }

  /**
   * @brief Get the number of expected values for the active input.
   */
  inline size_t get_n_expected() {
    return this->expected_ptr->size();
  }
  // id_g is the ID of the instance of the DensityComponentBase class.
  // this is like a memory tracker.
  // Assigning each one its own ID is a way to keep track of
  // all the instances of the DensityComponentBase class.
  /**
   * @brief Global unique identifier for distribution modules.
   */
  static uint32_t id_g;

  /**
   * @brief Total log probability density contribution of the distribution.
   *
   */
  Type lpdf;

  /**
   * @brief ID of observed data component.
   */
  int observed_data_id_m = -999;

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
   * @brief Constructor. Initializes unified pointers to local storage so
   * functors are safe to call before SetupDistributions() wires them.
   */
  DensityComponentBase() {
    this->observed_ptr = &this->observed_values;
    this->expected_ptr = &this->expected_values;
    this->id = DensityComponentBase::id_g++;
  }

  virtual ~DensityComponentBase() {}
  /**
   * @brief Returns the name of the concrete distribution functor.
   * @details Used in error messages so they identify the calling class without
   * hardcoding the class name as a string literal.
   */
  virtual std::string name() const = 0;

  /**
   * @brief Initialise lpdf_vec and lpdf for a new evaluation pass.
   * @details Replaces the three-line resize/fill/zero block that was repeated
   * at the top of every functor's evaluate().
   * @param n Number of observations for this evaluation.
   */
  inline void prepare_lpdf(size_t n) {
    this->lpdf_vec.resize(n);
    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(),
              static_cast<Type>(0));
    this->lpdf = static_cast<Type>(0);
  }

  /**
   * @brief Adjust n_x to respect the expected-values vector size.
   * @details Preserves existing behaviour: if expected is scalar (size == 1)
   * n_x is unchanged; if n_x exceeds a non-scalar expected vector n_x is
   * clamped to that size.
   * @param n_x Candidate number of observed values.
   * @return Adjusted n_x.
   */
  inline size_t check_n_x(size_t n_x) {
    size_t n_expected = this->get_n_expected();
    if (n_expected > 1 && n_x > n_expected) {
      return n_expected;
    }
    return n_x;
  }

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
