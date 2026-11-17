/** @file dirichlet_multinomial_lpmf.hpp
 * @brief Dirichlet-multinomial composition likelihood with scalar
 * concentration.
 */
#ifndef FIMS_DIRICHLET_MULTINOMIAL_LPMF_HPP
#define FIMS_DIRICHLET_MULTINOMIAL_LPMF_HPP
#include <limits>
#include "multinomial_lpmf.hpp"
#include "../../common/fims_math.hpp"

namespace fims_distributions {
/** Counts have the same row-major layout and missing-row convention as
 * multinomial. Fractional counts evaluate the gamma extension (a weighted
 * likelihood, not a PMF). For integer counts: q ~ Dirichlet(kappa*p), x|q ~
 * Multinomial(n,q).
 */
template <typename Type>
struct DirichletMultinomialLPMF : public MultinomialLPMF<Type> {
  fims::Vector<Type> log_concentration;
  static Type LogGamma(const Type& x) {
#ifdef TMB_MODEL
    return ::lgamma(x);
#else
    return std::lgamma(x);
#endif
  }

  const Type evaluate() override {
    if (this->input_type != "" && this->input_type != "data")
      throw std::invalid_argument(
          "DM supports data or standalone compositions only");
    if (log_concentration.size() != 1)
      throw std::invalid_argument("DM requires scalar log_concentration");
    const Type kappa = fims_math::exp(log_concentration[0]);
    if (!std::isfinite(fims_math::Value(kappa)) || fims_math::Value(kappa) <= 0)
      throw std::invalid_argument(
          "DM concentration must be finite and positive");
    if (this->input_type == "data") {
      if (!this->data_observed_values || !this->data_expected_values)
        throw std::invalid_argument("DM data and predictions must be linked");
      this->dims = {this->data_observed_values->get_imax(),
                    this->data_observed_values->get_jmax()};
    }
    if (this->dims.size() != 2 || this->dims[0] == 0 || this->dims[1] == 0)
      throw std::invalid_argument(
          "DM requires positive row and bin dimensions");
    const size_t n_cells = this->dims[0] * this->dims[1];
    const size_t n_expected = this->input_type == "data"
                                  ? this->data_expected_values->size()
                                  : this->expected_values.size();
    const size_t n_observed =
        this->input_type == "data" ? n_cells : this->observed_values.size();
    if (n_expected != n_cells || n_observed != n_cells)
      throw std::invalid_argument(
          "DM dimensions must match observed and expected vectors");
    this->lpdf = Type(0);
    this->lpdf_vec.resize(n_cells);
    std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), Type(0));
    for (size_t row = 0; row < this->dims[0]; ++row) {
      bool missing = false;
      for (size_t j = 0; j < this->dims[1]; ++j)
        if (this->input_type == "data" &&
            this->get_observed(row, j) == this->data_observed_values->na_value)
          missing = true;
      if (missing) continue;
      Type total = Type(0), mass = Type(0), value = Type(0);
      bool impossible = false;
      for (size_t j = 0; j < this->dims[1]; ++j) {
        const size_t index = row * this->dims[1] + j;
        const Type x = this->input_type == "data" ? this->get_observed(row, j)
                                                  : this->get_observed(index);
        const Type p = this->get_expected(index);
        if (!std::isfinite(fims_math::Value(x)) || x < Type(0) ||
            !std::isfinite(fims_math::Value(p)) || p < Type(0))
          throw std::invalid_argument(
              "DM requires nonnegative finite counts and probabilities");
        total += x;
        mass += p;
        value -= LogGamma(x + Type(1));
        // x=0 contributes exactly zero even at structural p=0; no pseudocounts.
        if (x > Type(0)) {
          if (p == Type(0))
            impossible = true;
          else
            value += LogGamma(kappa * p + x) - LogGamma(kappa * p);
        }
      }
      if (std::abs(fims_math::Value(mass) - 1.0) > 1e-6)
        throw std::invalid_argument("DM probabilities must sum to one");
      value +=
          LogGamma(total + Type(1)) + LogGamma(kappa) - LogGamma(kappa + total);
      if (impossible) value = Type(-std::numeric_limits<double>::infinity());
      this->lpdf += value;
      std::fill(this->lpdf_vec.begin() + row * this->dims[1],
                this->lpdf_vec.begin() + (row + 1) * this->dims[1],
                value / Type(this->dims[1]));
    }
    return this->lpdf;
  }
};
}  // namespace fims_distributions
#endif
