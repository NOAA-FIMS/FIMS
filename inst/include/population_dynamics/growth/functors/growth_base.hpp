/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 *
 * growth module_base file
 * The purpose of this file is to include any .hpp files within the
 * subfolders so that only this file needs to included in the model.hpp file.
 *
 * DEFINE guards for growth module outline to define the
 * module_name_base hpp file if not already defined.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_BASE_HPP
#define POPULATION_DYNAMICS_GROWTH_BASE_HPP

#include "../../../common/model_object.hpp"

namespace fims {

/**
 * @brief Base class for all growth functors.
 *
 * @tparam Type The type of the growth functor.
 * */
template <typename Type>
struct GrowthBase : public FIMSObject<Type> {
  // id_g is the ID of the instance of the  growthBase class.
  // this is like a memory tracker.
  // Assigning each one its own ID is a way to keep track of
  // all the instances of the  growthBase class.
  static uint32_t id_g; /*!< reference id for growth object*/

  /**
   * @brief Constructor.
   */
  GrowthBase() { this->id = GrowthBase::id_g++; }

  virtual ~GrowthBase() {}

  /**
   * @brief Calculates the  growth at the independent variable value.
   * @param a The age at which to return weight of the fish (in kg).
   */
  virtual const Type evaluate(const double& a) = 0;
};

template <typename Type>
uint32_t GrowthBase<Type>::id_g = 0;

}  // namespace fims

#endif /* POPULATION_DYNAMICS_GROWTH_BASE_HPP */
