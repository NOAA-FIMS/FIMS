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
    uint32_t id; /**< id of the population interface base */
    static std::map<uint32_t, PopulationInterfaceBase*>
    live_objects; /**< map associating the ids of PopulationInterfaceBase to
                       the objects */

    PopulationInterfaceBase() {
        this->id = PopulationInterfaceBase::id_g++;
        PopulationInterfaceBase::live_objects[this->id] = this;
        PopulationInterfaceBase::fims_interface_objects.push_back(this);
    }

    virtual ~PopulationInterfaceBase() {
    }

    /** @brief get_id method for child classes to inherit */
    virtual uint32_t get_id() = 0;
};

uint32_t PopulationInterfaceBase::id_g = 1;
std::map<uint32_t, PopulationInterfaceBase*>
PopulationInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for a new Population. To instantiate
 * from R:
 * population <- new(fims$population)
 */
class PopulationInterface : public PopulationInterfaceBase {
public:
    uint32_t nages; /**< number of ages */
    uint32_t nfleets; /**< number of fleets */
    uint32_t nseasons; /**< number of seasons */
    uint32_t nyears; /**< number of years */
    uint32_t maturity_id; /**< id of the maturity function*/
    uint32_t growth_id; /**< id of the growth function*/
    uint32_t recruitment_id; /**< id of the recruitment function*/
    Rcpp::NumericVector log_M; /**< log of the natural mortality of the stock*/
    Rcpp::NumericVector log_init_naa; /**<log of the initial numbers at age*/
    Rcpp::NumericVector ages; /**<vector of ages in the population; length nages*/
    double prop_female; /**< the proportion of female fish*/
    bool estimate_M; /**<whether parameter should be estimated*/
    bool estimate_initNAA; /**<whether parameter should be estimated*/

    PopulationInterface() : PopulationInterfaceBase() {
    }

    virtual ~PopulationInterface() {
    }

    virtual uint32_t get_id() {
        return this->id;
    }

    /**
     * @brief Set the unique id for the Maturity object
     *
     * @param maturity_id Unique id for the Maturity object
     */
    void SetMaturity(uint32_t maturity_id) {
        this->maturity_id = maturity_id;
    }

    /**
     * @brief Set the unique id for the growth object
     *
     * @param growth_id Unique id for the growth object
     */
    void SetGrowth(uint32_t growth_id) {
        this->growth_id = growth_id;
    }

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

#ifdef TMB_MODEL

    template<typename T>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims::Information<T> > info =
                fims::Information<T>::GetInstance();

        std::shared_ptr<fims::Population<T> > population =
                std::make_shared<fims::Population<T> >();

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
            population->log_M[i] = this->log_M[i];
            if (estimate_M) {
                info->RegisterParameter(population->log_M[i]);
            }
        }

        for (size_t i = 0; i < log_init_naa.size(); i++) {
            population->log_init_naa[i] = this->log_init_naa[i];
            if (estimate_initNAA) {
                info->RegisterParameter(population->log_init_naa[i]);
            }

        }
        for (size_t i = 0; i < ages.size(); i++) {
            population->ages[i] = this->ages[i];
        }

        // add to Information
        info->populations[population->id] = population;

        return true;
    }

    /** @brief this adds the parameter values and derivatives to the TMB model
     * object */
    virtual bool add_to_fims_tmb() {

        this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
        this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
        this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

        return true;
    }

#endif
};

#endif
