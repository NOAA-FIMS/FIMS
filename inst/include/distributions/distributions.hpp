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
FIMS_DEPENDS(density_components_base);
// #include "functors/lognormal_lpdf.hpp"
FIMS_DEPENDS(lognormal_lpdf);
// #include "functors/multinomial_lpmf.hpp"
FIMS_DEPENDS(multinomial_lpmf);
// #include "functors/normal_lpdf.hpp"
FIMS_DEPENDS(normal_lpdf);

#endif /* FIMS_DISTRIBUTIONS_HPP */
