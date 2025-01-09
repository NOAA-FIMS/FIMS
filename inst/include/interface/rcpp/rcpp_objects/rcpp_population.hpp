/**
 * @file rcpp_population.hpp
 * @brief The Rcpp interface to declare different types of populations. Allows
 * for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_POPULATION_HPP

#include "../../../population_dynamics/population/population.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp population
 * interfaces. This type should be inherited and not called from R directly.
 */
class PopulationInterfaceBase : public FIMSRcppInterfaceBase {
public:
    /**
     * @brief The static id of the PopulationInterfaceBase object.
     */
    static uint32_t id_g;
    /**
     * @brief The local id of the PopulationInterfaceBase object.
     */
    uint32_t id;
    /**
     * @brief The map associating the IDs of PopulationInterfaceBase to the objects.
     * This is a live object, which is an object that has been created and lives
     * in memory.
     */
    static std::map<uint32_t, PopulationInterfaceBase*> live_objects;

    /**
     * @brief The constructor.
     */
    PopulationInterfaceBase() {
        this->id = PopulationInterfaceBase::id_g++;
        /* Create instance of map: key is id and value is pointer to
        PopulationInterfaceBase */
        PopulationInterfaceBase::live_objects[this->id] = this;
        PopulationInterfaceBase::fims_interface_objects.push_back(this);
    }

    /**
     * @brief The destructor.
     */
    virtual ~PopulationInterfaceBase() {
    }

    /**
     * @brief Get the ID for the child population interface objects to inherit.
     */
    virtual uint32_t get_id() = 0;
};
// static id of the PopulationInterfaceBase object
uint32_t PopulationInterfaceBase::id_g = 1;
// local id of the PopulationInterfaceBase object map relating the ID of the
// PopulationInterfaceBase to the PopulationInterfaceBase objects
std::map<uint32_t, PopulationInterfaceBase*>
PopulationInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for a new Population to instantiate from R:
 * population <- methods::new(population)
 */
class PopulationInterface : public PopulationInterfaceBase {
public:
    /**
     * @brief The number of age bins.
     */
    uint32_t nages;
    /**
     * @brief The number of fleets.
     */
    uint32_t nfleets;
    /**
     * @brief The number of seasons.
     * TODO: Remove seasons because we do not model them.
     */
    uint32_t nseasons;
    /**
     * @brief The number of years.
     */
    uint32_t nyears;
    /**
     * @brief The number of length bins.
     */
    uint32_t nlengths;
    /**
     * @brief The ID of the maturity module.
     */
    uint32_t maturity_id;
    /**
     * @brief The ID of the growth module.
     */
    uint32_t growth_id;
    /**
     * @brief The ID of the recruitment module.
     */
    uint32_t recruitment_id;
    /**
     * @brief The natural log of the natural mortality for each year.
     */
    ParameterVector log_M;
    /**
     * @brief The natural log of the initial numbers at age.
     */
    ParameterVector log_init_naa;
    /**
     * @brief Numbers at age.
     */
    ParameterVector numbers_at_age;
    /**
     * @brief Ages that are modeled in the population, the length of this vector
     * should equal \"nages\".
     */
    Rcpp::NumericVector ages;
    /**
     * @brief Derived spawning biomass.
     * TODO: This should be sb not ssb if left as an acronym.
     */
    Rcpp::NumericVector derived_ssb;
    /**
     * @brief Derived numbers at age.
     */
    Rcpp::NumericVector derived_naa;
    /**
     * @brief Derived biomass (mt).
     */
    Rcpp::NumericVector derived_biomass;
    /**
     * @brief Derived recruitment.
     * TODO: document the unit.
     */
    Rcpp::NumericVector derived_recruitment;
    /**
     * @brief Estimated natural log of mortality.
     */
    Rcpp::NumericVector estimated_log_M;
    /**
     * @brief Estimated natural log of initial numbers at age.
     */
    Rcpp::NumericVector estimated_log_init_naa;
    /**
     * @brief The name.
     * TODO: Document name better.
     */
    std::string name;

    /**
     * @brief The constructor.
     */
    PopulationInterface() : PopulationInterfaceBase() {
    }

    /**
     * @brief The destructor.
     */
    virtual ~PopulationInterface() {
    }

    /**
     * @brief Gets the ID of the interface base object.
     * @return The ID.
     */
    virtual uint32_t get_id() {
        return this->id;
    }

    /**
     * @brief Sets the unique ID for the Maturity object.
     * @param maturity_id Unique ID for the Maturity object.
     */
    void SetMaturity(uint32_t maturity_id) {
        this->maturity_id = maturity_id;
    }

    /**
     * @brief Set the unique ID for the growth object.
     * @param growth_id Unique ID for the growth object.
     */
    void SetGrowth(uint32_t growth_id) {
        this->growth_id = growth_id;
    }

    /**
     * @brief Set the unique ID for the recruitment object.
     * @param recruitment_id Unique ID for the recruitment object.
     */
    void SetRecruitment(uint32_t recruitment_id) {
        this->recruitment_id = recruitment_id;
    }

    /**
     * @brief Evaluate the population function.
     */
    virtual void evaluate() {
        fims_popdy::Population<double> population;
        return population.Evaluate();
    }

    /** 
     * @brief Extracts derived quantities back to the Rcpp interface object from
     * the Information object. 
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

            this->derived_naa = Rcpp::NumericVector(pop->numbers_at_age.size());
            this->derived_ssb = Rcpp::NumericVector(pop->spawning_biomass.size());
            this->derived_biomass = Rcpp::NumericVector(pop->biomass.size());
            this->derived_recruitment = Rcpp::NumericVector(pop->expected_recruitment.size());

            //set naa from Information/
            for (R_xlen_t i = 0; i < this->derived_naa.size(); i++) {
                this->derived_naa[i] = pop->numbers_at_age[i];
            }

            //set ssb from Information/
            for (R_xlen_t i = 0; i < this->derived_ssb.size(); i++) {
                this->derived_ssb[i] = pop->spawning_biomass[i];
            }

            //set biomass from Information
            for (R_xlen_t i = 0; i < this->derived_biomass.size(); i++) {
                this->derived_biomass[i] = pop->biomass[i];
            }

            //set recruitment from Information/
            for (R_xlen_t i = 0; i < this->derived_recruitment.size(); i++) {
                this->derived_recruitment[i] = pop->expected_recruitment[i];
            }

        }

    }

    /**
     * @brief Converts the data to json representation for the output.
     * @return A string is returned specifying that the module relates to the
     * population interface. It also returns the ID for each associated module
     * and the values associated with that module. Then it returns several
     * derived quantities. This string is formatted for a json file.
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
        ss << " \"values\": [" << this->log_M << "]\n},\n";

        ss << " \"parameter\": {\n";
        ss << "  \"name\": \"log_init_naa\",\n";
        ss << "  \"id\":" << -999 << ",\n";
        ss << "  \"type\": \"vector\",\n";
        ss << "  \"values\":[" << this->log_init_naa << "] \n},\n";

        ss << " \"derived_quantity\": {\n";
        ss << "  \"name\": \"ssb\",\n";
        ss << "  \"values\":[";
        if (this->derived_ssb.size() == 0) {
            ss << "]\n";
        } else {
            for (R_xlen_t i = 0; i < this->derived_ssb.size() - 1; i++) {
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
            for (R_xlen_t i = 0; i < this->derived_naa.size() - 1; i++) {
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
            for (R_xlen_t i = 0; i < this->derived_biomass.size() - 1; i++) {
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
            for (R_xlen_t i = 0; i < this->derived_recruitment.size() - 1; i++) {
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

        population->numbers_at_age.resize((nyears + 1) * nages);
        info->variable_map[this->numbers_at_age.id_m] = &(population)->numbers_at_age;

        // add to Information
        info->populations[population->id] = population;

        return true;
    }

    /**
     * @brief Adds the parameters to the TMB model.
     * @return A boolean of true.
     */
    virtual bool add_to_fims_tmb() {
        FIMS_INFO_LOG("adding Population object to TMB");
#ifdef TMBAD_FRAMEWORK
        this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
        this->add_to_fims_tmb_internal<TMB_AD_TYPE>();
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
