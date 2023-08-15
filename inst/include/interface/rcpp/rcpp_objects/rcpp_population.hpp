/*
 * File:   rcpp_population.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP

#include "../../../population_dynamics/population/population.hpp"
#include "rcpp_interface_base.hpp"

/****************************************************************
 * Population Rcpp interface                                   *
 ***************************************************************/
/**
 * @brief PopulationInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible Population function
 * */
class PopulationInterfaceBase : public FIMSRcppInterfaceBase {
public:
  static uint32_t id_g; /**< static id of the population interface base*/
  uint32_t id;          /**< id of the population interface base */
  static std::map<uint32_t, PopulationInterfaceBase *>
      live_objects; /**< map associating the ids of PopulationInterfaceBase to
                       the objects */

  PopulationInterfaceBase() {
    this->id = PopulationInterfaceBase::id_g++;
    PopulationInterfaceBase::live_objects[this->id] = this;
    PopulationInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~PopulationInterfaceBase() {}

  /** @brief get_id method for child classes to inherit */
  virtual uint32_t get_id() = 0;
};

uint32_t PopulationInterfaceBase::id_g = 1;
std::map<uint32_t, PopulationInterfaceBase *>
    PopulationInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for a new Population. To instantiate
 * from R:
 * population <- new(fims$population)
 */
class PopulationInterface : public PopulationInterfaceBase {
public:
  uint32_t nages;            /**< number of ages */
  uint32_t nfleets;          /**< number of fleets */
  uint32_t nseasons;         /**< number of seasons */
  uint32_t nyears;           /**< number of years */
  uint32_t maturity_id;      /**< id of the maturity function*/
  uint32_t growth_id;        /**< id of the growth function*/
  uint32_t recruitment_id;   /**< id of the recruitment function*/
  Rcpp::NumericVector log_M; /**< log of the natural mortality of the stock*/
  Rcpp::NumericVector log_init_naa; /**<log of the initial numbers at age*/
  Rcpp::NumericVector ages; /**<vector of ages in the population; length nages*/
  double prop_female;       /**< the proportion of female fish*/
  bool estimate_M;          /**<whether parameter should be estimated*/
  bool estimate_initNAA;    /**<whether parameter should be estimated*/

  PopulationInterface() : PopulationInterfaceBase() {}

  virtual ~PopulationInterface() {}

  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Set the unique id for the Maturity object
   *
   * @param maturity_id Unique id for the Maturity object
   */
  void SetMaturity(uint32_t maturity_id) { this->maturity_id = maturity_id; }

  /**
   * @brief Set the unique id for the growth object
   *
   * @param growth_id Unique id for the growth object
   */
  void SetGrowth(uint32_t growth_id) { this->growth_id = growth_id; }

  /**
   * @brief Set the unique id for the Maturity object
   *
   * @param recruitment_id Unique id for the Maturity object
   */
  void SetRecruitment(uint32_t recruitment_id) {
    this->recruitment_id = recruitment_id;
  }

  /** @brief evaluate the population function */
  virtual void evaluate() {
    fims::Population<double> population;
    return population.Evaluate();
  }

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    // base model
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE>> d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::Population<TMB_FIMS_REAL_TYPE>> b0 =
        std::make_shared<fims::Population<TMB_FIMS_REAL_TYPE>>();

    // set relative info
    b0->id = this->id;
    b0->nyears = this->nyears;
    b0->nfleets = this->nfleets;
    b0->nseasons = this->nseasons;
    b0->nages = this->nages;
    if (this->nages == this->ages.size()) {
      b0->ages.resize(this->nages);
    } else {
      warning("The ages vector is not of size nages.");
    }

    b0->growth_id = this->growth_id;
    b0->recruitment_id = this->recruitment_id;
    b0->maturity_id = this->maturity_id;
    b0->log_M.resize(this->log_M.size());
    b0->log_init_naa.resize(this->log_init_naa.size());
    for (int i = 0; i < log_M.size(); i++) {
      b0->log_M[i] = this->log_M[i];
      if (estimate_M) {
        d0->RegisterParameter(b0->log_M[i]);
      }
    }

    for (int i = 0; i < log_init_naa.size(); i++) {
      b0->log_init_naa[i] = this->log_init_naa[i];
      if (estimate_initNAA) {
        d0->RegisterParameter(b0->log_init_naa[i]);
      }
    }
    for (int i = 0; i < ages.size(); i++) {
      b0->ages[i] = this->ages[i];
    }

    // add to Information
    d0->populations[b0->id] = b0;

    // first-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER>> d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::Population<TMB_FIMS_FIRST_ORDER>> b1 =
        std::make_shared<fims::Population<TMB_FIMS_FIRST_ORDER>>();

    // set relative info
    b1->id = this->id;
    b1->nyears = this->nyears;
    b1->nfleets = this->nfleets;
    b1->nseasons = this->nseasons;
    b1->nages = this->nages;
    b1->ages.resize(this->nages);

    b1->growth_id = this->growth_id;
    b1->recruitment_id = this->recruitment_id;
    b1->maturity_id = this->maturity_id;

    b1->log_M.resize(this->log_M.size());
    b1->log_init_naa.resize(this->log_init_naa.size());
    for (int i = 0; i < log_M.size(); i++) {
      b1->log_M[i] = this->log_M[i];
      if (estimate_M) {
        d1->RegisterParameter(b1->log_M[i]);
      }
    }
    for (int i = 0; i < log_init_naa.size(); i++) {
      b1->log_init_naa[i] = this->log_init_naa[i];
      if (estimate_initNAA) {
        d1->RegisterParameter(b1->log_init_naa[i]);
      }
    }
    for (int i = 0; i < ages.size(); i++) {
      b1->ages[i] = this->ages[i];
    }

    // add to Information
    d1->populations[b1->id] = b1;

    // second-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER>> d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::Population<TMB_FIMS_SECOND_ORDER>> b2 =
        std::make_shared<fims::Population<TMB_FIMS_SECOND_ORDER>>();

    // set relative info
    b2->id = this->id;
    b2->nyears = this->nyears;
    b2->nfleets = this->nfleets;
    b2->nseasons = this->nseasons;
    b2->nages = this->nages;
    b2->log_M.resize(this->log_M.size());
    b2->ages.resize(nages);

    b2->growth_id = this->growth_id;
    b2->recruitment_id = this->recruitment_id;
    b2->maturity_id = this->maturity_id;

    b2->log_init_naa.resize(this->log_init_naa.size());
    for (int i = 0; i < log_M.size(); i++) {
      b2->log_M[i] = this->log_M[i];
      if (estimate_M) {
        d2->RegisterParameter(b2->log_M[i]);
      }
    }
    for (int i = 0; i < log_init_naa.size(); i++) {
      b2->log_init_naa[i] = this->log_init_naa[i];
      if (estimate_initNAA) {
        d2->RegisterParameter(b2->log_init_naa[i]);
      }
    }

    for (int i = 0; i < ages.size(); i++) {
      b2->ages[i] = this->ages[i];
    }

    // add to Information
    d2->populations[b2->id] = b2;

    // third-order derivative
    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER>> d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::Population<TMB_FIMS_THIRD_ORDER>> b3 =
        std::make_shared<fims::Population<TMB_FIMS_THIRD_ORDER>>();

    // set relative info
    b3->id = this->id;
    b3->nyears = this->nyears;
    b3->nfleets = this->nfleets;
    b3->nseasons = this->nseasons;
    b3->nages = this->nages;
    b3->log_M.resize(this->log_M.size());
    b3->log_init_naa.resize(this->log_init_naa.size());
    b3->ages.resize(this->nages);

    b3->growth_id = this->growth_id;
    b3->recruitment_id = this->recruitment_id;
    b3->maturity_id = this->maturity_id;

    for (int i = 0; i < log_M.size(); i++) {
      b3->log_M[i] = this->log_M[i];
      if (estimate_M) {
        d3->RegisterParameter(b3->log_M[i]);
      }
    }
    for (int i = 0; i < log_init_naa.size(); i++) {
      b3->log_init_naa[i] = this->log_init_naa[i];
      if (estimate_initNAA) {
        d3->RegisterParameter(b3->log_init_naa[i]);
      }
    }

    for (int i = 0; i < ages.size(); i++) {
      b3->ages[i] = this->ages[i];
    }

    // add to Information
    d3->populations[b3->id] = b3;

    return true;
  }
};

#endif
