/**
 * @file distributions.hpp
 * @brief This distributions module includes any .hpp files within the
 * subfolders so that only this file needs to included in the model.hpp file.
 * @details Defines guards for distributions module outline to define the
 * distributions hpp file if not already defined.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_DISTRIBUTIONS_HPP
#define FIMS_DISTRIBUTIONS_HPP

#include "distributions/functors/density_components_base.hpp"
#include "distributions/functors/lognormal_lpdf.hpp"
#include "distributions/functors/multinomial_lpmf.hpp"
#include "distributions/functors/normal_lpdf.hpp"
#include "distributions/functors/precision_builders.hpp"
#include "distributions/functors/gmrf.hpp"

#endif /* FIMS_DISTRIBUTIONS_HPP */
