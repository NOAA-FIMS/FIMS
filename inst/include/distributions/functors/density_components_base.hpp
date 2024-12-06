
/** \file density_components_base.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * density_components_base file
 * The purpose of this file is to declare the DensityComponentBase class
 * which is the base class for all distribution functors.

 * DEFINE guards for distributions module outline to define the
 * density_components_base hpp file if not already defined.
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
 static uint32_t id_g; /**< global unique identifier for distribution modules */
 uint32_t observed_data_id_m = -999; /*!< id of observed data component*/
 std::shared_ptr<fims_data_object::DataObject<Type>>
      observed_values; /**< observed data*/
 fims::Vector<Type > x; /**< input value of distribution function for priors or random effects*/
 fims::Vector<Type> expected_values; /**< expected value of distribution function */
 fims::Vector<Type> lpdf_vec; /**< vector to record observation level negative log-likelihood values */
 std::string  input_type; /**< string classifies the type of the negative log-likelihood; options are: "priors", "random_effects", and "data" */
 bool osa_flag = false; /**< Boolean; if true, osa residuals are calculated */
 bool simulate_flag = false; /**< Boolean; if true, data are simulated from the distribution */
 std::vector<uint32_t> key; /**< unique id for variable map that points to a fims::Vector */

  #ifdef TMB_MODEL
    ::objective_function<Type> *of; /**< Pointer to the TMB objective function */
  #endif

  /** @brief Constructor.
   */
  DensityComponentBase() { this->id = DensityComponentBase::id_g++; }

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
