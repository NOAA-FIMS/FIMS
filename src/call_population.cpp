/**
 * \file call_population.cpp
 * \brief Native .Call population interface for the FIMS framework.
 */

#include "../inst/include/interface/call/population.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include "../inst/include/interface/call/population_registry.hpp"

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

    R_xlen_t resolve_n_years(R_xlen_t log_m_length,
                             R_xlen_t log_f_multiplier_length,
                             R_xlen_t n_ages)
    {
        if (n_ages < 1)
        {
            Rf_error("`log_init_naa` must contain at least one value.");
        }

        R_xlen_t n_years = 1;

        if (log_f_multiplier_length > 1)
        {
            n_years = log_f_multiplier_length;
        }

        if (log_m_length > 1)
        {
            if (n_years == 1)
            {
                if (log_m_length % n_ages == 0)
                {
                    n_years = log_m_length / n_ages;
                }
                else
                {
                    n_years = log_m_length;
                }
            }
            else if (log_m_length != n_years &&
                     log_m_length != n_years * n_ages)
            {
                Rf_error(
                    "`log_m` must have length 1, n_years, or n_years * n_ages.");
            }
        }

        return n_years;
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
    uint32_t create_population(
        uint32_t population_id,
        SEXP log_m_sexp,
        SEXP log_f_multiplier_sexp,
        SEXP log_init_naa_sexp,
        SEXP log_m_estimation_type_sexp,
        SEXP log_f_multiplier_estimation_type_sexp,
        SEXP log_init_naa_estimation_type_sexp,
        SEXP maturity_id_sexp,
        SEXP growth_id_sexp,
        SEXP recruitment_id_sexp,
        SEXP fleet_ids_sexp,
        bool register_parameter)
    {
        SEXP log_m_real = PROTECT(coerce_numeric_vector(log_m_sexp, "log_m"));
        SEXP log_f_multiplier_real =
            PROTECT(coerce_numeric_vector(log_f_multiplier_sexp, "log_f_multiplier"));
        SEXP log_init_naa_real =
            PROTECT(coerce_numeric_vector(log_init_naa_sexp, "log_init_naa"));
        SEXP log_m_estimation_type_int =
            PROTECT(coerce_integer_vector(log_m_estimation_type_sexp,
                                          "log_m_estimation_type"));
        SEXP log_f_multiplier_estimation_type_int =
            PROTECT(coerce_integer_vector(log_f_multiplier_estimation_type_sexp,
                                          "log_f_multiplier_estimation_type"));
        SEXP log_init_naa_estimation_type_int =
            PROTECT(coerce_integer_vector(log_init_naa_estimation_type_sexp,
                                          "log_init_naa_estimation_type"));
        SEXP fleet_ids_int = PROTECT(coerce_integer_vector(fleet_ids_sexp, "fleet_ids"));

        const R_xlen_t log_m_length = XLENGTH(log_m_real);
        const R_xlen_t log_f_multiplier_length = XLENGTH(log_f_multiplier_real);
        const R_xlen_t log_init_naa_length = XLENGTH(log_init_naa_real);
        const R_xlen_t n_years =
            resolve_n_years(log_m_length, log_f_multiplier_length, log_init_naa_length);

        const R_xlen_t normalized_log_f_multiplier_length =
            normalize_parameter_length(log_f_multiplier_length,
                                       n_years,
                                       "log_f_multiplier");

        const bool log_m_is_year_age = (log_m_length == (n_years * log_init_naa_length));
        if (log_m_length != 1 &&
            log_m_length != n_years &&
            !log_m_is_year_age)
        {
            Rf_error("`log_m` must have length 1, n_years, or n_years * n_ages.");
        }

        const R_xlen_t log_m_estimation_type_length = XLENGTH(log_m_estimation_type_int);
        const R_xlen_t log_f_multiplier_estimation_type_length =
            XLENGTH(log_f_multiplier_estimation_type_int);
        const R_xlen_t log_init_naa_estimation_type_length =
            XLENGTH(log_init_naa_estimation_type_int);

        const R_xlen_t expected_log_m_estimation_type_length =
            log_m_is_year_age ? (n_years * log_init_naa_length) : n_years;
        const R_xlen_t normalized_log_m_estimation_type_length =
            normalize_parameter_length(log_m_estimation_type_length,
                                       expected_log_m_estimation_type_length,
                                       "log_m_estimation_type");
        const R_xlen_t normalized_log_f_multiplier_estimation_type_length =
            normalize_parameter_length(log_f_multiplier_estimation_type_length,
                                       n_years,
                                       "log_f_multiplier_estimation_type");
        const R_xlen_t normalized_log_init_naa_estimation_type_length =
            normalize_parameter_length(log_init_naa_estimation_type_length,
                                       log_init_naa_length,
                                       "log_init_naa_estimation_type");

        const double *log_m_ptr = REAL(log_m_real);
        const double *log_f_multiplier_ptr = REAL(log_f_multiplier_real);
        const double *log_init_naa_ptr = REAL(log_init_naa_real);
        const int *log_m_estimation_type_ptr = INTEGER(log_m_estimation_type_int);
        const int *log_f_multiplier_estimation_type_ptr =
            INTEGER(log_f_multiplier_estimation_type_int);
        const int *log_init_naa_estimation_type_ptr =
            INTEGER(log_init_naa_estimation_type_int);
        const int *fleet_ids_ptr = INTEGER(fleet_ids_int);
        const R_xlen_t fleet_ids_length = XLENGTH(fleet_ids_int);

        std::shared_ptr<NativePopulationRegistry<Type>> registry =
            NativePopulationRegistry<Type>::GetInstance();
        std::shared_ptr<fims_info::Information<Type>> info = registry->GetInformation();
        std::shared_ptr<fims_popdy::Population<Type>> population =
            registry->CreatePopulation(population_id);

        population->n_years = static_cast<size_t>(n_years);
        population->n_ages = static_cast<size_t>(log_init_naa_length);
        population->n_fleets = static_cast<size_t>(fleet_ids_length);
        population->ages.resize(log_init_naa_length);
        population->ages.set_name("population.ages");
        for (R_xlen_t age_index = 0; age_index < log_init_naa_length; ++age_index)
        {
            // Use 1-based ages by default to match most FIMS input fixtures.
            population->ages[age_index] = static_cast<double>(age_index + 1);
        }

        population->log_M.resize(n_years * log_init_naa_length);
        population->log_M.set_name("population.log_M");
        population->M.resize(n_years * log_init_naa_length);
        population->M.set_name("population.M");
        population->log_f_multiplier.resize(n_years);
        population->log_f_multiplier.set_name("population.log_f_multiplier");
        population->f_multiplier.resize(n_years);
        population->f_multiplier.set_name("population.f_multiplier");

        for (R_xlen_t index = 0; index < n_years; ++index)
        {
            const R_xlen_t log_f_multiplier_index =
                (normalized_log_f_multiplier_length == 1) ? 0 : index;
            const R_xlen_t log_f_multiplier_estimation_type_index =
                (normalized_log_f_multiplier_estimation_type_length == 1) ? 0 : index;
            population->log_f_multiplier[index] =
                static_cast<Type>(log_f_multiplier_ptr[log_f_multiplier_index]);

            register_parameter_if_estimable(
                info,
                population->log_f_multiplier[index],
                log_f_multiplier_estimation_type_ptr[log_f_multiplier_estimation_type_index],
                "Population." + std::to_string(population_id) + ".log_f_multiplier." +
                    std::to_string(index),
                register_parameter);
        }

        for (R_xlen_t age_index = 0; age_index < log_init_naa_length; ++age_index)
        {
            for (R_xlen_t year_index = 0; year_index < n_years; ++year_index)
            {
                const R_xlen_t i_age_year = age_index * n_years + year_index;

                const R_xlen_t log_m_index =
                    (log_m_length == 1)
                        ? 0
                        : (log_m_is_year_age ? i_age_year : year_index);

                const R_xlen_t log_m_estimation_type_index =
                    (normalized_log_m_estimation_type_length == 1)
                        ? 0
                        : (log_m_is_year_age ? i_age_year : year_index);

                population->log_M[i_age_year] = static_cast<Type>(log_m_ptr[log_m_index]);

                register_parameter_if_estimable(
                    info,
                    population->log_M[i_age_year],
                    log_m_estimation_type_ptr[log_m_estimation_type_index],
                    "Population." + std::to_string(population_id) + ".log_M." +
                        std::to_string(i_age_year),
                    register_parameter);
            }
        }

        population->log_init_naa.resize(log_init_naa_length);
        population->log_init_naa.set_name("population.log_init_naa");
        for (R_xlen_t index = 0; index < log_init_naa_length; ++index)
        {
            const R_xlen_t log_init_naa_estimation_type_index =
                (normalized_log_init_naa_estimation_type_length == 1) ? 0 : index;

            population->log_init_naa[index] = static_cast<Type>(log_init_naa_ptr[index]);

            register_parameter_if_estimable(
                info,
                population->log_init_naa[index],
                log_init_naa_estimation_type_ptr[log_init_naa_estimation_type_index],
                "Population." + std::to_string(population_id) + ".log_init_naa." +
                    std::to_string(index),
                register_parameter);
        }

        const int maturity_id = Rf_asInteger(maturity_id_sexp);
        if (maturity_id != NA_INTEGER)
        {
            auto maturity_it = info->maturity_models.find(static_cast<uint32_t>(maturity_id));
            if (maturity_it == info->maturity_models.end())
            {
                Rf_error("No maturity with id %d was found for this population.", maturity_id);
            }
            population->maturity_id = maturity_id;
            population->maturity = maturity_it->second;
        }

        const int growth_id = Rf_asInteger(growth_id_sexp);
        if (growth_id != NA_INTEGER)
        {
            auto growth_it = info->growth_models.find(static_cast<uint32_t>(growth_id));
            if (growth_it == info->growth_models.end())
            {
                Rf_error("No growth with id %d was found for this population.", growth_id);
            }
            population->growth_id = growth_id;
            population->growth = growth_it->second;
        }

        const int recruitment_id = Rf_asInteger(recruitment_id_sexp);
        if (recruitment_id != NA_INTEGER)
        {
            auto recruitment_it =
                info->recruitment_models.find(static_cast<uint32_t>(recruitment_id));
            if (recruitment_it == info->recruitment_models.end())
            {
                Rf_error("No recruitment with id %d was found for this population.",
                         recruitment_id);
            }
            population->recruitment_id = recruitment_id;
            population->recruitment = recruitment_it->second;

            population->recruitment->log_expected_recruitment.resize(n_years);
            population->recruitment->log_expected_recruitment.set_name(
                "recruitment.log_expected_recruitment");
            if (population->recruitment->log_recruit_devs.size() == 0 && n_years > 0)
            {
                population->recruitment->log_recruit_devs.resize(n_years - 1);
                population->recruitment->log_recruit_devs.set_name(
                    "recruitment.log_recruit_devs");
                for (R_xlen_t year_index = 0; year_index < (n_years - 1); ++year_index)
                {
                    population->recruitment->log_recruit_devs[year_index] =
                        static_cast<Type>(0.0);
                }
            }
            if (population->recruitment->process == nullptr)
            {
                population->recruitment->process = population->recruitment;
            }
        }

        population->fleets.clear();
        population->fleet_ids.clear();
        for (R_xlen_t index = 0; index < fleet_ids_length; ++index)
        {
            const int fleet_id = fleet_ids_ptr[index];
            if (fleet_id == NA_INTEGER)
            {
                continue;
            }

            auto fleet_it = info->fleets.find(static_cast<uint32_t>(fleet_id));
            if (fleet_it == info->fleets.end())
            {
                Rf_error("No fleet with id %d was found for this population.", fleet_id);
            }

            std::shared_ptr<fims_popdy::Fleet<Type>> fleet = fleet_it->second;
            fleet->n_years = static_cast<size_t>(n_years);
            fleet->n_ages = static_cast<size_t>(log_init_naa_length);

            if (fleet->n_ages > 0 &&
                fleet->age_to_length_conversion.size() > 0 &&
                fleet->age_to_length_conversion.size() % fleet->n_ages == 0)
            {
                fleet->n_lengths =
                    fleet->age_to_length_conversion.size() / fleet->n_ages;
            }
            else
            {
                fleet->n_lengths = 0;
            }

            population->fleet_ids.insert(static_cast<uint32_t>(fleet_id));
            population->fleets.push_back(fleet);
        }

        UNPROTECT(7);
        return population->id;
    }

    template <typename Type>
    std::shared_ptr<fims_popdy::Population<Type>> get_population(uint32_t population_id)
    {
        std::shared_ptr<NativePopulationRegistry<Type>> registry =
            NativePopulationRegistry<Type>::GetInstance();
        std::shared_ptr<fims_popdy::Population<Type>> population =
            registry->GetPopulation(population_id);

        if (population == nullptr)
        {
            Rf_error("No population with id %u was found.", population_id);
        }

        return population;
    }

} // namespace

extern "C" SEXP fims_call_create_population(SEXP log_m_sexp,
                                            SEXP log_f_multiplier_sexp,
                                            SEXP log_init_naa_sexp,
                                            SEXP log_m_estimation_type_sexp,
                                            SEXP log_f_multiplier_estimation_type_sexp,
                                            SEXP log_init_naa_estimation_type_sexp,
                                            SEXP maturity_id_sexp,
                                            SEXP growth_id_sexp,
                                            SEXP recruitment_id_sexp,
                                            SEXP fleet_ids_sexp)
{
    std::shared_ptr<NativePopulationRegistry<double>> registry =
        NativePopulationRegistry<double>::GetInstance();
    uint32_t population_id =
        static_cast<uint32_t>(registry->GetInformation()->populations.size() + 1);

    create_population<double>(population_id,
                              log_m_sexp,
                              log_f_multiplier_sexp,
                              log_init_naa_sexp,
                              log_m_estimation_type_sexp,
                              log_f_multiplier_estimation_type_sexp,
                              log_init_naa_estimation_type_sexp,
                              maturity_id_sexp,
                              growth_id_sexp,
                              recruitment_id_sexp,
                              fleet_ids_sexp,
                              true);

#ifdef TMB_MODEL
    create_population<TMBAD_FIMS_TYPE>(population_id,
                                       log_m_sexp,
                                       log_f_multiplier_sexp,
                                       log_init_naa_sexp,
                                       log_m_estimation_type_sexp,
                                       log_f_multiplier_estimation_type_sexp,
                                       log_init_naa_estimation_type_sexp,
                                       maturity_id_sexp,
                                       growth_id_sexp,
                                       recruitment_id_sexp,
                                       fleet_ids_sexp,
                                       true);
#endif

    return Rf_ScalarInteger(static_cast<int>(population_id));
}

extern "C" SEXP fims_call_population_prepare(SEXP population_id_sexp)
{
    const uint32_t population_id = static_cast<uint32_t>(Rf_asInteger(population_id_sexp));
    std::shared_ptr<fims_popdy::Population<double>> population =
        get_population<double>(population_id);

    const R_xlen_t n_years = static_cast<R_xlen_t>(population->n_years);
    const R_xlen_t n_m = static_cast<R_xlen_t>(population->log_M.size());

    if (population->log_f_multiplier.size() < static_cast<size_t>(n_years))
    {
        Rf_error("Population %u has log_f_multiplier size %zu, expected at least %zu.",
                 population_id,
                 population->log_f_multiplier.size(),
                 static_cast<size_t>(n_years));
    }

    population->M.resize(n_m);
    for (R_xlen_t index = 0; index < n_m; ++index)
    {
        population->M[index] = std::exp(population->log_M[index]);
    }

    population->f_multiplier.resize(n_years);
    for (R_xlen_t index = 0; index < n_years; ++index)
    {
        population->f_multiplier[index] = std::exp(population->log_f_multiplier[index]);
    }

    SEXP out = PROTECT(Rf_allocVector(VECSXP, 2));
    SEXP out_names = PROTECT(Rf_allocVector(STRSXP, 2));
    SET_STRING_ELT(out_names, 0, Rf_mkChar("M"));
    SET_STRING_ELT(out_names, 1, Rf_mkChar("f_multiplier"));

    SEXP m_out = PROTECT(Rf_allocVector(REALSXP, n_m));
    SEXP f_multiplier_out = PROTECT(Rf_allocVector(REALSXP, n_years));

    for (R_xlen_t index = 0; index < n_m; ++index)
    {
        REAL(m_out)
        [index] = population->M[index];
    }

    for (R_xlen_t index = 0; index < n_years; ++index)
    {
        REAL(f_multiplier_out)
        [index] = population->f_multiplier[index];
    }

    SET_VECTOR_ELT(out, 0, m_out);
    SET_VECTOR_ELT(out, 1, f_multiplier_out);
    Rf_setAttrib(out, R_NamesSymbol, out_names);

    UNPROTECT(4);
    return out;
}
