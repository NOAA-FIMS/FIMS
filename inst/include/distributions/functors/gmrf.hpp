/**
 * @file gmrf.hpp
 * @brief Implements the GMRF distribution functor used by FIMS to
 * evaluate the log-likelihood of Gaussian Markov Random Fields.
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
     * @brief The sparse precision matrix (Q) for the GMRF.
     * @details This matrix is typically assembled by a separate precision 
     * builder (e.g., the DSEM builder).
     */
    Eigen::SparseMatrix<Type> precision_matrix;

    /** @brief Constructor. */
    GMRF() : DensityComponentBase<Type>() {}

    /** @brief Destructor. */
    virtual ~GMRFLPDF() {}

    /**
     * @brief Evaluates the GMRF log probability density function.
     * @details The evaluation follows: 
     * \f[ \log(f(x)) = -\frac{1}{2} (x - \mu)^T Q (x - \mu) + \frac{1}{2} \log|Q| \f]
     * where \f$x\f$ is the vector of latent states, \f$\mu\f$ is the mean, 
     * and \f$Q\f$ is the precision matrix.
     * @return The calculated log-probability density.
     */ 
    virtual const Type evaluate() {
        const size_t n_x = this->get_n_x();

        // Are the following commented out lines needed?
        // this->lpdf_vec.resize(n_x); 
        // std::fill(this->lpdf_vec.begin(), this->lpdf_vec.end(), static_cast<Type>(0));
        this->lpdf = static_cast<Type>(0);

        // Centering: x - mu
        for (size_t i = 0; i < n_x; ++i) {
        x_centered(static_cast<int>(i)) = this->get_observed(i) - this->get_expected(i);
        }

        // Dimension Check
        if (static_cast<size_t>(precision_matrix.rows()) != n_x) {
        throw std::invalid_argument(
            "GMRFLPDF: Precision matrix dimension mismatch. Got rows: " + 
            std::to_string(precision_matrix.rows()) + " but expected: " + 
            std::to_string(n_x) + "."
        );
        }

        // Evaluate TMB GMRF
        this->lpdf = -1.0 * density::GMRF(precision_matrix)(x_centered);

        return this->lpdf;
    }
};

}  // namespace fims_distributions

#endif
