#ifndef FIMS_DISTRIBUTIONS_DISTRIBUTIONS_H
#define FIMS_DISTRIBUTIONS_DISTRIBUTIONS_H

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

// #include "functors/density_components_base.hpp"
// #include "functors/lognormal_lpdf.hpp"
// #include "functors/multinomial_lpmf.hpp"
// #include "functors/normal_lpdf.hpp"
// Use FIMS_DEPENDS for packaged headers when installing
// #include "functors/density_components_base.hpp"
#include "F_distributions_functors_density_components_base.h"
// #include "functors/lognormal_lpdf.hpp"
#include "F_distributions_functors_lognormal_lpdf.h"
// #include "functors/multinomial_lpmf.hpp"
#include "F_distributions_functors_multinomial_lpmf.h"
// #include "functors/normal_lpdf.hpp"
#include "F_distributions_functors_normal_lpdf.h"

#endif /* FIMS_DISTRIBUTIONS_HPP */

#endif
