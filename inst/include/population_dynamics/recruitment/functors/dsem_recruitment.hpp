#ifndef FIMS_POPULATION_DYNAMICS_RECRUITMENT_DSEM_RECRUITMENT_HPP
#define FIMS_POPULATION_DYNAMICS_RECRUITMENT_DSEM_RECRUITMENT_HPP

#include "recruitment_base.hpp"

namespace fims_popdy {

/**
 * @brief DSEM Recruitment functor.
 * @details This adapter applies DSEM-structured deviations to the mean 
 * stock-recruit relationship.
 */
template <typename Type>
struct DSEMRecruitment : public RecruitmentBase<Type> {
    DSEMRecruitment() : RecruitmentBase<Type>() {}
    virtual ~DSEMRecruitment() {}

    /**
     * @brief Implementation of the process error for DSEM.
     * @details Returns the combined log-recruitment: mean + DSEM latent state.
     */
    virtual const Type evaluate_process(size_t pos) override {
        // Return log_expected (from evaluate_mean) + the latent deviation
        return this->recruitment->log_expected_recruitment[pos] + 
               this->recruitment->log_recruit_devs[pos];
    }

    /**
     * @brief Evaluates the mean SRR. Specific math (Beverton-Holt, etc.) 
     * is handled by the primary recruitment module.
     */
    virtual const Type evaluate_mean(const Type& spawners, const Type& phi_0) override {
        return static_cast<Type>(0.0); // Adapter only handles the process layer
    }
};

} // namespace fims_popdy
#endif
