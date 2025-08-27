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
class FisheryModelInterfaceBase : public FIMSRcppInterfaceBase {
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
   * @brief The set of population IDs associated with the fishery model.
   */
  std::shared_ptr<std::set<uint32_t>> population_ids;
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
  FisheryModelInterfaceBase() {
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
      : id(other.id),
      population_ids(other.population_ids) {}

  /**
   * @brief The destructor.
   */
  virtual ~FisheryModelInterfaceBase() {}

  virtual std::string to_json() {
    return "std::string to_json() not yet implemented.";
  }

  /**
   * @brief A function to calculate reference points for the fishery model.
   *
   * @return Rcpp::List
   */
  virtual Rcpp::List calculate_reference_points() {
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
class CatchAtAgeInterface : public FisheryModelInterfaceBase {
  typedef typename std::set<uint32_t>::iterator population_id_iterator;

 public:
  /**
   * @brief The constructor.
   */
  CatchAtAgeInterface() : FisheryModelInterfaceBase() {
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
      : FisheryModelInterfaceBase(other) {}

  /**
   * Method to add a population id to the set of population ids.
   */
  void AddPopulation(uint32_t id) {
    this->population_ids->insert(id);

    std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
    pit = PopulationInterfaceBase::live_objects.find(id);
    if (pit != PopulationInterfaceBase::live_objects.end()) {
      std::shared_ptr<PopulationInterfaceBase> &pop = (*pit).second;
      pop->initialize_catch_at_age.set(true);
    } else {
      FIMS_ERROR_LOG("Population with id " + fims::to_string(id) +
                     " not found.");
    }
  }

  /**
   * @brief Method to get the population id.
   */
  virtual uint32_t get_id() {
    typename std::map<uint32_t,
                      std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
    return this->id;
  }

  /**
   * @brief Method to get the population ids.
   */
  void Show() {
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
  std::string population_to_json(PopulationInterface *population_interface) {
    std::stringstream ss;

    typename std::map<uint32_t,
                      std::shared_ptr<PopulationInterfaceBase>>::iterator
        pi_it;  // population interface iterator
    pi_it = PopulationInterfaceBase::live_objects.find(
        population_interface->get_id());
    if (pi_it == PopulationInterfaceBase::live_objects.end()) {
      FIMS_ERROR_LOG("Population with id " +
                     fims::to_string(population_interface->get_id()) +
                     " not found in live objects.");
      return "{}";  // Return empty JSON
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

    if (pit != info->populations.end()) {
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

      ss << " \"parameters\": [\n{\n";
      for (size_t i = 0; i < pop->log_M.size(); i++) {
        population_interface_ptr->log_M[i].final_value_m = pop->log_M[i];
      }

      ss << " \"name\": \"log_M\",\n";
      ss << " \"id\":" << population_interface->log_M.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"values\": " << population_interface->log_M << "\n},\n";

      ss << "{\n";

      for (size_t i = 0; i < pop->log_init_naa.size(); i++) {
        population_interface_ptr->log_init_naa[i].final_value_m =
            pop->log_init_naa[i];
      }
      ss << "  \"name\": \"log_init_naa\",\n";
      ss << "  \"id\":" << population_interface->log_init_naa.id_m << ",\n";
      ss << "  \"type\": \"vector\",\n";
      ss << "  \"values\":" << population_interface->log_init_naa << " \n}],\n";

      fims_popdy::CatchAtAge<double>::population_derived_quantities_iterator
          cit;
      ss << " \"derived_quantities\": [\n";
      cit = model_ptr->population_derived_quantities.find(
          population_interface->get_id());

      if (cit != model_ptr->population_derived_quantities.end()) {
        ss << model_ptr->population_derived_quantities_to_json(cit) << "]}\n";
      } else {
        ss << " ]}\n";
      }
    } else {
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
   * @brief Method to convert a fleet to a JSON string.
   */
  std::string fleet_to_json(FleetInterface *fleet_interface) {
    std::stringstream ss;

    typename std::map<uint32_t, std::shared_ptr<FleetInterfaceBase>>::iterator
        fi_it;  // fleet interface iterator
    fi_it = FleetInterfaceBase::live_objects.find(fleet_interface->get_id());
    if (fi_it == FleetInterfaceBase::live_objects.end()) {
      FIMS_ERROR_LOG("Fleet with id " +
                     fims::to_string(fleet_interface->get_id()) +
                     " not found in live objects.");
      return "{}";  // Return empty JSON
    }

    std::shared_ptr<FleetInterface> fleet_interface_ptr =
        std::dynamic_pointer_cast<FleetInterface>((*fi_it).second);

    if (!fleet_interface_ptr) {
      FIMS_ERROR_LOG("Fleet with id " +
                     fims::to_string(fleet_interface->get_id()) +
                     " not found in live objects.");
      return "{}";  // Return empty JSON
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

    if (fit != info->fleets.end()) {
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
      for (size_t i = 0; i < fleet_interface->log_Fmort.size(); i++) {
        fleet_interface->log_Fmort[i].final_value_m = fleet->log_Fmort[i];
      }

      ss << " \"name\": \"log_Fmort\",\n";
      ss << " \"id\":" << fleet_interface->log_Fmort.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"values\": " << fleet_interface->log_Fmort << "\n},\n";

      ss << " {\n";
      for (size_t i = 0; i < fleet->log_q.size(); i++) {
        fleet_interface->log_q[i].final_value_m = fleet->log_q[i];
      }
      ss << " \"name\": \"log_q\",\n";
      ss << " \"id\":" << fleet_interface->log_q.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"values\": " << fleet_interface->log_q << "\n},\n";
      if (fleet_interface->nlengths > 0) {
        ss << " {\n";
        for (size_t i = 0; i < fleet_interface->age_to_length_conversion.size();
             i++) {
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
      fims_popdy::CatchAtAge<double>::fleet_derived_quantities_iterator fit;
      fit = model_ptr->fleet_derived_quantities.find(fleet_interface->get_id());

      if (fit != model_ptr->fleet_derived_quantities.end()) {
        ss << model_ptr->fleet_derived_quantities_to_json(fit) << "]}\n";
      } else {
        ss << " ]}\n";
      }
    } else {
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
  virtual std::string to_json() {
    std::set<uint32_t> fleet_ids;
    // typename std::set<uint32_t>::iterator fleet_it;
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();
    std::shared_ptr<fims_popdy::CatchAtAge<double>> model =
        std::dynamic_pointer_cast<fims_popdy::CatchAtAge<double>>(
            info->models_map[this->get_id()]);

    std::stringstream ss;

    ss << "{\n";
    ss << " \"name\" : \"CatchAtAge\",\n";
    ss << " \"type\" : \"model\",\n";
    // ss << " \"tag\" : \"" << model->name << "\",\n";
    ss << " \"id\": " << this->get_id() << ",\n";
    ss << " \"population_ids\": [";
    typename std::set<uint32_t>::iterator pit;
    for (pit = this->population_ids->begin();
         pit != this->population_ids->end(); pit++) {
      ss << *pit;
      if (std::next(pit) != this->population_ids->end()) {
        ss << ", ";
      }
    }
    ss << "],\n";
    ss << "\"populations\": [\n";
    typename std::set<uint32_t>::iterator pop_it;
    typename std::set<uint32_t>::iterator pop_end_it;
    pop_end_it = this->population_ids->end();
    typename std::set<uint32_t>::iterator pop_second_to_last_it;
    if (pop_end_it != this->population_ids->begin()) {
      pop_second_to_last_it = std::prev(pop_end_it);
    } else {
      pop_second_to_last_it = pop_end_it;
    }

    for (pop_it = this->population_ids->begin();
         pop_it != pop_second_to_last_it; pop_it++) {
      std::shared_ptr<PopulationInterface> population_interface =
          std::dynamic_pointer_cast<PopulationInterface>(
              PopulationInterfaceBase::live_objects[*pop_it]);
      if (population_interface) {
        std::set<uint32_t>::iterator fids;
        for (fids = population_interface->fleet_ids->begin();
             fids != population_interface->fleet_ids->end(); fids++) {
          fleet_ids.insert(*fids);
        }
        ss << this->population_to_json(population_interface.get()) << ",";
      } else {
        FIMS_ERROR_LOG("Population with id " + fims::to_string(*pop_it) +
                       " not found in live objects.");
        ss << "{}";  // Return empty JSON for this population
      }
    }

    std::shared_ptr<PopulationInterface> population_interface =
        std::dynamic_pointer_cast<PopulationInterface>(
            PopulationInterfaceBase::live_objects[*pop_second_to_last_it]);
    if (population_interface) {
      std::set<uint32_t>::iterator fids;
      for (fids = population_interface->fleet_ids->begin();
           fids != population_interface->fleet_ids->end(); fids++) {
        fleet_ids.insert(*fids);
      }
      ss << this->population_to_json(population_interface.get());
    } else {
      FIMS_ERROR_LOG("Population with id " + fims::to_string(*pop_it) +
                     " not found in live objects.");
      ss << "{}";  // Return empty JSON for this population
    }

    ss << "]";
    ss << ",\n";
    ss << "\"fleets\": [\n";
    typename std::set<uint32_t>::iterator fleet_it;
    typename std::set<uint32_t>::iterator fleet_end_it;
    fleet_end_it = fleet_ids.end();
    typename std::set<uint32_t>::iterator fleet_second_to_last_it;
    if (fleet_end_it != fleet_ids.begin()) {
      fleet_second_to_last_it = std::prev(fleet_end_it);
    }

    for (fleet_it = fleet_ids.begin(); fleet_it != fleet_second_to_last_it;
         fleet_it++) {
      std::shared_ptr<FleetInterface> fleet_interface =
          std::dynamic_pointer_cast<FleetInterface>(
              FleetInterfaceBase::live_objects[*fleet_it]);
      if (fleet_interface) {
        ss << this->fleet_to_json(fleet_interface.get()) << ",";
      } else {
        FIMS_ERROR_LOG("Fleet with id " + fims::to_string(*fleet_it) +
                       " not found in live objects.");
        ss << "{}";  // Return empty JSON for this fleet
      }
    }
    std::shared_ptr<FleetInterface> fleet_interface =
        std::dynamic_pointer_cast<FleetInterface>(
            FleetInterfaceBase::live_objects[*fleet_second_to_last_it]);
    if (fleet_interface) {
      ss << this->fleet_to_json(fleet_interface.get());
    } else {
      FIMS_ERROR_LOG("Fleet with id " + fims::to_string(*fleet_it) +
                     " not found in live objects.");
      ss << "{}";  // Return empty JSON for this fleet
    }

    ss << "]\n}";
    return fims::JsonParser::PrettyFormatJSON(ss.str());
  }

  // TODO: Should these be moved to rcpp_interface_base to make usable for all
  // classes?
  /**
   * @brief Sum method to calculate the sum of an array or vector of doubles.
   *
   * @param v
   * @return double
   */
  double sum(const std::valarray<double> &v) {
    double sum = 0.0;
    for (size_t i = 0; i < v.size(); i++) {
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
  double sum(const std::vector<double> &v) {
    double sum = 0.0;
    for (size_t i = 0; i < v.size(); i++) {
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
  double min(const std::valarray<double> &v) {
    double min = v[0];
    for (size_t i = 1; i < v.size(); i++) {
      if (v[i] < min) {
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
  std::valarray<double> fabs(const std::valarray<double> &v) {
    std::valarray<double> result(v.size());
    for (size_t i = 0; i < v.size(); i++) {
      result[i] = std::fabs(v[i]);
    }
    return result;
  }

  /**
   * @brief Method to calculate reference points for a population.
   * @param population_interface
   * @param maxF The maximum fishing mortality to calculate reference points
   * for.
   * @param step The step size for the fishing mortality.
   * @return A list of reference points for the population.
   */
  Rcpp::List calculate_reference_points_population(
      PopulationInterface *population_interface, double maxF = 1.0,
      double step = 0.01) {
    //         //note: this algorithm is ported from the Meta-population
    //         assessment system project and
    //         //needs review

    Rcpp::List result;
    //         double spawning_season_offset = 0.0;
    //         std::shared_ptr<fims_info::Information<double>> info =
    //             fims_info::Information<double>::GetInstance();

    //         typename fims_info::Information<double>::population_iterator pit;

    //         pit = info->populations.find(population_interface->get_id());

    //         if (pit != info->populations.end())
    //         {
    //             std::shared_ptr<fims_popdy::Population<double>> &pop =
    //             (*pit).second;

    //             size_t year = pop->nyears - 1;
    //             size_t season = pop->nseasons - 1;
    //             size_t nages = pop->ages.size();

    //             std::vector<double> F;
    //             for (double f = 0.0; f <= maxF; f += step)
    //             {
    //                 F.push_back(f);
    //             }

    //             std::valarray<double> spr(F.size());       // equilibrium spr
    //             at F std::valarray<double> spr_ratio(F.size()); //
    //             equilibrium spr at F std::vector<double> S_eq(F.size()); //
    //             equilibrium SSB at F std::vector<double> R_eq(F.size()); //
    //             equilibrium recruitment at F std::vector<double>
    //             B_eq(F.size());        // equilibrium biomass at F
    //             std::vector<double> L_eq(F.size());        // equilibrium
    //             landings at F std::vector<double> D_eq(F.size());        //
    //             equilibrium dead discards at F std::vector<double>
    //             E_eq(F.size());        // equilibrium exploitation rate at F
    //             (landings only) std::valarray<double> L_eq_knum(F.size());
    //             std::valarray<double> SSB_eq(F.size());
    //             double spr_F0 = 0.0;

    //             std::vector<double> N0(pop->ages.size(), 1.0);
    //             for (int iage = 1; iage < nages; iage++)
    //             {
    //                 N0[iage] = N0[iage - 1] * std::exp(-1.0 * pop->M[iage -
    //                 1]);
    //             }
    //             N0[nages - 1] = N0[nages - 2] * std::exp(-1.0 * pop->M[nages
    //             - 2]) / (1.0 - std::exp(-1.0 * pop->M[nages - 1]));

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
    //                 reprod[a] =
    //                 pop->derived_quantities["weight_at_age"][index] *
    //                 (pop->derived_quantities["proportion_mature_at_age"][index]
    //                 * pop->proportion_female[0]); spr_F0 += N0[a] *
    //                 reprod[a]; selL[a] =
    //                 pop->derived_quantities["sum_selectivity"][index];
    //                 selZ[a] =
    //                 pop->derived_quantities["sum_selectivity"][index];
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
    //                     N_age[iage] = N_age[iage - 1] * std::exp(-1.0 *
    //                     Z_age[iage - 1]);
    //                 }

    //                 // last age is pooled
    //                 N_age[nages - 1] = N_age[nages - 2] * std::exp(-1.0 *
    //                 Z_age[nages - 2]) /
    //                                    (1.0 - std::exp(-1.0 * Z_age[nages -
    //                                    1]));

    //                 N_age_spawn = (N_age *
    //                                std::exp((-1.0 * Z_age *
    //                                spawning_season_offset)));

    //                 N_age_spawn[nages - 1] = (N_age_spawn[nages - 2] *
    //                 (std::exp(-1. * (Z_age[nages - 2] * (1.0 -
    //                 spawning_season_offset) +
    //                                                                                     Z_age[nages - 1] * spawning_season_offset)))) /
    //                                          (1.0 - std::exp(-1. *
    //                                          Z_age[nages - 1]));

    //                 spr[i] = sum(N_age * reprod);
    // #warning This is probably not correct
    //                 R_eq[i] = pop->recruitment->evaluate(spr[i], spr_F0);
    //                 // R_eq[i] = (R0 / ((5.0 * steep - 1.0) * spr[i])) *
    //                 //           (BC * 4.0 * steep * spr[i] - spr_F0 * (1.0 -
    //                 steep));
    //                 // R_eq[i] =
    //                 this->recruitment_model->CalculateEquilibriumRecruitment(
    //                 //
    //                 this->recruitment_model->CalculateEquilibriumSpawningBiomass(spr[i]));
    //                 //*1000*this->sex_fraction_value;

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
    //                                   (FL_age[iage] / Z_age[iage]) * (1. -
    //                                   std::exp(-1.0 * Z_age[iage]));
    //                     // D_age[iage] = N_age[iage] *
    //                     //               (FD_age[iage] / Z_age[iage]) * (1. -
    //                     exp(-1.0 * Z_age[iage]))
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

    //                 //                if (std::fabs(spr_ratio[i] - 0.001) ==
    //                 F01_dum) {
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
    //                     R_msy_out = R_eq[i] * 1000.0 *
    //                     pop->proportion_female[0]; msy_knum_out =
    //                     L_eq_knum[i]; F_msy_out = F[i]; spr_msy_out = spr[i];
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
    //             std::cout << "msy: " << F_msy_out * pop->proportion_female[0]
    //             << "\n"; std::cout << "spr_msy: " << spr[max_index] << "\n";
    //             std::cout << "SR_msy: " << spr_msy_out / spr_F0 << "\n";
    //             //                        std::cout << "D_msy_out" <<
    //             D_eq[max_index] << "\n"; std::cout << "R_msy: " <<
    //             R_eq[max_index] << "\n"; std::cout << "SSB_msy: " <<
    //             SSB_msy_out << "\n"; std::cout << "B_msy: " << B_msy_out <<
    //             "\n"; std::cout << "E_msy: " << E_eq[max_index] << "\n";
    //         }

    return result;
  }

  /**
   * @brief Method to calculate reference points for the model.
   */
  virtual Rcpp::List calculate_reference_points() {
    Rcpp::List result;
    // loop through populations for this model
    std::vector<uint32_t> pop_ids(this->population_ids->begin(),
                                  this->population_ids->end());
    for (size_t p = 0; p < pop_ids.size(); p++) {
      typename std::map<uint32_t,
                        std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
      pit = PopulationInterfaceBase::live_objects.find(pop_ids[p]);
      if (pit != PopulationInterfaceBase::live_objects.end()) {
        PopulationInterface *pop = (PopulationInterface *)(*pit).second.get();
        result.push_back(this->calculate_reference_points_population(pop));
      }
    }
    return result;
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::CatchAtAge<Type>> model =
        std::make_shared<fims_popdy::CatchAtAge<Type>>();

    model->id = this->id;
    population_id_iterator it;

    for (it = this->population_ids->begin(); it != this->population_ids->end();
         ++it) {
      model->AddPopulation((*it));
    }

    std::set<uint32_t> fleet_ids;  // all fleets in the model
    typedef typename std::set<uint32_t>::iterator fleet_ids_iterator;

    // add to Information
    info->models_map[this->get_id()] = model;

    for (it = this->population_ids->begin(); it != this->population_ids->end();
         ++it) {
      std::shared_ptr<PopulationInterface> population =
          std::dynamic_pointer_cast<PopulationInterface>(
              PopulationInterfaceBase::live_objects[(*it)]);

      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          model->population_derived_quantities[(*it)];

      // replace elements in the variable map
      info->variable_map[population->numbers_at_age.id_m] =
          &(derived_quantities["numbers_at_age"]);

      for (fleet_ids_iterator fit = population->fleet_ids->begin();
           fit != population->fleet_ids->end(); ++fit) {
        fleet_ids.insert(*fit);
      }
    }

    for (fleet_ids_iterator it = fleet_ids.begin(); it != fleet_ids.end();
         ++it) {
      std::shared_ptr<FleetInterface> fleet_interface =
          std::dynamic_pointer_cast<FleetInterface>(
              FleetInterfaceBase::live_objects[(*it)]);

      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          model->fleet_derived_quantities[fleet_interface->id];

      // initialize derive quantities
      // landings
      derived_quantities["landings_numbers_at_age"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());

      derived_quantities["landings_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());

      derived_quantities["landings_numbers_at_length"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nlengths.get());

      derived_quantities["landings_weight"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["landings_numbers"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["landings_expected"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["log_landings_expected"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["agecomp_proportion"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());

      derived_quantities["lengthcomp_proportion"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nlengths.get());
      // index
      derived_quantities["index_numbers_at_age"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());

      derived_quantities["index_weight_at_age"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());

      derived_quantities["index_numbers_at_length"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nlengths.get());

      derived_quantities["index_weight"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["index_numbers"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["index_expected"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["log_index_expected"] =
          fims::Vector<Type>(fleet_interface->nyears.get());
      //
      derived_quantities["catch_index"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["expected_catch"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["expected_index"] =
          fims::Vector<Type>(fleet_interface->nyears.get());

      derived_quantities["agecomp_expected"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nages.get());

      derived_quantities["lengthcomp_expected"] = fims::Vector<Type>(
          fleet_interface->nyears.get() * fleet_interface->nlengths.get());

      if (fleet_interface->nlengths.get() > 0) {
        derived_quantities["age_to_length_conversion"] = fims::Vector<Type>(
            fleet_interface->nyears.get() * fleet_interface->nlengths.get());
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
      if (fleet_interface->nlengths.get() > 0) {
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

  virtual bool add_to_fims_tmb() {
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

/**
 * @brief The SurplusProduction class is used to interface with the
 * SurplusProduction model. It inherits from the FisheryModelInterfaceBase
 * class.
 */
class SurplusProductionInterface : public FisheryModelInterfaceBase {
  typedef typename std::set<uint32_t>::iterator population_id_iterator;

 public:
  /**
   * @brief The constructor.
   */
  SurplusProductionInterface() : FisheryModelInterfaceBase() {
    this->population_ids = std::make_shared<std::set<uint32_t>>();
    std::shared_ptr<SurplusProductionInterface> surplus_production =
        std::make_shared<SurplusProductionInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(surplus_production);
    FisheryModelInterfaceBase::live_objects[this->id] = surplus_production;
  }

  /**
   * @brief Construct a new Surplus Production Interface object
   *
   * @param other
   */
  SurplusProductionInterface(const SurplusProductionInterface &other)
      : FisheryModelInterfaceBase(other) {}

  /**
   * Method to add a population id to the set of population ids.
   */
  void AddPopulation(uint32_t id) {
    this->population_ids->insert(id);
    Rcout << "Added population ID " << id << " to SurplusProductionInterface" << std::endl;
    std::map<uint32_t, std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
    pit = PopulationInterfaceBase::live_objects.find(id);
    if (pit != PopulationInterfaceBase::live_objects.end()) {
      std::shared_ptr<PopulationInterfaceBase> &pop = (*pit).second;
      pop->initialize_surplus_production.set(true);
    } else {
      FIMS_ERROR_LOG("Population with id " + fims::to_string(id) +
                     " not found.");
    }
  }

  /**
   * @brief Method to get the population id.
   */
  virtual uint32_t get_id() {
    typename std::map<uint32_t,
                      std::shared_ptr<PopulationInterfaceBase>>::iterator pit;
    return this->id;
  }

  /**
   * @brief Method to get the population ids.
   */
  void Show() {
    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_popdy::SurplusProduction<double> *model =
        (fims_popdy::SurplusProduction<double> *)info
            ->models_map[this->get_id()]
            .get();
    model->Show();
  }

  /**
   *
   */
  virtual void finalize() {}

  // TODO: implement population_to_json(), fleet_to_json()
  //  and to_json() methods

#ifdef TMB_MODEL
  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::SurplusProduction<Type>> model =
        std::make_shared<fims_popdy::SurplusProduction<Type>>();
    model->id = this->id;
    population_id_iterator it;
    for (it = this->population_ids->begin(); it != this->population_ids->end();
         ++it) {
      model->AddPopulation((*it));
    }

    std::set<uint32_t> fleet_ids;  // all fleets in the model
    typedef typename std::set<uint32_t>::iterator fleet_ids_iterator;

    // add to Information
    info->models_map[this->get_id()] = model;

    for (it = this->population_ids->begin(); it != this->population_ids->end();
         ++it) {
      std::shared_ptr<PopulationInterface> population =
          std::dynamic_pointer_cast<PopulationInterface>(
              PopulationInterfaceBase::live_objects[(*it)]);

      std::map<std::string, fims::Vector<Type>> &derived_quantities =
          model->population_derived_quantities[(*it)];

      derived_quantities["biomass"] =
          fims::Vector<Type>(population->nyears.get() + 1);

      derived_quantities["expected_depletion"] = fims::Vector<Type>(
          population->nyears.get() * population->nfleets.get());

      derived_quantities["observed_catch"] = fims::Vector<Type>(
          population->nyears.get() * population->nfleets.get());
    }

    return true;
  }

  virtual bool add_to_fims_tmb() {
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
