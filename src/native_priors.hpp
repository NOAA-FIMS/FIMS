/**
 * \file native_priors.hpp
 * \brief Internal prior target resolution and registration for native backends.
 *
 * Kept separate from model assembly so supported parameters and prior ownership
 * can be reviewed together. Templates are shared by double and AD backends.
 */
#ifndef FIMS_NATIVE_PRIORS_HPP
#define FIMS_NATIVE_PRIORS_HPP

#include "../inst/include/interface/call/fleet_registry.hpp"
#include "../inst/include/interface/call/maturity_registry.hpp"
#include "../inst/include/interface/call/population_registry.hpp"
#include "../inst/include/interface/call/recruitment_registry.hpp"
#include "../inst/include/interface/call/selectivity_registry.hpp"
#include "../inst/include/common/information.hpp"
#include <Rinternals.h>
#include <memory>
#include <string>

namespace fims_native
{
    enum PriorEstimationTypeCode
    {
        kPriorConstant = 0,
        kPriorFixedEffects = 1,
        kPriorRandomEffects = 2
    };

    template <typename Type>
    fims::Vector<Type> *find_fleet_prior_target(
        uint32_t object_id, const std::string &parameter)
    {
        auto object = NativeFleetRegistry<Type>::GetInstance()->GetFleet(object_id);
        if (!object) return NULL;
        if (parameter == "log_Fmort") return &object->log_Fmort;
        if (parameter == "log_q") return &object->log_q;
        return NULL;
    }

    template <typename Type>
    fims::Vector<Type> *find_population_prior_target(
        uint32_t object_id, const std::string &parameter)
    {
        auto object = NativePopulationRegistry<Type>::GetInstance()->GetPopulation(object_id);
        if (!object) return NULL;
        if (parameter == "log_M") return &object->log_M;
        if (parameter == "log_f_multiplier") return &object->log_f_multiplier;
        if (parameter == "log_init_naa") return &object->log_init_naa;
        return NULL;
    }

    template <typename Type>
    fims::Vector<Type> *find_recruitment_prior_target(
        uint32_t object_id, const std::string &parameter)
    {
        auto object = NativeRecruitmentRegistry<Type>::GetInstance()->GetBevertonHoltRecruitment(object_id);
        if (!object) return NULL;
        if (parameter == "logit_steep") return &object->logit_steep;
        if (parameter == "log_rzero") return &object->log_rzero;
        if (parameter == "log_devs") return &object->log_recruit_devs;
        return NULL;
    }

    template <typename Type>
    fims::Vector<Type> *find_maturity_prior_target(
        uint32_t object_id, const std::string &parameter)
    {
        auto object = NativeMaturityRegistry<Type>::GetInstance()->GetLogisticMaturity(object_id);
        if (!object) return NULL;
        if (parameter == "inflection_point") return &object->inflection_point;
        if (parameter == "slope") return &object->slope;
        return NULL;
    }

    template <typename Type>
    fims::Vector<Type> *find_selectivity_prior_target(
        uint32_t object_id, const std::string &parameter)
    {
        auto logistic = NativeSelectivityRegistry<Type>::GetInstance()->GetLogisticSelectivity(object_id);
        if (logistic)
        {
            if (parameter == "inflection_point") return &logistic->inflection_point;
            if (parameter == "slope") return &logistic->slope;
        }
        auto double_logistic = NativeSelectivityRegistry<Type>::GetInstance()->GetDoubleLogisticSelectivity(object_id);
        if (!double_logistic) return NULL;
        if (parameter == "inflection_point_asc") return &double_logistic->inflection_point_asc;
        if (parameter == "slope_asc") return &double_logistic->slope_asc;
        if (parameter == "inflection_point_desc") return &double_logistic->inflection_point_desc;
        if (parameter == "slope_desc") return &double_logistic->slope_desc;
        return NULL;
    }

    // Dispatch once by module; each resolver owns its supported parameter names.
    // Keep Selectivity's logistic-first fallback to double-logistic unchanged.
    template <typename Type>
    fims::Vector<Type> *find_prior_target(const std::string &module,
                                        uint32_t object_id,
                                        const std::string &parameter)
    {
        if (module == "Fleet")
            return find_fleet_prior_target<Type>(object_id, parameter);
        if (module == "Population")
            return find_population_prior_target<Type>(object_id, parameter);
        if (module == "Recruitment")
            return find_recruitment_prior_target<Type>(object_id, parameter);
        if (module == "Maturity")
            return find_maturity_prior_target<Type>(object_id, parameter);
        if (module == "Selectivity")
            return find_selectivity_prior_target<Type>(object_id, parameter);
        return NULL;
    }

    template <typename Type>
    void register_prior_parameter(std::shared_ptr<fims_info::Information<Type>> info,
                                  Type &value, int estimation_type,
                                  const std::string &name)
    {
        if (estimation_type == kPriorConstant) return;
        if (estimation_type == kPriorFixedEffects)
        {
            info->RegisterParameterName(name);
            info->RegisterParameter(value);
            return;
        }
        if (estimation_type == kPriorRandomEffects)
            Rf_error("Prior means and standard deviations cannot be random effects.");
        Rf_error("Unknown prior estimation type code %d.", estimation_type);
    }

    template <typename Type, typename Distribution>
    void add_prior_internal(const std::string &module, uint32_t object_id,
                            const std::string &parameter,
                            const double *mean, size_t mean_size,
                            const double *log_sd, size_t log_sd_size,
                            int mean_estimation_type,
                            int log_sd_estimation_type,
                            const std::string &distribution_name)
    {
        fims::Vector<Type> *target = find_prior_target<Type>(module, object_id, parameter);
        if (target == NULL)
            Rf_error("Could not find native prior target `%s.%u.%s`.",
                     module.c_str(), object_id, parameter.c_str());
        const size_t target_size = target->size();
        if (target_size == 0)
            Rf_error("Native prior target `%s.%u.%s` is empty.",
                     module.c_str(), object_id, parameter.c_str());
        if (mean_size != 1 && mean_size != target_size)
            Rf_error("`mean` must have length 1 or match the prior target length (%zu).", target_size);
        if (log_sd_size != 1 && log_sd_size != target_size)
            Rf_error("`log_sd` must have length 1 or match the prior target length (%zu).", target_size);

        auto info = fims_info::Information<Type>::GetInstance();
        auto prior = std::make_shared<Distribution>();
        prior->input_type = "prior";
        prior->priors.resize(1);
        prior->priors[0] = target;
        prior->expected_values.resize(target_size);
        prior->expected_mean.resize(mean_size);
        prior->use_mean = "yes";
        prior->log_sd.resize(log_sd_size);
        for (size_t i = 0; i < target_size; ++i) prior->expected_values[i] = static_cast<Type>(0.0);
        for (size_t i = 0; i < mean_size; ++i)
        {
            prior->expected_mean[i] = static_cast<Type>(mean[i]);
            register_prior_parameter(info, prior->expected_mean[i], mean_estimation_type,
                                     distribution_name + "." + fims::to_string(prior->id) + ".mean." + fims::to_string(i));
        }
        for (size_t i = 0; i < log_sd_size; ++i)
        {
            prior->log_sd[i] = static_cast<Type>(log_sd[i]);
            register_prior_parameter(info, prior->log_sd[i], log_sd_estimation_type,
                                     distribution_name + "." + fims::to_string(prior->id) + ".log_sd." + fims::to_string(i));
        }
        info->density_components[prior->id] = prior;
    }

} // namespace fims_native

#endif // FIMS_NATIVE_PRIORS_HPP
