/**
 * \file call_recruitment.cpp
 * \brief Native .Call recruitment interface for the FIMS framework.
 */

#include "../inst/include/interface/call/recruitment.hpp"

#include <cmath>
#include <string>

#include "../inst/include/interface/call/recruitment_registry.hpp"

#include <Rinternals.h>

namespace
{

    enum EstimationTypeCode
    {
        kConstant = 0,
        kFixedEffects = 1,
        kRandomEffects = 2
    };

    SEXP coerce_numeric_vector(SEXP sexp, const char *argument_name)
    {
        if (!Rf_isNumeric(sexp))
        {
            Rf_error("`%s` must be numeric.", argument_name);
        }

        return Rf_coerceVector(sexp, REALSXP);
    }

    SEXP coerce_integer_vector(SEXP sexp, const char *argument_name)
    {
        if (!Rf_isNumeric(sexp) && !Rf_isInteger(sexp))
        {
            Rf_error("`%s` must be integer or numeric.", argument_name);
        }

        return Rf_coerceVector(sexp, INTSXP);
    }

    R_xlen_t normalize_parameter_length(R_xlen_t parameter_length,
                                        R_xlen_t x_length,
                                        const char *argument_name)
    {
        if (parameter_length == 1 || parameter_length == x_length)
        {
            return parameter_length;
        }

        Rf_error("`%s` must have length 1 or match the length of the paired input.",
                 argument_name);
        return 0;
    }

    template <typename Type>
    void register_parameter_if_estimable(
        std::shared_ptr<fims_info::Information<Type>> info,
        Type &parameter,
        int estimation_type,
        const std::string &parameter_name,
        bool register_parameter)
    {
        if (!register_parameter)
        {
            return;
        }

        switch (estimation_type)
        {
        case kConstant:
            return;
        case kFixedEffects:
            info->RegisterParameterName(parameter_name);
            info->RegisterParameter(parameter);
            return;
        case kRandomEffects:
            info->RegisterRandomEffectName(parameter_name);
            info->RegisterRandomEffect(parameter);
            return;
        default:
            Rf_error(
                "Unknown estimation type code %d. Supported codes are 0 (constant), 1 (fixed_effects), and 2 (random_effects).",
                estimation_type);
        }
    }

    template <typename Type>
    uint32_t create_beverton_holt_recruitment(
        uint32_t recruitment_id,
        SEXP logit_steep_sexp,
        SEXP log_rzero_sexp,
        SEXP log_devs_sexp,
        SEXP logit_steep_estimation_type_sexp,
        SEXP log_rzero_estimation_type_sexp,
        SEXP log_devs_estimation_type_sexp,
        bool register_parameter)
    {
        SEXP logit_steep_real =
            PROTECT(coerce_numeric_vector(logit_steep_sexp, "logit_steep"));
        SEXP log_rzero_real =
            PROTECT(coerce_numeric_vector(log_rzero_sexp, "log_rzero"));
        SEXP log_devs_real =
            PROTECT(coerce_numeric_vector(log_devs_sexp, "log_devs"));
        SEXP logit_steep_estimation_type_int =
            PROTECT(coerce_integer_vector(logit_steep_estimation_type_sexp,
                                          "logit_steep_estimation_type"));
        SEXP log_rzero_estimation_type_int =
            PROTECT(coerce_integer_vector(log_rzero_estimation_type_sexp,
                                          "log_rzero_estimation_type"));
        SEXP log_devs_estimation_type_int =
            PROTECT(coerce_integer_vector(log_devs_estimation_type_sexp,
                                          "log_devs_estimation_type"));

        const R_xlen_t logit_steep_length = XLENGTH(logit_steep_real);
        const R_xlen_t log_rzero_length = XLENGTH(log_rzero_real);
        const R_xlen_t log_devs_length = XLENGTH(log_devs_real);
        const R_xlen_t parameter_length = std::max(logit_steep_length, log_rzero_length);
        const R_xlen_t normalized_logit_steep_length =
            normalize_parameter_length(logit_steep_length, parameter_length,
                                       "logit_steep");
        const R_xlen_t normalized_log_rzero_length =
            normalize_parameter_length(log_rzero_length, parameter_length,
                                       "log_rzero");

        const R_xlen_t logit_steep_estimation_type_length =
            XLENGTH(logit_steep_estimation_type_int);
        const R_xlen_t log_rzero_estimation_type_length =
            XLENGTH(log_rzero_estimation_type_int);
        const R_xlen_t log_devs_estimation_type_length =
            XLENGTH(log_devs_estimation_type_int);
        const R_xlen_t normalized_logit_steep_estimation_type_length =
            normalize_parameter_length(logit_steep_estimation_type_length,
                                       parameter_length,
                                       "logit_steep_estimation_type");
        const R_xlen_t normalized_log_rzero_estimation_type_length =
            normalize_parameter_length(log_rzero_estimation_type_length,
                                       parameter_length,
                                       "log_rzero_estimation_type");
        const R_xlen_t normalized_log_devs_estimation_type_length =
            normalize_parameter_length(log_devs_estimation_type_length,
                                       std::max<R_xlen_t>(log_devs_length, 1),
                                       "log_devs_estimation_type");

        const double *logit_steep_ptr = REAL(logit_steep_real);
        const double *log_rzero_ptr = REAL(log_rzero_real);
        const double *log_devs_ptr = REAL(log_devs_real);
        const int *logit_steep_estimation_type_ptr = INTEGER(logit_steep_estimation_type_int);
        const int *log_rzero_estimation_type_ptr = INTEGER(log_rzero_estimation_type_int);
        const int *log_devs_estimation_type_ptr = INTEGER(log_devs_estimation_type_int);

        std::shared_ptr<NativeRecruitmentRegistry<Type>> registry =
            NativeRecruitmentRegistry<Type>::GetInstance();
        std::shared_ptr<fims_info::Information<Type>> info = registry->GetInformation();
        std::shared_ptr<fims_popdy::SRBevertonHolt<Type>> recruitment =
            registry->CreateBevertonHoltRecruitment(recruitment_id);

        recruitment->logit_steep.resize(parameter_length);
        recruitment->log_rzero.resize(parameter_length);

        for (R_xlen_t index = 0; index < parameter_length; ++index)
        {
            const R_xlen_t logit_steep_index =
                (normalized_logit_steep_length == 1) ? 0 : index;
            const R_xlen_t log_rzero_index =
                (normalized_log_rzero_length == 1) ? 0 : index;
            const R_xlen_t logit_steep_estimation_type_index =
                (normalized_logit_steep_estimation_type_length == 1) ? 0 : index;
            const R_xlen_t log_rzero_estimation_type_index =
                (normalized_log_rzero_estimation_type_length == 1) ? 0 : index;

            recruitment->logit_steep[index] =
                static_cast<Type>(logit_steep_ptr[logit_steep_index]);
            recruitment->log_rzero[index] =
                static_cast<Type>(log_rzero_ptr[log_rzero_index]);

            register_parameter_if_estimable(
                info,
                recruitment->logit_steep[index],
                logit_steep_estimation_type_ptr[logit_steep_estimation_type_index],
                "Recruitment." + std::to_string(recruitment_id) +
                    ".logit_steep." + std::to_string(index),
                register_parameter);
            register_parameter_if_estimable(
                info,
                recruitment->log_rzero[index],
                log_rzero_estimation_type_ptr[log_rzero_estimation_type_index],
                "Recruitment." + std::to_string(recruitment_id) +
                    ".log_rzero." + std::to_string(index),
                register_parameter);
        }

        recruitment->log_recruit_devs.resize(log_devs_length);
        recruitment->log_recruit_devs.set_name("recruitment.log_recruit_devs");
        for (R_xlen_t index = 0; index < log_devs_length; ++index)
        {
            const R_xlen_t log_devs_estimation_type_index =
                (normalized_log_devs_estimation_type_length == 1) ? 0 : index;
            recruitment->log_recruit_devs[index] = static_cast<Type>(log_devs_ptr[index]);
            register_parameter_if_estimable(
                info,
                recruitment->log_recruit_devs[index],
                log_devs_estimation_type_ptr[log_devs_estimation_type_index],
                "Recruitment." + std::to_string(recruitment_id) +
                    ".log_devs." + std::to_string(index),
                register_parameter);
        }

        if (log_devs_length > 0)
        {
            std::shared_ptr<fims_popdy::LogDevs<Type>> process =
                registry->CreateLogDevsProcess();
            process->recruitment = recruitment;
            recruitment->process = process;
        }

        UNPROTECT(6);
        return recruitment->id;
    }

    template <typename Type>
    std::shared_ptr<fims_popdy::SRBevertonHolt<Type>> get_beverton_holt_recruitment(
        uint32_t recruitment_id)
    {
        std::shared_ptr<NativeRecruitmentRegistry<Type>> registry =
            NativeRecruitmentRegistry<Type>::GetInstance();
        std::shared_ptr<fims_popdy::SRBevertonHolt<Type>> recruitment =
            registry->GetBevertonHoltRecruitment(recruitment_id);

        if (recruitment == nullptr)
        {
            Rf_error("No Beverton-Holt recruitment with id %u was found.",
                     recruitment_id);
        }

        return recruitment;
    }

} // namespace

extern "C" SEXP fims_call_create_beverton_holt_recruitment(
    SEXP logit_steep_sexp,
    SEXP log_rzero_sexp,
    SEXP log_devs_sexp,
    SEXP logit_steep_estimation_type_sexp,
    SEXP log_rzero_estimation_type_sexp,
    SEXP log_devs_estimation_type_sexp)
{
    std::shared_ptr<NativeRecruitmentRegistry<double>> registry =
        NativeRecruitmentRegistry<double>::GetInstance();
    uint32_t recruitment_id = static_cast<uint32_t>(
        registry->GetInformation()->recruitment_models.size() + 1);

    create_beverton_holt_recruitment<double>(
        recruitment_id,
        logit_steep_sexp,
        log_rzero_sexp,
        log_devs_sexp,
        logit_steep_estimation_type_sexp,
        log_rzero_estimation_type_sexp,
        log_devs_estimation_type_sexp,
        true);

#ifdef TMB_MODEL
    create_beverton_holt_recruitment<TMBAD_FIMS_TYPE>(
        recruitment_id,
        logit_steep_sexp,
        log_rzero_sexp,
        log_devs_sexp,
        logit_steep_estimation_type_sexp,
        log_rzero_estimation_type_sexp,
        log_devs_estimation_type_sexp,
        true);
#endif

    return Rf_ScalarInteger(static_cast<int>(recruitment_id));
}

extern "C" SEXP fims_call_beverton_holt_evaluate_mean(
    SEXP spawners_sexp,
    SEXP phi_0_sexp,
    SEXP recruitment_id_sexp)
{
    SEXP spawners_real = PROTECT(coerce_numeric_vector(spawners_sexp, "spawners"));
    SEXP phi_0_real = PROTECT(coerce_numeric_vector(phi_0_sexp, "phi_0"));

    const uint32_t recruitment_id =
        static_cast<uint32_t>(Rf_asInteger(recruitment_id_sexp));
    const R_xlen_t spawners_length = XLENGTH(spawners_real);
    const R_xlen_t phi_0_length = XLENGTH(phi_0_real);
    const R_xlen_t evaluation_length = std::max(spawners_length, phi_0_length);
    const R_xlen_t normalized_spawners_length =
        normalize_parameter_length(spawners_length, evaluation_length, "spawners");
    const R_xlen_t normalized_phi_0_length =
        normalize_parameter_length(phi_0_length, evaluation_length, "phi_0");

    SEXP result = PROTECT(Rf_allocVector(REALSXP, evaluation_length));
    double *result_ptr = REAL(result);
    const double *spawners_ptr = REAL(spawners_real);
    const double *phi_0_ptr = REAL(phi_0_real);

    std::shared_ptr<fims_popdy::SRBevertonHolt<double>> recruitment =
        get_beverton_holt_recruitment<double>(recruitment_id);

    for (R_xlen_t index = 0; index < evaluation_length; ++index)
    {
        const R_xlen_t spawners_index =
            (normalized_spawners_length == 1) ? 0 : index;
        const R_xlen_t phi_0_index = (normalized_phi_0_length == 1) ? 0 : index;

        result_ptr[index] = recruitment->evaluate_mean(
            spawners_ptr[spawners_index], phi_0_ptr[phi_0_index]);
    }

    UNPROTECT(3);
    return result;
}
