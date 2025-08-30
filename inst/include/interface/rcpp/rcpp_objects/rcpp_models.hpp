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

/**
 * @brief The FisheryModelInterfaceBase class is the base class for all fishery
 * models in the FIMS Rcpp interface. It inherits from the
 * FIMSRcppInterfaceBase.
 *
 */
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
  static std::map<uint32_t, std::shared_ptr<FisheryModelInterfaceBase>>
      live_objects;

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
  FisheryModelInterfaceBase(const FisheryModelInterfaceBase &other)
      : id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~FisheryModelInterfaceBase() {}

  virtual std::string to_json()
  {
    return "std::string to_json() not yet implemented.";
  }

  /**
   * @brief A function to calculate reference points for the fishery model.
   *
   * @return Rcpp::List
   */
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
std::map<uint32_t, std::shared_ptr<FisheryModelInterfaceBase>>
    FisheryModelInterfaceBase::live_objects;

/**
 * @brief The CatchAtAgeInterface class is used to interface with the
 * CatchAtAge model. It inherits from the FisheryModelInterfaceBase class.
 */
class CatchAtAgeInterface : public FisheryModelInterfaceBase
{
  std::shared_ptr<std::set<uint32_t>> population_ids;
  typedef typename std::set<uint32_t>::iterator population_id_iterator;

  std::map<uint32_t, std::map<std::string, std::string>>
      fleet_derived_quantities_dim_strings;
  typedef typename std::map<uint32_t, std::map<std::string, std::string>>::iterator
      fleet_derived_quantities_dim_strings_iterator;

  std::map<uint32_t, std::map<std::string, std::string>>
      population_derived_quantities_dim_strings;
  typedef typename std::map<uint32_t, std::map<std::string, std::string>>::iterator
      population_derived_quantities_dim_strings_iterator;

public:
  /**
   * @brief The constructor.
   */
  CatchAtAgeInterface() : FisheryModelInterfaceBase()
  {
    this->population_ids = std::make_shared<std::set<uint32_t>>();
    std::shared_ptr<CatchAtAgeInterface> caa =
        std::make_shared<CatchAtAgeInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(caa);
    FisheryModelInterfaceBase::live_objects[this->id] = caa;
  }

  /**
   * @brief Construct a new Catch At Age Interface object
   *
   * @param other
   */
  CatchAtAgeInterface(const CatchAtAgeInterface &other)
      : FisheryModelInterfaceBase(other),
        population_ids(other.population_ids) {}

  /**
   * Method to add a population id to the set of population ids.
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
      FIMS_ERROR_LOG("Population with id " + fims::to_string(id) +
                     " not found.");
    }
  }

  /**
   * @brief Method to get the population id.
   */
  virtual uint32_t get_id()
  {
    typename std::map<uint32_t,
                      std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
    return this->id;
  }

  /**
   * @brief Method to get the population ids.
   */
  void Show()
  {
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_popdy::CatchAtAge<double> *model =
        (fims_popdy::CatchAtAge<double> *)info->models_map[this->get_id()]
            .get();
    model->Show();
  }

  /**
   *
   */
  virtual void finalize() {}

  /**
   * @brief Method to convert a population to a JSON string.
   */
  std::string population_to_json(PopulationInterface *population_interface)
  {
    std::stringstream ss;

    typename std::map<uint32_t,
                      std::shared_ptr<PopulationInterfaceBase>>::iterator
        pi_it; // population interface iterator
    pi_it = PopulationInterfaceBase::live_objects.find(
        population_interface->get_id());
    if (pi_it == PopulationInterfaceBase::live_objects.end())
    {
      FIMS_ERROR_LOG("Population with id " +
                     fims::to_string(population_interface->get_id()) +
                     " not found in live objects.");
      return "{}"; // Return empty JSON
    }

    std::shared_ptr<PopulationInterface> population_interface_ptr =
        std::dynamic_pointer_cast<PopulationInterface>((*pi_it).second);

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    typename fims_info::Information<double>::model_map_iterator model_it;
    model_it = info->models_map.find(this->get_id());
    std::shared_ptr<fims_popdy::CatchAtAge<double>> model_ptr =
        std::dynamic_pointer_cast<fims_popdy::CatchAtAge<double>>(
            (*model_it).second);

    typename fims_info::Information<double>::population_iterator pit;

    pit = info->populations.find(population_interface->get_id());

    if (pit != info->populations.end())
    {
      std::shared_ptr<fims_popdy::Population<double>> &pop = (*pit).second;
      // ToDo: add list of fleet ids operating on this population
      ss << "{\n";
      ss << " \"name\" : \"Population\",\n";

      ss << " \"type\" : \"population\",\n";
      ss << " \"tag\" : \"" << population_interface->name << "\",\n";
      ss << " \"id\": " << population_interface->id << ",\n";
      ss << " \"recruitment_id\": " << population_interface->recruitment_id
         << ",\n";
      ss << " \"growth_id\": " << population_interface->growth_id << ",\n";
      ss << " \"maturity_id\": " << population_interface->maturity_id << ",\n";

      ss << " \"parameters\": [\n";
      for (size_t i = 0; i < pop->log_M.size(); i++)
      {
        population_interface_ptr->log_M[i].final_value_m = pop->log_M[i];
      }

      ss << "{\n \"name\": \"log_M\",\n";
      ss << " \"id\":" << population_interface->log_M.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"values\": " << population_interface->log_M << "\n,\n";
      ss << " \"uncertainty\": {\n";
      // ss << "\"tmb\" : " << fims::Vector<double>(5, -999) << ",\n";
      // ss << "\"stan\" : " << fims::Vector<double>(5, -999) << ",\n";
      ss << "\"tmb\" : " << fims::Vector<double>(5, -999) << "\n";
      ss << "}},\n";

      for (size_t i = 0; i < pop->log_init_naa.size(); i++)
      {
        population_interface_ptr->log_init_naa[i].final_value_m =
            pop->log_init_naa[i];
      }
      ss << " {\n\"name\": \"log_init_naa\",\n";
      ss << "  \"id\":" << population_interface->log_init_naa.id_m << ",\n";
      ss << "  \"type\": \"vector\",\n";
      ss << "  \"values\":" << population_interface->log_init_naa << ",\n";
      ss << " \"uncertainty\": {\n";
      // ss << "\"tmb\" : " << fims::Vector<double>(population_interface->log_M.size(), -999) << ",\n";
      // ss << "\"stan\" : " << fims::Vector<double>(population_interface->log_M.size(), -999) << ",\n";
      ss << "\"tmb\" : " << fims::Vector<double>(5, -999) << "\n";
      ss << "}}],\n";

      fims_popdy::CatchAtAge<double>::population_derived_quantities_iterator
          cit;
      ss << " \"derived_quantities\": [\n";
      cit = model_ptr->population_derived_quantities.find(
          population_interface->get_id());

      if (cit != model_ptr->population_derived_quantities.end())
      {
        ss << model_ptr->population_derived_quantities_to_json(cit) << "]}\n";
      }
      else
      {
        ss << " ]}\n";
      }
    }
    else
    {
      ss << "{\n";
      ss << " \"name\" : \"Population\",\n";

      ss << " \"type\" : \"population\",\n";
      ss << " \"tag\" : \"" << population_interface->get_id()
         << " not found in Information.\",\n";
      ss << " \"id\": " << population_interface->get_id() << ",\n";
      ss << " \"recruitment_id\": " << population_interface->recruitment_id
         << ",\n";
      ss << " \"growth_id\": " << population_interface->growth_id << ",\n";
      ss << " \"maturity_id\": " << population_interface->maturity_id << ",\n";
      ss << " \"derived_quantities\": []}\n";
#warning Add error log here
    }

    return ss.str();
  }

  /**
   * This function is used to convert the derived quantities of a population or
   * fleet to a JSON string. This function is used to create the JSON output for
   * the CatchAtAge model.
   */
  std::string DerivedQuantityToJSON(
      std::map<std::string, fims::Vector<double>>::iterator it,
      const fims_popdy::DimensionInfo &dim_info)
  {
    std::stringstream ss;
    std::string name = (*it).first;
    fims::Vector<double> &dq = (*it).second;
    std::stringstream dim_entry;

    // gather dimension information
    switch (dim_info.ndims)
    {
    case 1:
      dim_entry << "\"dimensionality\": {\n";
      dim_entry << "  \"header\": [\"" << dim_info.dim_names[0] << "\"],\n";
      dim_entry << "  \"dimensions\": [";
      for (size_t i = 0; i < dim_info.dims.size(); ++i)
      {
        if (i > 0)
          dim_entry << ", ";
        dim_entry << dim_info.dims[i];
      }
      dim_entry << "]\n";
      dim_entry << "}";
      break;
    case 2:
      dim_entry << "\"dimensionality\": {\n";
      dim_entry << "  \"header\": [\"" << dim_info.dim_names[0] << "\", \"" << dim_info.dim_names[1] << "\"],\n";
      dim_entry << "  \"dimensions\": [";
      for (size_t i = 0; i < dim_info.dims.size(); ++i)
      {
        if (i > 0)
          dim_entry << ", ";
        dim_entry << dim_info.dims[i];
      }
      dim_entry << "]\n";
      dim_entry << "}";
      break;
    case 3:
      dim_entry << "\"dimensionality\": {\n";
      dim_entry << "  \"header\": [\"" << dim_info.dim_names[0] << "\", \"" << dim_info.dim_names[1] << "\", \"" << dim_info.dim_names[2] << "\"],\n";
      dim_entry << "  \"dimensions\": [";
      for (size_t i = 0; i < dim_info.dims.size(); ++i)
      {
        if (i > 0)
          dim_entry << ", ";
        dim_entry << dim_info.dims[i];
      }
      dim_entry << "]\n";
      dim_entry << "}";
      break;
    default:
      dim_entry << "\"dimensionality\": {\n";
      dim_entry << "  \"header\": [],\n";
      dim_entry << "  \"dimensions\": []\n";
      dim_entry << "}";
      break;
    }

    // build JSON string
    ss << "{\n";
    ss << "\"name\":\"" << (*it).first << "\",\n";
    ss << dim_entry.str() << ",\n";
    ss << "\"values\":[";

    if (dq.size() > 0)
    {
      for (size_t i = 0; i < dq.size() - 1; i++)
      {
        if (dq[i] != dq[i]) // check for NaN
        {
          ss << "\"nan\", ";
        }
        else
        {
          ss << dq[i] << ", ";
        }
      }
      if (dq[dq.size() - 1] != dq[dq.size() - 1]) // check for NaN
      {
        ss << "\"nan\"";
      }
      else
      {
        ss << dq[dq.size() - 1] << "]\n";
      }
    }
    else
    {
      ss << "]\n";
    }
    ss << "}";

    return ss.str();
  }

  /**
   * @brief Send the fleet-based derived quantities to the json file.
   * @return std::string
   */
  std::string derived_quantities_component_to_json(
      std::map<std::string, fims::Vector<double>> &dqs,
      std::map<std::string, fims_popdy::DimensionInfo> &dim_info)
  {

    std::stringstream ss;
    std::map<std::string, fims_popdy::DimensionInfo>::iterator dim_info_it;
    std::map<std::string, fims::Vector<double>>::iterator it;
    std::map<std::string, fims::Vector<double>>::iterator end_it;
    end_it = dqs.end();
    typename std::map<std::string, fims::Vector<double>>::iterator second_to_last;
    second_to_last = dqs.end();
    if (it != end_it)
    {
      second_to_last--;
    }

    it = dqs.begin();
    for (; it != second_to_last; ++it)
    {
      dim_info_it = dim_info.find(it->first);
      ss << this->DerivedQuantityToJSON(it, dim_info_it->second) << ",\n";
    }
    dim_info_it = dim_info.find(second_to_last->first);
    if (dim_info_it != dim_info.end())
    {
      ss << this->DerivedQuantityToJSON(second_to_last, dim_info_it->second) << "\n";
    }
    else
    {
      // Handle case where dimension info is not found
    }
    return ss.str();
  }
  /**
   * @brief Send the population-based derived quantities to the json file.
   * @return std::string
   */
  std::string population_derived_quantities_to_json(
      typename fims_popdy::CatchAtAge<double>::population_derived_quantities_iterator pdqit)
  {
    std::stringstream ss;
    // ss << std::fixed;
    // ss << "{\n";
    // ss << "\"id\": " << (*pdqit).first << ",\n";
    // ss << "\"derived_quantities\": [\n";
    population_derived_quantities_dim_strings_iterator str_it = this->population_derived_quantities_dim_strings.find((*pdqit).first);

    typename std::map<std::string, fims::Vector<double>>::iterator it;
    typename std::map<std::string, fims::Vector<double>>::iterator end_it;
    end_it = (*pdqit).second.end();
    typename std::map<std::string, fims::Vector<double>>::iterator second_to_last;
    second_to_last = (*pdqit).second.end();
    if (it != end_it)
    {
      second_to_last--;
    }

    std::string dims;
    it = (*pdqit).second.begin();
    for (; it != second_to_last; ++it)
    {
      dims = "\"dimensions\" : []";
      if (str_it != this->fleet_derived_quantities_dim_strings.end())
      {
        dims = str_it->second[it->first];
        // ss << this->DerivedQuantityToJSON(it, dims) << ",\n";
      }
    }

    dims = "\"dimensions\" : []";
    if (str_it != this->fleet_derived_quantities_dim_strings.end())
    {
      dims = str_it->second[second_to_last->first];
      //    ss << this->DerivedQuantityToJSON(second_to_last, dims) << "\n";
    }
    return ss.str();
  }

  /**
   * @brief Method to convert a fleet to a JSON string.
   */
  std::string
  fleet_to_json(FleetInterface *fleet_interface)
  {
    std::stringstream ss;

    typename std::map<uint32_t, std::shared_ptr<FleetInterfaceBase>>::iterator
        fi_it; // fleet interface iterator
    fi_it = FleetInterfaceBase::live_objects.find(fleet_interface->get_id());
    if (fi_it == FleetInterfaceBase::live_objects.end())
    {
      FIMS_ERROR_LOG("Fleet with id " +
                     fims::to_string(fleet_interface->get_id()) +
                     " not found in live objects.");
      return "{}"; // Return empty JSON
    }

    std::shared_ptr<FleetInterface> fleet_interface_ptr =
        std::dynamic_pointer_cast<FleetInterface>((*fi_it).second);

    if (!fleet_interface_ptr)
    {
      FIMS_ERROR_LOG("Fleet with id " +
                     fims::to_string(fleet_interface->get_id()) +
                     " not found in live objects.");
      return "{}"; // Return empty JSON
    }

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    typename fims_info::Information<double>::model_map_iterator model_it;
    model_it = info->models_map.find(this->get_id());
    std::shared_ptr<fims_popdy::CatchAtAge<double>> model_ptr =
        std::dynamic_pointer_cast<fims_popdy::CatchAtAge<double>>(
            (*model_it).second);

    typename fims_info::Information<double>::fleet_iterator fit;

    fit = info->fleets.find(fleet_interface->get_id());

    if (fit != info->fleets.end())
    {
      std::shared_ptr<fims_popdy::Fleet<double>> &fleet = (*fit).second;

      ss << "{\n";
      ss << " \"name\" : \"Fleet\",\n";

      ss << " \"type\" : \"fleet\",\n";
      ss << " \"tag\" : \"" << fleet_interface->name << "\",\n";
      ss << " \"id\": " << fleet_interface->id << ",\n";
      // ss << " \"is_survey\": " << fleet_interface->is_survey << ",\n";
      ss << " \"nlengths\": " << fleet_interface->nlengths.get() << ",\n";
      ss << "\"parameters\": [\n";
      ss << "{\n";
      for (size_t i = 0; i < fleet_interface->log_Fmort.size(); i++)
      {
        fleet_interface->log_Fmort[i].final_value_m = fleet->log_Fmort[i];
      }

      ss << " \"name\": \"log_Fmort\",\n";
      ss << " \"id\":" << fleet_interface->log_Fmort.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"values\": " << fleet_interface->log_Fmort << "\n},\n";

      ss << " {\n";
      for (size_t i = 0; i < fleet->log_q.size(); i++)
      {
        fleet_interface->log_q[i].final_value_m = fleet->log_q[i];
      }
      ss << " \"name\": \"log_q\",\n";
      ss << " \"id\":" << fleet_interface->log_q.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"values\": " << fleet_interface->log_q << "\n},\n";
      if (fleet_interface->nlengths > 0)
      {
        ss << " {\n";
        for (size_t i = 0; i < fleet_interface->age_to_length_conversion.size();
             i++)
        {
          fleet_interface->age_to_length_conversion[i].final_value_m =
              fleet->age_to_length_conversion[i];
        }
        ss << " \"name\": \"age_to_length_conversion\",\n";
        ss << " \"id\":" << fleet_interface->age_to_length_conversion.id_m
           << ",\n";
        ss << " \"type\": \"vector\",\n";
        ss << " \"values\": " << fleet_interface->age_to_length_conversion
           << "\n}\n";
      }

      ss << "], \"derived_quantities\": [";

      std::map<std::string, fims::Vector<double>> dqs =
          model_ptr->GetFleetDerivedQuantities(fleet_interface->get_id());
      std::map<std::string, fims_popdy::DimensionInfo> dim_info =
          model_ptr->GetFleetDimensionInfo(fleet_interface->get_id());
      ss << this->derived_quantities_component_to_json(dqs, dim_info);
    }
    else
    {
      ss << "{\n";
      ss << " \"name\" : \"Fleet\",\n";
      ss << " \"type\" : \"fleet\",\n";
      ss << " \"tag\" : \"" << fleet_interface->get_id()
         << " not found in Information.\",\n";
      ss << " \"derived_quantities\": []}\n";
    }
    return ss.str();
  }

  /**
   * @brief Method to convert the model to a JSON string.
   */
  virtual std::string to_json()
  {
    std::set<uint32_t> fleet_ids;
    // typename std::set<uint32_t>::iterator fleet_it;
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();
    std::shared_ptr<fims_popdy::CatchAtAge<double>> model =
        std::dynamic_pointer_cast<fims_popdy::CatchAtAge<double>>(
            info->models_map[this->get_id()]);

    std::stringstream ss;
    ss.str("");

    ss << "{\n";
    ss << " \"name\" : \"CatchAtAge\",\n";
    ss << " \"type\" : \"model\",\n";
    // ss << " \"tag\" : \"" << model->name << "\",\n";
    ss << " \"id\": " << this->get_id() << ",\n";
    ss << " \"population_ids\": [";
    typename std::set<uint32_t>::iterator pit;
    for (pit = this->population_ids->begin();
         pit != this->population_ids->end(); pit++)
    {
      ss << *pit;
      if (std::next(pit) != this->population_ids->end())
      {
        ss << ", ";
      }
    }
    ss << "],\n";
    // ss << "\"populations\": [\n";
    // typename std::set<uint32_t>::iterator pop_it;
    // typename std::set<uint32_t>::iterator pop_end_it;
    // pop_end_it = this->population_ids->end();
    // typename std::set<uint32_t>::iterator pop_second_to_last_it;
    // if (pop_end_it != this->population_ids->begin())
    // {
    //   pop_second_to_last_it = std::prev(pop_end_it);
    // }
    // else
    // {
    //   pop_second_to_last_it = pop_end_it;
    // }

    // for (pop_it = this->population_ids->begin();
    //      pop_it != pop_second_to_last_it; pop_it++)
    // {
    //   std::shared_ptr<PopulationInterface> population_interface =
    //       std::dynamic_pointer_cast<PopulationInterface>(
    //           PopulationInterfaceBase::live_objects[*pop_it]);
    //   if (population_interface)
    //   {
    //     std::set<uint32_t>::iterator fids;
    //     for (fids = population_interface->fleet_ids->begin();
    //          fids != population_interface->fleet_ids->end(); fids++)
    //     {
    //       fleet_ids.insert(*fids);
    //     }
    //     ss << this->population_to_json(population_interface.get()) << ",";
    //   }
    //   else
    //   {
    //     FIMS_ERROR_LOG("Population with id " + fims::to_string(*pop_it) +
    //                    " not found in live objects.");
    //     ss << "{}"; // Return empty JSON for this population
    //   }
    // }

    // std::shared_ptr<PopulationInterface> population_interface =
    //     std::dynamic_pointer_cast<PopulationInterface>(
    //         PopulationInterfaceBase::live_objects[*pop_second_to_last_it]);
    // if (population_interface)
    // {
    //   std::set<uint32_t>::iterator fids;
    //   for (fids = population_interface->fleet_ids->begin();
    //        fids != population_interface->fleet_ids->end(); fids++)
    //   {
    //     fleet_ids.insert(*fids);
    //   }
    //   ss << this->population_to_json(population_interface.get());
    // }
    // else
    // {
    //   FIMS_ERROR_LOG("Population with id " + fims::to_string(*pop_it) +
    //                  " not found in live objects.");
    //   ss << "{}"; // Return empty JSON for this population
    // }

    // ss << "]";
    // ss << ",\n";
    ss << "\"fleets\": [\n";
    typename std::set<uint32_t>::iterator fleet_it;
    typename std::set<uint32_t>::iterator fleet_end_it;
    fleet_end_it = fleet_ids.end();
    typename std::set<uint32_t>::iterator fleet_second_to_last_it;
    if (fleet_end_it != fleet_ids.begin())
    {
      fleet_second_to_last_it = std::prev(fleet_end_it);
    }

    for (fleet_it = fleet_ids.begin(); fleet_it != fleet_second_to_last_it;
         fleet_it++)
    {
      std::shared_ptr<FleetInterface> fleet_interface =
          std::dynamic_pointer_cast<FleetInterface>(
              FleetInterfaceBase::live_objects[*fleet_it]);
      if (fleet_interface)
      {
        ss << this->fleet_to_json(fleet_interface.get()) << ",";
      }
      else
      {
        FIMS_ERROR_LOG("Fleet with id " + fims::to_string(*fleet_it) +
                       " not found in live objects.");
        ss << "{}"; // Return empty JSON for this fleet
      }
    }
    std::shared_ptr<FleetInterface> fleet_interface =
        std::dynamic_pointer_cast<FleetInterface>(
            FleetInterfaceBase::live_objects[*fleet_second_to_last_it]);
    if (fleet_interface)
    {
      ss << this->fleet_to_json(fleet_interface.get());
    }
    else
    {
      FIMS_ERROR_LOG("Fleet with id " + fims::to_string(*fleet_it) +
                     " not found in live objects.");
      ss << "{}"; // Return empty JSON for this fleet
    }

    ss << "]\n}";
    return fims::JsonParser::PrettyFormatJSON(ss.str());
  }

  /**
   * @brief Sum method to calculate the sum of an array or vector of doubles.
   *
   * @param v
   * @return double
   */
  double sum(const std::valarray<double> &v)
  {
    double sum = 0.0;
    for (size_t i = 0; i < v.size(); i++)
    {
      sum += v[i];
    }
    return sum;
  }

  /**
   * @brief Sum method for a vector of doubles.
   *
   * @param v
   * @return double
   */
  double sum(const std::vector<double> &v)
  {
    double sum = 0.0;
    for (size_t i = 0; i < v.size(); i++)
    {
      sum += v[i];
    }
    return sum;
  }

  /**
   * @brief Minimum method to calculate the minimum of an array or vector
   * of doubles.
   *
   * @param v
   * @return double
   */
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
  /**
   * @brief TODO: document this method.
   *
   * @param v
   * @return std::valarray<double>
   */
  std::valarray<double> fabs(const std::valarray<double> &v)
  {
    std::valarray<double> result(v.size());
    for (size_t i = 0; i < v.size(); i++)
    {
      result[i] = std::fabs(v[i]);
    }
    return result;
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal()
  {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::CatchAtAge<Type>> model =
        std::make_shared<fims_popdy::CatchAtAge<Type>>();

    population_id_iterator it;

    for (it = this->population_ids->begin(); it != this->population_ids->end();
         ++it)
    {
      model->AddPopulation((*it));
    }

    std::set<uint32_t> fleet_ids; // all fleets in the model
    typedef typename std::set<uint32_t>::iterator fleet_ids_iterator;

    // add to Information
    info->models_map[this->get_id()] = model;

    for (it = this->population_ids->begin(); it != this->population_ids->end();
         ++it)
    {
      std::shared_ptr<PopulationInterface> population =
          std::dynamic_pointer_cast<PopulationInterface>(
              PopulationInterfaceBase::live_objects[(*it)]);

      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          model->GetPopulationDerivedQuantities((*it));

      std::map<std::string, fims_popdy::DimensionInfo> &derived_quantities_dim_info =
          model->GetPopulationDimensionInfo((*it));

      std::stringstream ss;

      derived_quantities["total_landings_weight"] =
          fims::Vector<Type>(population->nyears.get());
      derived_quantities_dim_info["total_landings_weight"] =
          fims_popdy::DimensionInfo("total_landings_weight",
                                    fims::Vector<int>{(int)population->nyears.get() + 1},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["total_landings_numbers"] =
          fims::Vector<Type>(population->nyears.get());
      derived_quantities_dim_info["total_landings_numbers"] =
          fims_popdy::DimensionInfo("total_landings_numbers",
                                    fims::Vector<int>{population->nyears.get() + 1},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["mortality_F"] = fims::Vector<Type>(
          population->nyears.get() * population->nages.get());
      derived_quantities_dim_info["mortality_F"] =
          fims_popdy::DimensionInfo("mortality_F",
                                    fims::Vector<int>{population->nyears.get() + 1, population->nages.get() + 1},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["mortality_Z"] = fims::Vector<Type>(
          population->nyears.get() * population->nages.get());
      derived_quantities_dim_info["mortality_Z"] =
          fims_popdy::DimensionInfo("mortality_Z",
                                    fims::Vector<int>{population->nyears.get() + 1, population->nages.get() + 1},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["weight_at_age"] = fims::Vector<Type>(
          population->nyears.get() * population->nages.get());
      derived_quantities_dim_info["weight_at_age"] =
          fims_popdy::DimensionInfo("weight_at_age",
                                    fims::Vector<int>{population->nyears.get() + 1, population->nages.get() + 1},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["numbers_at_age"] = fims::Vector<Type>(
          (population->nyears.get() + 1) * population->nages.get());
      derived_quantities_dim_info["numbers_at_age"] =
          fims_popdy::DimensionInfo("numbers_at_age",
                                    fims::Vector<int>{(population->nyears.get() + 1), population->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["unfished_numbers_at_age"] = fims::Vector<Type>(
          (population->nyears.get() + 1) * population->nages.get());
      derived_quantities_dim_info["unfished_numbers_at_age"] =
          fims_popdy::DimensionInfo("unfished_numbers_at_age",
                                    fims::Vector<int>{(population->nyears.get() + 1), population->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["biomass"] =
          fims::Vector<Type>((population->nyears.get() + 1));
      derived_quantities_dim_info["biomass"] =
          fims_popdy::DimensionInfo("biomass",
                                    fims::Vector<int>{(population->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["spawning_biomass"] =
          fims::Vector<Type>((population->nyears.get() + 1));
      derived_quantities_dim_info["spawning_biomass"] =
          fims_popdy::DimensionInfo("spawning_biomass",
                                    fims::Vector<int>{(population->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["unfished_biomass"] =
          fims::Vector<Type>((population->nyears.get() + 1));
      derived_quantities_dim_info["unfished_biomass"] =
          fims_popdy::DimensionInfo("unfished_biomass",
                                    fims::Vector<int>{(population->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["unfished_spawning_biomass"] =
          fims::Vector<Type>((population->nyears.get() + 1));
      derived_quantities_dim_info["unfished_spawning_biomass"] =
          fims_popdy::DimensionInfo("unfished_spawning_biomass",
                                    fims::Vector<int>{(population->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["proportion_mature_at_age"] = fims::Vector<Type>(
          (population->nyears.get() + 1) * population->nages.get());
      derived_quantities_dim_info["proportion_mature_at_age"] =
          fims_popdy::DimensionInfo("proportion_mature_at_age",
                                    fims::Vector<int>{(population->nyears.get() + 1), population->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["expected_recruitment"] =
          fims::Vector<Type>((population->nyears.get() + 1));
      derived_quantities_dim_info["expected_recruitment"] =
          fims_popdy::DimensionInfo("expected_recruitment",
                                    fims::Vector<int>{(population->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["sum_selectivity"] = fims::Vector<Type>(
          population->nyears.get() * population->nages.get());
      derived_quantities_dim_info["sum_selectivity"] =
          fims_popdy::DimensionInfo("sum_selectivity",
                                    fims::Vector<int>{population->nyears.get(), population->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});

      // replace elements in the variable map
      info->variable_map[population->numbers_at_age.id_m] =
          &(derived_quantities["numbers_at_age"]);

      for (fleet_ids_iterator fit = population->fleet_ids->begin();
           fit != population->fleet_ids->end(); ++fit)
      {
        fleet_ids.insert(*fit);
      }
    }

    for (fleet_ids_iterator it = fleet_ids.begin(); it != fleet_ids.end();
         ++it)
    {

      std::shared_ptr<FleetInterface> fleet_interface =
          std::dynamic_pointer_cast<FleetInterface>(
              FleetInterfaceBase::live_objects[(*it)]);

      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          model->GetFleetDerivedQuantities(fleet_interface->id);

      std::map<std::string, fims_popdy::DimensionInfo> &derived_quantities_dim_info =
          model->GetFleetDimensionInfo(fleet_interface->id);

      // initialize derive quantities
      // landings
      derived_quantities["landings_numbers_at_age"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());
      derived_quantities_dim_info["landings_numbers_at_age"] =
          fims_popdy::DimensionInfo("landings_numbers_at_age",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), fleet_interface->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["landings_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());
      derived_quantities_dim_info["landings_weight_at_age"] =
          fims_popdy::DimensionInfo("landings_weight_at_age",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), fleet_interface->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["landings_numbers_at_length"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nlengths.get());
      derived_quantities_dim_info["landings_numbers_at_length"] =
          fims_popdy::DimensionInfo("landings_numbers_at_length",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), fleet_interface->nlengths.get()},
                                    fims::Vector<std::string>{"year", "length"});

      derived_quantities["landings_weight"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["landings_weight"] =
          fims_popdy::DimensionInfo("landings_weight",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["landings_numbers"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["landings_numbers"] =
          fims_popdy::DimensionInfo("landings_numbers",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["landings_expected"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["landings_expected"] =
          fims_popdy::DimensionInfo("landings_expected",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["log_landings_expected"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["log_landings_expected"] =
          fims_popdy::DimensionInfo("log_landings_expected",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["agecomp_proportion"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());
      derived_quantities_dim_info["agecomp_proportion"] =
          fims_popdy::DimensionInfo("agecomp_proportion",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), fleet_interface->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["lengthcomp_proportion"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nlengths.get());
      derived_quantities_dim_info["lengthcomp_proportion"] =
          fims_popdy::DimensionInfo("lengthcomp_proportion",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), fleet_interface->nlengths.get()},
                                    fims::Vector<std::string>{"year", "length"});

      // index
      derived_quantities["index_numbers_at_age"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());
      derived_quantities_dim_info["index_numbers_at_age"] =
          fims_popdy::DimensionInfo("index_numbers_at_age",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), fleet_interface->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["index_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());
      derived_quantities_dim_info["index_weight_at_age"] =
          fims_popdy::DimensionInfo("index_weight_at_age",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), fleet_interface->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});
      derived_quantities["index_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());
      derived_quantities_dim_info["index_weight_at_age"] =
          fims_popdy::DimensionInfo("index_weight_at_age",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), fleet_interface->nages.get()},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["index_numbers_at_length"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nlengths.get());
      derived_quantities_dim_info["index_numbers_at_length"] =
          fims_popdy::DimensionInfo("index_numbers_at_length",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), fleet_interface->nlengths.get()},
                                    fims::Vector<std::string>{"year", "length"});
      derived_quantities["index_weight"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["index_weight"] =
          fims_popdy::DimensionInfo("index_weight",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["index_numbers"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["index_numbers"] =
          fims_popdy::DimensionInfo("index_numbers",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["index_expected"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["index_expected"] =
          fims_popdy::DimensionInfo("index_expected",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["log_index_expected"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["log_index_expected"] =
          fims_popdy::DimensionInfo("log_index_expected",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["catch_index"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["catch_index"] =
          fims_popdy::DimensionInfo("catch_index",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["expected_catch"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["expected_catch"] =
          fims_popdy::DimensionInfo("expected_catch",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["expected_index"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      derived_quantities_dim_info["expected_index"] =
          fims_popdy::DimensionInfo("expected_index",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1)},
                                    fims::Vector<std::string>{"year"});

      derived_quantities["agecomp_expected"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());
      derived_quantities_dim_info["agecomp_expected"] =
          fims_popdy::DimensionInfo("agecomp_expected",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), (fleet_interface->nages.get() + 1)},
                                    fims::Vector<std::string>{"year", "age"});

      derived_quantities["lengthcomp_expected"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nlengths.get());
      derived_quantities_dim_info["lengthcomp_expected"] =
          fims_popdy::DimensionInfo("lengthcomp_expected",
                                    fims::Vector<int>{(fleet_interface->nyears.get() + 1), (fleet_interface->nlengths.get() + 1)},
                                    fims::Vector<std::string>{"year", "length"});

      if (fleet_interface->nlengths.get() > 0)
      {
        derived_quantities["age_to_length_conversion"] = fims::Vector<Type>(
            fleet_interface->nyears.get() * fleet_interface->nlengths.get());
        derived_quantities_dim_info["age_to_length_conversion"] =
            fims_popdy::DimensionInfo("age_to_length_conversion",
                                      fims::Vector<int>{(fleet_interface->nyears.get() + 1), (fleet_interface->nlengths.get() + 1)},
                                      fims::Vector<std::string>{"year", "length"});
      }
      // replace elements in the variable map
      info->variable_map[fleet_interface->log_landings_expected.id_m] =
          &(derived_quantities["log_landings_expected"]);
      info->variable_map[fleet_interface->log_index_expected.id_m] =
          &(derived_quantities["log_index_expected"]);
      info->variable_map[fleet_interface->agecomp_expected.id_m] =
          &(derived_quantities["agecomp_expected"]);
      info->variable_map[fleet_interface->agecomp_proportion.id_m] =
          &(derived_quantities["agecomp_proportion"]);
      info->variable_map[fleet_interface->lengthcomp_expected.id_m] =
          &(derived_quantities["lengthcomp_expected"]);
      if (fleet_interface->nlengths.get() > 0)
      {
        info->variable_map[fleet_interface->age_to_length_conversion.id_m] =
            &(derived_quantities["age_to_length_conversion"]);
      }
      info->variable_map[fleet_interface->lengthcomp_expected.id_m] =
          &(derived_quantities["lengthcomp_expected"]);
      info->variable_map[fleet_interface->lengthcomp_proportion.id_m] =
          &(derived_quantities["lengthcomp_proportion"]);
    }

    return true;
  }

  virtual bool add_to_fims_tmb()
  {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
#ifdef TMBAD_FRAMEWORK
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
#else
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();
#endif
    return true;
  }

#endif
};

#endif
