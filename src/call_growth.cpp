/**
 * \file call_growth.cpp
 * \brief Native .Call growth interface for the FIMS framework.
 */

#include "../inst/include/interface/call/growth.hpp"

#include <algorithm>

#include "../inst/include/interface/call/growth_registry.hpp"

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
    void validate_ewaa_dimensions(R_xlen_t ages_length,
                                  R_xlen_t weights_length,
                                  int n_years)
    {
        if (n_years < 1)
        {
            Rf_error("`n_years` must be at least 1.");
        }

        if (ages_length < 1 || weights_length < 1)
        {
            Rf_error("`ages` and `weights` must have at least one value.");
        }

        const R_xlen_t expanded_size = ages_length * static_cast<R_xlen_t>(n_years + 1);
        if (weights_length != ages_length && weights_length != expanded_size)
        {
            Rf_error("`weights` must have length equal to `ages` or `ages * (n_years + 1)`.");
        }
    }

    void validate_growth_estimation_type(SEXP weights_estimation_type_sexp)
    {
        SEXP weights_estimation_type_int =
            PROTECT(coerce_integer_vector(weights_estimation_type_sexp,
                                          "weights_estimation_type"));

        const int *estimation_type_ptr = INTEGER(weights_estimation_type_int);
        const R_xlen_t estimation_type_length = XLENGTH(weights_estimation_type_int);

        for (R_xlen_t index = 0; index < estimation_type_length; ++index)
        {
            if (estimation_type_ptr[index] != kConstant)
            {
                Rf_error(
                    "EWAA `weights` currently support only constant estimation type (0). Got code %d.",
                    estimation_type_ptr[index]);
            }
        }

        UNPROTECT(1);
    }

    template <typename Type>
    uint32_t create_ewaa_growth(uint32_t growth_id,
                                SEXP ages_sexp,
                                SEXP weights_sexp,
                                SEXP n_years_sexp,
                                SEXP weights_estimation_type_sexp)
    {
        SEXP ages_real = PROTECT(coerce_numeric_vector(ages_sexp, "ages"));
        SEXP weights_real = PROTECT(coerce_numeric_vector(weights_sexp, "weights"));

        const int n_years = Rf_asInteger(n_years_sexp);
        const R_xlen_t ages_length = XLENGTH(ages_real);
        const R_xlen_t weights_length = XLENGTH(weights_real);

        validate_ewaa_dimensions<Type>(ages_length, weights_length, n_years);
        validate_growth_estimation_type(weights_estimation_type_sexp);

        const double *ages_ptr = REAL(ages_real);
        const double *weights_ptr = REAL(weights_real);
        const bool single_age_vector = (weights_length == ages_length);

        std::shared_ptr<NativeGrowthRegistry<Type>> registry =
            NativeGrowthRegistry<Type>::GetInstance();
        std::shared_ptr<fims_popdy::EWAAGrowth<Type>> growth =
            registry->CreateEWAAGrowth(growth_id);

        for (int year = 0; year <= n_years; ++year)
        {
            for (R_xlen_t age_index = 0; age_index < ages_length; ++age_index)
            {
                const R_xlen_t weight_index =
                    single_age_vector
                        ? age_index
                        : (static_cast<R_xlen_t>(year) * ages_length + age_index);
                growth->ewaa[year][ages_ptr[age_index]] = weights_ptr[weight_index];
            }
        }

        UNPROTECT(2);
        return growth->id;
    }

    template <typename Type>
    std::shared_ptr<fims_popdy::EWAAGrowth<Type>> get_ewaa_growth(uint32_t growth_id)
    {
        std::shared_ptr<NativeGrowthRegistry<Type>> registry =
            NativeGrowthRegistry<Type>::GetInstance();
        std::shared_ptr<fims_popdy::EWAAGrowth<Type>> growth =
            registry->GetEWAAGrowth(growth_id);

        if (growth == nullptr)
        {
            Rf_error("No EWAA growth with id %u was found.", growth_id);
        }

        return growth;
    }

} // namespace

extern "C" SEXP fims_call_create_ewaa_growth(SEXP ages_sexp,
                                             SEXP weights_sexp,
                                             SEXP n_years_sexp,
                                             SEXP weights_estimation_type_sexp)
{
    std::shared_ptr<NativeGrowthRegistry<double>> registry =
        NativeGrowthRegistry<double>::GetInstance();
    uint32_t growth_id =
        static_cast<uint32_t>(registry->GetInformation()->growth_models.size() + 1);

    create_ewaa_growth<double>(growth_id,
                               ages_sexp,
                               weights_sexp,
                               n_years_sexp,
                               weights_estimation_type_sexp);

#ifdef TMB_MODEL
    create_ewaa_growth<TMBAD_FIMS_TYPE>(growth_id,
                                        ages_sexp,
                                        weights_sexp,
                                        n_years_sexp,
                                        weights_estimation_type_sexp);
#endif

    return Rf_ScalarInteger(static_cast<int>(growth_id));
}

extern "C" SEXP fims_call_ewaa_growth_evaluate(SEXP year_sexp,
                                               SEXP age_sexp,
                                               SEXP growth_id_sexp)
{
    SEXP year_int = PROTECT(coerce_integer_vector(year_sexp, "year"));
    SEXP age_real = PROTECT(coerce_numeric_vector(age_sexp, "age"));

    const uint32_t growth_id = static_cast<uint32_t>(Rf_asInteger(growth_id_sexp));
    const R_xlen_t year_length = XLENGTH(year_int);
    const R_xlen_t age_length = XLENGTH(age_real);
    const R_xlen_t output_length = std::max(year_length, age_length);
    const R_xlen_t normalized_year_length =
        normalize_parameter_length(year_length, output_length, "year");
    const R_xlen_t normalized_age_length =
        normalize_parameter_length(age_length, output_length, "age");

    std::shared_ptr<fims_popdy::EWAAGrowth<double>> growth =
        get_ewaa_growth<double>(growth_id);

    SEXP out = PROTECT(Rf_allocVector(REALSXP, output_length));
    const int *year_ptr = INTEGER(year_int);
    const double *age_ptr = REAL(age_real);
    double *out_ptr = REAL(out);

    for (R_xlen_t index = 0; index < output_length; ++index)
    {
        const R_xlen_t year_index = (normalized_year_length == 1) ? 0 : index;
        const R_xlen_t age_index = (normalized_age_length == 1) ? 0 : index;
        const int year = year_ptr[year_index];
        const double age = age_ptr[age_index];

        auto year_it = growth->ewaa.find(year);
        if (year_it == growth->ewaa.end())
        {
            Rf_error("Year %d not found in EWAA growth object %u.", year, growth_id);
        }

        auto age_it = year_it->second.find(age);
        if (age_it == year_it->second.end())
        {
            Rf_error("Age %f not found for year %d in EWAA growth object %u.", age,
                     year, growth_id);
        }

        out_ptr[index] = growth->evaluate(year, age);
    }

    UNPROTECT(3);
    return out;
}
