/**
 * @file log_r.hpp
 * @brief Encorporates error using the log recruitment approach.
 * @details This function inherits from recruitment base.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_LOG_R_HPP
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_LOG_R_HPP

#include "recruitment_base.hpp"
#include "../../../common/fims_vector.hpp"

namespace fims_popdy {

/** @brief Log Devs class that returns the log of the input added to the log of the recruitment deviations.
 */
template <typename Type>
struct LogR : public RecruitmentBase<Type> {
  LogR() : RecruitmentBase<Type>() {}

  virtual ~LogR() {}

  /** @brief Log of recruitment approach to adding error to expected recruitment.
   *
   * The Log Recruitment implementation:
   * \f$ \text{log expected recruitment} \f$
   * 
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate_process(size_t pos) {
    return this->recruitment->log_r[pos];
  }

  /** Empty return of base class function */
  virtual const Type evaluate_mean(const Type& spawners, const Type& phi_0){
    return 0;
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_LOG_R_HPP */
