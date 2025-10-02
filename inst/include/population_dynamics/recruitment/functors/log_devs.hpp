/**
 * @file log_devs.hpp
 * @brief Incorporates error using the log recruitment deviations approach.
 * @details This function inherits from recruitment base.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_LOG_DEVS_HPP
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_LOG_DEVS_HPP

#include "recruitment_base.hpp"
#include "../../../common/fims_vector.hpp"

namespace fims_popdy {

/** @brief Log Devs class that returns the log of the input added to the log of
 * the recruitment deviations.
 */
template <typename Type>
struct LogDevs : public RecruitmentBase<Type> {
  LogDevs() : RecruitmentBase<Type>() {}

  virtual ~LogDevs() {}

  /** @brief Log of the recruitment deviations approach to adding error to
   * expected recruitment.
   *
   * The Log Recruitment Deviation implementation:
   * \f$ \text{log expected recruitment} + log_devs \f$
   *
   * @param pos Position index, e.g., which year.
   */
  virtual const Type evaluate_process(size_t pos) {
    return this->recruitment->log_expected_recruitment[pos] +
           this->recruitment->log_recruit_devs[pos];
  }

  /** Empty return of base class function */
  virtual const Type evaluate_mean(const Type& spawners, const Type& phi_0) {
    return 0;
  }
  /**
   * @brief Create a map of report vectors for the recruitment object.
   */
  virtual std::map<std::string, fims::Vector<Type>>
  create_report_vectors_map() {
    std::map<std::string, fims::Vector<Type>> report_vectors;
    return report_vectors;
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_RECRUITMENT_LOG_DEVS_HPP */
