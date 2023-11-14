/*
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * maturity_base.hpp
 * The purpose of this file is to declare the MaturityBase class
 * which is the base class for all maturity functors.
 *
 * DEFINE guards for maturity module outline to define the
 * maturity hpp file if not already defined.
 */
#ifndef POPULATION_DYNAMICS_MATURITY_BASE_HPP
#define POPULATION_DYNAMICS_MATURITY_BASE_HPP

#include "../../../common/model_object.hpp"

namespace fims_popdy {

/** @brief Base class for all maturity functors.
 *
 * @tparam Type The type of the maturity functor.
 */

template <typename Type>
struct MaturityBase : public fims_model_object::FIMSObject<Type> {
  // id_g is the ID of the instance of the MaturityBase class.
  // this is like a memory tracker.
  // Assigning each one its own ID is a way to keep track of
  // all the instances of the MaturityBase class.
  static uint32_t id_g; /**< The ID of the instance of the MaturityBase class */

  /** @brief Constructor.
   */
  MaturityBase() {
    // increment id of the singleton maturity class
    this->id = MaturityBase::id_g++;
  }

  /**
   * @brief Calculates the maturity.
   * @param x The independent variable in the maturity function (e.g., logistic
   * maturity at age or size).
   */
  virtual const Type evaluate(const Type& x) = 0;
};

// default id of the singleton maturity class
template <typename Type>
uint32_t MaturityBase<Type>::id_g = 0;

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_MATURITY_BASE_HPP */
