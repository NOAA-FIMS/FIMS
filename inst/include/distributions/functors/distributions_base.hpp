/** \file distributions_base.hpp
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 *
 * distributions_base file
 * The purpose of this file is to declare the DensityComponentBase class
 * which is the base class for all distribution functors.
 *
 * DEFINE guards for distributions module outline to define the
 * distributions_base hpp file if not already defined.
 */
#ifndef DISTRIBUTIONS_BASE_HPP
#define DISTRIBUTIONS_BASE_HPP

#include "../../common/model_object.hpp"
#include "../../interface/interface.hpp"
#include "../../common/fims_vector.hpp"

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
  static uint32_t id_g;
 fims::Vector<Type> observed_values;
 fims::Vector<Type> expected_values;
 std::string:: nll_type; //prior, re, data
 bool osa_flag = FALSE;
 bool simulate_flag = FALSE;

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
uint32_t DensityComponent<Type>::id_g = 0; 

}  // namespace fims_distributions

#endif /* DISTRIBUTIONS_BASE_HPP */
