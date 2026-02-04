/**
 * @file parameter_transformations_base.hpp
 * @brief Defines transformation strategies for parameters between transformed and natural scales
 */
#ifndef FIMS_TRANSFORMATIONS_PARAMETER_TRANSFORMATION_BASE_HPP
#define FIMS_TRANSFORMATIONS_PARAMETER_TRANSFORMATION_BASE_HPP

#include "../../common/fims_math.hpp"

namespace fims_transformations {

/**
 * @brief Base class for parameter transformations
 * @tparam Type The numeric type (double, AD type, etc.)
 */
template <typename Type>
class ParameterTransformationBase {
 public:
  virtual ~ParameterTransformationBase() {}
  
  /**
   * @brief Transform from primary scale to secondary scale
   * @param transform_value The transformed value
   * @param natural_value The natural-scale value
   */
  virtual void Transform(Type& transform_value, Type& natural_value) = 0;
};
} // namespace fims_transformations

#endif /* FIMS_TRANSFORMATIONS_PARAMETER_TRANSFORMATION_BASE_HPP */