/**
 * @file depletion_base.hpp
 * @brief Declares the DepletionBase class which is the base class for all
 * depletion functors.
 * @details Defines guards for depletion module outline to define the
 * depletion hpp file if not already defined.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_DEPLETION_BASE_HPP
#define POPULATION_DYNAMICS_DEPLETION_BASE_HPP

#include "../../../common/model_object.hpp"
#include "../../../common/fims_vector.hpp"
#include "../../../transformations/transformations.hpp"

namespace fims_popdy {

/** @brief Base class for all depletion functors.
 *
 * @tparam Type The type of the depletion functor.
 */

template <typename Type>
struct DepletionBase : public fims_model_object::FIMSObject<Type> {
  // id_g is the ID of the instance of the DepletionBase class.
  // this is like a memory tracker.
  // Assigning each one its own ID is a way to keep track of
  // all the instances of the DepletionBase class.
  static uint32_t
      id_g; /**< The ID of the instance of the DepletionBase class */
  fims::Vector<Type> log_depletion; /**< Natural log of the depletion used for
                                       random effects. */
  fims::Vector<Type>
      log_expected_depletion; /**< Expectation of the depletion process. */
  fims::Vector<Type> depletion; /**< The depletion process. */
  fims::Vector<Type>
      log_init_depletion; /*!< estimated parameter: natural log of initial depletion*/
  fims::Vector<Type> log_carrying_capacity; /**< Carrying capacity of the population. */
  fims::Vector<Type> log_growth_rate; /**< Intrinsic growth rate. */
  fims::Vector<Type> log_shape; /**< Shape parameter that adjusts the curvature of
                                   the growth function */
  fims::Vector<Type> carrying_capacity; /**< Carrying capacity of the population. */
  fims::Vector<Type> growth_rate; /**< Intrinsic growth rate. */
  fims::Vector<Type> shape; /**< Shape parameter that adjusts the curvature of the growth function */
  
  // Transformation modules
  std::shared_ptr<fims_transformations::ParameterTransformationBase<Type>> 
      growth_rate_transformation; /**< Pointer to the transformation function for the intrinsic growth rate */
  std::shared_ptr<fims_transformations::ParameterTransformationBase<Type>> 
      carrying_capacity_transformation; /**< Pointer to the transformation function for the carrying capacity */
  std::shared_ptr<fims_transformations::ParameterTransformationBase<Type>> 
      shape_transformation; /**< Pointer to the transformation function for the shape parameter */
  std::shared_ptr<fims_transformations::ParameterTransformationBase<Type>> 
      depletion_transformation; /**< Pointer to the transformation function for depletion */

  /** @brief Constructor.
   */
  DepletionBase() {
    // increment id of the singleton depletion class
    this->id = DepletionBase::id_g++;
  }

  virtual ~DepletionBase() {}

  
  /**
   * @brief Apply transformations between log and natural scales
   * The transformation objects know which direction to transform
   */
  virtual void ApplyLogTransformations() {
    // Always call transform - polymorphism handles the right direction
    growth_rate_transformation->Transform(this->log_growth_rate[0], this->growth_rate[0]);
    carrying_capacity_transformation->Transform(this->log_carrying_capacity[0], this->carrying_capacity[0]);
    shape_transformation->Transform(this->log_shape[0], this->shape[0]);
    for(int i=0; i < this->depletion.size(); i++){
        depletion_transformation->Transform(this->log_depletion[i], this->depletion[i]);
    }
  }

  /**
   * @brief Calculates the depletion.
   *
   * @param depletion_ym1 Expected depletion from previous time step.
   * @param catch_ym1 Catch from previous time step.
   */
  virtual const Type evaluate_mean(const Type& depletion_ym1,
                                   const Type& catch_ym1) = 0;
};

// default id of the singleton depletion class
template <typename Type>
uint32_t DepletionBase<Type>::id_g = 0;

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_DEPLETION_BASE_HPP */