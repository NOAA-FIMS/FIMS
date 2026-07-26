/**
 * @file beta_lpdf.hpp
 * @brief Implements the BetaLPDF distribution functor used by FIMS to evaluate
 * observation-level and total log-likelihood contributions under a Beta
 * distribution model for data, priors, and random effects.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef BETA_LPDF
#define BETA_LPDF

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "../../common/def.hpp"
#include "../../common/fims_math.hpp"
#include "../../common/fims_vector.hpp"
#include "density_components_base.hpp"

namespace fims_distributions {

/**
 * @copybrief beta_lpdf.hpp
 *
 * @details This implementation evaluates the Beta log probability density
 * function for a random variable \f$x \in (0, s)\f$ with shape parameters
 * \f$\alpha > 0\f$ (`shape1`), \f$\beta > 0\f$ (`shape2`), and upper bound
 * \f$s > 0\f$ (`scale`, default 1.0):
 * \f[
 * \ln f(x \mid \alpha, \beta, s) = (\alpha - 1) \ln(x/s) + (\beta - 1) \ln(1 - x/s)
 * - \ln \text{B}(\alpha, \beta) - \ln(s)
 * \f]
 * where \f$\text{B}(\alpha, \beta) = \frac{\Gamma(\alpha)\Gamma(\beta)}{\Gamma(\alpha+\beta)}\f$.
 *
 * Under `TMB_MODEL`, observations are passed to `dbeta(x / s, alpha, beta, true) - log(s)`.
 * Per-observation contributions are stored in `lpdf_vec`; the summed total is returned by
 * `evaluate()` and stored in `lpdf`.
 */
template <typename Type>
struct BetaLPDF : public DensityComponentBase<Type> {
  /** @brief Shape parameter 1 (alpha > 0). */
  fims::Vector<Type> shape1;

  /** @brief Shape parameter 2 (beta > 0). */
  fims::Vector<Type> shape2;

  /** @brief Scale / upper bound parameter (scale > 0, default 1.0). */
  fims::Vector<Type> scale;

  /** @brief Constructor. */
  BetaLPDF() : DensityComponentBase<Type>() {
    scale.resize(1);
    scale[0] = static_cast<Type>(1.0);
  }

  /** @brief Destructor. */
  virtual ~BetaLPDF() {}

  /**
   * @brief Evaluates the Beta log probability density function.
   * @return Summed log probability density value.
   */
  virtual const Type evaluate() {
    size_t n_x = (this->observed_values.size() > 0) ? this->observed_values.size()
                                                   : this->get_n_x();

    this->lpdf_vec.resize(n_x);
    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), static_cast<Type>(0));
    this->lpdf = static_cast<Type>(0);

    for (size_t i = 0; i < n_x; i++) {
      Type obs = this->get_observed(i);
      Type a = shape1.get_force_scalar(i);
      Type b = shape2.get_force_scalar(i);
      Type s = scale.size() > 0 ? scale.get_force_scalar(i) : static_cast<Type>(1.0);

      Type u = obs / s;

#ifdef TMB_MODEL
      if (this->input_type == "data") {
        if (this->get_observed(i) != this->data_observed_values->na_value) {
          this->lpdf_vec[i] = dbeta(u, a, b, true) - fims_math::log(s);
        } else {
          this->lpdf_vec[i] = static_cast<Type>(0);
        }
      } else {
        this->lpdf_vec[i] = dbeta(u, a, b, true) - fims_math::log(s);
      }
#else
      Type log_beta = fims_math::lgamma(a) + fims_math::lgamma(b) -
                       fims_math::lgamma(a + b);
      this->lpdf_vec[i] = (a - static_cast<Type>(1.0)) * fims_math::log(u) +
                          (b - static_cast<Type>(1.0)) * fims_math::log(static_cast<Type>(1.0) - u) -
                          log_beta - fims_math::log(s);
#endif

      this->lpdf += this->lpdf_vec[i];
    }

    return this->lpdf;
  }
};

}  // namespace fims_distributions

#endif  // BETA_LPDF
