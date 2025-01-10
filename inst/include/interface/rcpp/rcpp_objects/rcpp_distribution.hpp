/**
 * @file rcpp_distribution.hpp
 * @brief The Rcpp interface to declare different distributions, e.g.,
 * normal and log normal. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DISTRIBUTION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DISTRIBUTION_HPP

#include "../../../distributions/distributions.hpp"
#include "../../interface.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp distribution
 * interfaces. This type should be inherited and not called from R directly.
 */
class DistributionsInterfaceBase : public FIMSRcppInterfaceBase {
public:
    /**
     * @brief The static ID of the DistributionsInterfaceBase object.
     */
    static uint32_t id_g;
    /**
     * @brief The local ID of the DistributionsInterfaceBase object.
     */
    uint32_t id_m;
    /**
     * @brief The unique ID for the variable map that points to a fims::Vector.
     */
    std::vector<uint32_t> key_m;
    /**
     * @brief The type of density input. The options are prior, re, or data.
     */
    std::string input_type_m;
    /**
     * @brief The map associating the ID of the DistributionsInterfaceBase to the
       DistributionsInterfaceBase objects. This is a live object, which is an
       object that has been created and lives in memory.
     */
    static std::map<uint32_t, DistributionsInterfaceBase *> live_objects;
    /**
     * @brief The ID of the observed data object, which is set to -999.
     */
    uint32_t interface_observed_data_id_m = -999;

    /**
     * @brief The constructor.
     */
    DistributionsInterfaceBase() {
        this->id_m = DistributionsInterfaceBase::id_g++;
        /* Create instance of map: key is id and value is pointer to
        DistributionsInterfaceBase */
        DistributionsInterfaceBase::live_objects[this->id_m] = this;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
    }

    /**
     * @brief The destructor.
     */
    virtual ~DistributionsInterfaceBase() {
    }

    /**
     * @brief Get the ID for the child distribution interface objects to inherit.
     */
    virtual uint32_t get_id() = 0;

    /**
     * @brief Sets pointers for data observations, random effects, or priors.
     *
     * @param input_type String that sets whether the distribution type is for priors, random effects, or data.
     * @param ids Vector of unique ids for each linked parameter(s), derived
     * value(s), or observed data vector.
     */
    virtual bool set_distribution_links(std::string input_type, Rcpp::IntegerVector ids) {
        return false;
    }

    /**
     * @brief Set the unique ID for the observed data object.
     *
     * @param observed_data_id Unique ID for the Observed Age Comp Data
     * object
     */
    virtual bool set_observed_data(int observed_data_id) {
        return false;
    }

    /**
     * @brief A method for each child distribution interface object to inherit so
     * each distribution can have an evaluate() function.
     */
    virtual double evaluate() = 0;
};
// static id of the DistributionsInterfaceBase object
uint32_t DistributionsInterfaceBase::id_g = 1;
// local id of the DistributionsInterfaceBase object map relating the ID of the
// DistributionsInterfaceBase to the DistributionsInterfaceBase objects
std::map<uint32_t,
DistributionsInterfaceBase*> DistributionsInterfaceBase::live_objects;

/**
 * @brief The Rcpp interface for Dnorm to instantiate from R:
 * dnorm_ <- methods::new(DnormDistribution).
 */
class DnormDistributionsInterface : public DistributionsInterfaceBase {
public:
    /**
     * @brief Observed data.
     */
    ParameterVector x;
    /**
     * @brief The expected values, which would be the mean of x for this
     * distribution.
     */
    ParameterVector expected_values;
    /**
     * @brief The uncertainty, which would be the standard deviation of x for the
     * normal distribution.
     */
    ParameterVector log_sd;
    /**
     * @brief The vector. TODO: document this more.
     */
    Rcpp::NumericVector lpdf_vec; /**< The vector*/

    /**
     * @brief The constructor.
     */
    DnormDistributionsInterface() : DistributionsInterfaceBase() {
    }

    /**
     * @brief The destructor.
     */
    virtual ~DnormDistributionsInterface() {
    }

    /**
     * @brief Gets the ID of the interface base object.
     * @return The ID.
     */
    virtual uint32_t get_id() {
        return this->id_m;
    }

    /**
     * @brief Set the unique ID for the observed data object.
     * @param observed_data_id Unique ID for the observed data object.
     */
    virtual bool set_observed_data(int observed_data_id) {
        this->interface_observed_data_id_m = observed_data_id;
        return true;
    }

    /**
     * @brief Sets pointers for data observations, random effects, or priors.
     *
     * @param input_type String that sets whether the distribution type is for priors, random effects, or data.
     * @param ids Vector of unique ids for each linked parameter(s), derived
     * value(s), or observed data vector.
     */
    virtual bool set_distribution_links(std::string input_type, Rcpp::IntegerVector ids) {
        this->input_type_m = input_type;
        this->key_m.resize(ids.size());
        for (int i = 0; i < ids.size(); i++) {
            this->key_m[i] = ids[i];
        }
        return true;
    }

    /**
     * @brief Evaluate normal probability density function (pdf). The natural log
     * of the pdf is returned.
     * @return The natural log of the probability density function (pdf) is
     * returned.
     */
    virtual double evaluate() {
        fims_distributions::NormalLPDF<double> dnorm;
        dnorm.x.resize(this->x.size());
        dnorm.expected_values.resize(this->expected_values.size());
        dnorm.log_sd.resize(this->log_sd.size());
        for (size_t i = 0; i < x.size(); i++) {
            dnorm.x[i] = this->x[i].initial_value_m;
        }
        for (size_t i = 0; i < expected_values.size(); i++) {
            dnorm.expected_values[i] = this->expected_values[i].initial_value_m;
        }
        for (size_t i = 0; i < log_sd.size(); i++) {
            dnorm.log_sd[i] = this->log_sd[i].initial_value_m;
        }
        return dnorm.evaluate();
    }

    /**
     * @brief Extracts the derived quantities from `Information` to the Rcpp
     * object.
     */
    virtual void finalize() {
        if (this->finalized) {
            //log warning that finalize has been called more than once.
            FIMS_WARNING_LOG("DnormDistribution  " + fims::to_string(this->id_m) + " has been finalized already.");
        }

        this->finalized = true; //indicate this has been called already

        std::shared_ptr<fims_info::Information<double> > info =
                fims_info::Information<double>::GetInstance();

        fims_info::Information<double>::density_components_iterator it;

        //search for density component in Information
        it = info->density_components.find(this->id_m);
        //if not found, just return
        if (it == info->density_components.end()) {
            FIMS_WARNING_LOG("DnormDistribution " + fims::to_string(this->id_m) + " not found in Information.");
            return;
        } else {
            std::shared_ptr<fims_distributions::NormalLPDF<double> > dnorm =
                    std::dynamic_pointer_cast<fims_distributions::NormalLPDF<double> >(it->second);
            this->lpdf_vec = Rcpp::NumericVector(dnorm->lpdf_vec.size());
            for (R_xlen_t i = 0; i < this->lpdf_vec.size(); i++) {
                this->lpdf_vec[i] = dnorm->lpdf_vec[i];
            }
        }
    }

    /**
     * @brief Converts the data to json representation for the output.
     * @return A string is returned specifying that the module relates to the
     * distribution interface with a normal distribution. It also returns the ID
     * and the natural log of the probability density function values themselves.
     * This string is formatted for a json file.
     */
    virtual std::string to_json() {
        std::stringstream ss;
        ss << "\"module\" : {\n";
        ss << " \"name\": \"DnormDistribution\",\n";
        ss << " \"type\": \"normal\",\n";
        ss << " \"id\": " << this->id_m << ",\n";

        ss << " \"density_component\": {\n";
        ss << "  \"name\": \"lpdf_vec\",\n";
        ss << "  \"values\":[";
        if (this->lpdf_vec.size() == 0) {
            ss << "]\n";
        } else {
            for (R_xlen_t i = 0; i < this->lpdf_vec.size() - 1; i++) {
                ss << this->lpdf_vec[i] << ", ";
            }
            ss << this->lpdf_vec[this->lpdf_vec.size() - 1] << "]\n";
        }
        ss << " }\n}";

        return ss.str();
    }


#ifdef TMB_MODEL

    template <typename Type>
    bool add_to_fims_tmb_internal() {



        std::shared_ptr<fims_info::Information < Type>> info =
                fims_info::Information<Type>::GetInstance();

        std::shared_ptr<fims_distributions::NormalLPDF < Type>> distribution =
                std::make_shared<fims_distributions::NormalLPDF < Type >> ();

        // interface to data/parameter value

        distribution->observed_data_id_m =
                interface_observed_data_id_m;
        distribution->input_type = this->input_type_m;
        distribution->key.resize(this->key_m.size());
        for (size_t i = 0; i<this->key_m.size(); i++) {
            distribution->key[i] = this->key_m[i];
        }
        distribution->id = this->id_m;
        distribution->x.resize(this->x.size());
        for (size_t i = 0; i<this->x.size(); i++) {
            distribution->x[i] = this->x[i].initial_value_m;
        }
        // set relative info
        distribution->expected_values.resize(this->expected_values.size());
        for (size_t i = 0; i<this->expected_values.size(); i++) {
            distribution->expected_values[i] = this->expected_values[i].initial_value_m;
        }
        distribution->log_sd.resize(this->log_sd.size());
        for (size_t i = 0; i<this->log_sd.size(); i++) {
            distribution->log_sd[i] = this->log_sd[i].initial_value_m;
            if (this->log_sd[i].estimated_m) {
                info->RegisterParameterName("normal log_sd");
                info->RegisterParameter(distribution->log_sd[i]);
            }
            if (this->log_sd[i].is_random_effect_m) {
                error("standard deviations cannot be set to random effects");
            }
        }
        info->variable_map[this->log_sd.id_m] = &(distribution)->log_sd;

        info->density_components[distribution->id] = distribution;

        return true;
    }

    /**
     * @brief Adds the parameters to the TMB model.
     * @return A boolean of true.
     */
    virtual bool add_to_fims_tmb() {

#ifdef TMBAD_FRAMEWORK

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
 * @brief The Rcpp interface for Dlnorm to instantiate from R:
 * dlnorm_ <- methods::new(DlnormDistribution).
 */
class DlnormDistributionsInterface : public DistributionsInterfaceBase {
public:
    /**
     * @brief Observed data.
     */
    ParameterVector x;
    /**
     * @brief The expected values, which would be the mean of log(x) for this
     * distribution.
     */
    ParameterVector expected_values;
    /**
     * @brief The uncertainty, which would be the natural logarithm of the
       standard deviation (sd) of log(x) for this distribution. The natural log
       of the standard deviation is necessary because the exponential link
       function is applied to the log transformed standard deviation to insure
       standard deviation is positive.
     */
    ParameterVector log_sd;
    /**
     * @brief The vector. TODO: document this more.
     */
    Rcpp::NumericVector lpdf_vec; /**< The vector */

    /**
     * @brief The constructor.
     */
    DlnormDistributionsInterface() : DistributionsInterfaceBase() {
    }

    /**
     * @brief The destructor.
     */
    virtual ~DlnormDistributionsInterface() {
    }

    /**
     * @brief Gets the ID of the interface base object.
     * @return The ID.
     */
    virtual uint32_t get_id() {
        return this->id_m;
    }

    /**
     * @brief Set the unique ID for the observed data object.
     * @param observed_data_id Unique ID for the observed data object.
     */
    virtual bool set_observed_data(int observed_data_id) {
        this->interface_observed_data_id_m = observed_data_id;
        return true;
    }

    /**
     * @brief Sets pointers for data observations, random effects, or priors.
     *
     * @param input_type String that sets whether the distribution type is for priors, random effects, or data.
     * @param ids Vector of unique ids for each linked parameter(s), derived
     * value(s), or observed data vector.
     */
    virtual bool set_distribution_links(std::string input_type, Rcpp::IntegerVector ids) {
        this->input_type_m = input_type;
        this->key_m.resize(ids.size());
        for (int i = 0; i < ids.size(); i++) {
            this->key_m[i] = ids[i];
        }
        return true;
    }

    /**
     * @brief Evaluate lognormal probability density function (pdf). The natural
     * log of the pdf is returned.
     * @return The natural log of the probability density function (pdf) is
     * returned.
     */
    virtual double evaluate() {
        fims_distributions::LogNormalLPDF<double> dlnorm;
        dlnorm.x.resize(this->x.size());
        dlnorm.expected_values.resize(this->expected_values.size());
        dlnorm.log_sd.resize(this->log_sd.size());
        for (size_t i = 0; i < x.size(); i++) {
            dlnorm.x[i] = this->x[i].initial_value_m;
        }
        for (size_t i = 0; i < expected_values.size(); i++) {
            dlnorm.expected_values[i] = this->expected_values[i].initial_value_m;
        }
        for (size_t i = 0; i < log_sd.size(); i++) {
            dlnorm.log_sd[i] = this->log_sd[i].initial_value_m;
        }
        return dlnorm.evaluate();
    }

    /**
     * @brief Extracts the derived quantities from `Information` to the Rcpp
     * object.
     */
    virtual void finalize() {
        if (this->finalized) {
            //log warning that finalize has been called more than once.
            FIMS_WARNING_LOG("LogNormalLPDF  " + fims::to_string(this->id_m) + " has been finalized already.");
        }

        this->finalized = true; //indicate this has been called already

        std::shared_ptr<fims_info::Information<double> > info =
                fims_info::Information<double>::GetInstance();

        fims_info::Information<double>::density_components_iterator it;

        //search for density component in Information
        it = info->density_components.find(this->id_m);
        //if not found, just return
        if (it == info->density_components.end()) {
            FIMS_WARNING_LOG("LogNormalLPDF " + fims::to_string(this->id_m) + " not found in Information.");
            return;
        } else {
            std::shared_ptr<fims_distributions::LogNormalLPDF<double> > dlnorm =
                    std::dynamic_pointer_cast<fims_distributions::LogNormalLPDF<double> >(it->second);
            this->lpdf_vec = Rcpp::NumericVector(dlnorm->lpdf_vec.size());
            for (R_xlen_t i = 0; i < this->lpdf_vec.size(); i++) {
                this->lpdf_vec[i] = dlnorm->lpdf_vec[i];
            }
        }
    }

    /**
     * @brief Converts the data to json representation for the output.
     * @return A string is returned specifying that the module relates to the
     * distribution interface with a log_normal distribution. It also returns the
     * ID and the natural log of the probability density function values
     * themselves. This string is formatted for a json file.
     */
    virtual std::string to_json() {
        std::stringstream ss;

        ss << "\"module\" : {\n";
        ss << " \"name\": \"LogNormalLPDF\",\n";
        ss << " \"type\": \"log_normal\",\n";
        ss << " \"id\": " << this->id_m << ",\n";

        ss << " \"density_component\": {\n";
        ss << "  \"name\": \"lpdf_vec\",\n";
        ss << "  \"values\":[";
        if (this->lpdf_vec.size() == 0) {
            ss << "]\n";
        } else {
            for (R_xlen_t i = 0; i < this->lpdf_vec.size() - 1; i++) {
                ss << this->lpdf_vec[i] << ", ";
            }
            ss << this->lpdf_vec[this->lpdf_vec.size() - 1] << "]\n";
        }
        ss << " }\n}";

        return ss.str();
    }



#ifdef TMB_MODEL

    template <typename Type>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims_info::Information < Type>> info =
                fims_info::Information<Type>::GetInstance();

        std::shared_ptr<fims_distributions::LogNormalLPDF < Type>> distribution =
                std::make_shared<fims_distributions::LogNormalLPDF < Type >> ();

        // set relative info
        distribution->id = this->id_m;
        distribution->observed_data_id_m =
                interface_observed_data_id_m;
        distribution->input_type = this->input_type_m;
        distribution->key.resize(this->key_m.size());
        for (size_t i = 0; i<this->key_m.size(); i++) {
            distribution->key[i] = this->key_m[i];
        }
        distribution->x.resize(this->x.size());
        for (size_t i = 0; i<this->x.size(); i++) {
            distribution->x[i] = this->x[i].initial_value_m;
        }
        // set relative info
        distribution->expected_values.resize(this->expected_values.size());
        for (size_t i = 0; i<this->expected_values.size(); i++) {
            distribution->expected_values[i] = this->expected_values[i].initial_value_m;
        }
        distribution->log_sd.resize(this->log_sd.size());
        for (size_t i = 0; i<this->log_sd.size(); i++) {
            distribution->log_sd[i] = this->log_sd[i].initial_value_m;
            if (this->log_sd[i].estimated_m) {
                info->RegisterParameterName("lognormal log_sd");
                info->RegisterParameter(distribution->log_sd[i]);
            }
            if (this->log_sd[i].is_random_effect_m) {
                error("standard deviations cannot be set to random effects");
            }
        }
        info->variable_map[this->log_sd.id_m] = &(distribution)->log_sd;

        info->density_components[distribution->id] = distribution;

        return true;
    }

    /**
     * @brief Adds the parameters to the TMB model.
     * @return A boolean of true.
     */
    virtual bool add_to_fims_tmb() {
#ifdef TMBAD_FRAMEWORK

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
 * @brief The Rcpp interface for Dmultinom to instantiate from R:
 * dmultinom_ <- methods::new(DmultinomDistribution).
 */
class DmultinomDistributionsInterface : public DistributionsInterfaceBase {
public:
    /**
     * @brief Observed data, which should be a vector of length K of integers.
     */
    ParameterVector x;
    /**
     * @brief The expected values, which should be a vector of length K where
     * each value specifies the probability of class k. Note that, unlike in R,
     * these probabilities must sum to 1.0.
     */
    ParameterVector expected_values;
    /**
     * @brief The dimensions of the number of rows and columns of the
     * multivariate dataset.
     */
    Rcpp::NumericVector dims;
    /**
     * @brief The vector. TODO: document this more.
     */
    Rcpp::NumericVector lpdf_vec; /**< The vector */

    /**
     * @brief The constructor.
     */
    DmultinomDistributionsInterface() : DistributionsInterfaceBase() {
    }

    /**
     * @brief The destructor.
     */
    virtual ~DmultinomDistributionsInterface() {
    }

    /**
     * @brief Gets the ID of the interface base object.
     * @return The ID.
     */
    virtual uint32_t get_id() {
        return this->id_m;
    }

    /**
     * @brief Set the unique ID for the observed data object.
     * @param observed_data_id Unique ID for the observed data object.
     */
    virtual bool set_observed_data(int observed_data_id) {
        this->interface_observed_data_id_m = observed_data_id;
        return true;
    }

    /**
     * @brief Sets pointers for data observations, random effects, or priors.
     *
     * @param input_type String that sets whether the distribution type is for priors, random effects, or data.
     * @param ids Vector of unique ids for each linked parameter(s), derived
     * value(s), or observed data vector.
     */
    virtual bool set_distribution_links(std::string input_type, Rcpp::IntegerVector ids) {
        this->input_type_m = input_type;
        this->key_m.resize(ids.size());
        for (int i = 0; i < ids.size(); i++) {
            this->key_m[i] = ids[i];
        }
        return true;
    }

    virtual std::string to_json() {
        std::stringstream ss;
        return "\"module\":{}";
    }

    /**
     * @brief Evaluate multinomial probability density function (pdf). The log of
     * the pdf is returned.
     * @return The natural log of the probability density function (pdf) is
     * returned.
     */
    virtual double evaluate() {
        fims_distributions::MultinomialLPMF<double> dmultinom;
        // Declare TMBVector in this scope
        dmultinom.x.resize(this->x.size());
        dmultinom.expected_values.resize(this->expected_values.size());
        for (size_t i = 0; i < x.size(); i++) {
            dmultinom.x[i] = this->x[i].initial_value_m;
        }
        for (size_t i = 0; i < expected_values.size(); i++) {
            dmultinom.expected_values[i] = this->expected_values[i].initial_value_m;
        }
        dmultinom.dims.resize(2);
        dmultinom.dims[0] = this->dims[0];
        dmultinom.dims[1] = this->dims[1];
        return dmultinom.evaluate();
    }

#ifdef TMB_MODEL

    template <typename Type>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims_info::Information < Type>> info =
                fims_info::Information<Type>::GetInstance();

        std::shared_ptr<fims_distributions::MultinomialLPMF < Type>> distribution =
                std::make_shared<fims_distributions::MultinomialLPMF < Type >> ();

        distribution->id = this->id_m;
        distribution->observed_data_id_m =
                interface_observed_data_id_m;
        distribution->input_type = this->input_type_m;
        distribution->key.resize(this->key_m.size());
        for (size_t i = 0; i<this->key_m.size(); i++) {
            distribution->key[i] = this->key_m[i];
        }
        distribution->x.resize(this->x.size());
        for (size_t i = 0; i<this->x.size(); i++) {
            distribution->x[i] = this->x[i].initial_value_m;
        }
        // set relative info
        distribution->expected_values.resize(this->expected_values.size());
        for (size_t i = 0; i<this->expected_values.size(); i++) {
            distribution->expected_values[i] = this->expected_values[i].initial_value_m;
        }
        if (this->dims.size() > 0) {
            distribution->dims.resize(2);
            distribution->dims[0] = this->dims[0];
            distribution->dims[1] = this->dims[1];
        }

        info->density_components[distribution->id] = distribution;

        return true;
    }

    virtual bool add_to_fims_tmb() {
#ifdef TMBAD_FRAMEWORK
        this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
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
