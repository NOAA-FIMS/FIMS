/**
 * @file selectivity.hpp
 * @brief Includes any .hpp files within the subfolders so that only this file
 * needs to included in include statements to use all functions within this
 * folder.
 * @details Defines guards for selectivity module outline to define the
 * selectivity hpp file if not already defined.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_SELECTIVITY_HPP
#define FIMS_POPULATION_DYNAMICS_SELECTIVITY_HPP

// #include "functors/double_logistic.hpp"
// #include "functors/logistic.hpp"
// #include "functors/selectivity_base.hpp"
// Use FIMS_DEPENDS for packaged headers when installing
// #include "functors/double_logistic.hpp"
FIMS_DEPENDS(double_logistic.hpp);
// #include "functors/logistic.hpp"
FIMS_DEPENDS(logistic.hpp);
// #include "functors/selectivity_base.hpp"
FIMS_DEPENDS(selectivity_base.hpp);

#endif /* FIMS_POPULATION_DYNAMICS_SELECTIVITY_HPP */
