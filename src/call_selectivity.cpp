/**
 * \file call_selectivity.cpp
 * \brief Native .Call selectivity interface for the FIMS framework.
 */

#include "../inst/include/interface/call/selectivity.hpp"

#include <cmath>
#include <string>

#include "../inst/include/interface/call/selectivity_registry.hpp"

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

        Rf_error("`%s` must have length 1 or match the length of `x`.", argument_name);
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
    uint32_t create_logistic_selectivity(
        uint32_t selectivity_id,
        SEXP inflection_point_sexp,
        SEXP slope_sexp,
        SEXP inflection_point_estimation_type_sexp,
        SEXP slope_estimation_type_sexp,
        bool register_parameter)
    {
        SEXP inflection_real =
            PROTECT(coerce_numeric_vector(inflection_point_sexp, "inflection_point"));
        SEXP slope_real = PROTECT(coerce_numeric_vector(slope_sexp, "slope"));
        SEXP inflection_estimation_type_int =
            PROTECT(coerce_integer_vector(inflection_point_estimation_type_sexp,
                                          "inflection_point_estimation_type"));
        SEXP slope_estimation_type_int =
            PROTECT(coerce_integer_vector(slope_estimation_type_sexp,
                                          "slope_estimation_type"));

        const R_xlen_t inflection_length = XLENGTH(inflection_real);
        const R_xlen_t slope_length = XLENGTH(slope_real);

        const R_xlen_t parameter_length = std::max(inflection_length, slope_length);
        const R_xlen_t normalized_inflection_length =
            normalize_parameter_length(inflection_length, parameter_length,
                                       "inflection_point");
        const R_xlen_t normalized_slope_length =
            normalize_parameter_length(slope_length, parameter_length, "slope");

        const R_xlen_t inflection_estimation_type_length = XLENGTH(inflection_estimation_type_int);
        const R_xlen_t slope_estimation_type_length = XLENGTH(slope_estimation_type_int);
        const R_xlen_t normalized_inflection_estimation_type_length =
            normalize_parameter_length(inflection_estimation_type_length,
                                       parameter_length,
                                       "inflection_point_estimation_type");
        const R_xlen_t normalized_slope_estimation_type_length =
            normalize_parameter_length(slope_estimation_type_length,
                                       parameter_length,
                                       "slope_estimation_type");

        const double *inflection_ptr = REAL(inflection_real);
        const double *slope_ptr = REAL(slope_real);
        const int *inflection_estimation_type_ptr = INTEGER(inflection_estimation_type_int);
        const int *slope_estimation_type_ptr = INTEGER(slope_estimation_type_int);

        std::shared_ptr<NativeSelectivityRegistry<Type>> registry =
            NativeSelectivityRegistry<Type>::GetInstance();
        std::shared_ptr<fims_info::Information<Type>> info = registry->GetInformation();
        std::shared_ptr<fims_popdy::LogisticSelectivity<Type>> selectivity =
            registry->CreateLogisticSelectivity(selectivity_id);

        selectivity->inflection_point.resize(parameter_length);
        selectivity->slope.resize(parameter_length);

        for (R_xlen_t index = 0; index < parameter_length; ++index)
        {
            const R_xlen_t inflection_index =
                (normalized_inflection_length == 1) ? 0 : index;
            const R_xlen_t slope_index =
                (normalized_slope_length == 1) ? 0 : index;
            const R_xlen_t inflection_estimation_type_index =
                (normalized_inflection_estimation_type_length == 1) ? 0 : index;
            const R_xlen_t slope_estimation_type_index =
                (normalized_slope_estimation_type_length == 1) ? 0 : index;

            selectivity->inflection_point[index] =
                static_cast<Type>(inflection_ptr[inflection_index]);
            if (slope_ptr[slope_index] <= 0.0)
            {
                Rf_error("`slope` must be positive when using log-slope parameterization.");
            }
            selectivity->slope[index] =
                static_cast<Type>(std::log(slope_ptr[slope_index]));

            register_parameter_if_estimable(
                info,
                selectivity->inflection_point[index],
                inflection_estimation_type_ptr[inflection_estimation_type_index],
                "Selectivity." + std::to_string(selectivity_id) +
                    ".inflection_point." + std::to_string(index),
                register_parameter);
            register_parameter_if_estimable(
                info,
                selectivity->slope[index],
                slope_estimation_type_ptr[slope_estimation_type_index],
                "Selectivity." + std::to_string(selectivity_id) +
                    ".log_slope." + std::to_string(index),
                register_parameter);
        }

        UNPROTECT(4);
        return selectivity->id;
    }

    template <typename Type>
    std::shared_ptr<fims_popdy::LogisticSelectivity<Type>> get_logistic_selectivity(
        uint32_t selectivity_id)
    {
        std::shared_ptr<NativeSelectivityRegistry<Type>> registry =
            NativeSelectivityRegistry<Type>::GetInstance();
        std::shared_ptr<fims_popdy::LogisticSelectivity<Type>> selectivity =
            registry->GetLogisticSelectivity(selectivity_id);

        if (selectivity == nullptr)
        {
            Rf_error("No logistic selectivity with id %u was found.", selectivity_id);
        }

        return selectivity;
    }

    template <typename Type>
    uint32_t create_double_logistic_selectivity(
        uint32_t selectivity_id,
        SEXP inflection_point_asc_sexp,
        SEXP slope_asc_sexp,
        SEXP inflection_point_desc_sexp,
        SEXP slope_desc_sexp,
        SEXP inflection_point_asc_estimation_type_sexp,
        SEXP slope_asc_estimation_type_sexp,
        SEXP inflection_point_desc_estimation_type_sexp,
        SEXP slope_desc_estimation_type_sexp,
        bool register_parameter)
    {
        SEXP inflection_point_asc_real =
            PROTECT(coerce_numeric_vector(inflection_point_asc_sexp, "inflection_point_asc"));
        SEXP slope_asc_real =
            PROTECT(coerce_numeric_vector(slope_asc_sexp, "slope_asc"));
        SEXP inflection_point_desc_real =
            PROTECT(coerce_numeric_vector(inflection_point_desc_sexp, "inflection_point_desc"));
        SEXP slope_desc_real =
            PROTECT(coerce_numeric_vector(slope_desc_sexp, "slope_desc"));
        SEXP inflection_point_asc_estimation_type_int =
            PROTECT(coerce_integer_vector(inflection_point_asc_estimation_type_sexp,
                                          "inflection_point_asc_estimation_type"));
        SEXP slope_asc_estimation_type_int =
            PROTECT(coerce_integer_vector(slope_asc_estimation_type_sexp,
                                          "slope_asc_estimation_type"));
        SEXP inflection_point_desc_estimation_type_int =
            PROTECT(coerce_integer_vector(inflection_point_desc_estimation_type_sexp,
                                          "inflection_point_desc_estimation_type"));
        SEXP slope_desc_estimation_type_int =
            PROTECT(coerce_integer_vector(slope_desc_estimation_type_sexp,
                                          "slope_desc_estimation_type"));

        const R_xlen_t inflection_point_asc_length = XLENGTH(inflection_point_asc_real);
        const R_xlen_t slope_asc_length = XLENGTH(slope_asc_real);
        const R_xlen_t inflection_point_desc_length = XLENGTH(inflection_point_desc_real);
        const R_xlen_t slope_desc_length = XLENGTH(slope_desc_real);

        const R_xlen_t parameter_length = std::max(
            std::max(inflection_point_asc_length, slope_asc_length),
            std::max(inflection_point_desc_length, slope_desc_length));

        const R_xlen_t normalized_inflection_point_asc_length =
            normalize_parameter_length(inflection_point_asc_length, parameter_length,
                                       "inflection_point_asc");
        const R_xlen_t normalized_slope_asc_length =
            normalize_parameter_length(slope_asc_length, parameter_length,
                                       "slope_asc");
        const R_xlen_t normalized_inflection_point_desc_length =
            normalize_parameter_length(inflection_point_desc_length, parameter_length,
                                       "inflection_point_desc");
        const R_xlen_t normalized_slope_desc_length =
            normalize_parameter_length(slope_desc_length, parameter_length,
                                       "slope_desc");

        const R_xlen_t inflection_point_asc_estimation_type_length = XLENGTH(inflection_point_asc_estimation_type_int);
        const R_xlen_t slope_asc_estimation_type_length = XLENGTH(slope_asc_estimation_type_int);
        const R_xlen_t inflection_point_desc_estimation_type_length = XLENGTH(inflection_point_desc_estimation_type_int);
        const R_xlen_t slope_desc_estimation_type_length = XLENGTH(slope_desc_estimation_type_int);

        const R_xlen_t normalized_inflection_point_asc_estimation_type_length =
            normalize_parameter_length(inflection_point_asc_estimation_type_length,
                                       parameter_length,
                                       "inflection_point_asc_estimation_type");
        const R_xlen_t normalized_slope_asc_estimation_type_length =
            normalize_parameter_length(slope_asc_estimation_type_length,
                                       parameter_length,
                                       "slope_asc_estimation_type");
        const R_xlen_t normalized_inflection_point_desc_estimation_type_length =
            normalize_parameter_length(inflection_point_desc_estimation_type_length,
                                       parameter_length,
                                       "inflection_point_desc_estimation_type");
        const R_xlen_t normalized_slope_desc_estimation_type_length =
            normalize_parameter_length(slope_desc_estimation_type_length,
                                       parameter_length,
                                       "slope_desc_estimation_type");

        const double *inflection_point_asc_ptr = REAL(inflection_point_asc_real);
        const double *slope_asc_ptr = REAL(slope_asc_real);
        const double *inflection_point_desc_ptr = REAL(inflection_point_desc_real);
        const double *slope_desc_ptr = REAL(slope_desc_real);
        const int *inflection_point_asc_estimation_type_ptr = INTEGER(inflection_point_asc_estimation_type_int);
        const int *slope_asc_estimation_type_ptr = INTEGER(slope_asc_estimation_type_int);
        const int *inflection_point_desc_estimation_type_ptr = INTEGER(inflection_point_desc_estimation_type_int);
        const int *slope_desc_estimation_type_ptr = INTEGER(slope_desc_estimation_type_int);

        std::shared_ptr<NativeSelectivityRegistry<Type>> registry =
            NativeSelectivityRegistry<Type>::GetInstance();
        std::shared_ptr<fims_info::Information<Type>> info = registry->GetInformation();
        std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<Type>> selectivity =
            registry->CreateDoubleLogisticSelectivity(selectivity_id);

        selectivity->inflection_point_asc.resize(parameter_length);
        selectivity->slope_asc.resize(parameter_length);
        selectivity->inflection_point_desc.resize(parameter_length);
        selectivity->slope_desc.resize(parameter_length);

        for (R_xlen_t index = 0; index < parameter_length; ++index)
        {
            const R_xlen_t inflection_point_asc_index =
                (normalized_inflection_point_asc_length == 1) ? 0 : index;
            const R_xlen_t slope_asc_index =
                (normalized_slope_asc_length == 1) ? 0 : index;
            const R_xlen_t inflection_point_desc_index =
                (normalized_inflection_point_desc_length == 1) ? 0 : index;
            const R_xlen_t slope_desc_index =
                (normalized_slope_desc_length == 1) ? 0 : index;

            const R_xlen_t inflection_point_asc_estimation_type_index =
                (normalized_inflection_point_asc_estimation_type_length == 1)
                    ? 0
                    : index;
            const R_xlen_t slope_asc_estimation_type_index =
                (normalized_slope_asc_estimation_type_length == 1) ? 0 : index;
            const R_xlen_t inflection_point_desc_estimation_type_index =
                (normalized_inflection_point_desc_estimation_type_length == 1)
                    ? 0
                    : index;
            const R_xlen_t slope_desc_estimation_type_index =
                (normalized_slope_desc_estimation_type_length == 1) ? 0 : index;

            selectivity->inflection_point_asc[index] =
                static_cast<Type>(inflection_point_asc_ptr[inflection_point_asc_index]);
            if (slope_asc_ptr[slope_asc_index] <= 0.0)
            {
                Rf_error("`slope_asc` must be positive when using log-slope parameterization.");
            }
            selectivity->slope_asc[index] =
                static_cast<Type>(std::log(slope_asc_ptr[slope_asc_index]));
            selectivity->inflection_point_desc[index] =
                static_cast<Type>(inflection_point_desc_ptr[inflection_point_desc_index]);
            if (slope_desc_ptr[slope_desc_index] <= 0.0)
            {
                Rf_error("`slope_desc` must be positive when using log-slope parameterization.");
            }
            selectivity->slope_desc[index] =
                static_cast<Type>(std::log(slope_desc_ptr[slope_desc_index]));

            register_parameter_if_estimable(
                info,
                selectivity->inflection_point_asc[index],
                inflection_point_asc_estimation_type_ptr[inflection_point_asc_estimation_type_index],
                "Selectivity." + std::to_string(selectivity_id) +
                    ".inflection_point_asc." + std::to_string(index),
                register_parameter);
            register_parameter_if_estimable(
                info,
                selectivity->slope_asc[index],
                slope_asc_estimation_type_ptr[slope_asc_estimation_type_index],
                "Selectivity." + std::to_string(selectivity_id) +
                    ".log_slope_asc." + std::to_string(index),
                register_parameter);
            register_parameter_if_estimable(
                info,
                selectivity->inflection_point_desc[index],
                inflection_point_desc_estimation_type_ptr[inflection_point_desc_estimation_type_index],
                "Selectivity." + std::to_string(selectivity_id) +
                    ".inflection_point_desc." + std::to_string(index),
                register_parameter);
            register_parameter_if_estimable(
                info,
                selectivity->slope_desc[index],
                slope_desc_estimation_type_ptr[slope_desc_estimation_type_index],
                "Selectivity." + std::to_string(selectivity_id) +
                    ".log_slope_desc." + std::to_string(index),
                register_parameter);
        }

        UNPROTECT(8);
        return selectivity->id;
    }

    template <typename Type>
    std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<Type>>
    get_double_logistic_selectivity(uint32_t selectivity_id)
    {
        std::shared_ptr<NativeSelectivityRegistry<Type>> registry =
            NativeSelectivityRegistry<Type>::GetInstance();
        std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<Type>> selectivity =
            registry->GetDoubleLogisticSelectivity(selectivity_id);

        if (selectivity == nullptr)
        {
            Rf_error("No double logistic selectivity with id %u was found.",
                     selectivity_id);
        }

        return selectivity;
    }

} // namespace

extern "C" SEXP fims_call_create_logistic_selectivity(
    SEXP inflection_point_sexp,
    SEXP slope_sexp,
    SEXP inflection_point_estimation_type_sexp,
    SEXP slope_estimation_type_sexp)
{
    std::shared_ptr<NativeSelectivityRegistry<double>> registry =
        NativeSelectivityRegistry<double>::GetInstance();
    uint32_t selectivity_id = static_cast<uint32_t>(
        registry->GetInformation()->selectivity_models.size() + 1);

    create_logistic_selectivity<double>(selectivity_id,
                                        inflection_point_sexp,
                                        slope_sexp,
                                        inflection_point_estimation_type_sexp,
                                        slope_estimation_type_sexp,
                                        true);

#ifdef TMB_MODEL
    create_logistic_selectivity<TMBAD_FIMS_TYPE>(selectivity_id,
                                                 inflection_point_sexp,
                                                 slope_sexp,
                                                 inflection_point_estimation_type_sexp,
                                                 slope_estimation_type_sexp,
                                                 true);
#endif
#ifdef QUADRA_MODEL
    create_logistic_selectivity<QUADRA_FIMS_TYPE>(
        selectivity_id, inflection_point_sexp, slope_sexp,
        inflection_point_estimation_type_sexp, slope_estimation_type_sexp,
        true);
#endif

    return Rf_ScalarInteger(static_cast<int>(selectivity_id));
}

extern "C" SEXP fims_call_logistic_selectivity(SEXP x_sexp,
                                               SEXP selectivity_id_sexp)
{
    SEXP x_real = PROTECT(coerce_numeric_vector(x_sexp, "x"));
    const uint32_t selectivity_id =
        static_cast<uint32_t>(Rf_asInteger(selectivity_id_sexp));

    const R_xlen_t x_length = XLENGTH(x_real);
    SEXP result = PROTECT(Rf_allocVector(REALSXP, x_length));

    if (x_length == 0)
    {
        UNPROTECT(2);
        return result;
    }

    double *result_ptr = REAL(result);
    const double *x_ptr = REAL(x_real);
    std::shared_ptr<fims_popdy::LogisticSelectivity<double>> selectivity =
        get_logistic_selectivity<double>(selectivity_id);

    for (R_xlen_t index = 0; index < x_length; ++index)
    {
        result_ptr[index] = selectivity->evaluate(x_ptr[index], index);
    }

    UNPROTECT(2);
    return result;
}

extern "C" SEXP fims_call_create_double_logistic_selectivity(
    SEXP inflection_point_asc_sexp,
    SEXP slope_asc_sexp,
    SEXP inflection_point_desc_sexp,
    SEXP slope_desc_sexp,
    SEXP inflection_point_asc_estimation_type_sexp,
    SEXP slope_asc_estimation_type_sexp,
    SEXP inflection_point_desc_estimation_type_sexp,
    SEXP slope_desc_estimation_type_sexp)
{
    std::shared_ptr<NativeSelectivityRegistry<double>> registry =
        NativeSelectivityRegistry<double>::GetInstance();
    uint32_t selectivity_id = static_cast<uint32_t>(
        registry->GetInformation()->selectivity_models.size() + 1);

    create_double_logistic_selectivity<double>(
        selectivity_id,
        inflection_point_asc_sexp,
        slope_asc_sexp,
        inflection_point_desc_sexp,
        slope_desc_sexp,
        inflection_point_asc_estimation_type_sexp,
        slope_asc_estimation_type_sexp,
        inflection_point_desc_estimation_type_sexp,
        slope_desc_estimation_type_sexp,
        true);

#ifdef TMB_MODEL
    create_double_logistic_selectivity<TMBAD_FIMS_TYPE>(
        selectivity_id,
        inflection_point_asc_sexp,
        slope_asc_sexp,
        inflection_point_desc_sexp,
        slope_desc_sexp,
        inflection_point_asc_estimation_type_sexp,
        slope_asc_estimation_type_sexp,
        inflection_point_desc_estimation_type_sexp,
        slope_desc_estimation_type_sexp,
        true);
#endif
#ifdef QUADRA_MODEL
    create_double_logistic_selectivity<QUADRA_FIMS_TYPE>(
        selectivity_id, inflection_point_asc_sexp, slope_asc_sexp,
        inflection_point_desc_sexp, slope_desc_sexp,
        inflection_point_asc_estimation_type_sexp,
        slope_asc_estimation_type_sexp,
        inflection_point_desc_estimation_type_sexp,
        slope_desc_estimation_type_sexp, true);
#endif

    return Rf_ScalarInteger(static_cast<int>(selectivity_id));
}

extern "C" SEXP fims_call_double_logistic_selectivity(SEXP x_sexp,
                                                      SEXP selectivity_id_sexp)
{
    SEXP x_real = PROTECT(coerce_numeric_vector(x_sexp, "x"));
    const uint32_t selectivity_id =
        static_cast<uint32_t>(Rf_asInteger(selectivity_id_sexp));

    const R_xlen_t x_length = XLENGTH(x_real);
    SEXP result = PROTECT(Rf_allocVector(REALSXP, x_length));

    if (x_length == 0)
    {
        UNPROTECT(2);
        return result;
    }

    double *result_ptr = REAL(result);
    const double *x_ptr = REAL(x_real);
    std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<double>> selectivity =
        get_double_logistic_selectivity<double>(selectivity_id);

    for (R_xlen_t index = 0; index < x_length; ++index)
    {
        result_ptr[index] = selectivity->evaluate(x_ptr[index], index);
    }

    UNPROTECT(2);
    return result;
}

extern "C" SEXP fims_call_information_parameter_counts()
{
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    SEXP out = PROTECT(Rf_allocVector(INTSXP, 4));
    SEXP out_names = PROTECT(Rf_allocVector(STRSXP, 4));

    SET_STRING_ELT(out_names, 0, Rf_mkChar("fixed_effects_parameters"));
    SET_STRING_ELT(out_names, 1, Rf_mkChar("random_effects_parameters"));
    SET_STRING_ELT(out_names, 2, Rf_mkChar("parameter_names"));
    SET_STRING_ELT(out_names, 3, Rf_mkChar("random_effects_names"));

    INTEGER(out)
    [0] = static_cast<int>(info->fixed_effects_parameters.size());
    INTEGER(out)
    [1] = static_cast<int>(info->random_effects_parameters.size());
    INTEGER(out)
    [2] = static_cast<int>(info->parameter_names.size());
    INTEGER(out)
    [3] = static_cast<int>(info->random_effects_names.size());

    Rf_setAttrib(out, R_NamesSymbol, out_names);

    UNPROTECT(2);
    return out;
}
