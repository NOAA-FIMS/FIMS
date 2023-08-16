/*
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 * SelectivityBase  file
 * The purpose of this file is to declare the SelectivityBase class
 * which is the base class for all selectivity functors.
 *
 * DEFINE guards for selectivity module outline to define the
 * selectivity hpp file if not already defined.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_BASE_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_BASE_HPP

#include "../../../common/model_object.hpp"

namespace fims
{

/** @brief Base class for all selectivity functors.
 *
 * @tparam T The type of the selectivity functor.
 */

template <typename T> struct SelectivityBase : public FIMSObject<T>
{
  // id_g is the ID of the instance of the SelectivityBase class.
  // this is like a memory tracker.
  // Assigning each one its own ID is a way to keep track of
  // all the instances of the SelectivityBase class.
  static uint32_t
      id_g; /*!< The ID of the instance of the SelectivityBase class */

  /** @brief Constructor.
   */
  SelectivityBase ()
  {
    // increment id of the singleton selectivity class
    this->id = SelectivityBase::id_g++;
  }

  virtual ~SelectivityBase () {}

  /**
   * @brief Calculates the selectivity.
   * @param x The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual const T evaluate (const T &x) = 0;
};

// default id of the singleton selectivity class
template <typename T> uint32_t SelectivityBase<T>::id_g = 0;

} // namespace fims

#endif /* POPULATION_DYNAMICS_SELECTIVITY_BASE_HPP */
