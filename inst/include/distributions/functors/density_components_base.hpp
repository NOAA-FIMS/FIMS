
/** \file density_components_base.hpp
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 *
 * density_components_base file
 * The purpose of this file is to declare the DensityComponentBase class
 * which is the base class for all distribution functors.

 * DEFINE guards for distributions module outline to define the
 * density_components_base hpp file if not already defined.
 */
#ifndef DENSITY_COMPONENT_BASE_HPP
#define DENSITY_COMPONENT_BASE_HPP

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
 fims::Vector<Type> observed_values; /**< input value of distribution function */
 fims::Vector<Type> expected_values; /**< expected value of distribution function */
 fims::Vector<Type> nll_vec; /**< vector to record observation level negative log-likelihood values */
 std::string  nll_type; /**< string classifies the type of the negative log-likelihood; options are: prior, re, data */
 bool osa_flag = false; /**< Boolean; if true, osa residuals are calculated */
 bool simulate_flag = false; /**< Boolean; if true, data are simulated from the distribution */

  /** @brief Constructor.
   */
  DensityComponentBase() { this->id = DensityComponentBase::id_g++; }

  virtual ~DensityComponentBase() {}
  /**
   * @brief Generic probability density function. Calculates the pdf at the
   * independent variable value.
   * @param do_log Boolean; if true, log densities are returned
   */
  virtual const Type evaluate(const bool& do_log) = 0;
};

/** @brief Default id of the singleton distribution class
 */
template <typename Type>
uint32_t DensityComponentBase<Type>::id_g = 0;

}  // namespace fims_distributions

#endif /* DENSITY_COMPONENT_BASE_HPP */
