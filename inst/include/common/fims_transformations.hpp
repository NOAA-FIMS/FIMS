/**
 * @file fims_transformations.hpp
 * @brief Defines transformations for parameters in FIMS.
 */
#ifndef FIMS_COMMON_FIMS_TRANSFORMATIONS_HPP
#define FIMS_COMMON_FIMS_TRANSFORMATIONS_HPP

#include "def.hpp" 
#include "fims_math.hpp"

namespace fims_transformations {

  template <typename Type>
  inline Type ApplyBackTransformation(
      const Type& value, 
      const fims::Transformation& transformation) {

    const auto label = transformation.label;
    const auto uncertainty_label = transformation.uncertainty_label;
    const auto args = transformation.args;
    
    Type transformed_value;
    switch (label) {
      case fims::Transformation::Label::identity:
        transformed_value = value;
        break;
      case fims::Transformation::Label::log:
        transformed_value = fims_math::exp(value);
        break;
      case fims::Transformation::Label::logit:
        transformed_value = fims_math::inv_logit(
            Type(args.lower), Type(args.upper), value);
        break;
      case fims::Transformation::Label::square:
        transformed_value = fims_math::sqrt(value);
        break;
      default:
        throw std::invalid_argument(
        std::string("Unknown transformation applied to a parameter, ") + 
        TransformationLabelToString(label) + 
        std::string(". Valid transformations are identity, log, logit, and square."));
        break;
    }
    switch (uncertainty_label) {
      case fims::Transformation::UncertaintyLabel::var:
        return fims_math::sqrt(transformed_value); 
      default:
        return transformed_value; // No uncertainty transformation
    }
  }

  //TODO: check if these are correct (I don't think logit is correct)
  template <typename Type>
  inline Type AddLogJacobian(const Type& value, fims::Transformation::Label label,
    const fims::Transformation::Args& args = fims::Transformation::Args{}) {
    switch (label) {
      case fims::Transformation::Label::log:
        return value; // log transformation has a Jacobian of exp(value) which is the original value
      case fims::Transformation::Label::logit:
        return fims_math::log(args.upper - args.lower) - fims_math::log((args.upper - value) * (value - args.lower));
      case fims::Transformation::Label::square:
        return fims_math::log(2.0 * value); // square transformation has a Jacobian of 2*value
      default:
        return 0.0; // Identity transformation has no Jacobian adjustment
    } 
    
  }

} // namespace fims_transformations
#endif /* FIMS_COMMON_FIMS_TRANSFORMATIONS_HPP */