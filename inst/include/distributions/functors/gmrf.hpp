/**
 * @file gmrf.hpp
 * @brief Implements the GMRF distribution functor used by FIMS to
 * evaluate the log-likelihood of Gaussian Markov Random Fields. This is the 
 * "calculator" that goes into the PrecisionMatrixBuilder which is the "architect" 
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */

#ifndef FIMS_DISTRIBUTIONS_GMRF_HPP
#define FIMS_DISTRIBUTIONS_GMRF_HPP

#include "density_components_base.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/def.hpp"

namespace fims_distributions {

/**
 * @brief GMRF class that returns the log-probability density function (lpdf)
 * of a Gaussian Markov Random Field.
 * @details This implementation relies on TMB's GMRF utility, which efficiently
 * evaluates the density using a sparse precision matrix.
 * @tparam Type The numeric type (e.g., double, TMBad::ad_aug).
 */
template <typename Type>
struct GMRF : public DensityComponentBase<Type> {
    /**
     * @brief The pointer to the sparse precision matrix (Q) for the GMRF.
     * @details The matrix is assembled by a separate precision 
     * builder (e.g., the DSEM builder).
     */
    Eigen::SparseMatrix<Type> precision_matrix_ptr = nullptr;

    /** @brief Constructor. */
    GMRF() : DensityComponentBase<Type>() {}

    /** @brief Destructor. */
    virtual ~GMRF() {}

    /**
     * @brief Evaluates the GMRF log probability density function.
     * @details The evaluation follows: 
     * \f[ \log(f(x)) = -\frac{1}{2} (x - \mu)^T Q (x - \mu) + \frac{1}{2} \log|Q| -\frac{n}{2} \log(2\pi) \f]
     * where \f$x\f$ is the vector of latent states, \f$\mu\f$ is the mean, 
     * \f$Q\f$ is the precision matrix, and \f$n\f$ is the dimension of \f$x\f$.
     * @return The calculated log-probability density.
     */ 
    virtual const Type evaluate() {
        const size_t n_x = this->get_n_x();

        if (this->precision_matrix_ptr == nullptr) {
            throw std::runtime_error(
                "GMRF: Precision matrix pointer is null. "
                "Check GMRF ID and Information::SetupRandomEffects() linkage.");
        }

        if (static_cast<size_t>(this->precision_matrix_ptr->rows()) != n_x) {
            throw std::invalid_argument(
                "GMRF: Dimension mismatch. Precision matrix rows (" + 
                std::to_string(this->precision_matrix_ptr->rows()) + 
                ") must match random effect vector size (" + 
                std::to_string(n_x) + ").");
        }

        // Centering: x - mu because TMB's GMRF expects input centered around a mean of 0.
        vector<Type> x_centered(n_x);
        for (size_t i = 0; i < n_x; ++i) {
            x_centered(static_cast<int>(i)) = this->get_observed(i) - this->get_expected(i);
        }

        // Evaluate TMB GMRF and multiply by -1 to convert from negative log-likelihood to log-likelihood.
        this->lpdf = -1.0 * density::GMRF(*(this->precision_matrix_ptr))(x_centered);

        // Store the scalar result in a length-1 vector.
        this->lpdf_vec.resize(1);
        this->lpdf_vec = this->lpdf;

        return this->lpdf;
    }
};

}  // namespace fims_distributions

#endif
