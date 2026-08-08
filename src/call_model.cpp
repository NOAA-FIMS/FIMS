/**
 * \file call_model.cpp
 * \brief Native .Call model-construction interface for the FIMS framework.
 */

#include "../inst/include/interface/call/model.hpp"

#include "../inst/include/common/information.hpp"
#include "../inst/include/common/model.hpp"
#include "../inst/include/models/fisheries_models.hpp"

#include <Rinternals.h>

#include <cmath>
#include <memory>
#include <sstream>

namespace
{

    template <typename Type>
    void resize_with_recycled_last(fims::Vector<Type> &values, size_t new_size,
                                   const Type &default_value)
    {
        if (new_size == 0)
        {
            values.clear();
            return;
        }

        if (values.size() == 0)
        {
            values.resize(new_size);
            for (size_t i = 0; i < new_size; ++i)
            {
                values[i] = default_value;
            }
            return;
        }

        if (values.size() == 1 && new_size > 1)
        {
            const Type scalar = values[0];
            values.resize(new_size);
            for (size_t i = 0; i < new_size; ++i)
            {
                values[i] = scalar;
            }
            return;
        }

        if (values.size() != new_size)
        {
            const Type last_value = values[values.size() - 1];
            const size_t old_size = values.size();
            values.resize(new_size);
            for (size_t i = old_size; i < new_size; ++i)
            {
                values[i] = last_value;
            }
        }
    }

    template <typename Type>
    void reconcile_native_population_layout(
        std::shared_ptr<fims_info::Information<Type>> info)
    {
        for (auto population_it = info->populations.begin();
             population_it != info->populations.end(); ++population_it)
        {
            std::shared_ptr<fims_popdy::Population<Type>> population =
                population_it->second;
            if (population == nullptr)
            {
                continue;
            }

            const size_t n_years = population->n_years;
            const size_t n_ages = population->n_ages;
            const size_t n_age_year = n_years * n_ages;

            if (population->ages.size() != n_ages)
            {
                population->ages.resize(n_ages);
                for (size_t age = 0; age < n_ages; ++age)
                {
                    population->ages[age] = static_cast<double>(age + 1);
                }
            }

            if (population->log_M.size() != n_age_year)
            {
                Rf_error(
                    "Population %u has log_M size %zu but expected n_years*n_ages=%zu. "
                    "Refusing to resize estimable vectors after parameter registration.",
                    static_cast<unsigned int>(population->GetId()),
                    population->log_M.size(),
                    n_age_year);
            }

            resize_with_recycled_last<Type>(population->M, n_age_year,
                                            static_cast<Type>(0.0));
            if (population->log_f_multiplier.size() != n_years)
            {
                Rf_error(
                    "Population %u has log_f_multiplier size %zu but expected n_years=%zu. "
                    "Refusing to resize estimable vectors after parameter registration.",
                    static_cast<unsigned int>(population->GetId()),
                    population->log_f_multiplier.size(),
                    n_years);
            }
            resize_with_recycled_last<Type>(population->f_multiplier, n_years,
                                            static_cast<Type>(1.0));
            if (population->log_init_naa.size() != n_ages)
            {
                Rf_error(
                    "Population %u has log_init_naa size %zu but expected n_ages=%zu. "
                    "Refusing to resize estimable vectors after parameter registration.",
                    static_cast<unsigned int>(population->GetId()),
                    population->log_init_naa.size(),
                    n_ages);
            }

            if (population->recruitment != nullptr)
            {
                if (population->recruitment->log_rzero.size() < 1)
                {
                    Rf_error("Population %u recruitment log_rzero has size 0.",
                             static_cast<unsigned int>(population->GetId()));
                }
                resize_with_recycled_last<Type>(
                    population->recruitment->log_expected_recruitment, n_years,
                    static_cast<Type>(0.0));

                if (n_years > 0)
                {
                    resize_with_recycled_last<Type>(
                        population->recruitment->log_recruit_devs, n_years - 1,
                        static_cast<Type>(0.0));
                }
            }

            population->n_fleets = population->fleet_ids.size();
            for (size_t fleet_index = 0; fleet_index < population->fleets.size();
                 ++fleet_index)
            {
                std::shared_ptr<fims_popdy::Fleet<Type>> fleet =
                    population->fleets[fleet_index];
                if (fleet == nullptr)
                {
                    continue;
                }

                fleet->n_years = n_years;
                fleet->n_ages = n_ages;

                if (fleet->log_Fmort.size() != n_years)
                {
                    Rf_error(
                        "Fleet %u has log_Fmort size %zu but expected n_years=%zu. "
                        "Refusing to resize estimable vectors after parameter registration.",
                        static_cast<unsigned int>(fleet->GetId()),
                        fleet->log_Fmort.size(),
                        n_years);
                }
                resize_with_recycled_last<Type>(fleet->Fmort, n_years,
                                                static_cast<Type>(0.0));
                // Catchability may be scalar (constant through time) or
                // year-specific. Preserve the parameter dimensionality supplied
                // by the interface; Fleet::q.get_force_scalar(year) handles the
                // scalar broadcast during model evaluation.
                if (fleet->log_q.size() != 1 && fleet->log_q.size() != n_years)
                {
                    Rf_error(
                        "Fleet %u has log_q size %zu but expected 1 or n_years=%zu. "
                        "Refusing to resize estimable vectors after parameter registration.",
                        static_cast<unsigned int>(fleet->GetId()),
                        fleet->log_q.size(),
                        n_years);
                }
                resize_with_recycled_last<Type>(fleet->q, fleet->log_q.size(),
                                                static_cast<Type>(1.0));

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
            }
        }
    }

    template <typename Type>
    [[noreturn]] void fail_dimension_check(const std::string &message)
    {
        Rf_error("Native model dimension check failed: %s", message.c_str());
    }

    template <typename Type>
    void validate_population_and_fleet_dimensions(
        std::shared_ptr<fims_popdy::Population<Type>> population)
    {
        if (population == nullptr)
        {
            fail_dimension_check<Type>("Encountered null population pointer.");
        }

        const size_t expected_age_year_size = population->n_years * population->n_ages;

        if (population->n_years < 1)
        {
            fail_dimension_check<Type>("Population has n_years < 1.");
        }

        if (population->n_ages < 1)
        {
            fail_dimension_check<Type>("Population has n_ages < 1.");
        }

        if (population->ages.size() != population->n_ages)
        {
            std::stringstream ss;
            ss << "Population " << population->GetId()
               << " ages size mismatch: expected " << population->n_ages
               << ", got " << population->ages.size() << ".";
            fail_dimension_check<Type>(ss.str());
        }

        if (population->log_init_naa.size() != population->n_ages)
        {
            std::stringstream ss;
            ss << "Population " << population->GetId()
               << " log_init_naa size mismatch: expected " << population->n_ages
               << ", got " << population->log_init_naa.size() << ".";
            fail_dimension_check<Type>(ss.str());
        }

        if (population->log_M.size() != expected_age_year_size)
        {
            std::stringstream ss;
            ss << "Population " << population->GetId()
               << " log_M size mismatch: expected " << expected_age_year_size
               << ", got " << population->log_M.size() << ".";
            fail_dimension_check<Type>(ss.str());
        }

        if (population->M.size() != expected_age_year_size)
        {
            std::stringstream ss;
            ss << "Population " << population->GetId()
               << " M size mismatch: expected " << expected_age_year_size
               << ", got " << population->M.size() << ".";
            fail_dimension_check<Type>(ss.str());
        }

        if (population->log_f_multiplier.size() != population->n_years)
        {
            std::stringstream ss;
            ss << "Population " << population->GetId()
               << " log_f_multiplier size mismatch: expected " << population->n_years
               << ", got " << population->log_f_multiplier.size() << ".";
            fail_dimension_check<Type>(ss.str());
        }

        if (population->f_multiplier.size() != population->n_years)
        {
            std::stringstream ss;
            ss << "Population " << population->GetId()
               << " f_multiplier size mismatch: expected " << population->n_years
               << ", got " << population->f_multiplier.size() << ".";
            fail_dimension_check<Type>(ss.str());
        }

        if (population->recruitment == nullptr)
        {
            fail_dimension_check<Type>("Population is missing recruitment pointer.");
        }

        if (population->growth == nullptr)
        {
            fail_dimension_check<Type>("Population is missing growth pointer.");
        }

        if (population->maturity == nullptr)
        {
            fail_dimension_check<Type>("Population is missing maturity pointer.");
        }

        if (population->recruitment->log_rzero.size() < 1)
        {
            fail_dimension_check<Type>("Recruitment log_rzero has size 0.");
        }

        if (population->recruitment->log_expected_recruitment.size() != population->n_years)
        {
            std::stringstream ss;
            ss << "Recruitment log_expected_recruitment size mismatch: expected "
               << population->n_years << ", got "
               << population->recruitment->log_expected_recruitment.size() << ".";
            fail_dimension_check<Type>(ss.str());
        }

        if (population->recruitment->process == nullptr)
        {
            fail_dimension_check<Type>("Recruitment process pointer is null.");
        }

        if (population->fleets.size() != population->n_fleets)
        {
            std::stringstream ss;
            ss << "Population " << population->GetId()
               << " fleet count mismatch: expected " << population->n_fleets
               << ", got " << population->fleets.size() << ".";
            fail_dimension_check<Type>(ss.str());
        }

        for (size_t fleet_index = 0; fleet_index < population->fleets.size(); ++fleet_index)
        {
            std::shared_ptr<fims_popdy::Fleet<Type>> fleet = population->fleets[fleet_index];
            if (fleet == nullptr)
            {
                fail_dimension_check<Type>("Encountered null fleet pointer.");
            }

            if (fleet->n_years != population->n_years)
            {
                std::stringstream ss;
                ss << "Fleet " << fleet->GetId() << " n_years mismatch: expected "
                   << population->n_years << ", got " << fleet->n_years << ".";
                fail_dimension_check<Type>(ss.str());
            }

            if (fleet->n_ages != population->n_ages)
            {
                std::stringstream ss;
                ss << "Fleet " << fleet->GetId() << " n_ages mismatch: expected "
                   << population->n_ages << ", got " << fleet->n_ages << ".";
                fail_dimension_check<Type>(ss.str());
            }

            if (fleet->log_Fmort.size() != fleet->n_years)
            {
                std::stringstream ss;
                ss << "Fleet " << fleet->GetId() << " log_Fmort size mismatch: expected "
                   << fleet->n_years << ", got " << fleet->log_Fmort.size() << ".";
                fail_dimension_check<Type>(ss.str());
            }

            if (fleet->Fmort.size() != fleet->n_years)
            {
                std::stringstream ss;
                ss << "Fleet " << fleet->GetId() << " Fmort size mismatch: expected "
                   << fleet->n_years << ", got " << fleet->Fmort.size() << ".";
                fail_dimension_check<Type>(ss.str());
            }

            if (fleet->log_q.size() < 1)
            {
                std::stringstream ss;
                ss << "Fleet " << fleet->GetId() << " log_q has size 0.";
                fail_dimension_check<Type>(ss.str());
            }

            if (fleet->q.size() != fleet->log_q.size())
            {
                std::stringstream ss;
                ss << "Fleet " << fleet->GetId() << " q size mismatch: expected "
                   << fleet->log_q.size() << ", got " << fleet->q.size() << ".";
                fail_dimension_check<Type>(ss.str());
            }

            if (fleet->selectivity == nullptr)
            {
                std::stringstream ss;
                ss << "Fleet " << fleet->GetId() << " is missing selectivity pointer.";
                fail_dimension_check<Type>(ss.str());
            }

            if (fleet->n_lengths > 0 &&
                fleet->age_to_length_conversion.size() != (fleet->n_ages * fleet->n_lengths))
            {
                std::stringstream ss;
                ss << "Fleet " << fleet->GetId()
                   << " age_to_length_conversion size mismatch: expected "
                   << (fleet->n_ages * fleet->n_lengths) << ", got "
                   << fleet->age_to_length_conversion.size() << ".";
                fail_dimension_check<Type>(ss.str());
            }
        }
    }

    template <typename Type>
    void validate_backend_model_inputs(std::shared_ptr<fims_info::Information<Type>> info)
    {
        for (auto population_it = info->populations.begin();
             population_it != info->populations.end();
             ++population_it)
        {
            validate_population_and_fleet_dimensions<Type>(population_it->second);
        }
    }

    template <typename Type>
    void assign_population_derived_vectors(
        std::map<std::string, fims::Vector<Type>> &derived_quantities,
        size_t n_years,
        size_t n_ages)
    {
        derived_quantities["total_landings_weight"] = fims::Vector<Type>(
            n_years, Type(), "total_landings_weight");
        derived_quantities["total_landings_numbers"] = fims::Vector<Type>(
            n_years, Type(), "total_landings_numbers");
        derived_quantities["mortality_F"] = fims::Vector<Type>(
            n_years * n_ages, Type(), "mortality_F");
        derived_quantities["mortality_M"] = fims::Vector<Type>(
            n_years * n_ages, Type(), "mortality_M");
        derived_quantities["mortality_Z"] = fims::Vector<Type>(
            n_years * n_ages, Type(), "mortality_Z");
        derived_quantities["numbers_at_age"] = fims::Vector<Type>(
            (n_years + 1) * n_ages, Type(), "numbers_at_age");
        derived_quantities["unfished_numbers_at_age"] = fims::Vector<Type>(
            (n_years + 1) * n_ages, Type(), "unfished_numbers_at_age");
        derived_quantities["biomass"] =
            fims::Vector<Type>(n_years + 1, Type(), "biomass");
        derived_quantities["spawning_biomass"] =
            fims::Vector<Type>(n_years + 1, Type(), "spawning_biomass");
        derived_quantities["unfished_biomass"] =
            fims::Vector<Type>(n_years + 1, Type(), "unfished_biomass");
        derived_quantities["unfished_spawning_biomass"] = fims::Vector<Type>(
            n_years + 1, Type(), "unfished_spawning_biomass");
        derived_quantities["proportion_mature_at_age"] = fims::Vector<Type>(
            (n_years + 1) * n_ages, Type(), "proportion_mature_at_age");
        derived_quantities["expected_recruitment"] = fims::Vector<Type>(
            n_years + 1, Type(), "expected_recruitment");
        derived_quantities["sum_selectivity"] = fims::Vector<Type>(
            n_years * n_ages, Type(), "sum_selectivity");
    }

    template <typename Type>
    void assign_fleet_derived_vectors(
        std::map<std::string, fims::Vector<Type>> &derived_quantities,
        size_t n_years,
        size_t n_ages,
        size_t n_lengths)
    {
        derived_quantities["landings_numbers_at_age"] =
            fims::Vector<Type>(n_years * n_ages, Type(), "landings_numbers_at_age");
        derived_quantities["landings_weight_at_age"] =
            fims::Vector<Type>(n_years * n_ages, Type(), "landings_weight_at_age");
        derived_quantities["landings_numbers_at_length"] =
            fims::Vector<Type>(n_years * n_lengths, Type(), "landings_numbers_at_length");
        derived_quantities["landings_weight"] =
            fims::Vector<Type>(n_years, Type(), "landings_weight");
        derived_quantities["landings_numbers"] =
            fims::Vector<Type>(n_years, Type(), "landings_numbers");
        derived_quantities["landings_expected"] =
            fims::Vector<Type>(n_years, Type(), "landings_expected");
        derived_quantities["log_landings_expected"] =
            fims::Vector<Type>(n_years, Type(), "log_landings_expected");
        derived_quantities["agecomp_proportion"] =
            fims::Vector<Type>(n_years * n_ages, Type(), "agecomp_proportion");
        derived_quantities["lengthcomp_proportion"] =
            fims::Vector<Type>(n_years * n_lengths, Type(), "lengthcomp_proportion");
        derived_quantities["index_numbers_at_age"] =
            fims::Vector<Type>(n_years * n_ages, Type(), "index_numbers_at_age");
        derived_quantities["index_weight_at_age"] =
            fims::Vector<Type>(n_years * n_ages, Type(), "index_weight_at_age");
        derived_quantities["index_numbers_at_length"] =
            fims::Vector<Type>(n_years * n_lengths, Type(), "index_numbers_at_length");
        derived_quantities["index_weight"] =
            fims::Vector<Type>(n_years, Type(), "index_weight");
        derived_quantities["index_numbers"] =
            fims::Vector<Type>(n_years, Type(), "index_numbers");
        derived_quantities["index_expected"] =
            fims::Vector<Type>(n_years, Type(), "index_expected");
        derived_quantities["log_index_expected"] =
            fims::Vector<Type>(n_years, Type(), "log_index_expected");
        derived_quantities["agecomp_expected"] =
            fims::Vector<Type>(n_years * n_ages, Type(), "agecomp_expected");
        derived_quantities["lengthcomp_expected"] =
            fims::Vector<Type>(n_years * n_lengths, Type(), "lengthcomp_expected");

        // Compatibility aliases used by some links in objective components.
        derived_quantities["age_comp_proportion"] =
            fims::Vector<Type>(n_years * n_ages, Type(), "age_comp_proportion");
        derived_quantities["length_comp_proportion"] =
            fims::Vector<Type>(n_years * n_lengths, Type(), "length_comp_proportion");
        derived_quantities["age_comp_expected"] =
            fims::Vector<Type>(n_years * n_ages, Type(), "age_comp_expected");
        derived_quantities["length_comp_expected"] =
            fims::Vector<Type>(n_years * n_lengths, Type(), "length_comp_expected");
    }

    template <typename Type>
    void initialize_model_derived_quantity_maps(
        std::shared_ptr<fims_info::Information<Type>> info)
    {
        for (auto model_it = info->models_map.begin();
             model_it != info->models_map.end();
             ++model_it)
        {
            std::shared_ptr<fims_popdy::FisheryModelBase<Type>> model = model_it->second;
            if (model == nullptr)
            {
                continue;
            }

            for (auto population_id_it = model->population_ids.begin();
                 population_id_it != model->population_ids.end();
                 ++population_id_it)
            {
                model->InitializePopulationDerivedQuantities(*population_id_it);
            }

            for (auto population_it = model->populations.begin();
                 population_it != model->populations.end();
                 ++population_it)
            {
                if (*population_it == nullptr)
                {
                    continue;
                }

                model->InitializePopulationDerivedQuantities((*population_it)->GetId());
                std::map<std::string, fims::Vector<Type>> &population_derived_quantities =
                    model->GetPopulationDerivedQuantities((*population_it)->GetId());
                assign_population_derived_vectors<Type>(population_derived_quantities,
                                                        (*population_it)->n_years,
                                                        (*population_it)->n_ages);

                for (auto fleet_id_it = (*population_it)->fleet_ids.begin();
                     fleet_id_it != (*population_it)->fleet_ids.end();
                     ++fleet_id_it)
                {
                    model->InitializeFleetDerivedQuantities(*fleet_id_it);
                }
            }

            for (auto fleet_it = model->fleets.begin();
                 fleet_it != model->fleets.end();
                 ++fleet_it)
            {
                model->InitializeFleetDerivedQuantities(fleet_it->first);
                std::map<std::string, fims::Vector<Type>> &fleet_derived_quantities =
                    model->GetFleetDerivedQuantities(fleet_it->first);
                assign_fleet_derived_vectors<Type>(fleet_derived_quantities,
                                                   fleet_it->second->n_years,
                                                   fleet_it->second->n_ages,
                                                   fleet_it->second->n_lengths);
            }
        }
    }

    template <typename Type>
    void reset_backend_id_counters()
    {
        fims_data_object::DataObject<Type>::id_g = 0;
        fims_distributions::DensityComponentBase<Type>::id_g = 0;
        fims_popdy::Fleet<Type>::id_g = 0;
        fims_popdy::Population<Type>::id_g = 0;
        fims_popdy::GrowthBase<Type>::id_g = 0;
        fims_popdy::MaturityBase<Type>::id_g = 0;
        fims_popdy::RecruitmentBase<Type>::id_g = 0;
        fims_popdy::SelectivityBase<Type>::id_g = 0;
    }

    /**
     * @brief Ensure a CatchAtAge shell and its derived-quantity maps exist
     * without finalizing Information::CreateModel().
     *
     * Native likelihood components need stable references to the same model
     * derived-quantity vectors used by the objective. Rcpp creates and links
     * its data/distributions before CreateTMBModel(); this helper provides the
     * corresponding native pre-assembly state while deliberately postponing
     * Information::CreateModel() until fims_call_create_model().
     */
    template <typename Type>
    void prepare_backend_model_shell_internal()
    {
        std::shared_ptr<fims_info::Information<Type>> info =
            fims_info::Information<Type>::GetInstance();

        if (info->populations.empty())
        {
            Rf_error("No populations are registered. Create at least one population before building likelihood components.");
        }

        reconcile_native_population_layout<Type>(info);

        if (info->models_map.empty())
        {
            std::shared_ptr<fims_popdy::CatchAtAge<Type>> model =
                std::make_shared<fims_popdy::CatchAtAge<Type>>();

            for (auto population_it = info->populations.begin();
                 population_it != info->populations.end();
                 ++population_it)
            {
                if (population_it->second == nullptr)
                {
                    continue;
                }

                model->AddPopulation(population_it->first);
                model->InitializePopulationDerivedQuantities(population_it->first);

                std::map<std::string, fims::Vector<Type>> &population_dq =
                    model->GetPopulationDerivedQuantities(population_it->first);
                assign_population_derived_vectors<Type>(
                    population_dq,
                    population_it->second->n_years,
                    population_it->second->n_ages);

                for (auto fleet_id_it = population_it->second->fleet_ids.begin();
                     fleet_id_it != population_it->second->fleet_ids.end();
                     ++fleet_id_it)
                {
                    auto fleet_it = info->fleets.find(*fleet_id_it);
                    if (fleet_it == info->fleets.end() || fleet_it->second == nullptr)
                    {
                        Rf_error("Population %u references missing fleet %u.",
                                 static_cast<unsigned int>(population_it->first),
                                 static_cast<unsigned int>(*fleet_id_it));
                    }

                    std::shared_ptr<fims_popdy::Fleet<Type>> fleet = fleet_it->second;
                    fleet->n_years = population_it->second->n_years;
                    fleet->n_ages = population_it->second->n_ages;
                    if (fleet->n_ages > 0 &&
                        fleet->age_to_length_conversion.size() > 0 &&
                        fleet->age_to_length_conversion.size() % fleet->n_ages == 0)
                    {
                        fleet->n_lengths =
                            fleet->age_to_length_conversion.size() / fleet->n_ages;
                    }

                    model->InitializeFleetDerivedQuantities(*fleet_id_it);
                    std::map<std::string, fims::Vector<Type>> &fleet_dq =
                        model->GetFleetDerivedQuantities(*fleet_id_it);
                    assign_fleet_derived_vectors<Type>(
                        fleet_dq,
                        fleet->n_years,
                        fleet->n_ages,
                        fleet->n_lengths);
                }
            }

            info->models_map[1] = model;
        }
        else
        {
            // A shell already exists. Refresh dimensions/maps without calling
            // Information::CreateModel(); std::map node addresses remain stable,
            // so likelihood pointers to the vector objects remain valid.
            for (auto model_it = info->models_map.begin();
                 model_it != info->models_map.end();
                 ++model_it)
            {
                std::shared_ptr<fims_popdy::FisheryModelBase<Type>> model =
                    model_it->second;
                if (model == nullptr)
                {
                    continue;
                }

                for (auto population_it = info->populations.begin();
                     population_it != info->populations.end();
                     ++population_it)
                {
                    if (population_it->second == nullptr)
                    {
                        continue;
                    }

                    // The shell already owns this population id. AddPopulation()
                    // is a CatchAtAge-only API and is not available through
                    // FisheryModelBase, so only refresh the derived-quantity map.
                    model->InitializePopulationDerivedQuantities(population_it->first);
                    std::map<std::string, fims::Vector<Type>> &population_dq =
                        model->GetPopulationDerivedQuantities(population_it->first);
                    assign_population_derived_vectors<Type>(
                        population_dq,
                        population_it->second->n_years,
                        population_it->second->n_ages);

                    for (auto fleet_id_it = population_it->second->fleet_ids.begin();
                         fleet_id_it != population_it->second->fleet_ids.end();
                         ++fleet_id_it)
                    {
                        auto fleet_it = info->fleets.find(*fleet_id_it);
                        if (fleet_it == info->fleets.end() || fleet_it->second == nullptr)
                        {
                            continue;
                        }

                        std::shared_ptr<fims_popdy::Fleet<Type>> fleet = fleet_it->second;
                        fleet->n_years = population_it->second->n_years;
                        fleet->n_ages = population_it->second->n_ages;
                        if (fleet->n_ages > 0 &&
                            fleet->age_to_length_conversion.size() > 0 &&
                            fleet->age_to_length_conversion.size() % fleet->n_ages == 0)
                        {
                            fleet->n_lengths =
                                fleet->age_to_length_conversion.size() / fleet->n_ages;
                        }

                        model->InitializeFleetDerivedQuantities(*fleet_id_it);
                        std::map<std::string, fims::Vector<Type>> &fleet_dq =
                            model->GetFleetDerivedQuantities(*fleet_id_it);
                        assign_fleet_derived_vectors<Type>(
                            fleet_dq,
                            fleet->n_years,
                            fleet->n_ages,
                            fleet->n_lengths);
                    }
                }
            }
        }
    }

    template <typename Type>
    bool create_backend_model_internal()
    {
        std::shared_ptr<fims_info::Information<Type>> info =
            fims_info::Information<Type>::GetInstance();

        if (info->populations.empty())
        {
            Rf_error("No populations are registered. Create at least one population before calling fims_call_create_model().");
        }

        if (info->models_map.empty())
        {
            std::shared_ptr<fims_popdy::CatchAtAge<Type>> model =
                std::make_shared<fims_popdy::CatchAtAge<Type>>();

            for (auto population_it = info->populations.begin();
                 population_it != info->populations.end();
                 ++population_it)
            {
                model->AddPopulation(population_it->first);
                model->InitializePopulationDerivedQuantities(population_it->first);

                std::shared_ptr<fims_popdy::Population<Type>> population = population_it->second;
                if (population != nullptr)
                {
                    for (auto fleet_id_it = population->fleet_ids.begin();
                         fleet_id_it != population->fleet_ids.end();
                         ++fleet_id_it)
                    {
                        model->InitializeFleetDerivedQuantities(*fleet_id_it);
                    }
                }
            }

            info->models_map[1] = model;
        }

        // Information::CreateModel() appends links into population->fleets and
        // model->{populations,fleets}. Clear containers first so repeated
        // CreateModel calls are idempotent in the native call path.
        for (auto population_it = info->populations.begin();
             population_it != info->populations.end();
             ++population_it)
        {
            if (population_it->second == nullptr)
            {
                continue;
            }
            population_it->second->fleets.clear();
        }

        for (auto model_it = info->models_map.begin();
             model_it != info->models_map.end();
             ++model_it)
        {
            std::shared_ptr<fims_popdy::FisheryModelBase<Type>> model =
                model_it->second;
            if (model == nullptr)
            {
                continue;
            }
            model->populations.clear();
            model->fleets.clear();
        }

        reconcile_native_population_layout<Type>(info);
        const bool model_created = info->CreateModel();
        initialize_model_derived_quantity_maps<Type>(info);
        validate_backend_model_inputs<Type>(info);
        const bool model_valid = info->CheckModel();
        return model_created && model_valid;
    }

    SEXP require_numeric_vector(SEXP sexp, const char *argument_name)
    {
        if (!Rf_isNumeric(sexp))
        {
            Rf_error("`%s` must be numeric.", argument_name);
        }
        return Rf_coerceVector(sexp, REALSXP);
    }

    template <typename Type>
    std::shared_ptr<fims_data_object::DataObject<Type>> create_data_object_1d(
        std::shared_ptr<fims_info::Information<Type>> info,
        SEXP data_sexp,
        size_t expected_size,
        const char *argument_name)
    {
        SEXP data_real = PROTECT(require_numeric_vector(data_sexp, argument_name));
        if (static_cast<size_t>(XLENGTH(data_real)) != expected_size)
        {
            UNPROTECT(1);
            Rf_error("`%s` must have length %zu.", argument_name, expected_size);
        }

        std::shared_ptr<fims_data_object::DataObject<Type>> data_object =
            std::make_shared<fims_data_object::DataObject<Type>>(expected_size);
        const double *data_ptr = REAL(data_real);
        for (size_t index = 0; index < expected_size; ++index)
        {
            data_object->at(index) = static_cast<Type>(data_ptr[index]);
            data_object->uncertainty[index] = static_cast<Type>(0.0);
        }

        info->data_objects[data_object->id] = data_object;
        UNPROTECT(1);
        return data_object;
    }

    template <typename Type>
    std::shared_ptr<fims_data_object::DataObject<Type>> create_data_object_2d(
        std::shared_ptr<fims_info::Information<Type>> info,
        SEXP data_sexp,
        size_t n_years,
        size_t n_bins,
        const char *argument_name)
    {
        const size_t expected_size = n_years * n_bins;
        SEXP data_real = PROTECT(require_numeric_vector(data_sexp, argument_name));
        if (static_cast<size_t>(XLENGTH(data_real)) != expected_size)
        {
            UNPROTECT(1);
            Rf_error("`%s` must have length n_years * n_bins = %zu.",
                     argument_name,
                     expected_size);
        }

        std::shared_ptr<fims_data_object::DataObject<Type>> data_object =
            std::make_shared<fims_data_object::DataObject<Type>>(n_years, n_bins);
        const double *data_ptr = REAL(data_real);
        for (size_t year = 0; year < n_years; ++year)
        {
            for (size_t bin = 0; bin < n_bins; ++bin)
            {
                const size_t index = year * n_bins + bin;
                data_object->at(year, bin) = static_cast<Type>(data_ptr[index]);
                data_object->uncertainty[index] = static_cast<Type>(0.0);
            }
        }

        info->data_objects[data_object->id] = data_object;
        UNPROTECT(1);
        return data_object;
    }

    template <typename Type>
    std::shared_ptr<fims_popdy::Fleet<Type>> require_fleet(
        std::shared_ptr<fims_info::Information<Type>> info,
        uint32_t fleet_id,
        const char *argument_name)
    {
        auto fleet_it = info->fleets.find(fleet_id);
        if (fleet_it == info->fleets.end() || fleet_it->second == nullptr)
        {
            Rf_error("No fleet with id %u was found for `%s`.",
                     static_cast<unsigned int>(fleet_id),
                     argument_name);
        }
        return fleet_it->second;
    }

    template <typename Type>
    fims::Vector<Type> *require_fleet_derived_quantity(
        std::shared_ptr<fims_info::Information<Type>> info,
        uint32_t fleet_id,
        const std::string &derived_name)
    {
        std::shared_ptr<fims_popdy::FisheryModelBase<Type>> model =
            info->models_map.begin()->second;
        if (model == nullptr)
        {
            Rf_error("Model pointer is null while resolving derived quantity `%s`.",
                     derived_name.c_str());
        }

        std::map<std::string, fims::Vector<Type>> &derived_quantities =
            model->GetFleetDerivedQuantities(fleet_id);
        auto derived_it = derived_quantities.find(derived_name);
        if (derived_it == derived_quantities.end())
        {
            Rf_error("Fleet %u is missing derived quantity `%s`.",
                     static_cast<unsigned int>(fleet_id),
                     derived_name.c_str());
        }

        return &(derived_it->second);
    }

    template <typename Type>
    void add_lognormal_component(
        std::shared_ptr<fims_info::Information<Type>> info,
        std::shared_ptr<fims_data_object::DataObject<Type>> observed_data,
        fims::Vector<Type> *expected_values,
        size_t n_years,
        double cv)
    {
        if (expected_values == nullptr)
        {
            Rf_error("Lognormal expected-values pointer is null.");
        }
        if (expected_values->size() != n_years)
        {
            Rf_error("Lognormal expected-values size mismatch: expected %zu, got %zu.",
                     n_years,
                     expected_values->size());
        }

        if (!std::isfinite(cv) || cv <= 0.0)
        {
            Rf_error("Lognormal CV must be finite and > 0.");
        }

        const Type log_sd_value = static_cast<Type>(
            std::log(std::sqrt(std::log(cv * cv + 1.0))));

        std::shared_ptr<fims_distributions::LogNormalLPDF<Type>> distribution =
            std::make_shared<fims_distributions::LogNormalLPDF<Type>>();
        distribution->input_type = "data";
        distribution->observed_data_id_m =
            static_cast<int>(observed_data->id);
        distribution->data_observed_values = observed_data;
        distribution->data_expected_values = expected_values;
        distribution->log_sd.resize(n_years);
        for (size_t year = 0; year < n_years; ++year)
        {
            distribution->log_sd[year] = log_sd_value;
        }

        info->density_components[distribution->id] = distribution;
    }

    template <typename Type>
    void add_normal_observed_component(
        std::shared_ptr<fims_info::Information<Type>> info,
        fims::Vector<Type> *values,
        double sd)
    {
        if (values == nullptr || values->size() == 0)
        {
            return;
        }
        if (!std::isfinite(sd) || sd <= 0.0)
        {
            Rf_error("Normal random-effects SD must be finite and > 0.");
        }

        std::shared_ptr<fims_distributions::NormalLPDF<Type>> distribution =
            std::make_shared<fims_distributions::NormalLPDF<Type>>();

        // Match the Rcpp DnormDistribution link:
        //
        //   set_distribution_links("random_effects", recruitment$log_devs$id)
        //
        // Model::Evaluate() only accumulates "prior", "random_effects", and
        // "data" density components. The old native "process" input type was
        // therefore silently excluded from the objective.
        distribution->input_type = "random_effects";
        distribution->re = values;
        distribution->expected_values.resize(values->size());
        for (size_t index = 0; index < values->size(); ++index)
        {
            distribution->expected_values[index] = static_cast<Type>(0.0);
        }
        distribution->re_expected_values = &(distribution->expected_values);
        distribution->log_sd.resize(1);
        distribution->log_sd[0] = static_cast<Type>(std::log(sd));

        info->density_components[distribution->id] = distribution;
    }

    template <typename Type>
    void add_multinomial_component(
        std::shared_ptr<fims_info::Information<Type>> info,
        std::shared_ptr<fims_data_object::DataObject<Type>> observed_data,
        fims::Vector<Type> *expected_values,
        size_t n_years,
        size_t n_bins)
    {
        const size_t expected_size = n_years * n_bins;
        if (expected_values == nullptr)
        {
            Rf_error("Multinomial expected-values pointer is null.");
        }
        if (expected_values->size() != expected_size)
        {
            Rf_error("Multinomial expected-values size mismatch: expected %zu, got %zu.",
                     expected_size,
                     expected_values->size());
        }

        std::shared_ptr<fims_distributions::MultinomialLPMF<Type>> distribution =
            std::make_shared<fims_distributions::MultinomialLPMF<Type>>();
        distribution->input_type = "data";
        distribution->observed_data_id_m =
            static_cast<int>(observed_data->id);
        distribution->data_observed_values = observed_data;
        distribution->data_expected_values = expected_values;
        distribution->dims.resize(2);
        distribution->dims[0] = n_years;
        distribution->dims[1] = n_bins;

        info->density_components[distribution->id] = distribution;
    }

    template <typename Type>
    void build_default_likelihood_internal(
        uint32_t fishing_fleet_id,
        uint32_t survey_fleet_id,
        SEXP landings_sexp,
        double landings_cv,
        SEXP landings_age_comp_sexp,
        SEXP landings_length_comp_sexp,
        SEXP survey_index_sexp,
        double survey_cv,
        SEXP survey_age_comp_sexp,
        SEXP survey_length_comp_sexp,
        double recruitment_log_sd,
        size_t n_years,
        size_t n_ages,
        size_t n_lengths)
    {
        std::shared_ptr<fims_info::Information<Type>> info =
            fims_info::Information<Type>::GetInstance();

        // Match the Rcpp lifecycle: create data/distributions and their links
        // before final Information::CreateModel() assembly. This creates only
        // the CatchAtAge shell and derived-quantity storage required for links.
        prepare_backend_model_shell_internal<Type>();

        std::shared_ptr<fims_popdy::Fleet<Type>> fishing_fleet =
            require_fleet<Type>(info, fishing_fleet_id, "fishing_fleet_id");
        std::shared_ptr<fims_popdy::Fleet<Type>> survey_fleet =
            require_fleet<Type>(info, survey_fleet_id, "survey_fleet_id");
        bool enable_length_comp =
            !Rf_isNull(landings_length_comp_sexp) &&
            !Rf_isNull(survey_length_comp_sexp) &&
            n_lengths > 0;

        if (fishing_fleet->n_years != n_years || survey_fleet->n_years != n_years)
        {
            Rf_error("`n_years` (%zu) does not match model fleet dimensions (%zu, %zu).",
                     n_years,
                     fishing_fleet->n_years,
                     survey_fleet->n_years);
        }
        if (fishing_fleet->n_ages != n_ages || survey_fleet->n_ages != n_ages)
        {
            Rf_error("`n_ages` (%zu) does not match model fleet dimensions (%zu, %zu).",
                     n_ages,
                     fishing_fleet->n_ages,
                     survey_fleet->n_ages);
        }
        if (enable_length_comp)
        {
            if (fishing_fleet->n_lengths == 0 && survey_fleet->n_lengths == 0)
            {
                Rf_warning("Length composition data were provided but model fleets have n_lengths = 0. Skipping length-composition likelihood components.");
                enable_length_comp = false;
            }
            else if (fishing_fleet->n_lengths != n_lengths || survey_fleet->n_lengths != n_lengths)
            {
                Rf_error("`n_lengths` (%zu) does not match model fleet dimensions (%zu, %zu).",
                         n_lengths,
                         fishing_fleet->n_lengths,
                         survey_fleet->n_lengths);
            }
        }

        info->data_objects.clear();
        info->density_components.clear();

        if (!Rf_isNull(landings_sexp) && XLENGTH(landings_sexp) > 0)
        {
            std::shared_ptr<fims_data_object::DataObject<Type>> fishing_landings_data =
                create_data_object_1d<Type>(
                    info,
                    landings_sexp,
                    n_years,
                    "landings");
            fishing_fleet->fleet_observed_landings_data_id_m =
                static_cast<int>(fishing_landings_data->id);
            fishing_fleet->observed_landings_data = fishing_landings_data;
            add_lognormal_component<Type>(
                info,
                fishing_landings_data,
                require_fleet_derived_quantity<Type>(
                    info,
                    fishing_fleet_id,
                    "log_landings_expected"),
                n_years,
                landings_cv);
        }
        else
        {
            fishing_fleet->fleet_observed_landings_data_id_m = -999;
            fishing_fleet->observed_landings_data.reset();
        }

        if (!Rf_isNull(survey_index_sexp) && XLENGTH(survey_index_sexp) > 0)
        {
            std::shared_ptr<fims_data_object::DataObject<Type>> survey_index_data =
                create_data_object_1d<Type>(
                    info,
                    survey_index_sexp,
                    n_years,
                    "survey_index");
            survey_fleet->fleet_observed_index_data_id_m =
                static_cast<int>(survey_index_data->id);
            survey_fleet->observed_index_data = survey_index_data;
            add_lognormal_component<Type>(
                info,
                survey_index_data,
                require_fleet_derived_quantity<Type>(
                    info,
                    survey_fleet_id,
                    "log_index_expected"),
                n_years,
                survey_cv);
        }
        else
        {
            survey_fleet->fleet_observed_index_data_id_m = -999;
            survey_fleet->observed_index_data.reset();
        }

        if (!Rf_isNull(landings_age_comp_sexp) && XLENGTH(landings_age_comp_sexp) > 0)
        {
            std::shared_ptr<fims_data_object::DataObject<Type>> fishing_age_comp_data =
                create_data_object_2d<Type>(
                    info,
                    landings_age_comp_sexp,
                    n_years,
                    n_ages,
                    "landings_age_comp");
            fishing_fleet->fleet_observed_agecomp_data_id_m =
                static_cast<int>(fishing_age_comp_data->id);
            fishing_fleet->observed_agecomp_data = fishing_age_comp_data;
            add_multinomial_component<Type>(
                info,
                fishing_age_comp_data,
                require_fleet_derived_quantity<Type>(
                    info,
                    fishing_fleet_id,
                    "agecomp_proportion"),
                n_years,
                n_ages);
        }
        else
        {
            fishing_fleet->fleet_observed_agecomp_data_id_m = -999;
            fishing_fleet->observed_agecomp_data.reset();
        }

        if (!Rf_isNull(survey_age_comp_sexp) && XLENGTH(survey_age_comp_sexp) > 0)
        {
            std::shared_ptr<fims_data_object::DataObject<Type>> survey_age_comp_data =
                create_data_object_2d<Type>(
                    info,
                    survey_age_comp_sexp,
                    n_years,
                    n_ages,
                    "survey_age_comp");
            survey_fleet->fleet_observed_agecomp_data_id_m =
                static_cast<int>(survey_age_comp_data->id);
            survey_fleet->observed_agecomp_data = survey_age_comp_data;
            add_multinomial_component<Type>(
                info,
                survey_age_comp_data,
                require_fleet_derived_quantity<Type>(
                    info,
                    survey_fleet_id,
                    "agecomp_proportion"),
                n_years,
                n_ages);
        }
        else
        {
            survey_fleet->fleet_observed_agecomp_data_id_m = -999;
            survey_fleet->observed_agecomp_data.reset();
        }

        if (enable_length_comp && !Rf_isNull(landings_length_comp_sexp) &&
            XLENGTH(landings_length_comp_sexp) > 0)
        {
            std::shared_ptr<fims_data_object::DataObject<Type>>
                fishing_length_comp_data = create_data_object_2d<Type>(
                    info,
                    landings_length_comp_sexp,
                    n_years,
                    n_lengths,
                    "landings_length_comp");
            fishing_fleet->fleet_observed_lengthcomp_data_id_m =
                static_cast<int>(fishing_length_comp_data->id);
            fishing_fleet->observed_lengthcomp_data = fishing_length_comp_data;
            add_multinomial_component<Type>(
                info,
                fishing_length_comp_data,
                require_fleet_derived_quantity<Type>(
                    info,
                    fishing_fleet_id,
                    "lengthcomp_proportion"),
                n_years,
                n_lengths);

            std::shared_ptr<fims_data_object::DataObject<Type>>
                survey_length_comp_data = create_data_object_2d<Type>(
                    info,
                    survey_length_comp_sexp,
                    n_years,
                    n_lengths,
                    "survey_length_comp");
            survey_fleet->fleet_observed_lengthcomp_data_id_m =
                static_cast<int>(survey_length_comp_data->id);
            survey_fleet->observed_lengthcomp_data = survey_length_comp_data;
            add_multinomial_component<Type>(
                info,
                survey_length_comp_data,
                require_fleet_derived_quantity<Type>(
                    info,
                    survey_fleet_id,
                    "lengthcomp_proportion"),
                n_years,
                n_lengths);
        }
        else
        {
            fishing_fleet->fleet_observed_lengthcomp_data_id_m = -999;
            fishing_fleet->observed_lengthcomp_data.reset();
            survey_fleet->fleet_observed_lengthcomp_data_id_m = -999;
            survey_fleet->observed_lengthcomp_data.reset();
        }

        if (std::isfinite(recruitment_log_sd) && recruitment_log_sd > 0.0 &&
            !info->populations.empty())
        {
            std::shared_ptr<fims_popdy::Population<Type>> population =
                info->populations.begin()->second;
            if (population != nullptr && population->recruitment != nullptr)
            {
                add_normal_observed_component<Type>(
                    info,
                    &(population->recruitment->log_recruit_devs),
                    recruitment_log_sd);
            }
        }
    }

} // namespace

extern "C" SEXP fims_call_create_model()
{
    bool created = create_backend_model_internal<double>();

#ifdef TMB_MODEL
    created = create_backend_model_internal<TMBAD_FIMS_TYPE>() && created;
#endif

    std::shared_ptr<fims_model::Model<TMB_FIMS_REAL_TYPE>> model_singleton =
        fims_model::Model<TMB_FIMS_REAL_TYPE>::GetInstance();
    (void)model_singleton;

    return Rf_ScalarLogical(created ? TRUE : FALSE);
}

extern "C" SEXP fims_call_build_default_likelihood(
    SEXP fishing_fleet_id_sexp,
    SEXP survey_fleet_id_sexp,
    SEXP landings_sexp,
    SEXP landings_cv_sexp,
    SEXP landings_age_comp_sexp,
    SEXP landings_length_comp_sexp,
    SEXP survey_index_sexp,
    SEXP survey_cv_sexp,
    SEXP survey_age_comp_sexp,
    SEXP survey_length_comp_sexp,
    SEXP recruitment_log_sd_sexp,
    SEXP n_years_sexp,
    SEXP n_ages_sexp,
    SEXP n_lengths_sexp)
{
    const uint32_t fishing_fleet_id =
        static_cast<uint32_t>(Rf_asInteger(fishing_fleet_id_sexp));
    const uint32_t survey_fleet_id =
        static_cast<uint32_t>(Rf_asInteger(survey_fleet_id_sexp));
    const double landings_cv = Rf_asReal(landings_cv_sexp);
    const double survey_cv = Rf_asReal(survey_cv_sexp);
    const double recruitment_log_sd = Rf_asReal(recruitment_log_sd_sexp);
    const size_t n_years = static_cast<size_t>(Rf_asInteger(n_years_sexp));
    const size_t n_ages = static_cast<size_t>(Rf_asInteger(n_ages_sexp));
    const size_t n_lengths = static_cast<size_t>(Rf_asInteger(n_lengths_sexp));

    if (n_years < 1 || n_ages < 1)
    {
        Rf_error("`n_years` and `n_ages` must be positive integers.");
    }

    build_default_likelihood_internal<double>(
        fishing_fleet_id,
        survey_fleet_id,
        landings_sexp,
        landings_cv,
        landings_age_comp_sexp,
        landings_length_comp_sexp,
        survey_index_sexp,
        survey_cv,
        survey_age_comp_sexp,
        survey_length_comp_sexp,
        recruitment_log_sd,
        n_years,
        n_ages,
        n_lengths);

#ifdef TMB_MODEL
    build_default_likelihood_internal<TMBAD_FIMS_TYPE>(
        fishing_fleet_id,
        survey_fleet_id,
        landings_sexp,
        landings_cv,
        landings_age_comp_sexp,
        landings_length_comp_sexp,
        survey_index_sexp,
        survey_cv,
        survey_age_comp_sexp,
        survey_length_comp_sexp,
        recruitment_log_sd,
        n_years,
        n_ages,
        n_lengths);
#endif

    return Rf_ScalarLogical(TRUE);
}

extern "C" SEXP fims_call_information_model_counts()
{
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    SEXP out = PROTECT(Rf_allocVector(INTSXP, 5));
    SEXP out_names = PROTECT(Rf_allocVector(STRSXP, 5));

    SET_STRING_ELT(out_names, 0, Rf_mkChar("models_map"));
    SET_STRING_ELT(out_names, 1, Rf_mkChar("populations"));
    SET_STRING_ELT(out_names, 2, Rf_mkChar("fleets"));
    SET_STRING_ELT(out_names, 3, Rf_mkChar("density_components"));
    SET_STRING_ELT(out_names, 4, Rf_mkChar("data_objects"));

    INTEGER(out)
    [0] = static_cast<int>(info->models_map.size());
    INTEGER(out)
    [1] = static_cast<int>(info->populations.size());
    INTEGER(out)
    [2] = static_cast<int>(info->fleets.size());
    INTEGER(out)
    [3] = static_cast<int>(info->density_components.size());
    INTEGER(out)
    [4] = static_cast<int>(info->data_objects.size());

    Rf_setAttrib(out, R_NamesSymbol, out_names);

    UNPROTECT(2);
    return out;
}

extern "C" SEXP fims_call_information_clear()
{
    std::shared_ptr<fims_info::Information<double>> info_double =
        fims_info::Information<double>::GetInstance();
    info_double->Clear();
    reset_backend_id_counters<double>();

#ifdef TMB_MODEL
    std::shared_ptr<fims_info::Information<TMBAD_FIMS_TYPE>> info_ad =
        fims_info::Information<TMBAD_FIMS_TYPE>::GetInstance();
    info_ad->Clear();
    reset_backend_id_counters<TMBAD_FIMS_TYPE>();
#endif

    return R_NilValue;
}

extern "C" SEXP fims_call_information_get_fixed()
{
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    const size_t n = info->fixed_effects_parameters.size();
    SEXP out = PROTECT(Rf_allocVector(REALSXP, static_cast<R_xlen_t>(n)));
    for (size_t i = 0; i < n; ++i)
    {
        REAL(out)
        [i] = *(info->fixed_effects_parameters[i]);
    }

    UNPROTECT(1);
    return out;
}

extern "C" SEXP fims_call_information_get_random()
{
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    const size_t n = info->random_effects_parameters.size();
    SEXP out = PROTECT(Rf_allocVector(REALSXP, static_cast<R_xlen_t>(n)));
    for (size_t i = 0; i < n; ++i)
    {
        REAL(out)
        [i] = *(info->random_effects_parameters[i]);
    }

    UNPROTECT(1);
    return out;
}

extern "C" SEXP fims_call_information_get_parameter_names()
{
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    const size_t n = info->parameter_names.size();
    SEXP out = PROTECT(Rf_allocVector(STRSXP, static_cast<R_xlen_t>(n)));
    for (size_t i = 0; i < n; ++i)
    {
        SET_STRING_ELT(out, i, Rf_mkChar(info->parameter_names[i].c_str()));
    }

    UNPROTECT(1);
    return out;
}