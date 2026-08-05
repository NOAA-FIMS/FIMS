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

    // Builds the precision matrix Q
    virtual Eigen::SparseMatrix<Type> BuildPrecisionMatrixSparse() const = 0;

    // Optional mean offset. Defaults to a zero vector.
    virtual Eigen::Matrix<Type, Eigen::Dynamic, 1> GetMeanOffset(
        size_t n_k) const {
        return Eigen::Matrix<Type, Eigen::Dynamic, 1>::Zero(n_k);
    }
};

/**
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
        /** @brief 1 = A path effect (Rho), 2 = Variance (Gamma) */
        int type = 0;
        /** @brief The variable the arrow starts from */
        int from = 0;
        /** @brief The variable the arrow points to */
        int to = 0;
        /** @brief 1-based index into beta_z for this arrow's coefficient. */
        int beta_index = 0;
    };

    // The dimensions of the "grid" of numbers
    /** @brief Total years in the model */
    size_t n_time = 0;
    /** @brief Total things being modeled (e.g., species + temperature) */
    size_t n_variables = 0;
    /** @brief A list of every arrow drawn by the user */
    fims::Vector<RAMPath> paths;
    /** @brief The numeric strengths of those arrows */
    fims::Vector<Type> beta_z;
    /** @brief Initial conditions for the mean offset */
    fims::Vector<Type> delta0_j;

    DSEMPrecisionMatrixBuilder() : PrecisionMatrixBuilderBase<Type>() {}
    virtual ~DSEMPrecisionMatrixBuilder() {}

    /**
     * @brief The engine that builds the matrix, choosing the optimal strategy.
     */
    virtual Eigen::SparseMatrix<Type> BuildPrecisionMatrixSparse() const override {
        #ifndef TMB_MODEL
            throw std::invalid_argument(
                "DSEMPrecisionMatrixBuilder: BuildPrecisionMatrixSparse() requires "
                "compilation with TMB_MODEL defined.");
        #else
            bool has_cross_year_cov = false;
            // Pre-scan paths to determine the optimal inversion strategy
            for (size_t r = 0; r < this->paths.size(); ++r) {
                if (this->paths[r].type == 2) {
                    size_t path_t_from = (this->paths[r].from - 1) / this->n_variables;
                    size_t path_t_to = (this->paths[r].to - 1) / this->n_variables;

                    if (path_t_from != path_t_to) {
                        has_cross_year_cov = true;
                        break;  // Stop scanning as soon as we find one
                    }
                }
            }

            if (has_cross_year_cov) {
                // Fallback: Slower, but mathematically robust for complex time linkages
                return BuildQ_FullInversion();
            } else {
                // Fast Path: Highly optimized block-diagonal inversion
                return BuildQ_BlockInversion();
            }
        #endif
    }

    virtual Eigen::Matrix<Type, Eigen::Dynamic, 1> GetMeanOffset(
        size_t n_k) const override {
        #ifndef TMB_MODEL
            return Eigen::Matrix<Type, Eigen::Dynamic, 1>::Zero(n_k);
        #else
            if (this->delta0_j.size() == 0) {
                return Eigen::Matrix<Type, Eigen::Dynamic, 1>::Zero(n_k);
            }

            // Initialize the offset matrix (n_time x n_variables)
            Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> delta_tj(
                this->n_time, this->n_variables);
            delta_tj.setZero();

            // Loop forward through time
            for (size_t t = 0; t < this->n_time; ++t) {
                // 1. Gather incoming effects from previous time steps
                Eigen::Matrix<Type, Eigen::Dynamic, 1> incoming_delta =
                    Eigen::Matrix<Type, Eigen::Dynamic, 1>::Zero(this->n_variables);

                // At t=0, the "incoming" effect is just the user-provided initial
                // condition
                if (t == 0) {
                    if (this->delta0_j.size() != this->n_variables) {
                        throw std::invalid_argument(
                            "DSEMPrecisionMatrixBuilder: delta0_j size must match n_variables.");
                    }
                    for (size_t j = 0; j < this->n_variables; ++j) {
                        incoming_delta(j) = this->delta0_j[j];
                    }
                }

                // Add effects propagating from PAST time steps
                for (size_t r = 0; r < this->paths.size(); ++r) {
                    if (this->paths[r].type == 1) {  // Type 1 = Causal Path (P matrix)
                        size_t t_from = (this->paths[r].from - 1) / this->n_variables;
                        size_t t_to = (this->paths[r].to - 1) / this->n_variables;

                        if (t_to == t && t_from < t) {
                            size_t j_from = (this->paths[r].from - 1) % this->n_variables;
                            size_t j_to = (this->paths[r].to - 1) % this->n_variables;

                            Type path_val = this->beta_z[this->paths[r].beta_index - 1];

                            incoming_delta(j_to) += path_val * delta_tj(t_from, j_from);
                        }
                    }
                }

                // 2. Gather WITHIN-YEAR causal paths (creates a local I - P matrix)
                Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> I_minus_P_local =
                    Eigen::Matrix<Type, Eigen::Dynamic,
                    Eigen::Dynamic>::Identity(this->n_variables,
                                           this->n_variables);

                bool has_local_paths = false;
                for (size_t r = 0; r < this->paths.size(); ++r) {
                    if (this->paths[r].type == 1) {
                        size_t t_from = (this->paths[r].from - 1) / this->n_variables;
                        size_t t_to = (this->paths[r].to - 1) / this->n_variables;

                        if (t_to == t && t_from == t) {
                            size_t j_from = (this->paths[r].from - 1) % this->n_variables;
                            size_t j_to = (this->paths[r].to - 1) % this->n_variables;

                            Type path_val = this->beta_z[this->paths[r].beta_index - 1];
                            I_minus_P_local(j_to, j_from) -= path_val;
                            has_local_paths = true;
                        }
                    }
                }

                // 3. Solve for this year's final delta
                if (has_local_paths) {
                    delta_tj.row(t) =
                        I_minus_P_local.colPivHouseholderQr().solve(incoming_delta).transpose();
                } else {
                    delta_tj.row(t) = incoming_delta.transpose();
                }
            }

            // Flatten the matrix into a column vector for the final offset
            Eigen::Matrix<Type, Eigen::Dynamic, 1> computed_delta_vector(n_k);
            for (size_t t = 0; t < this->n_time; ++t) {
                for (size_t j = 0; j < this->n_variables; ++j) {
                    computed_delta_vector(t * this->n_variables + j) = delta_tj(t, j);
                }
            }
            return computed_delta_vector;
        #endif
    }

   private:
    /**
     * @brief Builds Q using block inversion; fast but requires no cross-year
     * covariance.
     */
    Eigen::SparseMatrix<Type> BuildQ_BlockInversion() const {
        #ifdef TMB_MODEL
            const size_t n_k = this->n_time * this->n_variables;
            
            // 1. Safety check: If the grid is size zero, stop and tell the user.
            if (n_k == 0) {
                throw std::invalid_argument(
                    "DSEMPrecisionMatrixBuilder: n_time and n_variables must be > 0.");
            }

            Eigen::SparseMatrix<Type> I_kk(static_cast<int>(n_k), static_cast<int>(n_k));
            I_kk.setIdentity();

            // 3. Translate the "arrows" (RAMPath) into triplet lists for efficient sparse matrix construction.
            fims::Vector<Eigen::Triplet<Type>> rho_triplets;
            fims::Vector<Eigen::Triplet<Type>> vinv_triplets;
            rho_triplets.reserve(this->paths.size());
            vinv_triplets.reserve(this->n_time * this->n_variables * this->n_variables);

            // 4. Build Rho (Directed Paths)
            for (size_t r = 0; r < this->paths.size(); ++r) {
                if (this->paths[r].type == 1) { // Directed path (Rho)
                    const int from_global = this->paths[r].from - 1; 
                    const int to_global = this->paths[r].to - 1;
                    if (from_global < 0 || to_global < 0 ||
                        static_cast<size_t>(from_global) >= n_k ||
                        static_cast<size_t>(to_global) >= n_k) {
                        throw std::invalid_argument(
                            "DSEMPrecisionMatrixBuilder: RAM indices out of bounds.");
                    }
                    const int b_idx = this->paths[r].beta_index - 1;
                    if (b_idx < 0 || static_cast<size_t>(b_idx) >= this->beta_z.size()) {
                        throw std::invalid_argument(
                            "DSEMPrecisionMatrixBuilder: beta_index out of bounds for beta_z.");
                    }
                    rho_triplets.emplace_back(to_global, from_global, this->beta_z[b_idx]);
                }
            }
        
            // 5. Build V_inv via Block Inversion
            // This assumes global indices are generated as: 
            // global_index = (time_step * n_variables) + variable_index.
            // If your R data is vectorized column-major (time inside, variables outside),
            // adjust the division and modulo math below accordingly.
            for (size_t t = 0; t < this->n_time; ++t) {
                Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> V_block =
                    Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic>::Zero(
                        this->n_variables, this->n_variables);
                bool has_variance_for_block = false;

                for (size_t r = 0; r < this->paths.size(); ++r) {
                    if (this->paths[r].type == 2) {
                        int from_global = this->paths[r].from - 1;
                        int to_global = this->paths[r].to - 1;
                        size_t path_t_from = from_global / this->n_variables;

                        if (path_t_from == t) {
                            has_variance_for_block = true;
                            int from_local = from_global % this->n_variables;
                            int to_local = to_global % this->n_variables;
                            const int b_idx = this->paths[r].beta_index - 1;
                            Type value = this->beta_z[b_idx];

                            if (from_local == to_local) {
                                V_block(from_local, to_local) = value * value;
                            } else {
                                V_block(from_local, to_local) = value;
                                V_block(to_local, from_local) = value;
                            }
                        }
                    }
                }
            
                // Invert the dense block and map it back to the global sparse format
                if (has_variance_for_block) {
                    Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> Vinv_block = V_block.inverse();
                    
                    for (size_t i = 0; i < this->n_variables; ++i) {
                        for (size_t j = 0; j < this->n_variables; ++j) {
                            if (Vinv_block(i, j) != Type(0.0)) {
                                int global_i = (t * this->n_variables) + i;
                                int global_j = (t * this->n_variables) + j;
                                vinv_triplets.emplace_back(global_i, global_j, Vinv_block(i, j));
                            }
                        }
                    }
                }
            }

        // 6. Final Sparse Matrix Assembly
        Eigen::SparseMatrix<Type> Rho_kk(static_cast<int>(n_k), static_cast<int>(n_k));
        Rho_kk.setFromTriplets(rho_triplets.begin(), rho_triplets.end());
        Eigen::SparseMatrix<Type> Vinv_sparse(static_cast<int>(n_k),
                                              static_cast<int>(n_k));
        Vinv_sparse.setFromTriplets(vinv_triplets.begin(), vinv_triplets.end());
        Eigen::SparseMatrix<Type> IminusRho_kk = I_kk - Rho_kk;
        return IminusRho_kk.transpose() * Vinv_sparse * IminusRho_kk;

        #endif
    
    return Eigen::SparseMatrix<Type>();
    }

    // Calculate Q = (I - P)^T * V^-1 * (I - P)
    /**
     * @brief Builds Q using full matrix inversion; general but slower.
     */
    Eigen::SparseMatrix<Type> BuildQ_FullInversion() const {
        #ifdef TMB_MODEL
        const size_t n_k = this->n_time * this->n_variables;
        if (n_k == 0) {
            throw std::invalid_argument(
                "DSEMPrecisionMatrixBuilder: n_time and n_variables must be > 0.");
        }

        Eigen::SparseMatrix<Type> I_kk(static_cast<int>(n_k), static_cast<int>(n_k));
        I_kk.setIdentity();

        fims::Vector<Eigen::Triplet<Type>> rho_triplets;
        rho_triplets.reserve(this->paths.size());
        for (size_t r = 0; r < this->paths.size(); ++r) {
            if (this->paths[r].type == 1) {
                const int from_global = this->paths[r].from - 1;
                const int to_global = this->paths[r].to - 1;
                if (from_global < 0 || to_global < 0 ||
                    static_cast<size_t>(from_global) >= n_k ||
                    static_cast<size_t>(to_global) >= n_k) {
                    throw std::invalid_argument(
                        "DSEMPrecisionMatrixBuilder: RAM indices out of bounds.");
                }
                const int b_idx = this->paths[r].beta_index - 1;
                if (b_idx < 0 || static_caste<size_t>(b_idx) >= this->beta_z.size()) {
                    throw std::invalid_argument(
                        "DSEMPrecisionMatrixBuilder: beta_index out of bounds for beta_z.");
                }
                rho_triplets.emplace_back(to_global, from_global, this->beta_z[b_idx]);
            }
        }
        Eigen::SparseMatrix<Type> Rho_kk(static_cast<int>(n_k),
                                         static_cast<int>(n_k));
        Rho_kk.setFromTriplets(rho_triplets.begin(), rho_triplets.end());

        Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> V_dense =
            Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic>::Zero(n_k, n_k);
        for (size_t r = 0; r < this->paths.size(); ++r) {
            if (this->paths[r].type == 2) {
                int from_global = this->paths[r].from - 1;
                int to_global = this->paths[r].to - 1;
                const int b_idx = this->paths[r].beta_index - 1;
                Type value = this->beta_z[b_idx];

                if (from_global == to_global) {
                    V_dense(from_global, to_global) = value * value;
                } else {
                    V_dense(from_global, to_global) = value;
                    V_dense(to_global, from_global) = value;
                }
            }
        }

        Eigen::Matrix<Type, Eigen::Dynamic, Eigen::Dynamic> Vinv_dense =
            V_dense.inverse();
        Eigen::SparseMatrix<Type> Vinv_sparse = Vinv_dense.sparseView();

        Eigen::SparseMatrix<Type> IminusRho_kk = I_kk - Rho_kk;
        return IminusRho_kk.transpose() * Vinv_sparse * IminusRho_kk;
        #endif
        return Eigen::SparseMatrix<Type>();
    }
};

}  // namespace fims_distributions

#endif