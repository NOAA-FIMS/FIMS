/**
 * @file edm.hpp
 * @brief Umbrella header for all EDM (Empirical Dynamic Modelling) components.
 *
 * @details Including this single header brings in:
 *  - DelayEmbeddingMatrix, MakeDelayEmbedding, MakeDelayEmbeddingDropMissing
 *  - SquaredEuclideanDistance, SimplexWeights, SMapWeights, NormalizeWeights
 *  - GaussianElimination (shared linear algebra utility)
 *  - ARDKernelElement, BuildCovarianceMatrix, BuildKStarVector (GP kernels)
 *  - EDMPredictorBase (abstract base for all prediction functors)
 *  - SimplexProjection (Sugihara & May, 1990)
 *  - SMapProjection (Sugihara, 1994)
 *  - GPEdmProjection (Munch et al. 2017; Rogers 2023 GPEDM)
 *  - EDMModel (FIMS module: runs a predictor and exposes predictions in
 *              Information<Type>::variable_map for data likelihoods)
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_HPP
#define FIMS_EDM_HPP

#include "functors/delay_embedding.hpp"
#include "utilities/edm_distance_weights.hpp"
#include "utilities/edm_linear_algebra.hpp"
#include "utilities/edm_kernels.hpp"
#include "functors/edm_predictor_base.hpp"
#include "functors/simplex_projection.hpp"
#include "functors/smap_projection.hpp"
#include "functors/gp_edm_projection.hpp"
#include "edm_model.hpp"

#endif  // FIMS_EDM_HPP

