/**
 * @file rcpp_population.hpp
 * @brief TODO: provide a brief description.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP

#include "../../../population_dynamics/population/population.hpp"
#include "rcpp_interface_base.hpp"

/**
 * Population Rcpp interface
 */

/**
 * @brief PopulationInterfaceBase class should be inherited to define different
 * Rcpp interfaces for each possible Population function.
 */
class PopulationInterfaceBase : public FIMSRcppInterfaceBase {
 public:
    static uint32_t id_g; /**< static id of the population interface base*/
  uint32_t id;          /**< id of the population interface base */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, PopulationInterfaceBase*>
      live_objects; /**< map associating the ids of PopulationInterfaceBase to
                         the objects */

  PopulationInterfaceBase() {
    this->id = PopulationInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    PopulationInterfaceBase */
    PopulationInterfaceBase::live_objects[this->id] = this;
    PopulationInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~PopulationInterfaceBase() {}

  /** @brief get_id method for child classes to inherit */
  virtual uint32_t get_id() = 0;
};

uint32_t PopulationInterfaceBase::id_g = 1;
std::map<uint32_t, PopulationInterfaceBase*>
    PopulationInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for a new Population. To instantiate
 * from R:
 * population <- new(population)
 */
class PopulationInterface : public PopulationInterfaceBase {
 public:
  uint32_t nages;            /**< number of ages */
  uint32_t nfleets;          /**< number of fleets */
  uint32_t nseasons;         /**< number of seasons */
  uint32_t nyears;           /**< number of years */
  uint32_t nlengths;         /**< number of lengths */
  uint32_t maturity_id;      /**< id of the maturity function*/
  uint32_t growth_id;        /**< id of the growth function*/
  uint32_t recruitment_id;   /**< id of the recruitment function*/
  ParameterVector log_M; /**< log of the natural mortality of the stock*/
  ParameterVector log_init_naa; /**<log of the initial numbers at age*/
  ParameterVector numbers_at_age; /**<log of the initial numbers at age*/
  Rcpp::NumericVector ages; /**<vector of ages in the population; length nages*/
  Rcpp::NumericVector proportion_female; /**<double representing the proportion
                                            of female individuals */
  bool estimate_prop_female; /**<whether proportion female should be estimated*/



    Rcpp::NumericVector derived_ssb; /**< derived quantity: spawning stock biomass */
    Rcpp::NumericVector derived_naa; /**< derived quantity: numbers at age */
    Rcpp::NumericVector derived_biomass; /**< derived quantity: biomass */
    Rcpp::NumericVector derived_recruitment; /**< derived quantity: recruitment */

    Rcpp::NumericVector estimated_log_M; /**< derived quantity: natural log mortality */
    Rcpp::NumericVector estimated_log_init_naa; /**< derived quantity: natural log initial numbers at age */
    Rcpp::NumericVector estimated_proportion_female; /**< TODO: delete */
    std::string name; /**< TODO: document*/

  PopulationInterface() : PopulationInterfaceBase() {}

  virtual ~PopulationInterface() {}

  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Sets the unique id for the Maturity object.
   *
   * @param maturity_id Unique id for the Maturity object.
   */
  void SetMaturity(uint32_t maturity_id) { this->maturity_id = maturity_id; }

  /**
   * @brief Set the unique id for the growth object.
   *
   * @param growth_id Unique id for the growth object.
   */
  void SetGrowth(uint32_t growth_id) { this->growth_id = growth_id; }

  /**
   * @brief Set the unique id for the recruitment object.
   *
   * @param recruitment_id Unique id for the recruitment object.
   */
  void SetRecruitment(uint32_t recruitment_id) {
    this->recruitment_id = recruitment_id;
  }

  /** @brief evaluate the population function */
  virtual void evaluate() {
    fims_popdy::Population<double> population;
    return population.Evaluate();
  }

    /** 
     * @brief finalize function. Extracts derived quantities back to 
     * the Rcpp interface object from the Information object. 
     */
    virtual void finalize() {

        if (this->finalized) {
            //log warning that finalize has been called more than once.
            FIMS_WARNING_LOG("Population " + fims::to_string(this->id) + " has been finalized already.");
        }

        this->finalized = true; //indicate this has been called already

        std::shared_ptr<fims_info::Information<double> > info =
                fims_info::Information<double>::GetInstance();




        this->estimated_log_M = Rcpp::NumericVector(this->log_M.size());
        for (size_t i = 0; i < this->log_M.size(); i++) {
            this->estimated_log_M[i] = this->log_M[i].initial_value_m;
        }

        this->estimated_log_init_naa = Rcpp::NumericVector(this->log_init_naa.size());
        for (size_t i = 0; i < this->log_init_naa.size(); i++) {
            this->estimated_log_init_naa[i] = this->log_init_naa[i].initial_value_m;
        }

        this->estimated_proportion_female = Rcpp::NumericVector(this->proportion_female.size());
        for (size_t i = 0; i < this->proportion_female.size(); i++) {
            this->estimated_proportion_female[i] = this->proportion_female[i];
        }

        fims_info::Information<double>::population_iterator it;


        it = info->populations.find(this->id);

        std::shared_ptr<fims_popdy::Population<double> > pop =
                info->populations[this->id];
        it = info->populations.find(this->id);
        if (it == info->populations.end()) {
            FIMS_WARNING_LOG("Population " + fims::to_string(this->id) + " not found in Information.");
            return;
        } else {

            if (this->estimated_log_M) {
                for (size_t i = 0; i < this->log_M.size(); i++) {
                    this->estimated_log_M[i] = pop->log_M[i];
                }
            }

            if (this->estimated_log_init_naa) {
                for (size_t i = 0; i < this->log_init_naa.size(); i++) {
                    this->estimated_log_init_naa[i] = pop->log_init_naa[i];
                }
            }

            if (this->estimate_prop_female) {
                for (size_t i = 0; i < this->proportion_female.size(); i++) {
                    this->estimated_proportion_female[i] = pop->proportion_female[i];
                }
            }

            this->derived_naa = Rcpp::NumericVector(pop->numbers_at_age.size());
            this->derived_ssb = Rcpp::NumericVector(pop->spawning_biomass.size());
            this->derived_biomass = Rcpp::NumericVector(pop->biomass.size());
            this->derived_recruitment = Rcpp::NumericVector(pop->expected_recruitment.size());

            //set naa from Information/
            for (size_t i = 0; i < this->derived_naa.size(); i++) {
                this->derived_naa[i] = pop->numbers_at_age[i];
            }

            //set ssb from Information/
            for (size_t i = 0; i < this->derived_ssb.size(); i++) {
                this->derived_ssb[i] = pop->spawning_biomass[i];
            }

            //set biomass from Information
            for (size_t i = 0; i < this->derived_biomass.size(); i++) {
                this->derived_biomass[i] = pop->biomass[i];
            }

            //set recruitment from Information/
            for (size_t i = 0; i < this->derived_recruitment.size(); i++) {
                this->derived_recruitment[i] = pop->expected_recruitment[i];
            }

        }

    }

    /**
     * @brief Convert the data to json representation for the output.
     */
    virtual std::string to_json() {
        std::stringstream ss;

        ss << "\"module\" : {\n";
        ss << " \"name\" : \"Population\",\n";

        ss << " \"type\" : \"population\",\n";
        ss << " \"tag\" : \"" << this->name << "\",\n";
        ss << " \"id\": " << this->id << ",\n";
        ss << " \"recruitment_id\": " << this->recruitment_id << ",\n";
        ss << " \"growth_id\": " << this->growth_id << ",\n";
        ss << " \"maturity_id\": " << this->maturity_id << ",\n";

        ss << " \"parameter\": {\n";
        ss << " \"name\": \"log_M\",\n";
        ss << " \"id\":" << -999 << ",\n";
        ss << " \"type\": \"vector\",\n";
        ss << " \"values\": " << this->log_M << "\n},\n";

        ss << " \"parameter\": {\n";
        ss << "  \"name\": \"log_init_naa\",\n";
        ss << "  \"id\":" << -999 << ",\n";
        ss << "  \"type\": \"vector\",\n";
        ss << "  \"values\":" << this->log_init_naa << " \n},\n";

        ss << " \"parameter\": {\n";
        ss << "  \"name\": \"proportion_female\",\n";
        ss << "  \"id\":" << -999 << ",\n";
        ss << "  \"type\": \"vector\",\n";
        ss << "  \"values\":" << this->proportion_female << "\n},\n";


        ss << " \"derived_quantity\": {\n";
        ss << "  \"name\": \"ssb\",\n";
        ss << "  \"values\":[";
        if (this->derived_ssb.size() == 0) {
            ss << "]\n";
        } else {
            for (size_t i = 0; i < this->derived_ssb.size() - 1; i++) {
                ss << this->derived_ssb[i] << ", ";
            }
            ss << this->derived_ssb[this->derived_ssb.size() - 1] << "]\n";
        }
        ss << " },\n";

        ss << " \"derived_quantity\": {\n";
        ss << "   \"name\": \"naa\",\n";
        ss << "   \"values\":[";
        if (this->derived_naa.size() == 0) {
            ss << "]\n";
        } else {
            for (size_t i = 0; i < this->derived_naa.size() - 1; i++) {
                ss << this->derived_naa[i] << ", ";
            }
            ss << this->derived_naa[this->derived_naa.size() - 1] << "]\n";
        }
        ss << " },\n";

        ss << " \"derived_quantity\": {\n";
        ss << "   \"name\": \"biomass\",\n";
        ss << "   \"values\":[";
        if (this->derived_biomass.size() == 0) {
            ss << "]\n";
        } else {
            for (size_t i = 0; i < this->derived_biomass.size() - 1; i++) {
                ss << this->derived_biomass[i] << ", ";
            }
            ss << this->derived_biomass[this->derived_biomass.size() - 1] << "]\n";
        }
        ss << " },\n";

        ss << " \"derived_quantity\": {\n";
        ss << "   \"name\": \"recruitment\",\n";
        ss << "   \"values\":[";
        if (this->derived_recruitment.size() == 0) {
            ss << "]\n";
        } else {
            for (size_t i = 0; i < this->derived_recruitment.size() - 1; i++) {
                ss << this->derived_recruitment[i] << ", ";
            }
            ss << this->derived_recruitment[this->derived_recruitment.size() - 1] << "]\n";
        }
        ss << " }\n";

        ss << "}";

        return ss.str();
    }


#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::Population<Type> > population =
        std::make_shared<fims_popdy::Population<Type> >();

    // set relative info
    population->id = this->id;
    population->nyears = this->nyears;
    population->nfleets = this->nfleets;
    population->nseasons = this->nseasons;
    population->nages = this->nages;
    if (this->nages == this->ages.size()) {
      population->ages.resize(this->nages);
    } else {
      warning("The ages vector is not of size nages.");
    }

    population->growth_id = this->growth_id;
    population->recruitment_id = this->recruitment_id;
    population->maturity_id = this->maturity_id;
    population->log_M.resize(this->log_M.size());
    population->log_init_naa.resize(this->log_init_naa.size());
    for (size_t i = 0; i < log_M.size(); i++) {
      population->log_M[i] = this->log_M[i].initial_value_m;
      if (this->log_M[i].estimated_m) {
          info->RegisterParameterName("log_M");
          info->RegisterParameter(population->log_M[i]);
      }
    }
    info->variable_map[this->log_M.id_m] = &(population)->log_M;

    for (size_t i = 0; i < log_init_naa.size(); i++) {
      population->log_init_naa[i] = this->log_init_naa[i].initial_value_m;
      if (this->log_init_naa[i].estimated_m) {
        info->RegisterParameterName("log_init_naa");
        info->RegisterParameter(population->log_init_naa[i]);
      }
    }
    info->variable_map[this->log_init_naa.id_m] = &(population)->log_init_naa;
    for (int i = 0; i < ages.size(); i++) {
      population->ages[i] = this->ages[i];
    }
    for (int i = 0; i < proportion_female.size(); i++) {
      population->proportion_female[i] = this->proportion_female[i];
      if (estimate_prop_female) {
        info->RegisterParameter(population->proportion_female[i]);
      }
    }

    population->numbers_at_age.resize((nyears + 1) * nages);
    info->variable_map[this->numbers_at_age.id_m] = &(population)->numbers_at_age;

    // add to Information
    info->populations[population->id] = population;

    return true;
  }

  /**
   * @brief Adds the parameter values and derivatives to the TMB model object.
   *
  */
  virtual bool add_to_fims_tmb() {
    FIMS_INFO_LOG("adding Population object to TMB");
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

#endif
