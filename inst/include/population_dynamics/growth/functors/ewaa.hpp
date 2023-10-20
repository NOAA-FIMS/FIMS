/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to declare the growth functor class
 * which is the base class for all growth functors.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_EWAA_HPP
#define POPULATION_DYNAMICS_GROWTH_EWAA_HPP

//#include "../../../interface/interface.hpp"
#include <map>

#include "growth_base.hpp"

namespace fims {

/**
 *  @brief EWAAgrowth class that returns the EWAA function value.
 */
template <typename Type>
struct EWAAgrowth : public GrowthBase<Type> {
  // add submodule class members here
  // these include parameters of the submodule
  // a map looks up values based on a reference key
  // in this case, our key is age (first double), and
  //  the value is the weight at that age (second double)
  std::map<double, double> ewaa; /*!<map of doubles for EWAA values by age,
          where age starts at zero > */

  EWAAgrowth() : GrowthBase<Type>() {}

  virtual ~EWAAgrowth() {}

  /**
   * @brief Returns the weight at age a (in kg) from the input vector.
   *
   * @param a  age of the fish, the age vector must start at zero
   */
  virtual const Type evaluate(const double& a) {
    Type ret = ewaa[a];
    return ret;
  }
};
}  // namespace fims
#endif /* POPULATION_DYNAMICS_GROWTH_EWAA_HPP */