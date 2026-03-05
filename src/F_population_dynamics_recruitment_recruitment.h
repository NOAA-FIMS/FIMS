#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_RECRUITMENT_H
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_RECRUITMENT_H

/**
 * @file recruitment.hpp
 * @brief Includes any .hpp files within the subfolders so that only this file
 * needs to included in include statements to use all functions within this
 * folder.
 * @details Defines guards for recruitment module outline to define the
 * recruitment hpp file if not already defined.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_HPP
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_HPP

// #include "functors/recruitment_base.hpp"
// #include "functors/sr_beverton_holt.hpp"
// #include "functors/log_devs.hpp"
// #include "functors/log_r.hpp"

#include "F_population_dynamics_recruitment_functors_recruitment_base.h"
#include "F_population_dynamics_recruitment_functors_sr_beverton_holt.h"
#include "F_population_dynamics_recruitment_functors_log_devs.h"
#include "F_population_dynamics_recruitment_functors_log_r.h"

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_HPP */

#endif
