#ifndef FIMS_MODELS_CATCH_AT_AGE_HPP
#define FIMS_MODELS_CATCH_AT_AGE_HPP

#include <set>
#include <regex>

#include "fishery_model_base.hpp"

namespace fims_popdy
{

  // TODO: add a function to compute length composition
  template <typename Type>
  /**
   * @brief CatchAtAge is a class containing a catch-at-age model, which is
   * just one of many potential fishery models that can be used in FIMS. The
   * CatchAtAge class inherits from the FisheryModelBase class and can be used
   * to fit both age and length data even though it is called CatchAtAge.
   *
   */
  class CatchAtAge : public FisheryModelBase<Type>
  {
  public:
    /**
     * @brief The name of the model.
     *
     */
    std::string name_m;

    /**
     * @brief Iterate the derived quantities.
     *
     */
    typedef typename std::map<std::string, fims::Vector<Type>>::iterator
        derived_quantities_iterator;

    /**
     * @brief Used to iterate through fleet-based derived quantities.
     *
     */
    typedef typename std::map<uint32_t,
                              std::map<std::string, fims::Vector<Type>>>::iterator
        fleet_derived_quantities_iterator;

    /**
     * @brief Used to iterate through fleet-based derived quantities dimensions.
     */
    typedef typename std::map<uint32_t, std::map<std::string, fims::Vector<size_t>>>::iterator
        fleet_derived_quantities_dims_iterator;
    /**
     * @brief Used to iterate through population-based derived quantities.
     *
     */
    typedef typename std::map<uint32_t,
                              std::map<std::string, fims::Vector<Type>>>::iterator
        population_derived_quantities_iterator;

    /**
     * @brief Used to iterate through population-based derived quantities dimensions.
     */
    typedef typename std::map<uint32_t, std::map<std::string, fims::Vector<size_t>>>::iterator
        population_derived_quantities_dims_iterator;

    /**
     * @brief Iterate through fleets.
     *
     */
    typedef typename std::map<uint32_t,
                              std::shared_ptr<fims_popdy::Fleet<Type>>>::iterator
        fleet_iterator;
    /**
     * @brief Iterate through derived quantities.
     *
     */
    typedef
        typename std::map<std::string, fims::Vector<Type>>::iterator dq_iterator;

  public:
    std::vector<Type> ages; /*!< vector of the ages for referencing*/
    /**
     * Constructor for the CatchAtAge class. This constructor initializes the
     * name of the model and sets the id of the model.
     */
    CatchAtAge() : FisheryModelBase<Type>()
    {
      std::stringstream ss;
      ss << "caa_" << this->GetId() << "_";
      this->name_m = ss.str();
      this->model_type_m = "caa";
    }

    /**
     * @brief Copy constructor for the CatchAtAge class.
     *
     * @param other The other CatchAtAge object to copy from.
     */
    CatchAtAge(const CatchAtAge &other)
        : FisheryModelBase<Type>(other), name_m(other.name_m),
          ages(other.ages)
    {
      this->model_type_m = "caa";
    }

    /**
     * @brief Destroy the Catch At Age object.
     *
     */
    virtual ~CatchAtAge() {}

    /**
     * This function is called once at the beginning of the model run. It
     * initializes the derived quantities for the populations and fleets.
     */
    virtual void Initialize()
    {
      //       // The following are initialized in the rcpp interface: ages, log_init_naa,
      //       //   numbers_at_age, log_Fmort, log_q
      for (size_t p = 0; p < this->populations.size(); p++)
      {
        //         std::map<std::string, fims::Vector<Type>> &derived_quantities =
        //             this->population_derived_quantities[this->populations[p]->GetId()];

        //         std::map<std::string, fims::Vector<size_t>> &derived_quantities_dims =
        //             this->population_derived_quantities_dims[this->populations[p]->GetId()];

        //         derived_quantities["total_landings_weight"] =
        //             fims::Vector<Type>(this->populations[p]->nyears);
        //         derived_quantities_dims["total_landings_weight"] =
        //             {static_cast<size_t>(this->populations[p]->nyears)};

        //         derived_quantities["total_landings_numbers"] =
        //             fims::Vector<Type>(this->populations[p]->nyears);
        //         derived_quantities_dims["total_landings_numbers"] =
        //             {static_cast<size_t>(this->populations[p]->nyears)};

        //         derived_quantities["mortality_F"] = fims::Vector<Type>(
        //             this->populations[p]->nyears * this->populations[p]->nages);

        //         derived_quantities_dims["mortality_F"] =
        //             {static_cast<size_t>(this->populations[p]->nyears),
        //              static_cast<size_t>(this->populations[p]->nages)};

        //         derived_quantities["mortality_Z"] = fims::Vector<Type>(
        //             this->populations[p]->nyears * this->populations[p]->nages);

        //         derived_quantities_dims["mortality_Z"] =
        //             {static_cast<size_t>(this->populations[p]->nyears),
        //              static_cast<size_t>(this->populations[p]->nages)};
        //         derived_quantities["weight_at_age"] =
        //             fims::Vector<Type>(this->populations[p]->nages);

        //         // TODO: numbers_at_age are resized in rcpp_population, should this be
        //         // removed?
        //         derived_quantities["numbers_at_age"] = fims::Vector<Type>(
        //             (this->populations[p]->nyears + 1) * this->populations[p]->nages);

        //         derived_quantities_dims["numbers_at_age"] =
        //             {static_cast<size_t>(this->populations[p]->nyears + 1),
        //              static_cast<size_t>(this->populations[p]->nages)};

        //         derived_quantities["unfished_numbers_at_age"] = fims::Vector<Type>(
        //             (this->populations[p]->nyears + 1) * this->populations[p]->nages);
        //         derived_quantities_dims["unfished_numbers_at_age"] =
        //             {static_cast<size_t>(this->populations[p]->nyears + 1),
        //              static_cast<size_t>(this->populations[p]->nages)};

        //         derived_quantities["biomass"] =
        //             fims::Vector<Type>(this->populations[p]->nyears + 1);
        //         derived_quantities_dims["biomass"] =
        //             {static_cast<size_t>(this->populations[p]->nyears + 1)};

        //         derived_quantities["spawning_biomass"] =
        //             fims::Vector<Type>(this->populations[p]->nyears + 1);
        //         derived_quantities_dims["spawning_biomass"] =
        //             {static_cast<size_t>(this->populations[p]->nyears + 1)};

        //         derived_quantities["unfished_biomass"] =
        //             fims::Vector<Type>(this->populations[p]->nyears + 1);
        //         derived_quantities_dims["unfished_biomass"] =
        //             {static_cast<size_t>(this->populations[p]->nyears + 1)};

        //         derived_quantities["unfished_spawning_biomass"] =
        //             fims::Vector<Type>(this->populations[p]->nyears + 1);
        //         derived_quantities_dims["unfished_spawning_biomass"] =
        //             {static_cast<size_t>(this->populations[p]->nyears + 1)};

        //         derived_quantities["proportion_mature_at_age"] = fims::Vector<Type>(
        //             (this->populations[p]->nyears + 1) * this->populations[p]->nages);
        // derived_quantities_dims["proportion_mature_at_age"] =
        //             {static_cast<size_t>(this->populations[p]->nyears + 1),
        //              static_cast<size_t>(this->populations[p]->nages)};

        //         derived_quantities["expected_recruitment"] =
        //             fims::Vector<Type>(this->populations[p]->nyears + 1);
        //             derived_quantities_dims["expected_recruitment"] =
        //                 {static_cast<size_t>(this->populations[p]->nyears + 1)};

        //         derived_quantities["sum_selectivity"] = fims::Vector<Type>(
        //             this->populations[p]->nyears * this->populations[p]->nages);
        // derived_quantities_dims["sum_selectivity"] =
        //             {static_cast<size_t>(this->populations[p]->nyears),
        //              static_cast<size_t>(this->populations[p]->nages)};

        this->populations[p]->proportion_female.resize(
            this->populations[p]->nages);

        this->populations[p]->M.resize(this->populations[p]->nyears *
                                       this->populations[p]->nages);
      }

      for (fleet_iterator fit = this->fleets.begin(); fit != this->fleets.end();
           ++fit)
      {
        std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
        //         std::map<std::string, fims::Vector<Type>> &derived_quantities =
        //             this->fleet_derived_quantities[fleet->GetId()];

        //             std::map<std::string, fims::Vector<size_t>> &derived_quantities_dims =
        //                 this->fleet_derived_quantities_dims[fleet->GetId()];

        //         // initialize derive quantities
        //         // landings
        //         derived_quantities["landings_numbers_at_age"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nages);
        //         derived_quantities_dims["landings_numbers_at_age"] =
        //             {static_cast<size_t>(fleet->nyears),
        //              static_cast<size_t>(fleet->nages)};

        //         derived_quantities["landings_weight_at_age"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nages);
        //         derived_quantities_dims["landings_weight_at_age"] =
        //             {static_cast<size_t>(fleet->nyears),
        //              static_cast<size_t>(fleet->nages)};

        //         derived_quantities["landings_numbers_at_length"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nlengths);
        //         derived_quantities_dims["landings_numbers_at_length"] =
        //             {static_cast<size_t>(fleet->nyears),
        //              static_cast<size_t>(fleet->nlengths)};

        //         derived_quantities["landings_weight"] = fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["landings_weight"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["landings_numbers"] =
        //             fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["landings_numbers"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["landings_expected"] =
        //             fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["landings_expected"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["log_landings_expected"] =
        //             fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["log_landings_expected"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["agecomp_proportion"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nages);
        //         derived_quantities_dims["agecomp_proportion"] =
        //             {static_cast<size_t>(fleet->nyears),
        //              static_cast<size_t>(fleet->nages)};

        //         derived_quantities["lengthcomp_proportion"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nlengths);
        //         derived_quantities_dims["lengthcomp_proportion"] =
        //             {static_cast<size_t>(fleet->nyears),
        //              static_cast<size_t>(fleet->nlengths)};

        //         // index
        //         derived_quantities["index_numbers_at_age"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nages);
        //             derived_quantities_dims["index_numbers_at_age"] =
        //                 {static_cast<size_t>(fleet->nyears),
        //                  static_cast<size_t>(fleet->nages)};

        //         derived_quantities["index_weight_at_age"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nages);
        //             derived_quantities_dims["index_weight_at_age"] =
        //                 {static_cast<size_t>(fleet->nyears),
        //                  static_cast<size_t>(fleet->nages)};

        //         derived_quantities["index_numbers_at_length"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nlengths);
        //         derived_quantities_dims["index_numbers_at_length"] =
        //             {static_cast<size_t>(fleet->nyears),
        //              static_cast<size_t>(fleet->nlengths)};

        //         derived_quantities["index_weight"] = fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["index_weight"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["index_numbers"] = fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["index_numbers"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["index_expected"] = fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["index_expected"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["log_index_expected"] =
        //             fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["log_index_expected"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         //
        //         derived_quantities["catch_index"] = fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["catch_index"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["expected_catch"] = fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["expected_catch"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["expected_index"] = fims::Vector<Type>(fleet->nyears);
        //         derived_quantities_dims["expected_index"] =
        //             {static_cast<size_t>(fleet->nyears)};

        //         derived_quantities["agecomp_expected"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nages);
        //         derived_quantities_dims["agecomp_expected"] =
        //             {static_cast<size_t>(fleet->nyears),
        //              static_cast<size_t>(fleet->nages)};

        //         derived_quantities["lengthcomp_expected"] =
        //             fims::Vector<Type>(fleet->nyears * fleet->nlengths);
        //         derived_quantities_dims["lengthcomp_expected"] =
        //             {static_cast<size_t>(fleet->nyears),
        //              static_cast<size_t>(fleet->nlengths)};

        //         if (fleet->nlengths > 0)
        //         {
        //           derived_quantities["age_to_length_conversion"] =
        //               fims::Vector<Type>(fleet->nages * fleet->nlengths);
        //           derived_quantities_dims["age_to_length_conversion"] =
        //               {static_cast<size_t>(fleet->nages),
        //                static_cast<size_t>(fleet->nlengths)};
        //         }

        if (fleet->log_q.size() == 0)
        {
          fleet->log_q.resize(1);
          fleet->log_q[0] = static_cast<Type>(0.0);
        }
        fleet->q.resize(fleet->log_q.size());
        fleet->Fmort.resize(fleet->nyears);
      }
    }



    /**
     * This function is used to convert the derived quantities of a population or
     * fleet to a JSON string. This function is used to create the JSON output for
     * the CatchAtAge model.
     */
    std::string DerivedQuantityToJSON(derived_quantities_iterator it)
    {
      fims::Vector<Type> &dq = (*it).second;
      std::stringstream ss;
      // ss << std::fixed;
      ss << "{\n";
      ss << "\"name\":\"" << (*it).first << "\",\n";
      ss << "\"value\":[";

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
    std::string fleet_derived_quantities_to_json(
        fleet_derived_quantities_iterator fdqit)
    {
      std::stringstream ss;
      // ss << std::fixed;

      // ss << "\"derived_quantities\": [\n";

      typename std::map<std::string, fims::Vector<Type>>::iterator it;
      typename std::map<std::string, fims::Vector<Type>>::iterator end_it;
      end_it = (*fdqit).second.end();
      typename std::map<std::string, fims::Vector<Type>>::iterator second_to_last;
      second_to_last = (*fdqit).second.end();
      if (it != end_it)
      {
        second_to_last--;
      }

      it = (*fdqit).second.begin();
      for (; it != second_to_last; ++it)
      {
        ss << this->DerivedQuantityToJSON(it) << ",\n";
      }
      //                    ++it;
      ss << this->DerivedQuantityToJSON(second_to_last) << "\n";

      return ss.str();
    }
    /**
     * @brief Send the population-based derived quantities to the json file.
     * @return std::string
     */
    std::string population_derived_quantities_to_json(
        population_derived_quantities_iterator pdqit)
    {
      std::stringstream ss;
      // ss << std::fixed;
      // ss << "{\n";
      // ss << "\"id\": " << (*pdqit).first << ",\n";
      // ss << "\"derived_quantities\": [\n";

      typename std::map<std::string, fims::Vector<Type>>::iterator it;
      typename std::map<std::string, fims::Vector<Type>>::iterator end_it;
      end_it = (*pdqit).second.end();
      typename std::map<std::string, fims::Vector<Type>>::iterator second_to_last;
      second_to_last = (*pdqit).second.end();
      if (it != end_it)
      {
        second_to_last--;
      }

      it = (*pdqit).second.begin();
      for (; it != second_to_last; ++it)
      {
        ss << this->DerivedQuantityToJSON(it) << ",\n";
      }
      //                    ++it;
      ss << this->DerivedQuantityToJSON(second_to_last) << "\n";

      return ss.str();
    }

    // /**
    //  * This function is used to convert the derived quantities of a
    //  * population or fleet to a JSON string.
    //  */
    // std::string ToJSON()
    // {

    // typename std::map<uint32_t, std::map<std::string,
    // fims::Vector<Type>>>::iterator pit; typename std::map<uint32_t,
    // std::map<std::string, fims::Vector<Type>>>::iterator end_pit; end_pit =
    // this->population_derived_quantities.end(); typename std::map<uint32_t,
    // std::map<std::string, fims::Vector<Type>>>::iterator second_to_last_pit;
    // second_to_last_pit = this->population_derived_quantities.end();
    // if (pit != end_pit)
    // {
    //     second_to_last_pit--;
    // }

    // std::stringstream ss;
    // ss << "{\n";
    // ss << "\"Populations\":[\n";
    // pit = this->population_derived_quantities.begin();
    // for (; pit != second_to_last_pit; pit++)
    // {
    //     ss << "{\n";
    //     ss << "\"id\": " << (*pit).first << ",\n";
    //     ss << "\"derived_quantities\": [\n";

    //     typename fims_popdy::Population<Type>::derived_quantities_iterator it;
    //     typename fims_popdy::Population<Type>::derived_quantities_iterator
    //     end_it; typename
    //     fims_popdy::Population<Type>::derived_quantities_iterator
    //     second_to_last;

    //     it = (*pit).second.begin();
    //     end_it = (*pit).second.end();
    //     second_to_last = (*pit).second.end();
    //     if (it != end_it)
    //     {
    //         second_to_last--;
    //     }

    //     for (; it != second_to_last; ++it)
    //     {
    //         ss << this->DerivedQuantityToJSON(it) << ",\n";
    //     }
    //     //                    ++it;
    //     ss << this->DerivedQuantityToJSON(second_to_last) << "\n]},\n";
    // }

    // ss << "{\n";
    // ss << "\"id\": " << (*second_to_last_pit).first << ",\n";
    // ss << "\"derived_quantities\": [\n";

    // typename fims_popdy::Population<Type>::derived_quantities_iterator it;
    // typename fims_popdy::Population<Type>::derived_quantities_iterator end_it;
    // typename fims_popdy::Population<Type>::derived_quantities_iterator
    // second_to_last;

    // it = (*second_to_last_pit).second.begin();
    // end_it = (*second_to_last_pit).second.end();
    // second_to_last = (*second_to_last_pit).second.end();
    // if (it != end_it)
    // {
    //     second_to_last--;
    // }

    // for (; it != second_to_last; ++it)
    // {
    //     ss << this->DerivedQuantityToJSON(it) << ",\n";
    // }
    // //                    ++it;
    // ss << this->DerivedQuantityToJSON(second_to_last) << "\n]}\n";

    // ss << "],\n";

    // typename std::map<uint32_t, std::map<std::string,
    // fims::Vector<Type>>>::iterator fit; typename std::map<uint32_t,
    // std::map<std::string, fims::Vector<Type>>>::iterator end_fit; end_fit =
    // this->fleet_derived_quantities.end(); typename std::map<uint32_t,
    // std::map<std::string, fims::Vector<Type>>>::iterator second_to_last_fit;
    // second_to_last_fit = this->fleet_derived_quantities.end();
    // if (fit != end_fit)
    // {
    //     second_to_last_fit--;
    // }
    // ss << "\"Fleets\":[\n";

    // fit = this->fleet_derived_quantities.begin();
    // for (; fit != second_to_last_fit; fit++)
    // {
    //     ss << "{\n";
    //     ss << "\"id\": " << (*fit).first << ",\n";
    //     ss << "\"derived_quantities\": [\n";
    //     typename fims_popdy::Fleet<Type>::derived_quantities_iterator it;
    //     typename fims_popdy::Fleet<Type>::derived_quantities_iterator end_it;
    //     typename fims_popdy::Fleet<Type>::derived_quantities_iterator
    //     second_to_last; it = (*fit).second.begin(); end_it =
    //     (*fit).second.end(); second_to_last = (*fit).second.end(); if (it !=
    //     end_it)
    //     {
    //         second_to_last--;
    //     }

    //     for (; it != second_to_last; ++it)
    //     {
    //         ss << this->DerivedQuantityToJSON(it) << ",\n";
    //     }
    //     //                    ++it;
    //     ss << this->DerivedQuantityToJSON(second_to_last) << "\n]},\n";
    // }

    // ss << "{\n";
    // ss << "\"id\": " << (*second_to_last_fit).first << ",\n";
    // ss << "\"derived_quantities\": [\n";
    // typename fims_popdy::Fleet<Type>::derived_quantities_iterator it_;
    // typename fims_popdy::Fleet<Type>::derived_quantities_iterator end_it_;
    // typename fims_popdy::Fleet<Type>::derived_quantities_iterator
    // second_to_last_; it_ = (*second_to_last_fit).second.begin(); end_it_ =
    // (*second_to_last_fit).second.end(); second_to_last_ =
    // (*second_to_last_fit).second.end(); if (it_ != end_it_)
    // {
    //     second_to_last--;
    // }

    // for (; it_ != second_to_last_; ++it_)
    // {
    //     ss << this->DerivedQuantityToJSON(it) << ",\n";
    // }
    // //                    ++it;
    // ss << this->DerivedQuantityToJSON(second_to_last_) << "\n]}\n";
    // ss << "]\n";
    // ss << "}\n";

    // return ss.str();
    // }

    /**
     * This function is used to reset the derived quantities of a population or
     * fleet to a given value.
     */
    virtual void Prepare()
    {
      for (size_t p = 0; p < this->populations.size(); p++)
      {
        std::shared_ptr<fims_popdy::Population<Type>> &population =
            this->populations[p];
        auto &derived_quantities =
            this->GetPopulationDerivedQuantities(population->GetId());

        // Reset the derived quantities for the population
        typename fims_popdy::Population<Type>::derived_quantities_iterator it;
        for (it = derived_quantities.begin(); it != derived_quantities.end();
             it++)
        {
          fims::Vector<Type> &dq = (*it).second;
          this->ResetVector(dq);
        }

        // Prepare proportion_female
        for (size_t age = 0; age < population->nages; age++)
        {
          population->proportion_female[age] = 0.5;
        }

        // Transformation Section
        for (size_t age = 0; age < population->nages; age++)
        {
          for (size_t year = 0; year < population->nyears; year++)
          {
            size_t i_age_year = age * population->nyears + year;
            population->M[i_age_year] =
                fims_math::exp(population->log_M[i_age_year]);
            // TODO: is this still needed now that derived quantities are filled
            // with ResetVector?
            // mortality_F is a fims::Vector and therefore needs
            // to be filled within a loop
            // derived_quantities["mortality_F"][i_age_year] = 0.0;
          }
        }
      }

      for (fleet_iterator fit = this->fleets.begin(); fit != this->fleets.end();
           ++fit)
      {
        std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
        std::map<std::string, fims::Vector<Type>> &derived_quantities =
            this->GetFleetDerivedQuantities(fleet->GetId());
        typename fims_popdy::Population<Type>::derived_quantities_iterator it;

        for (it = derived_quantities.begin(); it != derived_quantities.end();
             it++)
        {
          fims::Vector<Type> &dq = (*it).second;
          this->ResetVector(dq);
        }

        // Transformation Section
        for (size_t i = 0; i < fleet->log_q.size(); i++)
        {
          fleet->q[i] = fims_math::exp(fleet->log_q[i]);
        }

        for (size_t year = 0; year < fleet->nyears; year++)
        {
          fleet->Fmort[year] = fims_math::exp(fleet->log_Fmort[year]);
        }

        // // TODO: does this age_length_to_conversion need to be a dq and parameter
        // // of fleet?
        // for (size_t i_length_age = 0;
        //      i_length_age < fleet->age_to_length_conversion.size();
        //      i_length_age++)
        // {
        //   derived_quantities["age_to_length_conversion"][i_length_age] =
        //       fleet->age_to_length_conversion[i_length_age];
        // }
      }
    }
    /**
     * This function is used to add a population id to the set of population ids.
     */
    void AddPopulation(uint32_t id) { this->population_ids.insert(id); }

    /**
     * @brief Get the population ids of the model.
     */
    std::set<uint32_t> &GetPopulationIds() { return this->population_ids; }

    /**
     * This function is used to get the populations of the model. It returns a
     * vector of shared pointers to the populations.
     * @return std::vector<std::shared_ptr<fims_popdy::Population<Type>>>&
     */
    std::vector<std::shared_ptr<fims_popdy::Population<Type>>> &GetPopulations()
    {
      return this->populations;
    }

    /**
     * This method is used to calculate the initial numbers at age for a
     * population. It takes a population object and an age as input and
     * calculates the initial numbers at age for that population.
     * @param population
     * @param i_age_year
     * @param a
     */
    void CalculateInitialNumbersAA(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t a)
    {
      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      dq_["numbers_at_age"][i_age_year] =
          fims_math::exp(population->log_init_naa[a]);
    }

    /**
     * * This method is used to calculate the numbers at age for a
     * population. It takes a population object, the index of the age
     * in the current year, the index of the age in the previous year,
     * and the age as input and calculates the numbers at age for that
     * population.
     * @param population
     * @param i_age_year
     * @param i_agem1_yearm1
     * @param age
     */
    void CalculateNumbersAA(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t i_agem1_yearm1, size_t age)
    {
      // using Z from previous age/year

      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      dq_["numbers_at_age"][i_age_year] =
          dq_["numbers_at_age"][i_agem1_yearm1] *
          (fims_math::exp(
              -dq_["mortality_Z"]
                  [i_agem1_yearm1]));

      // Plus group calculation
      if (age == (population->nages - 1))
      {
        dq_["numbers_at_age"][i_age_year] =
            dq_["numbers_at_age"][i_age_year] +
            dq_["numbers_at_age"][i_agem1_yearm1 + 1] *
                (fims_math::exp(
                    -dq_["mortality_Z"][i_agem1_yearm1 + 1]));
      }
    }

    /**
     * This method is used to calculate the unfished numbers at age for a
     * population. It takes a population object, the index of the age
     * in the current year, the index of the age in the previous year,
     * and the age as input and calculates the unfished numbers at age
     * for that population.
     * @param population
     * @param i_age_year
     * @param i_agem1_yearm1
     * @param age
     */
    void CalculateUnfishedNumbersAA(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t i_agem1_yearm1, size_t age)
    {

      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      // using M from previous age/year
      dq_["unfished_numbers_at_age"][i_age_year] =
          dq_["unfished_numbers_at_age"][i_agem1_yearm1] *
          (fims_math::exp(-population->M[i_agem1_yearm1]));

      // Plus group calculation
      if (age == (population->nages - 1))
      {
        dq_["unfished_numbers_at_age"][i_age_year] =
            dq_["unfished_numbers_at_age"][i_age_year] +
            dq_["unfished_numbers_at_age"][i_agem1_yearm1 + 1] *
                (fims_math::exp(-population->M[i_agem1_yearm1 + 1]));
      }
    }

    /**
     * * This method is used to calculate the mortality for a population. It takes
     * a population object, the index of the age in the current year, the year,
     * and the age as input and calculates the mortality for that population.
     * @param population
     * @param i_age_year
     * @param year
     * @param age
     */
    void CalculateMortality(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t year, size_t age)
    {
      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
      {
        // evaluate is a member function of the selectivity class
        Type s = population->fleets[fleet_]->selectivity->evaluate(
            population->ages[age]);

        dq_["mortality_F"][i_age_year] +=
            population->fleets[fleet_]->Fmort[year] * s;

        dq_["sum_selectivity"][i_age_year] += s;
      }
      dq_["mortality_Z"][i_age_year] =
          population->M[i_age_year] +
          dq_["mortality_F"][i_age_year];
    }

    /**
     * * This method is used to calculate the biomass for a population. It takes a
     * population object, the index of the age in the current year, the year,
     * and the age as input and calculates the biomass for that population.
     * @param population
     * @param i_age_year
     * @param year
     * @param age
     */
    void CalculateBiomass(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t year, size_t age)
    {
      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      dq_["biomass"][year] +=
          dq_["numbers_at_age"][i_age_year] *
          population->growth->evaluate(population->ages[age]);
    }

    /**
     * * This method is used to calculate the unfished biomass for a population.
     * It takes a population object, the index of the age in the current year, the
     * year, and the age as input and calculates the unfished biomass for that
     * population.
     * @param population
     * @param i_age_year
     * @param year
     * @param age
     */
    void CalculateUnfishedBiomass(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t year, size_t age)
    {
      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      dq_["unfished_biomass"][year] +=
          dq_["unfished_numbers_at_age"][i_age_year] *
          population->growth->evaluate(population->ages[age]);
    }

    /**
     * * This method is used to calculate the spawning biomass for a population.
     * It takes a population object, the index of the age in the current year, the
     * year, and the age as input and calculates the spawning biomass for that
     * population.
     * @param population
     * @param i_age_year
     * @param year
     * @param age
     */
    void CalculateSpawningBiomass(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t year, size_t age)
    {
      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      dq_["spawning_biomass"][year] +=
          population->proportion_female[age] *
          dq_["numbers_at_age"][i_age_year] *
          dq_["proportion_mature_at_age"][i_age_year] *
          population->growth->evaluate(population->ages[age]);
    }

    /**
     * This method is used to calculate the unfished spawning biomass for a
     * population. It takes a population object, the index of the age in the
     * current year, the year, and the age as input and calculates the unfished
     * spawning biomass for that population.
     * @param population
     * @param i_age_year
     * @param year
     * @param age
     */
    void CalculateUnfishedSpawningBiomass(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t year, size_t age)
    {
      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      dq_["unfished_spawning_biomass"][year] +=
          population->proportion_female[age] *
          dq_["unfished_numbers_at_age"][i_age_year] *
          dq_["proportion_mature_at_age"][i_age_year] *
          population->growth->evaluate(population->ages[age]);
    }

    /**
     * This method is used to calculate the spawning biomass per recruit for a
     * population. It takes a population object.
     */
    Type CalculateSBPR0(
        std::shared_ptr<fims_popdy::Population<Type>> &population)
    {

      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      std::vector<Type> numbers_spr(population->nages, 1.0);
      Type phi_0 = 0.0;
      phi_0 +=
          numbers_spr[0] * population->proportion_female[0] *
          dq_["proportion_mature_at_age"][0] *
          population->growth->evaluate(population->ages[0]);
      for (size_t a = 1; a < (population->nages - 1); a++)
      {
        numbers_spr[a] = numbers_spr[a - 1] * fims_math::exp(-population->M[a]);
        phi_0 +=
            numbers_spr[a] * population->proportion_female[a] *
            dq_["proportion_mature_at_age"][a] *
            population->growth->evaluate(population->ages[a]);
      }
      for (size_t a = 1; a < (population->nages - 1); a++)
      {
        numbers_spr[a] = numbers_spr[a - 1] * fims_math::exp(-population->M[a]);
        phi_0 +=
            numbers_spr[a] * population->proportion_female[a] *
            dq_["proportion_mature_at_age"][a] *
            population->growth->evaluate(population->ages[a]);
      }

      numbers_spr[population->nages - 1] =
          (numbers_spr[population->nages - 2] *
           fims_math::exp(-population->M[population->nages - 2])) /
          (1 - fims_math::exp(-population->M[population->nages - 1]));
      phi_0 +=
          numbers_spr[population->nages - 1] *
          population->proportion_female[population->nages - 1] *
          dq_["proportion_mature_at_age"][population->nages - 1] *
          population->growth->evaluate(population->ages[population->nages - 1]);

      return phi_0;
    }

    /**
     * This method is used to calculate the recruitment for a population.
     *
     */
    void CalculateRecruitment(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t year, size_t i_dev)
    {

      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      Type phi0 = CalculateSBPR0(population);

      if (i_dev == population->nyears)
      {
        dq_["numbers_at_age"][i_age_year] =
            population->recruitment->evaluate_mean(
                dq_["spawning_biomass"][year - 1],
                phi0);
        /*the final year of the time series has no data to inform recruitment
        devs, so this value is set to the mean recruitment.*/
      }
      else
      {
        // Why are we using evaluate_mean, how come a virtual function was
        // changed? AMH: there are now two virtual functions: evaluate_mean and
        // evaluate_process (see below)
        population->recruitment->log_expected_recruitment[year - 1] =
            fims_math::log(population->recruitment->evaluate_mean(
                dq_["spawning_biomass"][year - 1],
                phi0));

        dq_["numbers_at_age"][i_age_year] = fims_math::exp(
            population->recruitment->process->evaluate_process(year - 1));
      }

      dq_["expected_recruitment"][year] =
          dq_["numbers_at_age"][i_age_year];
    }

    /**
     * This method is used to calculate the maturity at age for a population. It
     * takes a population object, the index of the age in the current year, the
     * age as input and calculates the maturity at age for that population.
     * @param population
     * @param i_age_year
     * @param age
     */
    void CalculateMaturityAA(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t age)
    {
      std::map<std::string, fims::Vector<Type>> &dq_ = this->GetPopulationDerivedQuantities(population->GetId());

      dq_["proportion_mature_at_age"][i_age_year] =
          population->maturity->evaluate(population->ages[age]);
    }

    /**
     * This method is used to calculate the landings for a population and adds to
     * existing expected total landings by fleet. It takes a population object,
     * the year, and the age as input and calculates the landings for that
     * population.
     * @param population
     * @param year
     * @param age
     */
    void CalculateLandings(
        std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year,
        size_t age)
    {

      std::map<std::string, fims::Vector<Type>> &pdq_ = this->GetPopulationDerivedQuantities(population->GetId());

      for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());
        size_t i_age_year = year * population->nages + age;

        pdq_["total_landings_weight"][year] +=
            fdq_["landings_weight_at_age"][i_age_year];

        fdq_["landings_weight"][year] +=
            fdq_["landings_weight_at_age"][i_age_year];

        pdq_["total_landings_numbers"][year] +=
            fdq_["landings_numbers_at_age"][i_age_year];

        fdq_["landings_numbers"][year] +=
            fdq_["landings_numbers_at_age"][i_age_year];
      }
    }

    /**
     * This method is used to calculate the catch weight at age for a population.
     * It takes a population object, the index of the age in the current year, the
     * year, and the age as input and calculates the weight at age for that
     * population.
     * @param population
     * @param year
     * @param age
     */
    void CalculateLandingsWeightAA(
        std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year,
        size_t age)
    {

      int i_age_year = year * population->nages + age;
      for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

        fdq_["landings_weight_at_age"][i_age_year] =
            fdq_["landings_numbers_at_age"][i_age_year] *
            population->growth->evaluate(population->ages[age]);
      }
    }

    /**
     * @brief Calculate the numbers at age for landings in a population.
     *
     * @param population The population.
     * @param i_age_year The index of the age and year.
     * @param year The year.
     * @param age The age.
     */
    void CalculateLandingsNumbersAA(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t year, size_t age)
    {
      std::map<std::string, fims::Vector<Type>> &pdq_ = this->GetPopulationDerivedQuantities(population->GetId());

      for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

        // Baranov Catch Equation
        fdq_["landings_numbers_at_age"][i_age_year] +=
            (population->fleets[fleet_]->Fmort[year] *
             population->fleets[fleet_]->selectivity->evaluate(
                 population->ages[age])) /
            pdq_["mortality_Z"][i_age_year] *
            pdq_["numbers_at_age"][i_age_year] *
            (1 - fims_math::exp(
                     -(pdq_["mortality_Z"]
                           [i_age_year])));
      }
    }

    /**
     * @brief Calculate the index for a population.
     *
     * @param population The population.
     * @param i_age_year The index of the year and age.
     * @param year The year.
     * @param age The age.
     */
    void CalculateIndex(std::shared_ptr<fims_popdy::Population<Type>> &population,
                        size_t i_age_year, size_t year, size_t age)
    {
      for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

        fdq_["index_weight"][year] +=
            fdq_["index_weight_at_age"][i_age_year];

        fdq_["index_numbers"][year] +=
            fdq_["index_numbers_at_age"][i_age_year];
      }
    }

    /**
     * @brief Calculate the numbers at age for an index in the population.
     *
     * @param population The population.
     * @param i_age_year The index of the year and age.
     * @param year The year.
     * @param age The age.
     */
    void CalculateIndexNumbersAA(
        std::shared_ptr<fims_popdy::Population<Type>> &population,
        size_t i_age_year, size_t year, size_t age)
    {
      std::map<std::string, fims::Vector<Type>> &pdq_ = this->GetPopulationDerivedQuantities(population->GetId());

      for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

        fdq_["index_numbers_at_age"][i_age_year] +=
            (population->fleets[fleet_]->q.get_force_scalar(year) *
             population->fleets[fleet_]->selectivity->evaluate(
                 population->ages[age])) *
            pdq_["numbers_at_age"][i_age_year];
      }
    }

    /**
     * @brief Calculate the weight at age for an index in a population.
     *
     * @param population The population.
     * @param year The year.
     * @param age The age.
     */
    void CalculateIndexWeightAA(
        std::shared_ptr<fims_popdy::Population<Type>> &population, size_t year,
        size_t age)
    {

      int i_age_year = year * population->nages + age;
      for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

        fdq_["index_weight_at_age"][i_age_year] =
            fdq_["index_numbers_at_age"][i_age_year] *
            population->growth->evaluate(population->ages[age]);
      }
    }

    /**
     * This method is used to calculate the proportions for a population. It takes
     * a population object, the index of the age in the current year, the age as
     * input and calculates the proportions for that population.
     */
    void ComputeProportions()
    {
      for (size_t p = 0; p < this->populations.size(); p++)
      {
        std::shared_ptr<fims_popdy::Population<Type>> &population =
            this->populations[p];
        std::map<std::string, fims::Vector<Type>> &pdq_ = this->GetPopulationDerivedQuantities(population->GetId());

        for (size_t year = 0; year < population->nyears; year++)
        {
          for (size_t fleet_ = 0; fleet_ < population->nfleets; fleet_++)
          {
            std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities(population->fleets[fleet_]->GetId());

            size_t index_yf = year * population->nfleets + fleet_;
            Type sum_age = 0.0;
            Type sum_length = 0.0;
            for (size_t age = 0; age < population->nages; age++)
            {
              size_t i_age_year = year * population->nages + age;
              sum_age += fdq_["landings_numbers_at_age"][i_age_year];
            }

            for (size_t age = 0; age < population->nages; age++)
            {
              size_t i_age_year = year * population->nages + age;
              fdq_["age_comp_proportion"][i_age_year] =
                  fdq_["landings_numbers_at_age"][i_age_year] /
                  sum_age;
            }

            if (population->fleets[fleet_]->nlengths > 0)
            {
              for (size_t y = 0; y < population->fleets[fleet_]->nyears; y++)
              {
                fims::Vector<Type> &landings_numbers_at_length = fdq_["landings_numbers_at_length"];
                fims::Vector<Type> &landings_numbers_at_age = fdq_["landings_numbers_at_age"];
                fims::Vector<Type> &proportion_landings_numbers_at_length = fdq_["length_comp_proportion"];

                sum_length = 0.0;
                for (size_t l = 0; l < population->fleets[fleet_]->nlengths;
                     l++)
                {
                  size_t i_length_year =
                      y * population->fleets[fleet_]->nlengths + l;
                  for (size_t a = 0; a < population->fleets[fleet_]->nages; a++)
                  {
                    size_t i_age_year = y * population->fleets[fleet_]->nages + a;
                    size_t i_length_age =
                        a * population->fleets[fleet_]->nlengths + l;
                    fdq_["landings_numbers_at_length"][i_length_year] +=
                        fdq_["landings_numbers_at_age"][i_age_year] *
                        population->fleets[fleet_]
                            ->age_to_length_conversion[i_length_age];
                  }
                  sum_length += fdq_["landings_numbers_at_length"][i_length_year];
                }
                for (size_t l = 0; l < population->fleets[fleet_]->nlengths;
                     l++)
                {
                  size_t i_length_year =
                      y * population->fleets[fleet_]->nlengths + l;
                  fdq_["length_comp_proportion"][i_length_year] =
                      fdq_["landings_numbers_at_length"][i_length_year] /
                      sum_length;
                }
                for (size_t l = 0; l < population->fleets[fleet_]->nlengths;
                     l++)
                {
                  size_t i_length_year =
                      y * population->fleets[fleet_]->nlengths + l;
                  fdq_["length_comp_proportion"][i_length_year] =
                      fdq_["landings_numbers_at_length"][i_length_year] /
                      sum_length;
                }
              }
            }
          }
        }
      }
    }

    /**
     * Evaluate the proportion of landings numbers at age.
     */
    void evaluate_age_comp()
    {
      fleet_iterator fit;
      for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities((*fit).second->GetId());

        std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
        for (size_t y = 0; y < fleet->nyears; y++)
        {
          Type sum = static_cast<Type>(0.0);
          Type sum_obs = static_cast<Type>(0.0);
          // robust_add is a small value to add to expected composition
          // proportions at age to stabilize likelihood calculations
          // when the expected proportions are close to zero.
          // Type robust_add = static_cast<Type>(0.0); // zeroed out before
          // testing 0.0001; sum robust is used to calculate the total sum of
          // robust additions to ensure that proportions sum to 1. Type robust_sum
          // = static_cast<Type>(1.0);

          for (size_t a = 0; a < fleet->nages; a++)
          {
            size_t i_age_year = y * fleet->nages + a;
            // Here we have a check to determine if the age comp
            // should be calculated from the retained landings or
            // the total population. These values are slightly different.
            // In the future this will have more impact as we implement
            // timing rather than everything occurring at the start of
            // the year.
            if (fleet->fleet_observed_landings_data_id_m == -999)
            {
              fdq_["age_comp_expected"][i_age_year] =
                  fdq_["index_numbers_at_age"][i_age_year];
            }
            else
            {
              fdq_["age_comp_expected"][i_age_year] =
                  fdq_["landings_numbers_at_age"][i_age_year];
            }
            sum += fdq_["age_comp_expected"][i_age_year];
            // robust_sum -= robust_add;

            // This sums over the observed age composition data so that
            // the expected age composition can be rescaled to match the
            // total number observed. The check for na values should not
            // be needed as individual years should not have missing data.
            // This is need to be re-explored if/when we modify FIMS to
            // allow for composition bins that do not match the population
            // bins.
            if (fleet->fleet_observed_agecomp_data_id_m != -999)
            {
              if (fleet->observed_agecomp_data->at(i_age_year) !=
                  fleet->observed_agecomp_data->na_value)
              {
                sum_obs += fleet->observed_agecomp_data->at(i_age_year);
              }
            }
          }
          for (size_t a = 0; a < fleet->nages; a++)
          {
            size_t i_age_year = y * fleet->nages + a;
            fdq_["age_comp_proportion"][i_age_year] =
                fdq_["age_comp_expected"][i_age_year] /
                sum;
            // robust_add + robust_sum * this->agecomp_expected[i_age_year] / sum;

            if (fleet->fleet_observed_agecomp_data_id_m != -999)
            {
              fdq_["age_comp_expected"][i_age_year] =
                  fdq_["age_comp_proportion"][i_age_year] *
                  sum_obs;
            }
          }
        }
      }
    }

    /**
     * Evaluate the proportion of landings numbers at length.
     */
    void evaluate_length_comp()
    {
      fleet_iterator fit;
      for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities((*fit).second->GetId());

        std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

        if (fleet->nlengths > 0)
        {
          for (size_t y = 0; y < fleet->nyears; y++)
          {
            Type sum = static_cast<Type>(0.0);
            Type sum_obs = static_cast<Type>(0.0);
            // robust_add is a small value to add to expected composition
            // proportions at age to stabilize likelihood calculations
            // when the expected proportions are close to zero.
            // Type robust_add = static_cast<Type>(0.0); // 0.0001; zeroed out
            // before testing sum robust is used to calculate the total sum of
            // robust additions to ensure that proportions sum to 1. Type
            // robust_sum = static_cast<Type>(1.0);
            for (size_t l = 0; l < fleet->nlengths; l++)
            {
              size_t i_length_year = y * fleet->nlengths + l;
              for (size_t a = 0; a < fleet->nages; a++)
              {
                size_t i_age_year = y * fleet->nages + a;
                size_t i_length_age = a * fleet->nlengths + l;
                fdq_["length_comp_expected"][i_length_year] +=
                    fdq_["age_comp_expected"][i_age_year] *
                    fleet->age_to_length_conversion[i_length_age];

                fdq_["landings_numbers_at_length"][i_length_year] +=
                    fdq_["landings_numbers_at_age"][i_age_year] *
                    fleet->age_to_length_conversion[i_length_age];

                fdq_["index_numbers_at_length"][i_length_year] +=
                    fdq_["index_numbers_at_age"][i_age_year] *
                    fleet->age_to_length_conversion[i_length_age];
              }

              sum += fdq_["length_comp_expected"][i_length_year];
              // robust_sum -= robust_add;

              if (fleet->fleet_observed_lengthcomp_data_id_m != -999)
              {
                if (fleet->observed_lengthcomp_data->at(i_length_year) !=
                    fleet->observed_lengthcomp_data->na_value)
                {
                  sum_obs += fleet->observed_lengthcomp_data->at(i_length_year);
                }
              }
            }
            for (size_t l = 0; l < fleet->nlengths; l++)
            {
              size_t i_length_year = y * fleet->nlengths + l;
              fdq_["length_comp_proportion"][i_length_year] =
                  fdq_["length_comp_expected"][i_length_year] /
                  sum;
              // robust_add + robust_sum *
              // this->lengthcomp_expected[i_length_year] / sum;
              if (fleet->fleet_observed_lengthcomp_data_id_m != -999)
              {
                fdq_["length_comp_expected"][i_length_year] =
                    fdq_["length_comp_proportion"][i_length_year] *
                    sum_obs;
              }
            }
          }
        }
      }
    }

    /**
     * Evaluate the natural log of the expected index.
     */
    void evaluate_index()
    {
      fleet_iterator fit;
      for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities((*fit).second->GetId());
        std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

        for (size_t i = 0;
             i < fdq_["index_numbers"].size();
             i++)
        {
          if (fleet->observed_index_units == "number")
          {
            fdq_["index_expected"][i] =
                fdq_["index_numbers"][i];
          }
          else
          {
            fdq_["index_expected"][i] =
                fdq_["index_weight"][i];
          }
          fdq_["log_index_expected"][i] =
              log(fdq_["index_expected"][i]);
        }
      }
    }

    /**
     * Evaluate the natural log of the expected landings.
     */
    void evaluate_landings()
    {
      fleet_iterator fit;
      for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit)
      {
        std::map<std::string, fims::Vector<Type>> &fdq_ = this->GetFleetDerivedQuantities((*fit).second->GetId());
        std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;

        for (size_t i = 0; i < fdq_["landings_weight"].size();
             i++)
        {
          if (fleet->observed_landings_units == "number")
          {
            fdq_["landings_expected"][i] =
                fdq_["landings_numbers"][i];
          }
          else
          {
            fdq_["landings_expected"][i] =
                fdq_["landings_weight"][i];
          }
          fdq_["log_landings_expected"][i] =
              log(fdq_["landings_expected"][i]);
        }
      }
    }

    virtual void Evaluate()
    {
      /*
                 Sets derived vectors to zero
                 Performs parameters transformations
                 Sets recruitment deviations to mean 0.
       */
      Prepare();
      /*
       start at year=0, age=0;
       here year 0 is the estimated initial population structure and age 0 are
       recruits loops start at zero with if statements inside to specify unique
       code for initial structure and recruitment 0 loops. Could also have started
       loops at 1 with initial structure and recruitment setup outside the loops.

       year loop is extended to <= nyears because SSB is calculated as the start
       of the year value and by extending one extra year we get estimates of the
       population structure at the end of the final year. An alternative approach
       would be to keep initial numbers at age in it's own vector and each year to
       include the population structure at the end of the year. This is likely a
       null point given that we are planning to modify to an event/stanza based
       structure in later milestones which will eliminate this confusion by
       explicitly referencing the exact date (or period of averaging) at which any
       calculation or output is being made.
       */
      for (size_t p = 0; p < this->populations.size(); p++)
      {
        std::shared_ptr<fims_popdy::Population<Type>> &population =
            this->populations[p];
        std::map<std::string, fims::Vector<Type>> &pdq_ = this->GetPopulationDerivedQuantities(population->GetId());
        // CAAPopulationProxy<Type>& population = this->populations_proxies[p];

        for (size_t y = 0; y <= population->nyears; y++)
        {
          for (size_t a = 0; a < population->nages; a++)
          {
            /*
             index naming defines the dimensional folding structure
             i.e. i_age_year is referencing folding over years and ages.
             */
            size_t i_age_year = y * population->nages + a;
            /*
             Mortality rates are not estimated in the final year which is
             used to show expected population structure at the end of the model
             period. This is because biomass in year i represents biomass at the
             start of the year. Should we add complexity to track more values such
             as start, mid, and end biomass in all years where, start biomass=end
             biomass of the previous year? Referenced above, this is probably not
             worth exploring as later milestone changes will eliminate this
             confusion.
             */
            if (y < population->nyears)
            {
              /*
               First thing we need is total mortality aggregated across all fleets
               to inform the subsequent catch and change in numbers at age
               calculations. This is only calculated for years < nyears as these
               are the model estimated years with data. The year loop extends to
               y=nyears so that population numbers at age and SSB can be
               calculated at the end of the last year of the model
               */
              CalculateMortality(population, i_age_year, y, a);
            }
            CalculateMaturityAA(population, i_age_year, a);
            /* if statements needed because some quantities are only needed
            for the first year and/or age, so these steps are included here.
             */
            if (y == 0)
            {
              // Initial numbers at age is a user input or estimated parameter
              // vector.
              CalculateInitialNumbersAA(population, i_age_year, a);

              if (a == 0)
              {
                pdq_["unfished_numbers_at_age"][i_age_year] =
                    fims_math::exp(population->recruitment->log_rzero[0]);
              }
              else
              {
                CalculateUnfishedNumbersAA(population, i_age_year, a - 1, a);
              }

              /*
               Fished and unfished biomass vectors are summing biomass at
               age across ages.
               */

              CalculateBiomass(population, i_age_year, y, a);

              CalculateUnfishedBiomass(population, i_age_year, y, a);

              /*
               Fished and unfished spawning biomass vectors are summing biomass at
               age across ages to allow calculation of recruitment in the next
               year.
               */

              CalculateSpawningBiomass(population, i_age_year, y, a);

              CalculateUnfishedSpawningBiomass(population, i_age_year, y, a);

              /*
               Expected recruitment in year 0 is numbers at age 0 in year 0.
               */

              pdq_["expected_recruitment"]
                  [i_age_year] =
                      pdq_["numbers_at_age"]
                          [i_age_year];
            }
            else
            {
              if (a == 0)
              {
                // Set the nrecruits for age a=0 year y (use pointers instead of
                // functional returns) assuming fecundity = 1 and 50:50 sex ratio
                CalculateRecruitment(population, i_age_year, y, y);
                pdq_["unfished_numbers_at_age"]
                    [i_age_year] =
                        fims_math::exp(population->recruitment->log_rzero[0]);
              }
              else
              {
                size_t i_agem1_yearm1 = (y - 1) * population->nages + (a - 1);
                CalculateNumbersAA(population, i_age_year, i_agem1_yearm1, a);
                CalculateUnfishedNumbersAA(population, i_age_year, i_agem1_yearm1,
                                           a);
              }
              CalculateBiomass(population, i_age_year, y, a);
              CalculateSpawningBiomass(population, i_age_year, y, a);

              CalculateUnfishedBiomass(population, i_age_year, y, a);
              CalculateUnfishedSpawningBiomass(population, i_age_year, y, a);
            }

            /*
            Here composition, total catch, and index values are calculated for all
            years with reference data. They are not calculated for y=nyears as
            there is this is just to get final population structure at the end of
            the terminal year.
             */
            if (y < population->nyears)
            {
              CalculateLandingsNumbersAA(population, i_age_year, y, a);
              CalculateLandingsWeightAA(population, y, a);
              CalculateLandings(population, y, a);

              CalculateIndexNumbersAA(population, i_age_year, y, a);
              CalculateIndexWeightAA(population, y, a);
              CalculateIndex(population, i_age_year, y, a);
            }
          }
        }
      }
      evaluate_age_comp();
      evaluate_length_comp();
      evaluate_index();
      evaluate_landings();
      // ComputeProportions();
    }

    /**
     * * This method is used to generate TMB reports from the population dynamics
     * model.
     */
    virtual void Report()
    {
      int n_fleets = this->fleets.size();
      int n_pops = this->populations.size();
#ifdef TMB_MODEL

      // // Create vector lists to store output for reporting
      // // vector< vector<Type> > creates a nested vector structure where
      // // each vector can be a different dimension. Does not work with ADREPORT
      // // fleets
      // vector<vector<Type>> landings_w(n_fleets);
      // vector<vector<Type>> landings_n(n_fleets);
      // vector<vector<Type>> landings_exp(n_fleets);
      // vector<vector<Type>> landings_naa(n_fleets);
      // vector<vector<Type>> landings_waa(n_fleets);
      // vector<vector<Type>> landings_nal(n_fleets);
      // vector<vector<Type>> index_w(n_fleets);
      // vector<vector<Type>> index_n(n_fleets);
      // vector<vector<Type>> index_exp(n_fleets);
      // vector<vector<Type>> index_naa(n_fleets);
      // vector<vector<Type>> index_nal(n_fleets);
      // vector<vector<Type>> agecomp_exp(n_fleets);
      // vector<vector<Type>> lengthcomp_exp(n_fleets);
      // vector<vector<Type>> agecomp_prop(n_fleets);
      // vector<vector<Type>> lengthcomp_prop(n_fleets);
      // vector<vector<Type>> F_mort(n_fleets);
      // vector<vector<Type>> q(n_fleets);
      // // populations
      // vector<vector<Type>> naa(n_pops);
      // vector<vector<Type>> ssb(n_pops);
      // vector<vector<Type>> total_landings_w(n_pops);
      // vector<vector<Type>> total_landings_n(n_pops);
      // vector<vector<Type>> biomass(n_pops);
      // vector<vector<Type>> log_recruit_dev(n_pops);
      // vector<vector<Type>> log_r(n_pops);
      // vector<vector<Type>> recruitment(n_pops);
      // vector<vector<Type>> M(n_pops);
      // // initiate population index for structuring report out objects
      // int pop_idx = 0;
      // for (size_t p = 0; p < this->populations.size(); p++) {
      //   std::shared_ptr<fims_popdy::Population<Type>> &population =
      //       this->populations[p];
      //   std::map<std::string, fims::Vector<Type>> &derived_quantities =
      //       this->population_derived_quantities[this->populations[p]->GetId()];
      //   naa(pop_idx) = vector<Type>(derived_quantities["numbers_at_age"]);
      //   ssb(pop_idx) = vector<Type>(derived_quantities["spawning_biomass"]);
      //   total_landings_w(pop_idx) =
      //       vector<Type>(derived_quantities["total_landings_weight"]);
      //   total_landings_n(pop_idx) =
      //       vector<Type>(derived_quantities["total_landings_numbers"]);
      //   log_recruit_dev(pop_idx) =
      //       vector<Type>(population->recruitment->log_recruit_devs);
      //   log_r(pop_idx) = vector<Type>(derived_quantities["log_r"]);
      //   recruitment(pop_idx) =
      //       vector<Type>(derived_quantities["expected_recruitment"]);
      //   biomass(pop_idx) = vector<Type>(derived_quantities["biomass"]);
      //   M(pop_idx) = vector<Type>(population->M);

      //   pop_idx += 1;
      // }

      // // initiate fleet index for structuring report out objects
      // int fleet_idx = 0;
      // fleet_iterator fit;
      // for (fit = this->fleets.begin(); fit != this->fleets.end(); ++fit) {
      //   std::shared_ptr<fims_popdy::Fleet<Type>> &fleet = (*fit).second;
      //   std::map<std::string, fims::Vector<Type>> &derived_quantities =
      //       this->fleet_derived_quantities[fleet->GetId()];
      //   landings_w(fleet_idx) = derived_quantities["landings_weight"];
      //   landings_n(fleet_idx) = derived_quantities["landings_numbers"];
      //   landings_exp(fleet_idx) = derived_quantities["landings_expected"];
      //   landings_naa(fleet_idx) = derived_quantities["landings_numbers_at_age"];
      //   landings_waa(fleet_idx) = derived_quantities["landings_weight_at_age"];
      //   landings_nal(fleet_idx) =
      //       derived_quantities["landings_numbers_at_length"];
      //   index_w(fleet_idx) = derived_quantities["index_weight"];
      //   index_n(fleet_idx) = derived_quantities["index_numbers"];
      //   index_exp(fleet_idx) = derived_quantities["index_expected"];
      //   index_naa(fleet_idx) = derived_quantities["index_numbers_at_age"];
      //   index_nal(fleet_idx) = derived_quantities["index_numbers_at_length"];
      //   agecomp_exp(fleet_idx) = derived_quantities["agecomp_expected"];
      //   lengthcomp_exp(fleet_idx) = derived_quantities["lengthcomp_expected"];
      //   agecomp_prop(fleet_idx) = derived_quantities["agecomp_proportion"];
      //   lengthcomp_prop(fleet_idx) = derived_quantities["lengthcomp_proportion"];
      //   F_mort(fleet_idx) = derived_quantities["Fmort"];
      //   q(fleet_idx) = derived_quantities["q"];
      //   fleet_idx += 1;
      // }

      // // FIMS_REPORT_F(rec_nll, this->of);
      // // FIMS_REPORT_F(age_comp_nll, this->of);
      // // FIMS_REPORT_F(index_nll, this->of);
      // FIMS_REPORT_F(naa, this->of);
      // FIMS_REPORT_F(ssb, this->of);
      // FIMS_REPORT_F(log_recruit_dev, this->of);
      // FIMS_REPORT_F(log_r, this->of);
      // FIMS_REPORT_F(recruitment, this->of);
      // FIMS_REPORT_F(biomass, this->of);
      // FIMS_REPORT_F(M, this->of);
      // FIMS_REPORT_F(total_landings_w, this->of);
      // FIMS_REPORT_F(total_landings_n, this->of);
      // FIMS_REPORT_F(landings_w, this->of);
      // FIMS_REPORT_F(landings_n, this->of);
      // FIMS_REPORT_F(landings_exp, this->of);
      // FIMS_REPORT_F(landings_naa, this->of);
      // FIMS_REPORT_F(landings_waa, this->of);
      // FIMS_REPORT_F(landings_nal, this->of);
      // FIMS_REPORT_F(index_w, this->of);
      // FIMS_REPORT_F(index_n, this->of);
      // FIMS_REPORT_F(index_exp, this->of);
      // FIMS_REPORT_F(index_naa, this->of);
      // FIMS_REPORT_F(index_nal, this->of);
      // FIMS_REPORT_F(agecomp_exp, this->of);
      // FIMS_REPORT_F(lengthcomp_exp, this->of);
      // FIMS_REPORT_F(agecomp_prop, this->of);
      // FIMS_REPORT_F(lengthcomp_prop, this->of);
      // FIMS_REPORT_F(F_mort, this->of);
      // FIMS_REPORT_F(q, this->of);

      // /*ADREPORT using ADREPORTvector defined in
      //  * inst/include/interface/interface.hpp:
      //  * function collapses the nested vector into a single vector
      //  */
      // vector<Type> NAA = ADREPORTvector(naa);
      // vector<Type> Biomass = ADREPORTvector(biomass);
      // vector<Type> SSB = ADREPORTvector(ssb);
      // vector<Type> LogRecDev = ADREPORTvector(log_recruit_dev);
      // vector<Type> FMort = ADREPORTvector(F_mort);
      // vector<Type> Q = ADREPORTvector(q);
      // vector<Type> LandingsExpected = ADREPORTvector(landings_exp);
      // vector<Type> IndexExpected = ADREPORTvector(index_exp);
      // vector<Type> LandingsNumberAtAge = ADREPORTvector(landings_naa);
      // vector<Type> LandingsNumberAtLength = ADREPORTvector(landings_nal);
      // vector<Type> IndexNumberAtAge = ADREPORTvector(index_naa);
      // vector<Type> IndexNumberAtLength = ADREPORTvector(index_nal);
      // vector<Type> AgeCompositionExpected = ADREPORTvector(agecomp_exp);
      // vector<Type> LengthCompositionExpected = ADREPORTvector(lengthcomp_exp);
      // vector<Type> AgeCompositionProportion = ADREPORTvector(agecomp_prop);
      // vector<Type> LengthCompositionProportion = ADREPORTvector(lengthcomp_prop);

      // ADREPORT_F(NAA, this->of);
      // ADREPORT_F(Biomass, this->of);
      // ADREPORT_F(SSB, this->of);
      // ADREPORT_F(LogRecDev, this->of);
      // ADREPORT_F(FMort, this->of);
      // ADREPORT_F(Q, this->of);
      // ADREPORT_F(LandingsExpected, this->of);
      // ADREPORT_F(IndexExpected, this->of);
      // ADREPORT_F(LandingsNumberAtAge, this->of);
      // ADREPORT_F(LandingsNumberAtLength, this->of);
      // ADREPORT_F(IndexNumberAtAge, this->of);
      // ADREPORT_F(IndexNumberAtLength, this->of);
      // ADREPORT_F(AgeCompositionExpected, this->of);
      // ADREPORT_F(LengthCompositionExpected, this->of);
      // ADREPORT_F(AgeCompositionProportion, this->of);
      // ADREPORT_F(LengthCompositionProportion, this->of);

#endif
    }
  };

} // namespace fims_popdy

#endif
