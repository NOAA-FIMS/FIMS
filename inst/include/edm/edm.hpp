/**
 * @file edm.hpp
 * @brief Umbrella header for all EDM (Empirical Dynamic Modelling) components.
 *
 * @details Including this single header brings in:
 *  - DelayEmbeddingMatrix, MakeDelayEmbedding, MakeDelayEmbeddingDropMissing
 *  - SquaredEuclideanDistance, SimplexWeights, SMapWeights, NormalizeWeights
 *  - EDMPredictorBase (abstract base for all prediction functors)
 *  - SimplexProjection (Sugihara & May, 1990)
 *
 * Additional prediction algorithms (S-map, GP-EDM) will be added here as
 * further headers are implemented.
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_HPP
#define FIMS_EDM_HPP

#include "functors/delay_embedding.hpp"
#include "utilities/edm_distance_weights.hpp"
#include "functors/edm_predictor_base.hpp"
#include "functors/simplex_projection.hpp"

#endif  // FIMS_EDM_HPP
