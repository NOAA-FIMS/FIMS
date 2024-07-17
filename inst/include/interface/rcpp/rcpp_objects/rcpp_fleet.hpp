/*
 * File:   rcpp_fleet.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_FLEET_HPP

#include "../../../common/def.hpp"
#include "../../../population_dynamics/fleet/fleet.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for
 * Rcpp fleet interfaces. This type should be inherited and not
 * called from R directly.
 *
 */
class FleetInterfaceBase : public FIMSRcppInterfaceBase {
public:
    static uint32_t id_g; /**< static id of the FleetInterfaceBase object */
    uint32_t id; /**< local id of the FleetInterfaceBase object */
    // live objects in C++ are objects that have been created and live in memory
    static std::map<uint32_t, FleetInterfaceBase*> live_objects; /**<
map relating the ID of the FleetInterfaceBase to the FleetInterfaceBase
objects */

    FleetInterfaceBase() {
        this->id = FleetInterfaceBase::id_g++;
        /* Create instance of map: key is id and value is pointer to
        FleetInterfaceBase */
        FleetInterfaceBase::live_objects[this->id] = this;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);


    }

    virtual ~FleetInterfaceBase() {
    }

    /** @brief get_id method for child fleet interface objects to inherit */
    virtual uint32_t get_id() = 0;
};

uint32_t FleetInterfaceBase::id_g = 1;
std::map<uint32_t, FleetInterfaceBase*> FleetInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Fleet as an S4 object. To instantiate
 * from R:
 * fleet <- new(Fleet)
 *
 */
class FleetInterface : public FleetInterfaceBase {
    int interface_agecomp_likelihood_id_m =
            -999; /**< id of agecomp likelihood component. The "interface_" prefix
               indicates it belongs to the FleetInterface class, and the "_m"
               postfix signifies that it's a member variable. */
    int interface_index_likelihood_id_m =
            -999; /**< id of index likelihood component*/
    int interface_observed_agecomp_data_id_m =
            -999; /**< id of observed agecomp data object*/
    int interface_observed_index_data_id_m =
            -999; /**< id of observed index data object*/
    int interface_selectivity_id_m = -999; /**< id of selectivity component*/

public:
    std::string name; /**< name of this fleet */
    bool is_survey = false; /**< whether this is a survey fleet */
    int nages; /**< number of ages in the fleet data*/
    int nyears; /**< number of years in the fleet data */
    Parameter log_q; /**< log of catchability for the fleet*/
    Rcpp::NumericVector
    log_Fmort; /**< log of fishing mortality rate for the fleet*/
    bool estimate_F = false; /**< whether the parameter F should be estimated*/
    //    bool estimate_q = false; /**< whether the parameter q should be estimated*/
    bool estimate_obs_error = false; /**< whether the parameter log_obs_error
                                          should be estimated*/
    //    bool random_q = false; /**< whether q should be a random effect*/
    bool random_F = false; /**< whether F should be a random effect*/
    Rcpp::NumericVector log_obs_error; /**< the log of the observation error */

    Rcpp::NumericVector derived_cnaa; /**< derived quantity: catch numbers at age */
    Rcpp::NumericVector derived_cwaa; /**< derived quantity: catch weight at age */
    Rcpp::NumericVector derived_index; /**< derived quantity: expected index */
    Rcpp::NumericVector derived_age_composition; /**< derived quantity: expected catch */

    Rcpp::NumericVector estimated_log_Fmort; /**< estimated log of fishing mortality rate for the fleet*/
    double estimated_log_q; /**< estimated log of catchability for the fleet*/
    Rcpp::NumericVector estimated_log_obs_error; /**< estimated log of the observation error */

    FleetInterface() : FleetInterfaceBase() {
        std::stringstream ss;
        ss << "fleet_" << this->id;
        this->name = ss.str();
    }

    virtual ~FleetInterface() {
    }

    /** @brief returns the id for the fleet interface */
    virtual uint32_t get_id() {
        return this->id;
    }

    /**
     * @brief Set the unique id for the Age Comp Likelihood object
     *
     * @param agecomp_likelihood_id Unique id for the Age Comp Likelihood object
     */
    void SetAgeCompLikelihood(int agecomp_likelihood_id) {
        interface_agecomp_likelihood_id_m = agecomp_likelihood_id;
    }

    /**
     * @brief Set the unique id for the Index Likelihood object
     *
     * @param index_likelihood_id Unique id for the Index Likelihood object
     */
    void SetIndexLikelihood(int index_likelihood_id) {
        interface_index_likelihood_id_m = index_likelihood_id;
    }

    /**
     * @brief Set the unique id for the Observed Age Comp Data object
     *
     * @param observed_agecomp_data_id Unique id for the Observed Age Comp Data
     * object
     */
    void SetObservedAgeCompData(int observed_agecomp_data_id) {
        interface_observed_agecomp_data_id_m = observed_agecomp_data_id;
    }

    /**
     * @brief Set the unique id for the Observed Index Data object
     *
     * @param observed_index_data_id Unique id for the Observed Index Data object
     */
    void SetObservedIndexData(int observed_index_data_id) {
        interface_observed_index_data_id_m = observed_index_data_id;
    }

    /**
     * @brief Set the unique id for the Selectivity object
     *
     * @param selectivity_id Unique id for the Selectivity object
     */
    void SetSelectivity(int selectivity_id) {
        interface_selectivity_id_m = selectivity_id;
    }

    virtual void finalize() {


        if (this->finalized) {
            //log warning that finalize has been called more than once.
            FIMS_WARNING_LOG("Population " + fims::to_string(this->id) + " has been finalized already.");
        }

        this->finalized = true; //indicate this has been called already

        std::shared_ptr<fims_info::Information<double> > info =
                fims_info::Information<double>::GetInstance();



        this->estimated_log_Fmort = Rcpp::NumericVector(this->log_Fmort.size());
        for (size_t i = 0; i < this->estimated_log_Fmort.size(); i++) {
            this->estimated_log_Fmort[i] = this->log_Fmort[i];
        }

        this->estimated_log_obs_error = Rcpp::NumericVector(this->log_obs_error.size());
        for (size_t i = 0; i < this->estimated_log_obs_error.size(); i++) {
            this->estimated_log_obs_error[i] = this->log_obs_error[i];
        }

        this->estimated_log_q = this->log_q.value_m;

        fims_info::Information<double>::fleet_iterator it;

        it = info->fleets.find(this->id);

        if (it == info->fleets.end()) {
            FIMS_WARNING_LOG("Fleet " + fims::to_string(this->id) + " not found in Information.");
            return;
        } else {

            std::shared_ptr<fims_popdy::Fleet<double> > fleet =
                    std::dynamic_pointer_cast<fims_popdy::Fleet<double> >(it->second);

            if (this->log_q.estimated_m) {
                this->estimated_log_q = fleet->log_q;
            }

            if (this->estimate_F) {
                for (size_t i = 0; i < this->estimated_log_Fmort.size(); i++) {
                    this->estimated_log_Fmort[i] = fleet->log_Fmort[i];
                }
            }

            if (this->estimate_obs_error) {
                for (size_t i = 0; i < this->estimated_log_obs_error.size(); i++) {
                    this->estimated_log_obs_error[i] = fleet->log_obs_error[i];
                }
            }


            //resize and set cnaa from Information/
            this->derived_cnaa = Rcpp::NumericVector(fleet->catch_numbers_at_age.size());
            for (size_t i = 0; i < this->derived_cnaa.size(); i++) {
                this->derived_cnaa[i] = fleet->catch_numbers_at_age[i];
            }

            //resize and set ssb from Information/
            this->derived_cwaa = Rcpp::NumericVector(fleet->catch_weight_at_age.size());
            for (size_t i = 0; i < this->derived_cwaa.size(); i++) {
                this->derived_cwaa[i] = fleet->catch_weight_at_age[i];
            }

            //resize and set biomass from Information/
            this->derived_age_composition = Rcpp::NumericVector(fleet->age_composition.size());
            for (size_t i = 0; i < this->derived_age_composition.size(); i++) {
                this->derived_age_composition[i] = fleet->age_composition[i];
            }

            //resize and set recruitment from Information/
            this->derived_index = Rcpp::NumericVector(fleet->expected_index.size());
            for (size_t i = 0; i < this->derived_index.size(); i++) {
                this->derived_index[i] = fleet->expected_index[i];
            }
        }


    }

    virtual std::string to_json() {
        std::stringstream ss;

        ss << "\"module\" : {\n";
        ss << " \"name\": \"Fleet\",\n";
        if (this->is_survey) {
            ss << " \"type\" : \"survey\",\n";
        } else {
            ss << " \"type\" : \"fleet\",\n";
        }
        ss << " \"tag\" : \"" << this->name << "\",\n";
        ss << " \"id\": " << this->id << ",\n";
        ss << " \"agecomp_likelihood_id\": " << this->interface_agecomp_likelihood_id_m << ",\n";
        ss << " \"index_likelihood_id\": " << this->interface_index_likelihood_id_m << ",\n";
        ss << " \"observed_agecomp_data_id\": " << this->interface_observed_agecomp_data_id_m << ",\n";
        ss << " \"observed_index_data_id\": " << this->interface_observed_index_data_id_m << ",\n";
        ss << " \"selectivity_id\": " << this->interface_selectivity_id_m << ",\n";
        ss << " \"is_survey\": " << this->is_survey << ",\n";

        ss << " \"parameter\": {\n";
        ss << "   \"name\": \"log_q\",\n";
        ss << "   \"id\":" << this->log_q.id_m << ",\n";
        ss << "   \"type\": \"scalar\",\n";
        ss << "   \"value\":" << this->log_q.value_m << ",\n";
        ss << "   \"estimated_value\":" << this->estimated_log_q << ",\n";
        ss << "   \"is_estimated\":" << this->log_q.estimated_m << ",\n";
        ss << "   \"is_random_effect\":" << this->log_q.is_random_effect_m << "\n },\n";

        ss << " \"parameter\": {\n";
        ss << "   \"name\": \"log_Fmort\",\n";
        ss << "   \"id\":" << -999 << ",\n";
        ss << "   \"type\": \"vector\",\n";
        ss << "   \"values\":[";

        if (this->log_Fmort.size() == 0) {
            ss << "],\n";
        } else {
            for (size_t i = 0; i < this->log_Fmort.size() - 1; i++) {
                ss << this->log_Fmort[i] << ", ";
            }
            ss << this->log_Fmort[this->log_Fmort.size() - 1] << "],\n";
        }

        ss << "   \"estimated_values\":[";
        if (this->estimated_log_Fmort.size() == 0) {
            ss << "],\n";
        } else {
            for (size_t i = 0; i < this->estimated_log_Fmort.size() - 1; i++) {
                ss << this->estimated_log_Fmort[i] << ", ";
            }
            ss << this->estimated_log_Fmort[this->estimated_log_Fmort.size() - 1] << "],\n";
        }

        ss << "   \"is_estimated\":" << this->estimate_F << ",\n";
        ss << "   \"is_random_effect\":" << 0 << "\n },\n";

        ss << " \"parameter\": {\n";
        ss << "   \"name\": \"log_obs_error\",\n";
        ss << "   \"id\":" << -999 << ",\n";
        ss << "   \"type\": \"vector\",\n";
        ss << "   \"values\":[";

        if (this->log_obs_error.size() == 0) {
            ss << "],\n";
        } else {
            for (size_t i = 0; i < this->log_obs_error.size() - 1; i++) {
                ss << this->log_obs_error[i] << ", ";
            }
            ss << this->log_obs_error[this->log_obs_error.size() - 1] << "],\n";
        }

        ss << "   \"estimated_values\":[";
        if (this->estimated_log_obs_error.size() == 0) {
            ss << "],\n";
        } else {
            for (size_t i = 0; i < this->estimated_log_obs_error.size() - 1; i++) {
                ss << this->estimated_log_obs_error[i] << ", ";
            }
            ss << this->estimated_log_obs_error[this->estimated_log_obs_error.size() - 1] << "],\n";
        }

        ss << "   \"is_estimated\":" << this->estimate_obs_error << ",\n";
        ss << "   \"is_random_effect\":" << 0 << "\n },\n";

        ss << " \"derived_quantity\": {\n";
        ss << "   \"name\": \"cnaa\",\n";
        ss << "   \"values\":[";
        if (this->derived_cnaa.size() == 0) {
            ss << "]\n";
        } else {
            for (size_t i = 0; i < this->derived_cnaa.size() - 1; i++) {
                ss << this->derived_cnaa[i] << ", ";
            }
            ss << this->derived_cnaa[this->derived_cnaa.size() - 1] << "]\n";
        }
        ss << " },\n";

        ss << " \"derived_quantity\": {\n";
        ss << "   \"name\": \"cwaa\",\n";
        ss << "   \"values\":[";
        if (this->derived_cwaa.size() == 0) {
            ss << "]\n";
        } else {
            for (size_t i = 0; i < this->derived_cwaa.size() - 1; i++) {
                ss << this->derived_cwaa[i] << ", ";
            }
            ss << this->derived_cwaa[this->derived_cwaa.size() - 1] << "]\n";
        }
        ss << " },\n";

        ss << "  \"derived_quantity\": {\n";
        ss << "   \"name\": \"index\",\n";
        ss << "   \"values\":[";
        if (this->derived_index.size() == 0) {
            ss << "]\n";
        } else {
            for (size_t i = 0; i < this->derived_index.size() - 1; i++) {
                ss << this->derived_index[i] << ", ";
            }
            ss << this->derived_index[this->derived_index.size() - 1] << "]\n";
        }
        ss << " },\n";

        ss << " \"derived_quantity\": {\n";
        ss << "   \"name\": \"age_composition\",\n";
        ss << "   \"values\":[";
        if (this->derived_age_composition.size() == 0) {
            ss << "]\n";
        } else {
            for (size_t i = 0; i < this->derived_age_composition.size() - 1; i++) {
                ss << this->derived_age_composition[i] << ", ";
            }
            ss << this->derived_age_composition[this->derived_age_composition.size() - 1] << "]\n";
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

        std::shared_ptr<fims_popdy::Fleet<Type> > fleet =
                std::make_shared<fims_popdy::Fleet<Type> >();

        // set relative info
        fleet->id = this->id;
        fleet->is_survey = this->is_survey;
        fleet->nages = this->nages;
        fleet->nyears = this->nyears;
        fleet->fleet_agecomp_likelihood_id_m = interface_agecomp_likelihood_id_m;
        fleet->fleet_index_likelihood_id_m = interface_index_likelihood_id_m;
        fleet->fleet_observed_agecomp_data_id_m =
                interface_observed_agecomp_data_id_m;
        fleet->fleet_observed_index_data_id_m = interface_observed_index_data_id_m;
        fleet->fleet_selectivity_id_m = interface_selectivity_id_m;

        fleet->log_obs_error.resize(this->log_obs_error.size());
        for (int i = 0; i < log_obs_error.size(); i++) {
            fleet->log_obs_error[i] = this->log_obs_error[i];
            if (this->estimate_obs_error) {
                info->RegisterParameter(fleet->log_obs_error[i]);
            }
        }

        fleet->log_q = this->log_q.value_m;
        if (this->log_q.estimated_m) {
            if (this->log_q.is_random_effect_m) {
                info->RegisterRandomEffect(fleet->log_q);
            } else {
                info->RegisterParameter(fleet->log_q);
            }
        }

        fleet->log_Fmort.resize(this->log_Fmort.size());
        for (int i = 0; i < log_Fmort.size(); i++) {
            fleet->log_Fmort[i] = this->log_Fmort[i];

            if (this->estimate_F) {
                if (this->random_F) {
                    info->RegisterRandomEffect(fleet->log_Fmort[i]);
                } else {
                    info->RegisterParameter(fleet->log_Fmort[i]);
                }
            }
        }
        // add to Information
        info->fleets[fleet->id] = fleet;

        return true;
    }

    /** @brief this adds the values to the TMB model object */
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
