/**
 * @file gaussian_process_lpdf.hpp
 * @brief IID, stationary AR(1), and initialized random-walk Gaussian densities.
 */
#ifndef FIMS_GAUSSIAN_PROCESS_LPDF_HPP
#define FIMS_GAUSSIAN_PROCESS_LPDF_HPP

#include <cmath>
#include <stdexcept>
#include "../../common/fims_math.hpp"
#include "density_components_base.hpp"

namespace fims_distributions {
/**
 * @brief A Gaussian density on residuals from a declared mean vector.
 * @details log_sd is a scalar innovation log-SD. For AR(1), rho =
 * tanh(unconstrained_rho[0]) and the first residual has stationary SD
 * exp(log_sd[0]) * cosh(unconstrained_rho[0]). Targets must form one
 * contiguous series. This class does not change random-effect registration.
 */
template <typename Type>
struct GaussianProcessLPDF : public DensityComponentBase<Type> {
  bool rw1 =
      false; /*!< First-order random walk instead of a stationary process. */
  bool initial_fixed = false; /*!< Condition on the supplied first state. */
  Type initial_mean =
      static_cast<Type>(0); /*!< Proper initial residual mean. */
  Type log_initial_sd = static_cast<Type>(0); /*!< Proper initial log-SD. */
  bool ar1 = false;          /*!< Use stationary AR(1), otherwise IID. */
  fims::Vector<Type> log_sd; /*!< Scalar innovation log-SD. */
  fims::Vector<Type> unconstrained_rho; /*!< Scalar atanh correlation. */

  /** @brief Check metadata and links before accessing target values. */
  size_t Validate() {
    if (this->input_type != "" && this->input_type != "random_effects") {
      throw std::invalid_argument(
          "GaussianProcess supports process inputs only");
    }
    if (this->input_type == "random_effects" &&
        (this->re == nullptr || this->re_expected_values == nullptr)) {
      throw std::invalid_argument(
          "GaussianProcess target or mean is not linked");
    }
    const size_t n = this->get_n_x();
    const size_t n_mean = this->input_type == "random_effects"
                              ? this->re_expected_values->size()
                              : this->expected_values.size();
    if (n == 0 || (n_mean != 1 && n_mean != n)) {
      throw std::invalid_argument(
          "GaussianProcess requires a nonempty target "
          "and a scalar or matching mean");
    }
    if (log_sd.size() != 1 || unconstrained_rho.size() != 1) {
      throw std::invalid_argument(
          "GaussianProcess requires scalar log_sd and unconstrained_rho");
    }
    return n;
  }

  /** @brief The scalar or elementwise mean of the process target. */
  Type Mean(size_t i) const {
    const auto& means = this->input_type == "random_effects"
                            ? *this->re_expected_values
                            : this->expected_values;
    return means[means.size() == 1 ? 0 : i];
  }

  /** @brief Correlation on the natural scale. */
  Type Rho() const {
    using std::tanh;
    return rw1 ? static_cast<Type>(1)
               : (ar1 ? tanh(unconstrained_rho[0]) : static_cast<Type>(0));
  }

  /** @brief Stationary initial log-SD; avoids subtracting rho squared from one.
   */
  Type InitialLogSD() const {
    using std::cosh;
    if (rw1) return log_initial_sd;
    return log_sd[0] + (ar1 ? fims_math::log(cosh(unconstrained_rho[0]))
                            : static_cast<Type>(0));
  }

  /** @brief Return the joint log density including the initial normalizer. */
  const Type evaluate() override {
    const size_t n = Validate();
    const Type rho = Rho();
    Type previous = static_cast<Type>(0);
    this->lpdf = static_cast<Type>(0);
    this->lpdf_vec.resize(n);
    for (size_t i = 0; i < n; ++i) {
      const Type residual = this->get_observed(i) - Mean(i);
      if (i == 0 && rw1 && initial_fixed) {
        this->lpdf_vec[i] = static_cast<Type>(0);
        previous = residual;
        continue;
      }
      const Type log_scale = i == 0 ? InitialLogSD() : log_sd[0];
      const Type location = i == 0 && rw1 ? initial_mean : rho * previous;
      const Type standardized =
          (residual - location) * fims_math::exp(-log_scale);
      // log(sqrt(2*pi)), with all likelihood normalizers retained.
      this->lpdf_vec[i] = -static_cast<Type>(0.91893853320467274178) -
                          log_scale -
                          static_cast<Type>(0.5) * standardized * standardized;
      this->lpdf += this->lpdf_vec[i];
      previous = residual;
    }
    return this->lpdf;
  }

  /**
   * @brief Draw a path about the supplied means using a standard-normal draw.
   * @details Does not mutate registered targets or recompute biological means.
   * A full unconditional assessment simulation needs a separate time scheduler.
   */
  template <typename DrawNormal>
  fims::Vector<Type> Simulate(DrawNormal draw_normal) {
    return SimulateFrom(0, draw_normal);
  }

  /** @brief Continue a path conditional on all supplied targets before first.
   */
  template <typename DrawNormal>
  fims::Vector<Type> SimulateFrom(size_t first, DrawNormal draw_normal) {
    const size_t n = Validate();
    if (first > n) {
      throw std::invalid_argument(
          "GaussianProcess continuation index is out of range");
    }
    fims::Vector<Type> draws(n);
    Type previous = static_cast<Type>(0);
    const Type rho = Rho();
    for (size_t i = 0; i < n; ++i) {
      if (i < first || (i == 0 && rw1 && initial_fixed)) {
        draws[i] = this->get_observed(i);
        previous = draws[i] - Mean(i);
      } else {
        const Type log_scale = i == 0 ? InitialLogSD() : log_sd[0];
        const Type location = i == 0 && rw1 ? initial_mean : rho * previous;
        previous = location + fims_math::exp(log_scale) * draw_normal();
        draws[i] = Mean(i) + previous;
      }
    }
    return draws;
  }
};
}  // namespace fims_distributions
#endif
