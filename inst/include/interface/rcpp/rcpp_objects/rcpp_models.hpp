#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MODELS_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MODELS_HPP

#include <set>
#include "../../../common/def.hpp"
#include "../../../models/fisheries_models.hpp"
#include "../../../utilities/fims_json.hpp"
#include "rcpp_population.hpp"

#include "rcpp_interface_base.hpp"
#include <valarray>
#include <cmath>
#include <mutex>

class FisheryModelInterfaceBase : public FIMSRcppInterfaceBase
{
public:
    /**
     * @brief The static id of the FleetInterfaceBase object.
     */
    static uint32_t id_g;
    /**
     * @brief The local id of the FleetInterfaceBase object.
     */
    uint32_t id;
    /**
     * @brief The map associating the IDs of FleetInterfaceBase to the objects.
     * This is a live object, which is an object that has been created and lives
     * in memory.
     */
    static std::map<uint32_t, std::shared_ptr<FisheryModelInterfaceBase>> live_objects;

    /**
     * @brief The constructor.
     */
    FisheryModelInterfaceBase()
    {
        this->id = FisheryModelInterfaceBase::id_g++;
        /* Create instance of map: key is id and value is pointer to
        FleetInterfaceBase */
        // FisheryModelInterfaceBase::live_objects[this->id] = this;
    }

    /**
     * @brief Construct a new Data Interface Base object
     *
     * @param other
     */
    FisheryModelInterfaceBase(const FisheryModelInterfaceBase &other) : id(other.id)
    {
    }

    /**
     * @brief The destructor.
     */
    virtual ~FisheryModelInterfaceBase()
    {
    }

    virtual std::string to_json()
    {
        return "std::string to_json() not yet implemented.";
    }

    virtual Rcpp::List calculate_reference_points()
    {
        Rcpp::List result;
        return result;
    }

    /**
     * @brief Get the ID for the child fleet interface objects to inherit.
     */
    virtual uint32_t get_id() = 0;
};
// static id of the FleetInterfaceBase object
uint32_t FisheryModelInterfaceBase::id_g = 1;
// local id of the FleetInterfaceBase object map relating the ID of the
// FleetInterfaceBase to the FleetInterfaceBase objects
std::map<uint32_t, std::shared_ptr<FisheryModelInterfaceBase>> FisheryModelInterfaceBase::live_objects;

/**
 * @brief The CatchAtAgeInterface class is used to interface with the
 * CatchAtAge model. It inherits from the FisheryModelInterfaceBase class.
 */
class CatchAtAgeInterface : public FisheryModelInterfaceBase
{
    std::shared_ptr<std::set<uint32_t>> population_ids;
    typedef typename std::set<uint32_t>::iterator population_id_iterator;
    std::map<uint32_t, uint32_t> age_comp_density_components_links;               // fleet id, density component id
    std::map<uint32_t, uint32_t> index_density_components_links;                  // fleet id, density component id
    std::map<uint32_t, uint32_t> length_comp_density_components_links;            // fleet id, density component id
    std::map<uint32_t, uint32_t> recruitment_density_components_links;            // population id, density component id
    std::map<uint32_t, uint32_t> initial_numbers_at_age_density_components_links; // population id, density component id
    typedef typename std::map<uint32_t, uint32_t>::iterator density_component_iterator;

    static void build_R_enum()
    {
        Rcpp::Environment global_env = Rcpp::Environment::global_env();
        global_env["CatchAtAge::density_stuff"] = CatchAtAgeInterface::density_expected;
    }

public:
    static std::once_flag density_expected_flag;
    static Rcpp::List density_expected;
    /**
     * @brief The constructor.
     */
    CatchAtAgeInterface() : FisheryModelInterfaceBase()
    {
        this->population_ids = std::make_shared<std::set<uint32_t>>();
        std::shared_ptr<CatchAtAgeInterface> caa = std::make_shared<CatchAtAgeInterface>(*this);
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(caa);
        FisheryModelInterfaceBase::live_objects[this->id] = caa;

        std::call_once(density_expected_flag, CatchAtAgeInterface::build_R_enum);
    }

    /**
     * @brief Construct a new Catch At Age Interface object
     *
     * @param other
     */
    CatchAtAgeInterface(const CatchAtAgeInterface &other) : FisheryModelInterfaceBase(other),
                                                            population_ids(other.population_ids)
    {
    }

    /**
     * Metthod to add a population id to the set of population ids.
     */
    void AddPopulation(uint32_t id)
    {
        this->population_ids->insert(id);

        std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
        pit = PopulationInterfaceBase::live_objects.find(id);
        if (pit != PopulationInterfaceBase::live_objects.end())
        {
            std::shared_ptr<PopulationInterfaceBase> &pop = (*pit).second;
            pop->initialize_catch_at_age.set(true);
        }
        else
        {
            FIMS_ERROR_LOG("Population with id " + fims::to_string(id) + " not found.");
        }
    }

    /**
     * @brief Method to add an age composition density component.
     */
    void AddFleetDensityComponent(uint32_t fleet_id, uint32_t data, uint32_t density_component_id, std::string input_type = "constant")
    {
        this->age_comp_density_components_links[fleet_id] = density_component_id;
    }

    /**
     * @brief Method to get the population id.
     */
    virtual uint32_t get_id()
    {
        typename std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
        return this->id;
    }

    void Show()
    {
        std::shared_ptr<fims_info::Information<double>> info =
            fims_info::Information<double>::GetInstance();

        fims_popdy::CatchAtAge<double> *model = (fims_popdy::CatchAtAge<double> *)info->models_map[this->get_id()].get();
        model->Show();
        // std::cout << this->to_json(); // fims::JsonParser::PrettyFormatJSON(model->ToJSON());

        // std::ofstream o("test.json");
        // o << this->to_json();
        // o.close();
    }

    /**
     *
     */
    virtual void finalize()
    {
    }

    /**
     * @brief Method to convert a population to a JSON string.
     */
    std::string population_to_json(PopulationInterface *population_interface)
    {
        std::stringstream ss;

        std::shared_ptr<fims_info::Information<double>> info =
            fims_info::Information<double>::GetInstance();

        typename fims_info::Information<double>::population_iterator pit;

        pit = info->populations.find(population_interface->get_id());

        if (pit != info->populations.end())
        {
            std::shared_ptr<fims_popdy::Population<double>> &pop = (*pit).second;

            fims::Vector<double> &derived_ssb = pop->derived_quantities["spawning_biomass"];
            fims::Vector<double> &derived_naa = pop->derived_quantities["numbers_at_age"];
            fims::Vector<double> &derived_biomass = pop->derived_quantities["biomass"];
            fims::Vector<double> &derived_recruitment = pop->derived_quantities["recruitment"];

            // ToDo: add list of fleet ids operating on this population
            ss << "{\n";
            ss << " \"name\" : \"Population\",\n";

            ss << " \"type\" : \"population\",\n";
            ss << " \"tag\" : \"" << population_interface->name << "\",\n";
            ss << " \"id\": " << population_interface->id << ",\n";
            ss << " \"recruitment_id\": " << population_interface->recruitment_id << ",\n";
            ss << " \"growth_id\": " << population_interface->growth_id << ",\n";
            ss << " \"maturity_id\": " << population_interface->maturity_id << ",\n";

            ss << " \"parameters\": [\n{\n";
            ss << " \"name\": \"log_M\",\n";
            ss << " \"id\":" << population_interface->log_M.id_m << ",\n";
            ss << " \"type\": \"vector\",\n";
            ss << " \"values\": " << population_interface->log_M << "\n},\n";

            ss << "{\n";
            ss << "  \"name\": \"log_init_naa\",\n";
            ss << "  \"id\":" << population_interface->log_init_naa.id_m << ",\n";
            ss << "  \"type\": \"vector\",\n";
            ss << "  \"values\":" << population_interface->log_init_naa << " \n}],\n";

            ss << " \"derived_quantities\": [{\n";
            ss << "  \"name\": \"ssb\",\n";
            ss << " \"dimensions\" : [" << this->make_dimensions(1, population_interface->nyears + 1) << "],";
            ss << "  \"values\":[";
            if (derived_ssb.size() == 0)
            {
                ss << "]\n";
            }
            else
            {
                for (size_t i = 0; i < derived_ssb.size() - 1; i++)
                {
                    ss << derived_ssb[i] << ", ";
                }
                ss << derived_ssb[derived_ssb.size() - 1] << "]\n";
            }
            ss << ",stdev: [" << fims::Vector<double>(derived_ssb.size(), -999) << "]\n";
            ss << " },\n";

            ss << "{\n";
            ss << "   \"name\": \"naa\",\n";
            ss << " \"dimensions\" : [[" << this->make_dimensions(1, population_interface->nyears) << "],[" << this->make_dimensions(population_interface->ages[0], population_interface->ages[population_interface->ages.size() - 1], population_interface->nyears + 1) << "]],";
            ss << "   \"values\":[";
            if (derived_naa.size() == 0)
            {
                ss << "]\n";
            }
            else
            {
                for (size_t i = 0; i < derived_naa.size() - 1; i++)
                {
                    ss << derived_naa[i] << ", ";
                }
                ss << derived_naa[derived_naa.size() - 1] << "]\n";
            }
            ss << ",stdev: [" << fims::Vector<double>(derived_naa.size(), -999) << "]\n";
            ss << " },\n";

            ss << "{\n";
            ss << "   \"name\": \"biomass\",\n";
            ss << " \"dimensions\" : [" << population_interface->make_dimensions(1, population_interface->nyears + 1) << "],";
            ss << "   \"values\":[";
            if (derived_biomass.size() == 0)
            {
                ss << "]\n";
            }
            else
            {
                for (size_t i = 0; i < derived_biomass.size() - 1; i++)
                {
                    ss << derived_biomass[i] << ", ";
                }
                ss << derived_biomass[derived_biomass.size() - 1] << "]\n";
            }
            ss << ",stdev: [" << fims::Vector<double>(derived_biomass.size(), -999) << "]\n";
            ss << " },\n";

            ss << "{\n";
            ss << "   \"name\": \"recruitment\",\n";
            ss << " \"dimensions\" : [" << this->make_dimensions(1, population_interface->nyears + 1) << "],";
            ss << "   \"values\":[";
            if (derived_recruitment.size() == 0)
            {
                ss << "]\n";
            }
            else
            {
                for (size_t i = 0; i < derived_recruitment.size() - 1; i++)
                {
                    ss << derived_recruitment[i] << ", ";
                }
                ss << derived_recruitment[derived_recruitment.size() - 1] << "]\n";
            }
            ss << ",stdev: [" << fims::Vector<double>(derived_recruitment.size(), -999) << "]\n";
            ss << " }\n]\n";

            ss << "}";
        }
        else
        {
            ss << "{\n";
            ss << " \"name\" : \"Population\",\n";

            ss << " \"type\" : \"population\",\n";
            ss << " \"tag\" : \"" << population_interface->get_id() << " not found in Information.\",\n}";

#warning Add error log here
        }
        return ss.str();
    }

    /**
     * @brief Method to convert a fleet to a JSON string.
     */
    std::string fleets_to_json(FleetInterface *fleet_interface)
    {
        std::stringstream ss;

        std::shared_ptr<fims_info::Information<double>> info =
            fims_info::Information<double>::GetInstance();

        typename fims_info::Information<double>::fleet_iterator fit;

        fit = info->fleets.find(fleet_interface->get_id());

        if (fit != info->fleets.end())
        {

            std::shared_ptr<fims_popdy::Fleet<double>> &fleet = (*fit).second;

            fims::Vector<double> &derived_caa = fleet->derived_quantities["catch_numbers_at_age"];
            fims::Vector<double> &derived_cal = fleet->derived_quantities["catch_numbers_at_length"];
            fims::Vector<double> &derived_proportion_cnaa = fleet->derived_quantities["proportion_catch_numbers_at_age"];
            fims::Vector<double> &derived_proportion_cnal = fleet->derived_quantities["proportion_catch_numbers_at_length"];
            fims::Vector<double> &derived_index = fleet->derived_quantities["expected_index"];
            fims::Vector<double> &derived_catch = fleet->derived_quantities["expected_catch"];
            fims::Vector<double> &derived_cwaa = fleet->derived_quantities["catch_weight_at_age"];
            fims::Vector<double> &derived_age_comp = fleet->derived_quantities["age_composition"];
            fims::Vector<double> &derived_length_comp = fleet->derived_quantities["length_composition"];

            ss << "{\n";
            ss << " \"name\" : \"Fleet\",\n";

            ss << " \"type\" : \"fleet\",\n";
            ss << " \"tag\" : \"" << fleet_interface->name << "\",\n";
            ss << " \"id\": " << fleet_interface->id << ",\n";
            // ss << " \"is_survey\": " << fleet_interface->is_survey << ",\n";
            ss << " \"nlengths\": " << fleet_interface->nlengths.get() << ",\n";
            ss << "\"parameters\": [\n";
            ss << "{\n";
            ss << " \"name\": \"log_Fmort\",\n";
            ss << " \"id\":" << fleet_interface->log_Fmort.id_m << ",\n";
            ss << " \"type\": \"vector\",\n";
            ss << " \"values\": " << fleet_interface->log_Fmort << "\n},\n";

            ss << " {\n";
            ss << " \"name\": \"log_q\",\n";
            ss << " \"id\":" << fleet_interface->log_q.id_m << ",\n";
            ss << " \"type\": \"vector\",\n";
            ss << " \"values\": " << fleet_interface->log_q << "\n},\n";
            if (fleet_interface->nlengths > 0)
            {
                ss << " {\n";
                ss << " \"name\": \"age_length_conversion_matrix\",\n";
                // ss << " \"id\":" << fleet_interface->age_length_conversion_matrix.id_m << ",\n";
                ss << " \"type\": \"vector\",\n";
                // ss << " \"values\": " << fleet_interface->age_length_conversion_matrix << "\n}\n";
            }
            ss << "], \"derived_quantities\": [{\n";
            ss << "  \"name\": \"cnaa\",\n";
            ss << " \"dimensions\" : [" << this->make_dimensions(1, fleet->nyears + 1) << "],";
            ss << "  \"values\":[";
            if (derived_caa.size() == 0)
            {
                ss << "]\n";
            }
            else
            {
                for (size_t i = 0; i < derived_caa.size() - 1; i++)
                {
                    ss << derived_caa[i] << ", ";
                }
                ss << derived_caa[derived_caa.size() - 1] << "]\n";
            }
            ss << ",stdev: [" << fims::Vector<double>(derived_caa.size(), -999) << "]\n";
            ss << " },\n";
            ss << " {\n";
            ss << "  \"name\": \"cnal\",\n";
            ss << "  \"values\":[";
            if (derived_cal.size() == 0)
            {
                ss << "]\n";
            }
            else
            {
                for (size_t i = 0; i < derived_cal.size() - 1; i++)
                {
                    ss << derived_cal[i] << ", ";
                }
                ss << derived_cal[derived_cal.size() - 1] << "]\n";
            }
            ss << " },\n";
            ss << " {\n";
            ss << "  \"name\": \"cwaa\",\n";
            ss << "  \"values\":[";
            if (derived_cwaa.size() == 0)
            {
                ss << "]\n";
            }
            else
            {
                for (size_t i = 0; i < derived_cwaa.size() - 1; i++)
                {
                    ss << derived_cwaa[i] << ", ";
                }
                ss << derived_cwaa[derived_cwaa.size() - 1] << "]\n";
            }
            ss << ",stdev: [" << fims::Vector<double>(derived_cwaa.size(), -999) << "]\n";
            ss << " },\n";
            ss << " {\n";
            ss << "  \"name\": \"proportion_catch_numbers_at_age\",\n";
            ss << "  \"values\":[";
            if (derived_proportion_cnaa.size() == 0)
            {
                ss << "]\n";
            }
            else
            {
                for (size_t i = 0; i < derived_proportion_cnaa.size() - 1; i++)
                {
                    ss << derived_proportion_cnaa[i] << ", ";
                }
                ss << derived_proportion_cnaa[derived_proportion_cnaa.size() - 1] << "]\n";
            }
            ss << ",stdev: [" << fims::Vector<double>(derived_proportion_cnaa.size(), -999) << "]\n";
            ss << " },\n";
            ss << " {\n";
            ss << "  \"name\": \"proportion_catch_numbers_at_length\",\n";
            ss << "  \"values\":[";
            if (derived_proportion_cnal.size() == 0)
            {
                ss << "]\n";
                ss << ",stdev: []\n";
            }
            else
            {
                for (size_t i = 0; i < derived_proportion_cnal.size() - 1; i++)
                {
                    ss << derived_proportion_cnal[i] << ", ";
                }
                ss << derived_proportion_cnal[derived_proportion_cnal.size() - 1] << "]\n";
                ss << ",stdev: [" << fims::Vector<double>(derived_proportion_cnal.size(), -999) << "]\n";
            }

            ss << " }\n";
            // ss << " {\n";
            // ss << "  \"name\": \"expected_index\",\n";
            // ss << "  \"values\":[";
            // if (derived_index.size() == 0)
            // {
            //     ss << "]\n";
            // }
            // else
            // {
            //     for (size_t i = 0; i < derived_index.size() - 1; i++)
            //     {
            //         ss << derived_index[i] << ", ";
            //     }
            //     ss << derived_index[derived_index.size() - 1] << "]\n";
            // }
            // ss << " }\n";
            // ss << " {\n";
            // ss << "  \"name\": \"expected_catch\",\n";
            // ss << "  \"values\":[";
            // if (derived_catch.size() == 0)
            // {
            //     ss << "]\n";
            // }
            // else
            // {
            //     for (size_t i = 0; i < derived_catch.size() - 1; i++)
            //     {
            //         ss << derived_catch[i] << ", ";
            //     }
            //     ss << derived_catch[derived_catch.size() - 1] << "]\n";
            // }
            // ss << " },\n";
            // ss << " {\n";
            // ss << "  \"name\": \"age_composition \",\n";
            // ss << "  \"values\":[";
            // if (derived_age_comp.size() == 0)
            // {
            //     ss << "]\n";
            // }
            // else
            // {
            //     for (size_t i = 0; i < derived_age_comp.size() - 1; i++)
            //     {
            //         ss << derived_age_comp[i] << ", ";
            //     }
            //     ss << derived_age_comp[derived_age_comp.size() - 1] << "]\n";
            // }
            // ss << " },\n";
            // ss << " {\n";
            // ss << "  \"name\": \"length_composition \",\n";
            // ss << "  \"values\":[";
            // if (derived_length_comp.size() == 0)
            // {
            //     ss << "]\n";
            // }
            // else
            // {
            //     for (size_t i = 0; i < derived_length_comp.size() - 1; i++)
            //     {
            //         ss << derived_length_comp[i] << ", ";
            //     }
            //     ss << derived_length_comp[derived_length_comp.size() - 1] << "]\n";
            // }
            // ss << " }\n]\n";
            ss << "]}";
        }
        else
        {
            ss << "{\n";
            ss << " \"name\" : \"Fleet\",\n";

            ss << " \"type\" : \"fleet\",\n";
            ss << " \"tag\" : \"" << fleet_interface->get_id() << " not found in Information.\",\n}";
        }
        return ss.str();
    }

    /**
     * @brief Method to convert the model to a JSON string.
     */
    virtual std::string to_json()
    {
        this->Show();
        typename std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
        std::vector<uint32_t> pop_ids(this->population_ids->begin(), this->population_ids->end());

        std::stringstream ss;
        std::set<uint32_t> fleet_ids; // all fleets in the model
        ss << "{\n";
        ss << "\"model\" : \"catch_at_age\",\n";
        ss << "\"id\" : " << this->get_id() << ",\n";
        ss << "\"populations\" : [\n";
        // loop through populations for this model
        std::vector<std::string> pop_strings;
        for (size_t p = 0; p < pop_ids.size(); p++)
        {

            pit = PopulationInterfaceBase::live_objects.find(pop_ids[p]);
            if (pit != PopulationInterfaceBase::live_objects.end())
            {
                PopulationInterface *pop = (PopulationInterface *)(*pit).second.get();
                fleet_ids.insert(pop->fleet_ids->begin(), pop->fleet_ids->end());
                pop_strings.push_back(this->population_to_json(pop));
            }
        }
        if (pop_strings.size() > 0)
        {
            for (size_t i = 0; i < pop_strings.size() - 1; i++)
            {
                ss << pop_strings[i] << ",\n";
            }
            ss << pop_strings[pop_strings.size() - 1] << "\n";
        }
        ss << "],\n";
        ss << "\"fleets\" : [\n";
        typename std::map<uint32_t, std::shared_ptr<FleetInterfaceBase>>::iterator fit;
        // all fleets encapuslated in this model run
        std::vector<uint32_t> fids(fleet_ids.begin(), fleet_ids.end());
        // // loop through fleets for this model
        for (size_t f = 0; f < fids.size(); f++)
        {
            fit = FleetInterfaceBase::live_objects.find(fids[f]);
            if (fit != FleetInterfaceBase::live_objects.end())
            {
                if (f == fids.size() - 1)
                {
                    ss << this->fleets_to_json((FleetInterface *)(*fit).second.get()) << "\n";
                }
                else
                {
                    ss << this->fleets_to_json((FleetInterface *)(*fit).second.get()) << ",\n";
                }
            }
        }

        ss << "]\n}";

        return fims::JsonParser::PrettyFormatJSON(ss.str());
    }

    double sum(const std::valarray<double> &v)
    {
        double sum = 0.0;
        for (size_t i = 0; i < v.size(); i++)
        {
            sum += v[i];
        }
        return sum;
    }
    double sum(const std::vector<double> &v)
    {
        double sum = 0.0;
        for (size_t i = 0; i < v.size(); i++)
        {
            sum += v[i];
        }
        return sum;
    }

    double min(const std::valarray<double> &v)
    {
        double min = v[0];
        for (size_t i = 1; i < v.size(); i++)
        {
            if (v[i] < min)
            {
                min = v[i];
            }
        }
        return min;
    }
    std::valarray<double> fabs(const std::valarray<double> &v)
    {
        std::valarray<double> result(v.size());
        for (size_t i = 0; i < v.size(); i++)
        {
            result[i] = std::fabs(v[i]);
        }
        return result;
    }

    /**
     * @brief Method to calculate reference points for a population.
     * @param population_interface
     * @param maxF The maximum fishing mortality to calculate reference points for.
     * @param step The step size for the fishing mortality.
     * @return A list of reference points for the population.
     */
    Rcpp::List calculate_reference_points_population(PopulationInterface *population_interface,
                                                     double maxF = 1.0, double step = 0.01)
    {
        //         //note: this algoritm is ported from the Meta-population assessment system project and
        //         //needs review

        Rcpp::List result;
        //         double spawning_season_offset = 0.0;
        //         std::shared_ptr<fims_info::Information<double>> info =
        //             fims_info::Information<double>::GetInstance();

        //         typename fims_info::Information<double>::population_iterator pit;

        //         pit = info->populations.find(population_interface->get_id());

        //         if (pit != info->populations.end())
        //         {
        //             std::shared_ptr<fims_popdy::Population<double>> &pop = (*pit).second;

        //             size_t year = pop->nyears - 1;
        //             size_t season = pop->nseasons - 1;
        //             size_t nages = pop->ages.size();

        //             std::vector<double> F;
        //             for (double f = 0.0; f <= maxF; f += step)
        //             {
        //                 F.push_back(f);
        //             }

        //             std::valarray<double> spr(F.size());       // equilibrium spr at F
        //             std::valarray<double> spr_ratio(F.size()); // equilibrium spr at F
        //             std::vector<double> S_eq(F.size());        // equilibrium SSB at F
        //             std::vector<double> R_eq(F.size());        // equilibrium recruitment at F
        //             std::vector<double> B_eq(F.size());        // equilibrium biomass at F
        //             std::vector<double> L_eq(F.size());        // equilibrium landings at F
        //             std::vector<double> D_eq(F.size());        // equilibrium dead discards at F
        //             std::vector<double> E_eq(F.size());        // equilibrium exploitation rate at F (landings only)
        //             std::valarray<double> L_eq_knum(F.size());
        //             std::valarray<double> SSB_eq(F.size());
        //             double spr_F0 = 0.0;

        //             std::vector<double> N0(pop->ages.size(), 1.0);
        //             for (int iage = 1; iage < nages; iage++)
        //             {
        //                 N0[iage] = N0[iage - 1] * std::exp(-1.0 * pop->M[iage - 1]);
        //             }
        //             N0[nages - 1] = N0[nages - 2] * std::exp(-1.0 * pop->M[nages - 2]) / (1.0 - std::exp(-1.0 * pop->M[nages - 1]));

        //             std::valarray<double> reprod(nages);
        //             std::valarray<double> selL(nages);
        //             std::valarray<double> selZ(nages);
        //             std::valarray<double> selD(nages);
        //             std::valarray<double> M_age(nages);
        //             std::valarray<double> wgt(nages);

        //             for (int a = 0; a < pop->ages.size(); a++)
        //             {
        //                 // dimension folded index
        //                 size_t index = year * pop->ages.size() + a;

        //                 // is this ssb_unfished?
        //                 reprod[a] = pop->derived_quantities["weight_at_age"][index] * (pop->derived_quantities["proportion_mature_at_age"][index] * pop->proportion_female[0]);
        //                 spr_F0 += N0[a] * reprod[a];
        //                 selL[a] = pop->derived_quantities["sum_selectivity"][index];
        //                 selZ[a] = pop->derived_quantities["sum_selectivity"][index];
        //                 M_age[a] = pop->M[a];
        //                 wgt[a] = pop->derived_quantities["weight_at_age"][a];
        //             }

        //             std::valarray<double> L_age(nages); // #landings at age
        //             std::valarray<double> D_age(nages); // #dead discards at age
        //             std::valarray<double> F_age(nages); // #F at age
        //             std::valarray<double> Z_age(nages); // #Z at age

        //             // BEGIN ALGORITHM
        //             for (int i = 0; i < F.size(); i++)
        //             {

        //                 std::valarray<double> FL_age = F[i] * selL;
        //                 // std::valarray<REAL_T> FD_age = F[i] * selD;
        //                 std::valarray<double> Z_age = M_age + F[i] * selZ;

        //                 std::valarray<double> N_age(nages);
        //                 std::valarray<double> N_age_spawn(nages);

        //                 N_age[0] = 1.0;

        //                 for (int iage = 1; iage < nages; iage++)
        //                 {
        //                     N_age[iage] = N_age[iage - 1] * std::exp(-1.0 * Z_age[iage - 1]);
        //                 }

        //                 // last age is pooled
        //                 N_age[nages - 1] = N_age[nages - 2] * std::exp(-1.0 * Z_age[nages - 2]) /
        //                                    (1.0 - std::exp(-1.0 * Z_age[nages - 1]));

        //                 N_age_spawn = (N_age *
        //                                std::exp((-1.0 * Z_age * spawning_season_offset)));

        //                 N_age_spawn[nages - 1] = (N_age_spawn[nages - 2] * (std::exp(-1. * (Z_age[nages - 2] * (1.0 - spawning_season_offset) +
        //                                                                                     Z_age[nages - 1] * spawning_season_offset)))) /
        //                                          (1.0 - std::exp(-1. * Z_age[nages - 1]));

        //                 spr[i] = sum(N_age * reprod);
        // #warning This is propbably not correct
        //                 R_eq[i] = pop->recruitment->evaluate(spr[i], spr_F0);
        //                 // R_eq[i] = (R0 / ((5.0 * steep - 1.0) * spr[i])) *
        //                 //           (BC * 4.0 * steep * spr[i] - spr_F0 * (1.0 - steep));
        //                 // R_eq[i] = this->recruitment_model->CalculateEquilibriumRecruitment(
        //                 //     this->recruitment_model->CalculateEquilibriumSpawningBiomass(spr[i])); //*1000*this->sex_fraction_value;

        //                 if (R_eq[i] < 0.0000001)
        //                 {
        //                     R_eq[i] = 0.0000001;
        //                 }

        //                 N_age *= R_eq[i];
        //                 N_age_spawn *= R_eq[i];

        //                 S_eq[i] = sum(N_age * reprod);
        //                 B_eq[i] = sum(N_age * wgt);

        //                 for (int iage = 0; iage < nages; iage++)
        //                 {
        //                     L_age[iage] = N_age[iage] *
        //                                   (FL_age[iage] / Z_age[iage]) * (1. - std::exp(-1.0 * Z_age[iage]));
        //                     // D_age[iage] = N_age[iage] *
        //                     //               (FD_age[iage] / Z_age[iage]) * (1. - exp(-1.0 * Z_age[iage]))
        //                 }
        //                 SSB_eq[i] = sum((N_age_spawn * reprod));
        //                 L_eq[i] = sum(L_age * wgt);
        //                 E_eq[i] = sum(L_age) / sum(N_age);
        //                 L_eq_knum[i] = (sum(L_age) / 1000.0);
        //             }

        //             int max_index = 0;
        //             double max = std::numeric_limits<double>::min();
        //             spr_ratio = spr / spr_F0;
        //             double F01_dum = min(fabs(spr_ratio - 0.001));
        //             double F30_dum = min(fabs(spr_ratio - 0.3));
        //             double F35_dum = min(fabs(spr_ratio - 0.35));
        //             double F40_dum = min(fabs(spr_ratio - 0.4));
        //             size_t F01_out;
        //             size_t F30_out = 0;
        //             size_t F35_out = 0;
        //             size_t F40_out = 0;

        //             for (int i = 0; i < L_eq.size(); i++)
        //             {

        //                 if (L_eq[i] >= max)
        //                 {
        //                     max = L_eq[i];
        //                     max_index = i;
        //                 }

        //                 //                if (std::fabs(spr_ratio[i] - 0.001) == F01_dum) {
        //                 //                    F01_out = F[i];
        //                 //                }

        //                 if (std::fabs(spr_ratio[i] - 0.3) == F30_dum)
        //                 {
        //                     F30_out = i;
        //                 }
        //                 if (std::fabs(spr_ratio[i] - 0.35) == F35_dum)
        //                 {
        //                     F35_out = i;
        //                 }
        //                 if (std::fabs(spr_ratio[i] - 0.4) == F40_dum)
        //                 {
        //                     F40_out = i;
        //                 }
        //             }
        //             double msy_mt_out = max; // msy in whole weight
        //             double SSB_msy_out;
        //             double B_msy_out;
        //             double R_msy_out;
        //             double msy_knum_out;
        //             double F_msy_out;
        //             double spr_msy_out;
        //             int index_m = 0;
        //             for (int i = 0; i < F.size(); i++)
        //             {
        //                 if (L_eq[i] == msy_mt_out)
        //                 {

        //                     SSB_msy_out = SSB_eq[i];
        //                     B_msy_out = B_eq[i] * pop->proportion_female[0];
        //                     R_msy_out = R_eq[i] * 1000.0 * pop->proportion_female[0];
        //                     msy_knum_out = L_eq_knum[i];
        //                     F_msy_out = F[i];
        //                     spr_msy_out = spr[i];
        //                     index_m = i;
        //                 }
        //             }

        //             std::cout << std::fixed;
        //             //
        //             std::cout << "\n\nFmax: " << maxF << "\n";
        //             std::cout << "Step: " << step << "\n";
        //             std::cout << "\n\nF_msy: " << F[max_index] << "\n";
        //             std::cout << "spr_30: " << spr_ratio[F30_out] << "\n";
        //             std::cout << "spr_35: " << spr_ratio[F35_out] << "\n";
        //             std::cout << "spr_40: " << spr_ratio[F40_out] << "\n";
        //             std::cout << "F30: " << F[F30_out] << "\n";
        //             std::cout << "F35: " << F[F35_out] << "\n";
        //             std::cout << "F40: " << F[F40_out] << "\n";
        //             spr_msy_out = spr[max_index];
        //             std::cout << "msy: " << F_msy_out * pop->proportion_female[0] << "\n";
        //             std::cout << "spr_msy: " << spr[max_index] << "\n";
        //             std::cout << "SR_msy: " << spr_msy_out / spr_F0 << "\n";
        //             //                        std::cout << "D_msy_out" << D_eq[max_index] << "\n";
        //             std::cout << "R_msy: " << R_eq[max_index] << "\n";
        //             std::cout << "SSB_msy: " << SSB_msy_out << "\n";
        //             std::cout << "B_msy: " << B_msy_out << "\n";
        //             std::cout << "E_msy: " << E_eq[max_index] << "\n";
        //         }

        return result;
    }

    /**
     * @brief Method to calculate reference points for the model.
     */
    virtual Rcpp::List calculate_reference_points()
    {
        Rcpp::List result;
        // loop through populations for this model
        std::vector<uint32_t> pop_ids(this->population_ids->begin(), this->population_ids->end());
        for (size_t p = 0; p < pop_ids.size(); p++)
        {
            typename std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
            pit = PopulationInterfaceBase::live_objects.find(pop_ids[p]);
            if (pit != PopulationInterfaceBase::live_objects.end())
            {
                PopulationInterface *pop = (PopulationInterface *)(*pit).second.get();
                result.push_back(this->calculate_reference_points_population(pop));
            }
        }
        return result;
    }

#ifdef TMB_MODEL

    template <typename Type>
    bool add_to_fims_tmb_internal()
    {
        std::shared_ptr<fims_info::Information<Type>> info =
            fims_info::Information<Type>::GetInstance();

        std::shared_ptr<fims_popdy::CatchAtAge<Type>> model = std::make_shared<fims_popdy::CatchAtAge<Type>>();

        population_id_iterator it;

        for (it = this->population_ids->begin(); it != this->population_ids->end(); ++it)
        {
            model->AddPopulation((*it));
        }

        // add to Information
        info->models_map[this->get_id()] = model;

        return true;
    }

    virtual bool add_to_fims_tmb()
    {
        FIMS_INFO_LOG("adding CAA model object to TMB");
        this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
        this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

        return true;
    }

#endif
};
std::once_flag CatchAtAgeInterface::density_expected_flag;

Rcpp::List CatchAtAgeInterface::density_expected = Rcpp::List::create(
    Rcpp::Named("density_expected") = Rcpp::NumericVector::create(0L),
    Rcpp::Named("density_expected_stdev") = Rcpp::NumericVector::create(1L));
#endif
