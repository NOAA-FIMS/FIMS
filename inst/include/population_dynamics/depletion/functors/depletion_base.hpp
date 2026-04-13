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
#include "../../../common/fims_transformations.hpp"

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
  fims::Vector<Type> 
    depletion_input; /**< Transformed depletion input supplied by the user */
  fims::Vector<Type>
    log_expected_depletion; /**< Log expectation of the depletion process. */
  fims::Vector<Type> 
    depletion; /**< The depletion process, back-transformed from 
                    depletion_input. */
  fims::Vector<Type>
    init_depletion_input; /*!< Transformed initial depletion supplied by the user*/
  fims::Vector<Type>
    init_depletion; /*!< estimated parameter: initial depletion 
                        back-transformed from init_depletion_input*/
  fims::Vector<Type> 
    carrying_capacity_input; /**< Transformed carrying capacity of the 
                                population supplied by the user */
  fims::Vector<Type> 
    growth_rate_input; /**< Transformed intrinsic growth rate supplied 
                            by the user. */
  fims::Vector<Type> 
    shape_input; /**< Tranformed shape parameter that adjusts the curvature 
                        of the growth function */
  fims::Vector<Type> 
    carrying_capacity; /**< Carrying capacity of the population 
                            back-transformed from carrying_capacity_input. */
  fims::Vector<Type> 
    growth_rate; /**< Intrinsic growth rate back-transformed from 
                        growth_rate_input. */
  fims::Vector<Type> 
    shape; /**< Shape parameter that adjusts the curvature of the growth 
                function back-transformed from shape_input */
  
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
  virtual void ApplyTransformations() {
    this->growth_rate[0] = fims_transformations::ApplyBackTransformation(
        this->growth_rate_input[0], 
        this->growth_rate_input.get_transformation());
    this->carrying_capacity[0] = fims_transformations::ApplyBackTransformation(
        this->carrying_capacity_input[0], 
        this->carrying_capacity_input.get_transformation());
    this->shape[0] = fims_transformations::ApplyBackTransformation(
        this->shape_input[0], this->shape_input.get_transformation());
    this->init_depletion[0] = fims_transformations::ApplyBackTransformation(
        this->init_depletion_input[0], this->init_depletion_input.get_transformation());
    for(size_t i=0; i < this->depletion.size(); i++){
        this->depletion[i] = fims_transformations::ApplyBackTransformation(
            this->depletion_input[i], this->depletion_input.get_transformation());
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