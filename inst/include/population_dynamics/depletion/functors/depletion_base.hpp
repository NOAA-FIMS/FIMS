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
    log_depletion; /**< Transformed depletion input supplied by the user */
  fims::Vector<Type>
    log_expected_depletion; /**< Log expectation of the depletion process. */
  fims::Vector<Type> 
    depletion; /**< The depletion process, back-transformed from 
                    log_depletion. */
  fims::Vector<Type>
    log_init_depletion; /*!< Transformed initial depletion supplied by the user*/
  fims::Vector<Type>
    init_depletion; /*!< estimated parameter: initial depletion 
                        back-transformed from log_init_depletion*/
  fims::Vector<Type> 
    log_carrying_capacity; /**< Transformed carrying capacity of the 
                                population supplied by the user */
  fims::Vector<Type> 
    log_growth_rate; /**< Transformed intrinsic growth rate supplied 
                            by the user. */
  fims::Vector<Type> 
    log_shape; /**< Tranformed shape parameter that adjusts the curvature 
                        of the growth function */
  fims::Vector<Type> 
    carrying_capacity; /**< Carrying capacity of the population 
                            back-transformed from log_carrying_capacity. */
  fims::Vector<Type> 
    growth_rate; /**< Intrinsic growth rate back-transformed from 
                        log_growth_rate. */
  fims::Vector<Type> 
    shape; /**< Shape parameter that adjusts the curvature of the growth 
                function back-transformed from log_shape */
  
  /** @brief Constructor.
   */
  DepletionBase() {
    // increment id of the singleton depletion class
    this->id = DepletionBase::id_g++;
  }

  virtual ~DepletionBase() {}


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