/**
 * @file precision_builders.hpp
 * @brief Assembles sparse precision matrices for multivariate random-effects 
 * models.
  * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */

#ifndef FIMS_DISTRIBUTIONS_PRECISION_BUILDERS_HPP
#define FIMS_DISTRIBUTIONS_PRECISION_BUILDERS_HPP

#include "../../common/def.hpp"
#include "../../common/fims_vector.hpp"
#include "../../common/fims_math.hpp"

// Only include Eigen's sparse math if compiling for TMB.
// This prevents errors when running basic C++ unit tests.
#ifdef TMB_MODEL
#include <Eigen/Sparse>
#else
// Forward declaration allows this template to compile without TMB/Eigen headers
namespace Eigen {
    template <typename Scalar, int Options = 0, typename StorageIndex = int>
    class SparseMatrix;
}
#endif

namespace fims_distributions {

/**
 * @brief Base class for objects that assemble a sparse precision matrix Q.
 * @details This base class allows the GMRF distribution to work with ANY 
 * builder (DSEM, AR1, spatial).
 */
template <typename Type>
struct PrecisionMatrixBuilderBase {
    virtual ~PrecisionMatrixBuilderBase() {}

    /**
     * @brief Pure virtual method that forces every child (e.g., DSEM) 
     * to provide its own specific version of how to build a matrix.
     */
    virtual Eigen::SparseMatrix<Type> BuildPrecisionMatrixSparse() const = 0;
};

/**
 * @brief DSEM precision-matrix builder based on a Reticular Action Model (RAM).
 * @details This class handles the math for "arrow-and-lag" models (the RAM). DSEM is 
 * implemented here based on the [DSEM package](https://github.com/James-Thorson-NOAA/dsem) 
 * and the integration of DSEM in [RCEATTLE](https://github.com/grantdadams/Rceattle/tree/dev-DSEM).
 */
template <typename Type>
struct DSEMPrecisionMatrixBuilder : public PrecisionMatrixBuilderBase<Type> {
    /**
     * @brief A small container (struct) to hold the "story" of one arrow.
     */
    struct RAMPath {
        int type = 0;         /**< 1 = A path effect (Rho), 2 = Variance (Gamma) */
        int from = 0;         /**< The variable the arrow starts from */
        int to = 0;           /**< The variable the arrow points to */
        int beta_index = 0;   /**< 1-based index into beta_z for this arrow's coefficient. */
    };

    // The dimensions of the "grid" of numbers
    size_t n_time = 0;          /**< Total years in the model */
    size_t n_variables = 0;     /**< Total things being modeled (e.g., species + temperature) */
    std::vector<RAMPath> paths; /**< A list of every arrow drawn by the user */
    fims::Vector<Type> beta_z;  /**< The numeric strengths of those arrows */

    DSEMPrecisionMatrixBuilder() : PrecisionMatrixBuilderBase<Type>() {}
    virtual ~DSEMPrecisionMatrixBuilder() {}

    /**
     * @brief The engine that builds the matrix.
     */
    virtual Eigen::SparseMatrix<Type> BuildPrecisionMatrixSparse() const override {
        #ifndef TMB_MODEL
            throw std::invalid_argument(
                "DSEMPrecisionMatrixBuilder: BuildPrecisionMatrixSparse() requires "
                "compilation with TMB_MODEL defined.");
        #else
        // n_k is the total number of "slots" (years multiplied by variables).
        const size_t n_k = this->n_time * this->n_variables;
        
        // 1. Safety check: If the grid is size zero, stop and tell the user.
        if (n_k == 0) {
            throw std::invalid_argument(
                "DSEMPrecisionMatrixBuilder: n_time and n_variables must be > 0.");
        }

        // 2. Initialize sparse components
        // We create three empty grids that are "sparse"
        Eigen::SparseMatrix<Type> Rho_kk(static_cast<int>(n_k), static_cast<int>(n_k)); // Grid for causal paths (from -> to)
        Eigen::SparseMatrix<Type> Gamma_kk(static_cast<int>(n_k), static_cast<int>(n_k)); // Grid for variances (<->)
        Eigen::SparseMatrix<Type> I_kk(static_cast<int>(n_k), static_cast<int>(n_k));     // "Standard" grid (identity)
        I_kk.setIdentity(); // Fill diagonal with 1s

       // 3. Translate the "arrows" (RAMPath) into triplet lists for efficient sparse matrix construction.
        std::vector<Eigen::Triplet<Type>> rho_triplets;
        std::vector<Eigen::Triplet<Type>> gamma_triplets;
        rho_triplets.reserve(this->paths.size());
        gamma_triplets.reserve(this->paths.size());
        for (size_t r = 0; r < this->paths.size(); ++r) {
            // C++ starts counting at 0, but R starts at 1, so we subtract 1.
            const int from = this->paths[r].from - 1; 
            const int to = this->paths[r].to - 1;

            // Check if the user's arrow points to a slot that doesn't exist.
            if (from < 0 || to < 0 || static_cast<size_t>(from) >= n_k || 
                static_cast<size_t>(to) >= n_k) {
                throw std::invalid_argument(
                    "DSEMPrecisionMatrixBuilder: RAM indices out of bounds.");
            }

            // Get the path coefficient from beta_z using the 1-based index.
            const int b_idx = this->paths[r].beta_index - 1;
            if (b_idx < 0 || static_cast<size_t>(b_idx) >= this->beta_z.size()) {
                throw std::invalid_argument(
                    "DSEMPrecisionMatrixBuilder: beta_index out of bounds for beta_z.");
            }
            Type value = this->beta_z[b_idx];

            // Add the "strength" to the correct triplet list based on arrow type.
            if (this->paths[r].type == 1) {
                rho_triplets.emplace_back(from, to, value);
            } else if (this->paths[r].type == 2) {
                gamma_triplets.emplace_back(from, to, value);
            }
        }

        Rho_kk.setFromTriplets(rho_triplets.begin(), rho_triplets.end());
        Gamma_kk.setFromTriplets(gamma_triplets.begin(), gamma_triplets.end());

        // 4. The final math formula to calculate how likely the data is.
        // We are building: Q = (I - P)^T * V^-1 * (I - P)
        
        // IminusRho is the "causal filter" (I - Paths).
        Eigen::SparseMatrix<Type> IminusRho_kk = I_kk - Rho_kk;

        // V is the covariance grid (Gamma transposed times Gamma).
        Eigen::SparseMatrix<Type> V_kk = Gamma_kk.transpose() * Gamma_kk;
        
        // Vinv is the inverse of V. We use specialized FIMS helpers to make 
        // sure TMB can track this math for its calculus.
        Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> Vinv_dense = 
            fims_math::invertSparseMatrix(V_kk);
        Eigen::SparseMatrix<Type> Vinv_sparse = 
            fims_math::asSparseMatrix(Vinv_dense);

        // Assemble the final precision matrix (Q) and send it to the GMRF.
        return IminusRho_kk.transpose() * Vinv_sparse * IminusRho_kk;
#endif
    }
};

}  // namespace fims_distributions

#endif