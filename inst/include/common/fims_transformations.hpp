/**
 * @file fims_transformations.hpp
 * @brief Defines transformations for parameters in FIMS.

 *
 * @details This file provides functions for applying forward and inverse
 * transformations to parameters, transforming parameters between input
 * and prior spaces, and computing log Jacobian adjustments for
 * change-of-variables corrections in MCMC sampling.
 *
 * The transformation system supports the following transformations:
 * - `identity`: No transformation, parameter is on the natural scale.
 * - `exp`: Exponential transformation, parameter is on the log scale.
 * - `log`: Log transformation, parameter is on the natural scale.
 * - `logit`: Logit transformation with optional bounds.
 * - `square`: Square transformation.
 * - `sqrt`: Square root transformation.
 *
 * @see fims::Transformation
 * @see fims_math.hpp
 */

#ifndef FIMS_COMMON_FIMS_TRANSFORMATIONS_HPP
#define FIMS_COMMON_FIMS_TRANSFORMATIONS_HPP

#include "fims_math.hpp"
#include "types.hpp"

namespace fims_transformations {

/**
 * @brief Applies a forward transformation to a scalar value.
 *
 * @details Maps a value from natural scale to the transformed scale
 * specified by the transformation argument. The forward transformations
 * and are:
 *
 * | Label    | Forward          |
 * |----------|------------------|
 * | identity | x                |
 * | log      | log(x)           |
 * | exp      | exp(x)           |
 * | logit    | logit(x, lo, hi) |
 * | square   | x^2              |
 * | sqrt     | sqrt(x)          |
 *
 * @tparam Type The numeric type (e.g. double, TMBad::ad_aug).
 * @param value The input value on the natural scale.
 * @param transformation A fims::Transformation struct specifying the
 * transformation label and any required arguments (e.g. logit bounds).
 * @return The transformed value.
 * @throws std::invalid_argument if the transformation label is not supported.
 */
template <typename Type>
inline Type ApplyTransformation(const Type& value,
                                const fims::Transformation& transformation) {
  const auto label = transformation.label;
  const auto args = transformation.args;

  switch (label) {
    case fims::Transformation::Label::identity:
      return value;
    case fims::Transformation::Label::exp:
      return exp(value);
    case fims::Transformation::Label::log:
      return log(value);
    case fims::Transformation::Label::logit:
      return fims_math::logit(Type(args.lower), Type(args.upper), value);
    case fims::Transformation::Label::square:
      return value * value;
    case fims::Transformation::Label::sqrt:
      return sqrt(value);
    // Add more cases as needed
    default:
      throw std::invalid_argument("Unknown transformation label");
  }
}

/**
 * @brief Applies an inverse (back) transformation to a scalar value.
 *
 * @details Maps a value from the transformed scale back to the natural
 * scale. This is the inverse of ApplyTransformation(). For example, if
 * the input transformation is `log`, then ApplyBackTransformation applies
 * `exp` to recover the natural-scale value.
 *
 * | Label    | Back Transformation  |
 * |----------|----------------------|
 * | identity | x                    |
 * | log      | exp(x)               |
 * | exp      | log(x)               |
 * | logit    | inv_logit(x, lo, hi) |
 * | square   | sqrt(x)              |
 * | sqrt     | x^2                  |
 *
 * @tparam Type The numeric type (e.g. double, TMBad::ad_aug).
 * @param value The input value on the transformed scale.
 * @param transformation A fims::Transformation struct specifying the
 * transformation label and any required arguments (e.g. logit bounds).
 * @return The back-transformed value on the natural scale.
 * @throws std::invalid_argument if the transformation label is not supported.
 */
template <typename Type>
inline Type ApplyBackTransformation(
    const Type& value, const fims::Transformation& transformation) {
  const auto label = transformation.label;
  const auto args = transformation.args;

  Type transformed_value;
  switch (label) {
    case fims::Transformation::Label::identity:
      transformed_value = value;
      break;
    case fims::Transformation::Label::exp:
      transformed_value = fims_math::log(value);
      break;
    case fims::Transformation::Label::log:
      transformed_value = fims_math::exp(value);
      break;
    case fims::Transformation::Label::logit:
      transformed_value =
          fims_math::inv_logit(Type(args.lower), Type(args.upper), value);
      break;
    case fims::Transformation::Label::square:
      transformed_value = fims_math::sqrt(value);
      break;
    case fims::Transformation::Label::sqrt:
      transformed_value = value * value;
      break;
    default:
      throw std::invalid_argument(
          std::string("Unknown transformation applied to a parameter, ") +
          TransformationLabelToString(label) +
          std::string(". Valid transformations are identity, exp, log, logit, "
                      "square, and sqrt."));
      break;
  }

  return transformed_value;
}

/**
 * @brief Transforms a vector of parameters from input space to prior space.
 *
 * @details Applies the composite transformation:
 *   input space -> natural scale -> prior space
 *
 * This is used when evaluating a prior distribution where the parameter
 * is estimated in one space (e.g. log scale) but the prior is defined in
 * a different space (e.g. natural scale or variance scale).
 *
 * If input and prior transformations are identical, the input vector is
 * returned unchanged.
 *
 * @tparam Type The numeric type (e.g. double, TMBad::ad_aug).
 * @param input_value A vector of parameter values in input space.
 * @param input The transformation applied to the parameter in input space
 * (e.g. log if the parameter is stored as log(sd)).
 * @param prior The transformation applied to the parameter in prior space
 * (e.g. square if the prior is on variance = sd^2).
 * @return A vector of parameter values transformed to prior space.
 */
template <typename Type>
inline fims::Vector<Type> TransformPrior(const fims::Vector<Type>& input_value,
                                         fims::Transformation input,
                                         fims::Transformation prior) {
  if (input.label == prior.label) {
    return input_value;
  } else {
    size_t n = input_value.size();

    fims::Vector<Type> natural_parameter;
    fims::Vector<Type> prior_parameter;
    natural_parameter.resize(n);
    prior_parameter.resize(n);

    for (size_t i = 0; i < n; i++) {
      natural_parameter[i] = ApplyBackTransformation(input_value[i], input);
      prior_parameter[i] = ApplyTransformation(natural_parameter[i], prior);
    }

    return prior_parameter;
  }
}

/**
 * @brief Transforms a scalar parameter from input space to prior space.
 *
 * @details Scalar overload of TransformPrior() for use in get_observed()
 * where a single element is needed. Applies the composite transformation:
 *   input space -> natural scale -> prior space
 *
 * @tparam Type The numeric type (e.g. double, TMBad::ad_aug).
 * @param input_value A scalar parameter value in input space.
 * @param input The transformation applied to the parameter in input space.
 * @param prior The transformation applied to the parameter in prior space.
 * @return The scalar parameter value transformed to prior space.
 */
template <typename Type>
inline Type TransformPrior(const Type& input_value,
                           const fims::Transformation& input,
                           const fims::Transformation& prior) {
  Type natural_parameter = ApplyBackTransformation(input_value, input);
  return ApplyTransformation(natural_parameter, prior);
}

/**
 * @brief Computes the log absolute determinant of the Jacobian for a
 * change-of-variables correction in MCMC sampling.
 *
 * @details When a parameter is estimated in one space (input space) but
 * the prior is placed in a different space (prior space), MCMC sampling
 * requires a change-of-variables correction. This function computes
 * log |det J| where J is the Jacobian of the composite transformation
 * from input space to prior space.
 *
 * The Jacobian is computed using TMBad's sparse Jacobian functionality
 * via SpJacFun(), which avoids allocating a dense n x n matrix. For
 * element-wise transformations (the common case), the Jacobian is
 * diagonal and only n non-zero entries are stored. For future multivariate
 * or simplex transformations, the sparse pattern is detected automatically
 * during AD taping.
 *
 * @note This function is only available when compiled with TMB
 * (TMB_MODEL defined). For non-TMB builds it throws std::invalid_argument.
 *
 * @note Stan handles the Jacobian for the transformation from unconstrained
 * sampling space to bounded input space automatically. This function only
 * handles the additional correction from input space to prior space.
 *
 * @tparam Type The numeric type (e.g. double, TMBad::ad_aug).
 * @param input_value A vector of parameter values in input space.
 * @param input The transformation applied to the parameter in input space
 * (e.g. log if the parameter is stored as log(sd)).
 * @param prior The transformation applied to the parameter in prior space
 * (e.g. square if the prior is on variance = sd^2).
 * @return The log absolute determinant of the Jacobian, log |det J|.
 * @throws std::invalid_argument if called outside a TMB model context.
 *
 * @example
 * // Parameter estimated as log(sd), prior on variance (sd^2)
 * // Jacobian adjustment: log(2) + 2*log(sd)
 * fims::Transformation log_trans, square_trans;
 * log_trans.label = fims::Transformation::Label::log;
 * square_trans.label = fims::Transformation::Label::square;
 * Type jac = AddLogJacobian(log_sd_values, log_trans, square_trans);
 */
template <typename Type>
inline Type AddLogJacobian(const fims::Vector<Type>& input_value,
                           fims::Transformation input,
                           fims::Transformation prior) {
#ifdef TMB_MODEL

  size_t n = input_value.size();
  // Step 1: Extract double values for taping
  std::vector<double> x_val(n);
  for (size_t i = 0; i < n; i++) x_val[i] = asDouble(input_value[i]);

  // Step 2: Tape the composite transformation into an ADFun
  TMBad::ADFun<> f;
  {
    // Start recording

    std::vector<TMBad::ad_aug> x_ad(n);
    for (size_t i = 0; i < n; i++) x_ad[i] = x_val[i];

    f.glob.ad_start();
    f.glob.Independent(x_ad);  // vector version, not scalar

    std::vector<TMBad::ad_aug> y_ad(n);
    for (size_t i = 0; i < n; i++) {
      TMBad::ad_aug natural = ApplyBackTransformation(x_ad[i], input);
      y_ad[i] = ApplyTransformation(natural, prior);
    }

    TMBad::Dependent(y_ad);  // vector version
    f.glob.ad_stop();
  }

  // Step 3: Generate sparse Jacobian function
  TMBad::Sparse<TMBad::ADFun<>> spjac = f.SpJacFun();

  // Step 4: Evaluate sparse Jacobian at current values
  std::vector<double> jac_vals = spjac(x_val);

  // Step 5: Compute log abs det from sparse values
  // For diagonal case, all non-zeros are diagonal entries
  // For general case, need to use spjac.i and spjac.j for row/col indices
  Type log_abs_det = Type(0.0);

  if (n == spjac.i.size()) {
    // Diagonal case - all non-zeros are on diagonal
    for (size_t k = 0; k < jac_vals.size(); k++) {
      log_abs_det += fims_math::log(fims_math::ad_fabs(Type(jac_vals[k])));
    }
  } else {
    // General case - need log determinant of sparse matrix
    // Build Eigen sparse matrix from spjac pattern
    Eigen::SparseMatrix<double> J(n, n);
    std::vector<Eigen::Triplet<double>> triplets;
    for (size_t k = 0; k < jac_vals.size(); k++) {
      triplets.push_back(
          Eigen::Triplet<double>(spjac.i[k], spjac.j[k], jac_vals[k]));
    }
    J.setFromTriplets(triplets.begin(), triplets.end());

    // For sparse log determinant use Eigen's sparse LU
    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
    solver.compute(J);
    log_abs_det = Type(solver.logAbsDeterminant());
  }

  return log_abs_det;

#else

  throw std::invalid_argument(
      std::string("Jacobian adjustments currently only work for TMB models."));
#endif  // TMB_MODEL
}

}  // namespace fims_transformations
#endif /* FIMS_COMMON_FIMS_TRANSFORMATIONS_HPP */