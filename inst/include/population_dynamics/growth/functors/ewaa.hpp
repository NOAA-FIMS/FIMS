/**
 * @file ewaa.hpp
 * @brief Defines the EWAAGrowth class, which inherits from the GrowthBase
 * class.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_GROWTH_EWAA_HPP
#define POPULATION_DYNAMICS_GROWTH_EWAA_HPP

// #include "../../../interface/interface.hpp"
#include <map>

#include "growth_base.hpp"

namespace fims_popdy {

/**
 *  @brief EWAAGrowth class that returns the EWAA function value.
 */
template <typename Type>
struct EWAAGrowth : public GrowthBase<Type> {
  // add submodule class members here
  // these include parameters of the submodule
  // a map looks up values based on a reference key
  // in this case, our key is age (first double), and
  //  the value is the weight at that age (second double)
  std::map<double, double> ewaa; /**<map of doubles for EWAA values by age,
          where age starts at zero > */
  typedef typename std::map<double, double>::iterator
      weight_iterator; /**< Iterator for ewaa map object > */

  EWAAGrowth() : GrowthBase<Type>() {}

  virtual ~EWAAGrowth() {}

  /**
   * @brief Returns the weight at age a (in kg) from the input vector.
   *
   * @param a  age of the fish, the age vector must start at zero
   */
  virtual const Type evaluate(const double& a) {
    weight_iterator it = this->ewaa.find(a);
    if (it == this->ewaa.end()) {
      return 0.0;
    }
    Type ret = (*it).second;  // itewaa[a];
    return ret;
  }

  /**
   * @brief Create a map of report vectors for the growth object.
   */
  virtual void create_report_vectors(
    std::map<std::string, fims::Vector<fims::Vector<Type>> >& report_vectors){
    // fims::Vector<Type> ewaa_vector;
    // for (auto const& pair : ewaa) {
    //   ewaa_vector.push_back(pair.second);
    // }
    // report_vectors["ewaa"] = ewaa_vector;
  }
};
}  // namespace fims_popdy
#endif /* POPULATION_DYNAMICS_GROWTH_EWAA_HPP */
