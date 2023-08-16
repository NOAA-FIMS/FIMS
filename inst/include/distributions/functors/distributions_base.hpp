/*! \file distributions_base.hpp
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 *
 * distributions_base file
 * The purpose of this file is to declare the DistributionsBase class
 * which is the base class for all distribution functors.
 *
 * DEFINE guards for distributions module outline to define the
 * distributions_base hpp file if not already defined.
 */
#ifndef DISTRIBUTIONS_BASE_HPP
#define DISTRIBUTIONS_BASE_HPP

#include "../../common/model_object.hpp"
#include "../../interface/interface.hpp"

namespace fims
{

/** @brief Base class for all module_name functors.
 *
 * @tparam T The type of the module_name functor.
 *
 */
template <typename T> struct DistributionsBase : public FIMSObject<T>
{
  // id_g is the ID of the instance of the DistributionsBase class.
  // this is like a memory tracker.
  // Assigning each one its own ID is a way to keep track of
  // all the instances of the DistributionsBase class.
  static uint32_t id_g;

  /** @brief Constructor.
   */
  DistributionsBase () { this->id = DistributionsBase::id_g++; }

  virtual ~DistributionsBase () {}
  /**
   * @brief Generic probability density function. Calculates the pdf at the
   * independent variable value.
   * @param do_log Boolean; if true, log densities are returned
   */
  virtual const T evaluate (const bool &do_log) = 0;
};

/** @brief Default id of the singleton distribution class
 */
template <typename T> uint32_t DistributionsBase<T>::id_g = 0;

} // namespace fims

#endif /* DISTRIBUTIONS_BASE_HPP */
