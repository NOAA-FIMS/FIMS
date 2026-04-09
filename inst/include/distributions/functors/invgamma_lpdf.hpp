/**
 * @file invgamma_lpdf.hpp
 * @brief Implements the InvGammaLPDF distribution functor used by FIMS to
 * evaluate observation-level and total log-likelihood contributions under an
 * inverse gamma error model for data, priors, and random effects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef INVGAMMA_LPDF
#define INVGAMMA_LPDF

#include "density_components_base.hpp"
#include "../../common/fims_math.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

namespace fims_distributions {
/**
 * @copybrief invgamma_lpdf.hpp
 *
 * @details This implementation uses \ref fims_math::dinvgamma() for inverse
 * gamma log-density calculations. The shape and scale parameters are estimated
 * directly. For `data` input, values equal to `na_value` are skipped and
 * contribute zero to the objective. Per-observation contributions are stored
 * in `lpdf_vec`; the summed total is returned by `evaluate()` and stored in
 * `lpdf`.
 */
template <typename Type>
struct InvGammaLPDF : public DensityComponentBase<Type> {
  /**
   * @brief The shape parameter of the inverse gamma distribution. The argument
   * can be a vector or scalar, where the latter is referenced for each
   * instance through the use of
   * \ref fims::Vector::get_force_scalar(size_t) "get_force_scalar()".
   */
  fims::Vector<Type> shape;

  /**
   * @brief The scale parameter of the inverse gamma distribution. The argument
   * can be a vector or scalar, where the latter is referenced for each
   * instance through the use of
   * \ref fims::Vector::get_force_scalar(size_t) "get_force_scalar()".
   */
  fims::Vector<Type> scale;

  /** @brief Constructor.
   */
  InvGammaLPDF() : DensityComponentBase<Type>() {}

  /** @brief Destructor.
   */
  virtual ~InvGammaLPDF() {}

  /**
   * @brief Evaluates the inverse gamma log probability density function.
   * @details The following equation is the inverse gamma probability density
   * function, and thus, the log of it is evaluated:
   * \f[
   * f(x; \alpha, \beta) = \frac{\beta^{\alpha}}{\Gamma(\alpha)}
   *   x^{-(\alpha+1)} \exp\!\left(-\frac{\beta}{x}\right),
   * \f]
   * where \f$\alpha\f$ is the shape parameter and \f$\beta\f$ is the scale
   * parameter.
   */
  virtual const Type evaluate() {
    // set vector size based on input type (prior, process, or data)
    size_t n_x = this->get_n_x();
    // setup vector for recording the log probability density function values
    this->lpdf_vec.resize(n_x);
    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(),
              static_cast<Type>(0));
    this->lpdf = static_cast<Type>(0);

    if (this->shape.size() > 1 && n_x != this->shape.size()) {
      throw std::invalid_argument(
          "InvGammaLPDF::Vector index out of bounds. The size of observed "
          "data does not equal the size of the shape vector. The observed "
          "data vector is of size " +
          fims::to_string(n_x) + " and the shape vector is of size " +
          fims::to_string(this->shape.size()));
    }

    if (this->scale.size() > 1 && n_x != this->scale.size()) {
      throw std::invalid_argument(
          "InvGammaLPDF::Vector index out of bounds. The size of observed "
          "data does not equal the size of the scale vector. The observed "
          "data vector is of size " +
          fims::to_string(n_x) + " and the scale vector is of size " +
          fims::to_string(this->scale.size()));
    }

    for (size_t i = 0; i < n_x; i++) {
#ifdef TMB_MODEL
      if (this->input_type == "data") {
        // if data, check if there are any NA values and skip lpdf calculation
        // if there are
        if (this->get_observed(i) != this->data_observed_values->na_value) {
          this->lpdf_vec[i] = fims_math::dinvgamma(
              this->get_observed(i), this->shape.get_force_scalar(i),
              this->scale.get_force_scalar(i), true);
        } else {
          this->lpdf_vec[i] = 0;
        }
      } else {
        this->lpdf_vec[i] = fims_math::dinvgamma(this->get_observed(i),
                                                 this->shape.get_force_scalar(i),
                                                 this->scale.get_force_scalar(i),
                                                 true);
      }

      this->lpdf += this->lpdf_vec[i];
      if (this->simulate_flag) {
        FIMS_SIMULATE_F(this->of) {  // preprocessor definition in interface.hpp
          if (this->input_type == "data") {
            this->data_observed_values->at(i) =
                static_cast<Type>(1.0) /
                rgamma(this->shape.get_force_scalar(i),
                       static_cast<Type>(1.0) /
                           this->scale.get_force_scalar(i));
          }
          if (this->input_type == "random_effects") {
            (*this->re)[i] =
                static_cast<Type>(1.0) /
                rgamma(this->shape.get_force_scalar(i),
                       static_cast<Type>(1.0) /
                           this->scale.get_force_scalar(i));
          }
          if (this->input_type == "prior") {
            (*(this->priors[i]))[0] =
                static_cast<Type>(1.0) /
                rgamma(this->shape.get_force_scalar(i),
                       static_cast<Type>(1.0) /
                           this->scale.get_force_scalar(i));
          }
        }
      }
#endif
    }
    return (this->lpdf);
  }
};
}  // namespace fims_distributions
#endif
