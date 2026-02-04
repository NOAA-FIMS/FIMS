/**
 * @file parameter_transformations_log.hpp
 * @brief Defines transformation strategies for parameters between log and natural scales
 */
#ifndef FIMS_TRANSFORMATIONS_PARAMETER_TRANSFORMATIONS_LOG_HPP
#define FIMS_TRANSFORMATIONS_PARAMETER_TRANSFORMATIONS_LOG_HPP

namespace fims_transformations {


/**
 * @brief Transformation for log-scale parameters
 * Updates natural = exp(log)
 */
template <typename Type>
class LogTransformation : public ParameterTransformationBase<Type> {
 public:
  virtual void Transform(Type& log_value, Type& natural_value) override {
    natural_value = fims_math::exp(log_value);
  }
};

/**
 * @brief Transformation for natural-scale parameters 
 * Updates log = log(natural)
 */
template <typename Type>
class InverseLogTransformation : public ParameterTransformationBase<Type> {
 public:
  virtual void Transform(Type& log_value, Type& natural_value) override {
      log_value = fims_math::log(natural_value);
  }
};

/**
 * @brief Function to create appropriate transformation based on default values
 * @param log_initial Initial value of log-scale parameter (-999 if not set)
 * @param natural_initial Initial value of natural-scale parameter (-999 if not set)
 * @return Shared pointer to appropriate transformation strategy
 */
template <typename Type>
std::shared_ptr<ParameterTransformationBase<Type>> CreateLogTransformation(
    double log_initial, double natural_initial) {
  
  const double default_value = -999.0;
  
  // User set natural-scale parameter
  if (natural_initial != default_value && log_initial == default_value) {
    return std::make_shared<InverseLogTransformation<Type>>();
  }
  
  // Default or user set log-scale parameter
  return std::make_shared<LogTransformation<Type>>();
}
 

} // namespace fims_transformations

#endif /* FIMS_TRANSFORMATIONS_PARAMETER_TRANSFORMATIONS_LOG_HPP */
