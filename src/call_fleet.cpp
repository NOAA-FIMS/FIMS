/**
 * \file call_fleet.cpp
 * \brief Native .Call fleet interface for the FIMS framework.
 */

#include "../inst/include/interface/call/fleet.hpp"

#include <algorithm>
#include <string>

#include "../inst/include/interface/call/fleet_registry.hpp"

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
            Rf_error("`%s` must be an integer or numeric vector.", argument_name);
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
    uint32_t create_fleet(uint32_t fleet_id,
                          SEXP log_fmort_sexp,
                          SEXP log_q_sexp,
                          SEXP selectivity_id_sexp,
                          SEXP age_to_length_conversion_sexp,
                          SEXP log_fmort_estimation_type_sexp,
                          SEXP log_q_estimation_type_sexp,
                          bool register_parameter)
    {
        SEXP log_fmort_real = PROTECT(coerce_numeric_vector(log_fmort_sexp, "log_fmort"));
        SEXP log_q_real = PROTECT(coerce_numeric_vector(log_q_sexp, "log_q"));
        SEXP age_to_length_conversion_real = PROTECT(
            coerce_numeric_vector(age_to_length_conversion_sexp, "age_to_length_conversion"));
        SEXP log_fmort_estimation_type_int =
            PROTECT(coerce_integer_vector(log_fmort_estimation_type_sexp,
                                          "log_fmort_estimation_type"));
        SEXP log_q_estimation_type_int =
            PROTECT(coerce_integer_vector(log_q_estimation_type_sexp,
                                          "log_q_estimation_type"));

        const R_xlen_t log_fmort_length = XLENGTH(log_fmort_real);
        const R_xlen_t log_q_length = XLENGTH(log_q_real);
        const R_xlen_t age_to_length_conversion_length =
            XLENGTH(age_to_length_conversion_real);
        const R_xlen_t n_years = std::max(log_fmort_length, log_q_length);
        const R_xlen_t normalized_log_fmort_length =
            normalize_parameter_length(log_fmort_length, n_years, "log_fmort");
        const R_xlen_t normalized_log_q_length =
            normalize_parameter_length(log_q_length, n_years, "log_q");

        const R_xlen_t log_fmort_estimation_type_length = XLENGTH(log_fmort_estimation_type_int);
        const R_xlen_t log_q_estimation_type_length = XLENGTH(log_q_estimation_type_int);
        const R_xlen_t normalized_log_fmort_estimation_type_length =
            normalize_parameter_length(log_fmort_estimation_type_length,
                                       n_years,
                                       "log_fmort_estimation_type");
        const R_xlen_t normalized_log_q_estimation_type_length =
            normalize_parameter_length(log_q_estimation_type_length,
                                       n_years,
                                       "log_q_estimation_type");

        const double *log_fmort_ptr = REAL(log_fmort_real);
        const double *log_q_ptr = REAL(log_q_real);
        const double *age_to_length_conversion_ptr = REAL(age_to_length_conversion_real);
        const int *log_fmort_estimation_type_ptr = INTEGER(log_fmort_estimation_type_int);
        const int *log_q_estimation_type_ptr = INTEGER(log_q_estimation_type_int);

        std::shared_ptr<NativeFleetRegistry<Type>> registry =
            NativeFleetRegistry<Type>::GetInstance();
        std::shared_ptr<fims_info::Information<Type>> info = registry->GetInformation();
        std::shared_ptr<fims_popdy::Fleet<Type>> fleet = registry->CreateFleet(fleet_id);

        // Dimensions are set later when fleets are linked into a population.
        // Initialize explicitly to avoid undefined values in model loops.
        fleet->n_ages = 0;
        fleet->n_lengths = 0;
        fleet->n_years = static_cast<size_t>(n_years);
        fleet->log_Fmort.resize(n_years);
        fleet->log_Fmort.set_name("fleet.log_Fmort");
        fleet->Fmort.resize(n_years);
        fleet->Fmort.set_name("fleet.Fmort");
        fleet->log_q.resize(log_q_length);
        fleet->log_q.set_name("fleet.log_q");
        fleet->q.resize(n_years);
        fleet->q.set_name("fleet.q");
        fleet->age_to_length_conversion.resize(age_to_length_conversion_length);
        fleet->age_to_length_conversion.set_name("fleet.age_to_length_conversion");

        for (R_xlen_t index = 0; index < age_to_length_conversion_length; ++index)
        {
            fleet->age_to_length_conversion[index] =
                static_cast<Type>(age_to_length_conversion_ptr[index]);
        }

        for (R_xlen_t index = 0; index < n_years; ++index)
        {
            const R_xlen_t log_fmort_index =
                (normalized_log_fmort_length == 1) ? 0 : index;
            const R_xlen_t log_q_index = (normalized_log_q_length == 1) ? 0 : index;
            const R_xlen_t log_fmort_estimation_type_index =
                (normalized_log_fmort_estimation_type_length == 1) ? 0 : index;
            const R_xlen_t log_q_estimation_type_index =
                (normalized_log_q_estimation_type_length == 1) ? 0 : index;

            fleet->log_Fmort[index] = static_cast<Type>(log_fmort_ptr[log_fmort_index]);
            // fleet->log_q[index] = static_cast<Type>(log_q_ptr[log_q_index]);

            register_parameter_if_estimable(
                info,
                fleet->log_Fmort[index],
                log_fmort_estimation_type_ptr[log_fmort_estimation_type_index],
                "Fleet." + std::to_string(fleet_id) + ".log_Fmort." + std::to_string(index),
                register_parameter);
            // register_parameter_if_estimable(
            //     info,
            //     fleet->log_q[index],
            //     log_q_estimation_type_ptr[log_q_estimation_type_index],
            //     "Fleet." + std::to_string(fleet_id) + ".log_q." + std::to_string(index),
            //     register_parameter);
        }
        for (R_xlen_t index = 0; index < log_q_length; ++index)
        {
            const R_xlen_t estimation_index =
                (log_q_estimation_type_length == 1) ? 0 : index;

            fleet->log_q[index] =
                static_cast<Type>(log_q_ptr[index]);

            register_parameter_if_estimable(
                info,
                fleet->log_q[index],
                log_q_estimation_type_ptr[estimation_index],
                "Fleet." + std::to_string(fleet_id) +
                    ".log_q." + std::to_string(index),
                register_parameter);
        }

        const int selectivity_id = Rf_asInteger(selectivity_id_sexp);
        if (selectivity_id != NA_INTEGER)
        {
            auto selectivity_it =
                info->selectivity_models.find(static_cast<uint32_t>(selectivity_id));
            if (selectivity_it == info->selectivity_models.end())
            {
                Rf_error("No selectivity with id %d was found for this fleet.",
                         selectivity_id);
            }
            fleet->fleet_selectivity_id_m = selectivity_id;
            fleet->selectivity = selectivity_it->second;
        }

        UNPROTECT(5);
        return fleet->id;
    }

    template <typename Type>
    std::shared_ptr<fims_popdy::Fleet<Type>> get_fleet(uint32_t fleet_id)
    {
        std::shared_ptr<NativeFleetRegistry<Type>> registry =
            NativeFleetRegistry<Type>::GetInstance();
        std::shared_ptr<fims_popdy::Fleet<Type>> fleet = registry->GetFleet(fleet_id);

        if (fleet == nullptr)
        {
            Rf_error("No fleet with id %u was found.", fleet_id);
        }

        return fleet;
    }

} // namespace

extern "C" SEXP fims_call_create_fleet(SEXP log_fmort_sexp,
                                       SEXP log_q_sexp,
                                       SEXP selectivity_id_sexp,
                                       SEXP age_to_length_conversion_sexp,
                                       SEXP log_fmort_estimation_type_sexp,
                                       SEXP log_q_estimation_type_sexp)
{
    std::shared_ptr<NativeFleetRegistry<double>> registry =
        NativeFleetRegistry<double>::GetInstance();
    uint32_t fleet_id =
        static_cast<uint32_t>(registry->GetInformation()->fleets.size() + 1);

    create_fleet<double>(fleet_id,
                         log_fmort_sexp,
                         log_q_sexp,
                         selectivity_id_sexp,
                         age_to_length_conversion_sexp,
                         log_fmort_estimation_type_sexp,
                         log_q_estimation_type_sexp,
                         true);

#ifdef TMB_MODEL
    create_fleet<TMBAD_FIMS_TYPE>(fleet_id,
                                  log_fmort_sexp,
                                  log_q_sexp,
                                  selectivity_id_sexp,
                                  age_to_length_conversion_sexp,
                                  log_fmort_estimation_type_sexp,
                                  log_q_estimation_type_sexp,
                                  true);
#endif
#ifdef QUADRA_MODEL
    create_fleet<QUADRA_FIMS_TYPE>(fleet_id,
                                   log_fmort_sexp,
                                   log_q_sexp,
                                   selectivity_id_sexp,
                                   age_to_length_conversion_sexp,
                                   log_fmort_estimation_type_sexp,
                                   log_q_estimation_type_sexp,
                                   true);
#endif

    return Rf_ScalarInteger(static_cast<int>(fleet_id));
}

extern "C" SEXP fims_call_fleet_prepare(SEXP fleet_id_sexp)
{
    const uint32_t fleet_id = static_cast<uint32_t>(Rf_asInteger(fleet_id_sexp));
    std::shared_ptr<fims_popdy::Fleet<double>> fleet = get_fleet<double>(fleet_id);

    fleet->Prepare();

    SEXP out = PROTECT(Rf_allocVector(VECSXP, 2));
    SEXP out_names = PROTECT(Rf_allocVector(STRSXP, 2));
    SET_STRING_ELT(out_names, 0, Rf_mkChar("Fmort"));
    SET_STRING_ELT(out_names, 1, Rf_mkChar("q"));

    const R_xlen_t n_years = static_cast<R_xlen_t>(fleet->n_years);
    SEXP fmort_out = PROTECT(Rf_allocVector(REALSXP, n_years));
    SEXP q_out = PROTECT(Rf_allocVector(REALSXP, n_years));

    for (R_xlen_t index = 0; index < n_years; ++index)
    {
        REAL(fmort_out)
        [index] = fleet->Fmort[index];
        REAL(q_out)
        [index] = fleet->q[index];
    }

    SET_VECTOR_ELT(out, 0, fmort_out);
    SET_VECTOR_ELT(out, 1, q_out);
    Rf_setAttrib(out, R_NamesSymbol, out_names);

    UNPROTECT(4);
    return out;
}
