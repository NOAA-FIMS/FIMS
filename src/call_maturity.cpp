/**
 * \file call_maturity.cpp
 * \brief Native .Call maturity interface for the FIMS framework.
 */

#include "../inst/include/interface/call/maturity.hpp"

#include <cmath>
#include <string>

#include "../inst/include/interface/call/maturity_registry.hpp"

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
                                        R_xlen_t paired_length,
                                        const char *argument_name)
    {
        if (parameter_length == 1 || parameter_length == paired_length)
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
    uint32_t create_logistic_maturity(
        uint32_t maturity_id,
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

        std::shared_ptr<NativeMaturityRegistry<Type>> registry =
            NativeMaturityRegistry<Type>::GetInstance();
        std::shared_ptr<fims_info::Information<Type>> info = registry->GetInformation();
        std::shared_ptr<fims_popdy::LogisticMaturity<Type>> maturity =
            registry->CreateLogisticMaturity(maturity_id);

        maturity->inflection_point.resize(parameter_length);
        maturity->slope.resize(parameter_length);

        for (R_xlen_t index = 0; index < parameter_length; ++index)
        {
            const R_xlen_t inflection_index =
                (normalized_inflection_length == 1) ? 0 : index;
            const R_xlen_t slope_index = (normalized_slope_length == 1) ? 0 : index;
            const R_xlen_t inflection_estimation_type_index =
                (normalized_inflection_estimation_type_length == 1) ? 0 : index;
            const R_xlen_t slope_estimation_type_index =
                (normalized_slope_estimation_type_length == 1) ? 0 : index;

            maturity->inflection_point[index] =
                static_cast<Type>(inflection_ptr[inflection_index]);
            maturity->slope[index] = static_cast<Type>(slope_ptr[slope_index]);

            register_parameter_if_estimable(
                info,
                maturity->inflection_point[index],
                inflection_estimation_type_ptr[inflection_estimation_type_index],
                "Maturity." + std::to_string(maturity_id) +
                    ".inflection_point." + std::to_string(index),
                register_parameter);
            register_parameter_if_estimable(
                info,
                maturity->slope[index],
                slope_estimation_type_ptr[slope_estimation_type_index],
                "Maturity." + std::to_string(maturity_id) +
                    ".slope." + std::to_string(index),
                register_parameter);
        }

        UNPROTECT(4);
        return maturity->id;
    }

    template <typename Type>
    std::shared_ptr<fims_popdy::LogisticMaturity<Type>> get_logistic_maturity(
        uint32_t maturity_id)
    {
        std::shared_ptr<NativeMaturityRegistry<Type>> registry =
            NativeMaturityRegistry<Type>::GetInstance();
        std::shared_ptr<fims_popdy::LogisticMaturity<Type>> maturity =
            registry->GetLogisticMaturity(maturity_id);

        if (maturity == nullptr)
        {
            Rf_error("No logistic maturity with id %u was found.", maturity_id);
        }

        return maturity;
    }

} // namespace

extern "C" SEXP fims_call_create_logistic_maturity(
    SEXP inflection_point_sexp,
    SEXP slope_sexp,
    SEXP inflection_point_estimation_type_sexp,
    SEXP slope_estimation_type_sexp)
{
    std::shared_ptr<NativeMaturityRegistry<double>> registry =
        NativeMaturityRegistry<double>::GetInstance();
    uint32_t maturity_id = static_cast<uint32_t>(
        registry->GetInformation()->maturity_models.size() + 1);

    create_logistic_maturity<double>(maturity_id,
                                     inflection_point_sexp,
                                     slope_sexp,
                                     inflection_point_estimation_type_sexp,
                                     slope_estimation_type_sexp,
                                     true);

#ifdef TMB_MODEL
    create_logistic_maturity<TMBAD_FIMS_TYPE>(maturity_id,
                                              inflection_point_sexp,
                                              slope_sexp,
                                              inflection_point_estimation_type_sexp,
                                              slope_estimation_type_sexp,
                                              true);
#endif
#ifdef QUADRA_MODEL
    create_logistic_maturity<QUADRA_FIMS_TYPE>(
        maturity_id, inflection_point_sexp, slope_sexp,
        inflection_point_estimation_type_sexp, slope_estimation_type_sexp,
        true);
#endif

    return Rf_ScalarInteger(static_cast<int>(maturity_id));
}

extern "C" SEXP fims_call_logistic_maturity(SEXP x_sexp,
                                            SEXP maturity_id_sexp)
{
    SEXP x_real = PROTECT(coerce_numeric_vector(x_sexp, "x"));
    const uint32_t maturity_id = static_cast<uint32_t>(Rf_asInteger(maturity_id_sexp));

    const R_xlen_t x_length = XLENGTH(x_real);
    SEXP result = PROTECT(Rf_allocVector(REALSXP, x_length));

    if (x_length == 0)
    {
        UNPROTECT(2);
        return result;
    }

    double *result_ptr = REAL(result);
    const double *x_ptr = REAL(x_real);
    std::shared_ptr<fims_popdy::LogisticMaturity<double>> maturity =
        get_logistic_maturity<double>(maturity_id);

    for (R_xlen_t index = 0; index < x_length; ++index)
    {
        result_ptr[index] = maturity->evaluate(x_ptr[index], index);
    }

    UNPROTECT(2);
    return result;
}
