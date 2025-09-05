/**
 * @file rcpp_fleet.hpp
 * @brief The Rcpp interface to declare fleets. Allows for the use of
 * methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP

#include "../../../common/def.hpp"
#include "../../../population_dynamics/fleet/fleet.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp fleet
 * interfaces. This type should be inherited and not called from R directly.
 */
class FleetInterfaceBase : public FIMSRcppInterfaceBase
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
  static std::map<uint32_t, std::shared_ptr<FleetInterfaceBase>> live_objects;

  /**
   * @brief The constructor.
   */
  FleetInterfaceBase()
  {
    this->id = FleetInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    FleetInterfaceBase */
    // FleetInterfaceBase::live_objects[this->id] = this;
  }

  /**
   * @brief Construct a new Fleet Interface Base object
   *
   * @param other
   */
  FleetInterfaceBase(const FleetInterfaceBase &other) : id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~FleetInterfaceBase() {}

  /**
   * @brief Get the ID for the child fleet interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;
};
// static id of the FleetInterfaceBase object
uint32_t FleetInterfaceBase::id_g = 1;
// local id of the FleetInterfaceBase object map relating the ID of the
// FleetInterfaceBase to the FleetInterfaceBase objects
std::map<uint32_t, std::shared_ptr<FleetInterfaceBase>>
    FleetInterfaceBase::live_objects;
/**
 * @brief The Rcpp interface for Fleet to instantiate from R:
 * fleet <- methods::new(Fleet)
 */
class FleetInterface : public FleetInterfaceBase
{
  /**
   * @brief The ID of the observed age-composition data object.
   */
  SharedInt interface_observed_agecomp_data_id_m = -999;
  /**
   * @brief The ID of the observed length-composition data object.
   */
  SharedInt interface_observed_lengthcomp_data_id_m = -999;
  /**
   * @brief The ID of the observed index data object.
   */
  SharedInt interface_observed_index_data_id_m = -999;
  /**
   * @brief The ID of the observed landings data object.
   */
  SharedInt interface_observed_landings_data_id_m = -999;
  /**
   * @brief The ID of the selectivity object.
   */
  SharedInt interface_selectivity_id_m = -999;

public:
  /**
   * @brief The name of the fleet.
   */
  SharedString name = fims::to_string("NA");
  /**
   * @brief The number of age bins in the fleet data.
   */
  SharedInt nages = 0;
  /**
   * @brief The number of length bins in the fleet data.
   */
  SharedInt nlengths = 0;
  /**
   * @brief The number of years in the fleet data.
   */
  SharedInt nyears = 0;
  /**
   * @brief What units are the observed landings for this fleet measured in.
   * Options are weight or numbers, default is weight.
   */
  SharedString observed_landings_units = fims::to_string("weight");
  /**
   * @brief What units is the observed index of abundance for this fleet
   * measured in. Options are weight or numbers, default is weight.
   */
  SharedString observed_index_units = fims::to_string("weight");
  /**
   * @brief The natural log of the index of abundance scaling parameter
   * for this fleet.
   */
  ParameterVector log_q;
  /**
   * @brief The vector of the natural log of fishing mortality rates for this
   * fleet.
   */
  ParameterVector log_Fmort;
  /**
   * @brief The vector of natural log of the expected total landings for
   * the fleet.
   */
  ParameterVector log_landings_expected;
  /**
   * @brief The vector of natural log of the expected index of abundance
   * for the fleet.
   */
  ParameterVector log_index_expected;
  /**
   * @brief The vector of expected landings-at-age in numbers for the fleet.
   */
  ParameterVector agecomp_expected;
  /**
   * @brief The vector of expected landings-at-length in numbers for the fleet.
   */
  ParameterVector lengthcomp_expected;
  /**
   * @brief The vector of expected landings-at-age in numbers for the fleet.
   */
  ParameterVector agecomp_proportion;
  /**
   * @brief The vector of expected landings-at-length in numbers for the fleet.
   */
  ParameterVector lengthcomp_proportion;
  /**
   * @brief The vector of conversions to go from age to length, i.e., the
   * age-to-length-conversion matrix.
   */
  ParameterVector age_to_length_conversion;

  // derived quantities
  /**
   * @brief Derived landings-at-age in numbers.
   */
  Rcpp::NumericVector derived_landings_naa;
  /**
   * @brief Derived landings-at-length in numbers.
   */
  Rcpp::NumericVector derived_landings_nal;
  /**
   * @brief Derived landings-at-age in weight (mt).
   */
  Rcpp::NumericVector derived_landings_waa;
  /**
   * @brief Derived landings in observed units.
   */
  Rcpp::NumericVector derived_landings_expected;
  /**
   * @brief Derived landings in weight.
   */
  Rcpp::NumericVector derived_landings_w;
  /**
   * @brief Derived landings in numbers.
   */
  Rcpp::NumericVector derived_landings_n;
  /**
   * @brief Derived landings-at-age in numbers.
   */
  Rcpp::NumericVector derived_index_naa;
  /**
   * @brief Derived landings-at-length in numbers.
   */
  Rcpp::NumericVector derived_index_nal;
  /**
   * @brief Derived landings-at-age in weight (mt).
   */
  Rcpp::NumericVector derived_index_waa;
  /**
   * @brief Derived index in observed units.
   */
  Rcpp::NumericVector derived_index_expected;
  /**
   * @brief Derived index in weight.
   */
  Rcpp::NumericVector derived_index_w;
  /**
   * @brief Derived index in numbers.
   */
  Rcpp::NumericVector derived_index_n;
  /**
   * @brief Derived age composition proportions.
   */
  Rcpp::NumericVector derived_agecomp_proportion;
  /**
   * @brief Derived length composition proportions.
   */
  Rcpp::NumericVector derived_lengthcomp_proportion;
  /**
   * @brief Derived age compositions.
   */
  Rcpp::NumericVector derived_agecomp_expected;
  /**
   * @brief Derived length compositions.
   */
  Rcpp::NumericVector derived_lengthcomp_expected;

  /**
   * @brief The constructor.
   */
  FleetInterface() : FleetInterfaceBase()
  {
    std::shared_ptr<FleetInterface> fleet =
        std::make_shared<FleetInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(fleet);
    /* Create instance of map: key is id and value is pointer to
     FleetInterfaceBase */
    FleetInterfaceBase::live_objects[this->id] = fleet;
  }

  /**
   * @brief Construct a new Fleet Interface object
   *
   * @param other
   */
  FleetInterface(const FleetInterface &other)
      : FleetInterfaceBase(other),
        interface_observed_agecomp_data_id_m(
            other.interface_observed_agecomp_data_id_m),
        interface_observed_lengthcomp_data_id_m(
            other.interface_observed_lengthcomp_data_id_m),
        interface_observed_index_data_id_m(
            other.interface_observed_index_data_id_m),
        interface_observed_landings_data_id_m(
            other.interface_observed_landings_data_id_m),
        interface_selectivity_id_m(other.interface_selectivity_id_m),
        name(other.name),
        nages(other.nages),
        nlengths(other.nlengths),
        nyears(other.nyears),
        log_q(other.log_q),
        log_Fmort(other.log_Fmort),
        log_index_expected(other.log_index_expected),
        log_landings_expected(other.log_landings_expected),
        agecomp_proportion(other.agecomp_proportion),
        lengthcomp_proportion(other.lengthcomp_proportion),
        agecomp_expected(other.agecomp_expected),
        lengthcomp_expected(other.lengthcomp_expected),
        age_to_length_conversion(other.age_to_length_conversion),
        observed_landings_units(other.observed_landings_units),
        observed_index_units(other.observed_index_units),
        derived_landings_naa(other.derived_landings_naa),
        derived_landings_nal(other.derived_landings_nal),
        derived_landings_waa(other.derived_landings_waa),
        derived_index_expected(other.derived_index_expected),
        derived_index_w(other.derived_index_w),
        derived_index_n(other.derived_index_n),
        derived_landings_expected(other.derived_landings_expected),
        derived_landings_w(other.derived_landings_w),
        derived_landings_n(other.derived_landings_n),
        derived_agecomp_proportion(other.derived_agecomp_proportion),
        derived_lengthcomp_proportion(other.derived_lengthcomp_proportion),
        derived_agecomp_expected(other.derived_agecomp_expected),
        derived_lengthcomp_expected(other.derived_lengthcomp_expected) {}

  /**
   * @brief The destructor.
   */
  virtual ~FleetInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Sets the name of the fleet.
   * @param name The name to set.
   */
  void SetName(const std::string &name) { this->name.set(name); }

  /**
   * @brief Gets the name of the fleet.
   * @return The name.
   */
  std::string GetName() const { return this->name.get(); }

  /**
   * @brief Set the unique ID for the observed age-composition data object.
   * @param observed_agecomp_data_id Unique ID for the observed data object.
   */
  void SetObservedAgeCompDataID(int observed_agecomp_data_id)
  {
    interface_observed_agecomp_data_id_m.set(observed_agecomp_data_id);
  }

  /**
   * @brief Set the unique ID for the observed length-composition data object.
   * @param observed_lengthcomp_data_id Unique ID for the observed data object.
   */
  void SetObservedLengthCompDataID(int observed_lengthcomp_data_id)
  {
    interface_observed_lengthcomp_data_id_m.set(observed_lengthcomp_data_id);
  }

  /**
   * @brief Set the unique ID for the observed index data object.
   * @param observed_index_data_id Unique ID for the observed data object.
   */
  void SetObservedIndexDataID(int observed_index_data_id)
  {
    interface_observed_index_data_id_m.set(observed_index_data_id);
  }

  /**
   * @brief Set the unique ID for the observed landings data object.
   * @param observed_landings_data_id Unique ID for the observed data object.
   */
  void SetObservedLandingsDataID(int observed_landings_data_id)
  {
    interface_observed_landings_data_id_m.set(observed_landings_data_id);
  }
  /**
   * @brief Set the unique ID for the selectivity object.
   * @param selectivity_id Unique ID for the observed object.
   */
  void SetSelectivityID(int selectivity_id)
  {
    interface_selectivity_id_m.set(selectivity_id);
  }

  uint32_t GetSelectivityID()
  {
    return interface_selectivity_id_m.get();
  }

  /**
   * @brief Get the unique ID for the observed age-composition data object.
   */
  int GetObservedAgeCompDataID()
  {
    return interface_observed_agecomp_data_id_m.get();
  }

  /**
   * @brief Get the unique ID for the observed length-composition data
   * object.
   */
  int GetObservedLengthCompDataID()
  {
    return interface_observed_lengthcomp_data_id_m.get();
  }

  /**
   * @brief Get the unique id for the observed index data object.
   */
  int GetObservedIndexDataID()
  {
    return interface_observed_index_data_id_m.get();
  }

  /**
   * @brief Get the unique id for the observed landings data object.
   */
  int GetObservedLandingsDataID()
  {
    return interface_observed_landings_data_id_m.get();
  }
  /**
   * @brief Extracts the derived quantities from `Information` to the Rcpp
   * object.
   */
  virtual void finalize()
  {
    if (this->finalized)
    {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Fleet " + fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true; // indicate this has been called already

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::fleet_iterator it;

    it = info->fleets.find(this->id);

    if (it == info->fleets.end())
    {
      FIMS_WARNING_LOG("Fleet " + fims::to_string(this->id) +
                       " not found in Information.");
      return;
    }
    else
    {
      std::shared_ptr<fims_popdy::Fleet<double>> fleet =
          std::dynamic_pointer_cast<fims_popdy::Fleet<double>>(it->second);

      for (size_t i = 0; i < this->log_Fmort.size(); i++)
      {
        if (this->log_Fmort[i].estimation_type_m.get() == "constant")
        {
          this->log_Fmort[i].final_value_m = this->log_Fmort[i].initial_value_m;
        }
        else
        {
          this->log_Fmort[i].final_value_m = fleet->log_Fmort[i];
        }
      }

      for (size_t i = 0; i < this->log_q.size(); i++)
      {
        if (this->log_q[i].estimation_type_m.get() == "constant")
        {
          this->log_q[i].final_value_m = this->log_q[i].initial_value_m;
        }
        else
        {
          this->log_q[i].final_value_m = fleet->log_q[i];
        }
      }

      for (size_t i = 0; i < fleet->age_to_length_conversion.size(); i++)
      {
        if (this->age_to_length_conversion[i].estimation_type_m.get() ==
            "constant")
        {
          this->age_to_length_conversion[i].final_value_m =
              this->age_to_length_conversion[i].initial_value_m;
        }
        else
        {
          this->age_to_length_conversion[i].final_value_m =
              fleet->age_to_length_conversion[i];
        }
      }

      this->derived_landings_naa =
          Rcpp::NumericVector(fleet->landings_numbers_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_landings_naa.size(); i++)
      {
        this->derived_landings_naa[i] = fleet->landings_numbers_at_age[i];
      }

      this->derived_landings_nal =
          Rcpp::NumericVector(fleet->landings_numbers_at_length.size());
      for (R_xlen_t i = 0; i < this->derived_landings_nal.size(); i++)
      {
        this->derived_landings_nal[i] = fleet->landings_numbers_at_length[i];
      }

      this->derived_landings_waa =
          Rcpp::NumericVector(fleet->landings_weight_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_landings_waa.size(); i++)
      {
        this->derived_landings_waa[i] = fleet->landings_weight_at_age[i];
      }

      this->derived_index_naa =
          Rcpp::NumericVector(fleet->index_numbers_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_index_naa.size(); i++)
      {
        this->derived_index_naa[i] = fleet->index_numbers_at_age[i];
      }

      this->derived_index_nal =
          Rcpp::NumericVector(fleet->index_numbers_at_length.size());
      for (R_xlen_t i = 0; i < this->derived_index_nal.size(); i++)
      {
        this->derived_index_nal[i] = fleet->index_numbers_at_length[i];
      }

      this->derived_index_waa =
          Rcpp::NumericVector(fleet->index_weight_at_age.size());
      for (R_xlen_t i = 0; i < this->derived_index_waa.size(); i++)
      {
        this->derived_index_waa[i] = fleet->index_weight_at_age[i];
      }

      this->derived_index_w = Rcpp::NumericVector(fleet->index_weight.size());
      for (R_xlen_t i = 0; i < this->derived_index_w.size(); i++)
      {
        this->derived_index_w[i] = fleet->index_weight[i];
      }

      this->derived_index_n = Rcpp::NumericVector(fleet->index_numbers.size());
      for (R_xlen_t i = 0; i < this->derived_index_n.size(); i++)
      {
        this->derived_index_n[i] = fleet->index_numbers[i];
      }

      this->derived_index_expected =
          Rcpp::NumericVector(fleet->index_expected.size());
      for (R_xlen_t i = 0; i < this->derived_index_expected.size(); i++)
      {
        this->derived_index_expected[i] = fleet->index_expected[i];
      }

      this->derived_landings_expected =
          Rcpp::NumericVector(fleet->landings_expected.size());
      for (R_xlen_t i = 0; i < this->derived_landings_expected.size(); i++)
      {
        this->derived_landings_expected[i] = fleet->landings_expected[i];
      }

      this->derived_landings_w =
          Rcpp::NumericVector(fleet->landings_weight.size());
      for (R_xlen_t i = 0; i < this->derived_landings_w.size(); i++)
      {
        this->derived_landings_w[i] = fleet->landings_weight[i];
      }

      this->derived_landings_n =
          Rcpp::NumericVector(fleet->landings_numbers.size());
      for (R_xlen_t i = 0; i < this->derived_landings_n.size(); i++)
      {
        this->derived_landings_n[i] = fleet->landings_numbers[i];
      }

      this->derived_agecomp_proportion =
          Rcpp::NumericVector(fleet->agecomp_proportion.size());
      for (R_xlen_t i = 0; i < this->derived_agecomp_proportion.size(); i++)
      {
        this->derived_agecomp_proportion[i] = fleet->agecomp_proportion[i];
      }

      this->derived_lengthcomp_proportion =
          Rcpp::NumericVector(fleet->lengthcomp_proportion.size());
      for (R_xlen_t i = 0; i < this->derived_lengthcomp_proportion.size();
           i++)
      {
        this->derived_lengthcomp_proportion[i] =
            fleet->lengthcomp_proportion[i];
      }

      this->derived_agecomp_expected =
          Rcpp::NumericVector(fleet->agecomp_expected.size());
      for (R_xlen_t i = 0; i < this->derived_agecomp_expected.size(); i++)
      {
        this->derived_agecomp_expected[i] = fleet->agecomp_expected[i];
      }

      this->derived_lengthcomp_expected =
          Rcpp::NumericVector(fleet->lengthcomp_expected.size());
      for (R_xlen_t i = 0; i < this->derived_lengthcomp_expected.size(); i++)
      {
        this->derived_lengthcomp_expected[i] = fleet->lengthcomp_expected[i];
      }
    }
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * fleet interface. It returns the name and ID as well as all derived
   * quantities and parameter estimates. This string is formatted for a json
   * file.
   */
  virtual std::string to_json()
  {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"name\" : \"Fleet\",\n";

    ss << " \"type\" : \"fleet\",\n";
    ss << " \"tag\" : \"" << this->name.get() << "\",\n";
    ss << " \"id\": " << this->id << ",\n";
    ss << " \"nlengths\": " << this->nlengths.get() << ",\n";
    ss << "\"parameters\": [\n";
    ss << "{\n";
    ss << " \"name\": \"log_Fmort\",\n";
    ss << " \"id\":" << this->log_Fmort.id_m << ",\n";
    ss << " \"type\": \"vector\",\n";
    ss << " \"values\": " << this->log_Fmort << "\n},\n";

    ss << " {\n";
    ss << " \"name\": \"log_q\",\n";
    ss << " \"id\":" << this->log_q.id_m << ",\n";
    ss << " \"type\": \"vector\",\n";
    ss << " \"values\": " << this->log_q << "\n}";
    if (this->nlengths.get() > 0)
    {
      ss << " ,\n";
      ss << " {\n";
      ss << " \"name\": \"age_to_length_conversion\",\n";
      ss << " \"id\":" << this->age_to_length_conversion.id_m << ",\n";
      ss << " \"type\": \"vector\",\n";
      ss << " \"values\": " << this->age_to_length_conversion << "\n}";
    }
    ss << "\n],\n \"derived_quantities\":[\n";
    ss << "{\n";
    ss << "  \"name\": \"landings_naa\",\n";
    ss << "  \"values\":[";
    if (this->derived_landings_naa.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_landings_naa.size() - 1; i++)
      {
        ss << this->derived_landings_naa[i] << ", ";
      }
      ss << this->derived_landings_naa[this->derived_landings_naa.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << " {\n";
    ss << "  \"name\": \"landings_nal\",\n";
    ss << "  \"values\":[";
    if (this->derived_landings_nal.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_landings_nal.size() - 1; i++)
      {
        ss << this->derived_landings_nal[i] << ", ";
      }
      ss << this->derived_landings_nal[this->derived_landings_nal.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << " {\n";
    ss << "  \"name\": \"landings_waa\",\n";
    ss << "  \"values\":[";
    if (this->derived_landings_waa.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_landings_waa.size() - 1; i++)
      {
        ss << this->derived_landings_waa[i] << ", ";
      }
      ss << this->derived_landings_waa[this->derived_landings_waa.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << " {\n";
    ss << "  \"name\": \"index_naa\",\n";
    ss << "  \"values\":[";
    if (this->derived_index_naa.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_index_naa.size() - 1; i++)
      {
        ss << this->derived_index_naa[i] << ", ";
      }
      ss << this->derived_index_naa[this->derived_index_naa.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << " {\n";
    ss << "  \"name\": \"index_nal\",\n";
    ss << "  \"values\":[";
    if (this->derived_index_nal.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_index_nal.size() - 1; i++)
      {
        ss << this->derived_index_nal[i] << ", ";
      }
      ss << this->derived_index_nal[this->derived_index_nal.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << " {\n";
    ss << "  \"name\": \"index_waa\",\n";
    ss << "  \"values\":[";
    if (this->derived_index_waa.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_index_waa.size() - 1; i++)
      {
        ss << this->derived_index_waa[i] << ", ";
      }
      ss << this->derived_index_waa[this->derived_index_waa.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << "{\n";
    ss << "  \"name\": \"agecomp_expected \",\n";
    ss << "  \"values\":[";
    if (this->derived_agecomp_expected.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_agecomp_expected.size() - 1; i++)
      {
        ss << this->derived_agecomp_expected[i] << ", ";
      }
      ss << this->derived_agecomp_expected
                [this->derived_agecomp_expected.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << " {\n";
    ss << "  \"name\": \"lengthcomp_expected \",\n";
    ss << "  \"values\":[";
    if (this->derived_lengthcomp_expected.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_lengthcomp_expected.size() - 1;
           i++)
      {
        ss << this->derived_lengthcomp_expected[i] << ", ";
      }
      ss << this->derived_lengthcomp_expected
                [this->derived_lengthcomp_expected.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << "{\n";
    ss << "  \"name\": \"agecomp_proportion \",\n";
    ss << "  \"values\":[";
    if (this->derived_agecomp_proportion.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_agecomp_proportion.size() - 1;
           i++)
      {
        ss << this->derived_agecomp_proportion[i] << ", ";
      }
      ss << this->derived_agecomp_proportion
                [this->derived_agecomp_proportion.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << " {\n";
    ss << "  \"name\": \"lengthcomp_proportion \",\n";
    ss << "  \"values\":[";
    if (this->derived_lengthcomp_proportion.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_lengthcomp_proportion.size() - 1;
           i++)
      {
        ss << this->derived_lengthcomp_proportion[i] << ", ";
      }
      ss << this->derived_lengthcomp_proportion
                [this->derived_lengthcomp_proportion.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << "{\n";
    ss << "  \"name\": \"index_expected\",\n";
    ss << "  \"values\":[";
    if (this->derived_index_expected.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_index_expected.size() - 1; i++)
      {
        ss << this->derived_index_expected[i] << ", ";
      }
      ss << this->derived_index_expected[this->derived_index_expected.size() -
                                         1]
         << "]\n";
    }
    ss << " },\n";

    ss << "{\n";
    ss << "  \"name\": \"index_weight\",\n";
    ss << "  \"values\":[";
    if (this->derived_index_w.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_index_w.size() - 1; i++)
      {
        ss << this->derived_index_w[i] << ", ";
      }
      ss << this->derived_index_w[this->derived_index_w.size() - 1] << "]\n";
    }
    ss << " },\n";

    ss << "{\n";
    ss << "  \"name\": \"index_numbers\",\n";
    ss << "  \"values\":[";
    if (this->derived_index_n.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_index_n.size() - 1; i++)
      {
        ss << this->derived_index_n[i] << ", ";
      }
      ss << this->derived_index_n[this->derived_index_n.size() - 1] << "]\n";
    }
    ss << " },\n";

    ss << "{\n";
    ss << "  \"name\": \"landings_expected\",\n";
    ss << "  \"values\":[";
    if (this->derived_landings_expected.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_landings_expected.size() - 1;
           i++)
      {
        ss << this->derived_landings_expected[i] << ", ";
      }
      ss << this->derived_landings_expected
                [this->derived_landings_expected.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << "{\n";
    ss << "  \"name\": \"landings_weight\",\n";
    ss << "  \"values\":[";
    if (this->derived_landings_w.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_landings_w.size() - 1; i++)
      {
        ss << this->derived_landings_w[i] << ", ";
      }
      ss << this->derived_landings_w[this->derived_landings_w.size() - 1]
         << "]\n";
    }
    ss << " },\n";

    ss << "{\n";
    ss << "  \"name\": \"landings_numbers\",\n";
    ss << "  \"values\":[";
    if (this->derived_landings_n.size() == 0)
    {
      ss << "]\n";
    }
    else
    {
      for (R_xlen_t i = 0; i < this->derived_landings_n.size() - 1; i++)
      {
        ss << this->derived_landings_n[i] << ", ";
      }
      ss << this->derived_landings_n[this->derived_landings_n.size() - 1]
         << "]\n";
    }
    ss << " }\n]\n}";

    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal()
  {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::Fleet<Type>> fleet =
        std::make_shared<fims_popdy::Fleet<Type>>();

    std::stringstream ss;

    // set relative info
    fleet->id = this->id;
    fleet->nages = this->nages.get();
    fleet->nlengths = this->nlengths.get();
    fleet->nyears = this->nyears.get();
    fleet->observed_landings_units = this->observed_landings_units;
    fleet->observed_index_units = this->observed_index_units;

    fleet->fleet_observed_agecomp_data_id_m =
        interface_observed_agecomp_data_id_m.get();

    fleet->fleet_observed_lengthcomp_data_id_m =
        interface_observed_lengthcomp_data_id_m.get();

    fleet->fleet_observed_index_data_id_m =
        interface_observed_index_data_id_m.get();
    fleet->fleet_observed_landings_data_id_m =
        interface_observed_landings_data_id_m.get();

    fleet->fleet_selectivity_id_m = interface_selectivity_id_m.get();

    fleet->log_q.resize(this->log_q.size());
    for (size_t i = 0; i < this->log_q.size(); i++)
    {
      fleet->log_q[i] = this->log_q[i].initial_value_m;

      if (this->log_q[i].estimation_type_m.get() == "fixed_effects")
      {
        ss.str("");
        ss << "Fleet." << this->id << ".log_q." << this->log_q[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(fleet->log_q[i]);
      }
      if (this->log_q[i].estimation_type_m.get() == "random_effects")
      {
        ss.str("");
        ss << "Fleet." << this->id << ".log_q." << this->log_q[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(fleet->log_q[i]);
      }
    }

    FIMS_INFO_LOG("adding Fleet fmort object to TMB");
    fleet->log_Fmort.resize(this->log_Fmort.size());
    for (size_t i = 0; i < log_Fmort.size(); i++)
    {
      fleet->log_Fmort[i] = this->log_Fmort[i].initial_value_m;

      if (this->log_Fmort[i].estimation_type_m.get() == "fixed_effects")
      {
        ss.str("");
        ss << "Fleet." << this->id << ".log_Fmort." << this->log_Fmort[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(fleet->log_Fmort[i]);
      }
      if (this->log_Fmort[i].estimation_type_m.get() == "random_effects")
      {
        ss.str("");
        ss << "Fleet." << this->id << ".log_Fmort." << this->log_Fmort[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(fleet->log_Fmort[i]);
      }
    }
    // add to variable_map
    info->variable_map[this->log_Fmort.id_m] = &(fleet)->log_Fmort;

    // exp_landings
    fleet->log_landings_expected.resize(
        nyears); // assume landings is for all ages.
    info->variable_map[this->log_landings_expected.id_m] =
        &(fleet)->log_landings_expected;
    fleet->log_index_expected.resize(nyears); // assume index is for all ages.
    info->variable_map[this->log_index_expected.id_m] =
        &(fleet)->log_index_expected;

    fleet->agecomp_expected.resize(nyears.get() * nages.get());
    fleet->agecomp_proportion.resize(nyears.get() * nages.get());
    info->variable_map[this->agecomp_expected.id_m] =
        &(fleet)->agecomp_expected;
    info->variable_map[this->agecomp_proportion.id_m] =
        &(fleet)->agecomp_proportion;
    FIMS_INFO_LOG(fims::to_string(this->nyears.get()) + " " +
                  fims::to_string(this->nages.get()));
    FIMS_INFO_LOG(" adding Fleet length object to TMB");

    if (this->nlengths.get() > 0)
    {
      fleet->lengthcomp_expected.resize(this->nyears.get() *
                                        this->nlengths.get());
      fleet->lengthcomp_proportion.resize(this->nyears.get() *
                                          this->nlengths.get());
      fleet->age_to_length_conversion.resize(
          this->age_to_length_conversion.size());

      if (this->age_to_length_conversion.size() !=
          (this->nages.get() * this->nlengths.get()))
      {
        FIMS_ERROR_LOG(
            "age_to_length_conversion don't match, " +
            fims::to_string(this->age_to_length_conversion.size()) + " != " +
            fims::to_string((this->nages.get() * this->nlengths.get())));
      }

      for (size_t i = 0; i < fleet->age_to_length_conversion.size(); i++)
      {
        fleet->age_to_length_conversion[i] =
            this->age_to_length_conversion[i].initial_value_m;
        FIMS_INFO_LOG(" adding Fleet length object to TMB in loop " +
                      fims::to_string(i) + " of " +
                      fims::to_string(fleet->age_to_length_conversion.size()));

        if (this->age_to_length_conversion[i].estimation_type_m.get() ==
            "fixed_effects")
        {
          ss.str("");
          ss << "Fleet." << this->id << ".age_to_length_conversion."
             << this->age_to_length_conversion[i].id_m;
          info->RegisterParameterName(ss.str());
          info->RegisterParameter(fleet->age_to_length_conversion[i]);
        }
        if (this->age_to_length_conversion[i].estimation_type_m.get() ==
            "random_effects")
        {
          FIMS_ERROR_LOG(
              "age_to_length_conversion cannot be set to random effects");
        }
        FIMS_INFO_LOG(" adding Fleet length object to TMB in loop after if");
      }
      FIMS_INFO_LOG(" adding Fleet length object to TMB out loop");
      info->variable_map[this->age_to_length_conversion.id_m] =
          &(fleet)->age_to_length_conversion;
      info->variable_map[this->lengthcomp_expected.id_m] =
          &(fleet)->lengthcomp_expected;
      info->variable_map[this->lengthcomp_proportion.id_m] =
          &(fleet)->lengthcomp_proportion;
    }

    // add to Information
    info->fleets[fleet->id] = fleet;
    FIMS_INFO_LOG("done adding Fleet object to TMB");
    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb()
  {
#ifdef TMBAD_FRAMEWORK
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();
#else
    FIMS_INFO_LOG("adding Fleet object to TMB");
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
