/**
 * @file edm_linear_algebra.hpp
 * @brief Shared linear algebra utilities for EDM prediction algorithms.
 *
 * @details Linear solves across EDM functors (SMapProjection and GPEdmProjection)
 * are standardized on Eigen::LDLT Cholesky factorization for numerical stability
 * and differentiable linear algebra.
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_LINEAR_ALGEBRA_HPP
#define FIMS_EDM_LINEAR_ALGEBRA_HPP

namespace fims_edm {

// Linear algebra for EDM predictors (S-Map, GP-EDM) uses Eigen::LDLT directly
// for matrix factorization and solving linear systems.

}  // namespace fims_edm

#endif  // FIMS_EDM_LINEAR_ALGEBRA_HPP
