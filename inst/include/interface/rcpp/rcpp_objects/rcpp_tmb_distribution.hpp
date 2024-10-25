/*
 * File:   rcpp_distributions.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_TMB_DISTRIBUTION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_TMB_DISTRIBUTION_HPP

#include "../../../distributions/distributions.hpp"
#include "../../interface.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Distributions Rcpp Interface
 *
 */
class DistributionsInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t
      id_g;    /**< static id of the DistributionsInterfaceBase object */
  uint32_t id_m; /**< local id of the DistributionsInterfaceBase object */
  std::vector<uint32_t> key_m; /**< unique id for variable map that points to a fims::Vector */
  std::string input_type_m; /**< type of density input, options are: prior, re, or data */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, DistributionsInterfaceBase *> live_objects; /**<
map relating the ID of the DistributionsInterfaceBase to the
DistributionsInterfaceBase objects */
uint32_t interface_observed_data_id_m =
      -999; /**< id of observed data object*/

  DistributionsInterfaceBase() {
    this->id_m = DistributionsInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    DistributionsInterfaceBase */
    DistributionsInterfaceBase::live_objects[this->id_m] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~DistributionsInterfaceBase() {}

  /** @brief get_id method for child distribution interface objects to inherit
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief set_distribution_links sets pointers for data observations, random effects, or priors
   * 
   * @param input_type String that sets whether the distribution type is: priors, random_effects, or data.
   * @param ids Vector of unique ids for each linked parameter/s, derived value/s, or observed data vector
   */
  virtual bool set_distribution_links(std::string input_type, Rcpp::IntegerVector ids){
    return false;
  }


  /**
   * @brief Set the unique id for the Observed Data object
   *
   * @param observed_data_id Unique id for the Observed Age Comp Data
   * object
   */
  virtual bool set_observed_data(int observed_data_id){
    return false;
  }

  /** @brief evaluate method for child distribution interface objects to inherit
   */
  virtual double evaluate() = 0;
};

uint32_t DistributionsInterfaceBase::id_g =
    1; /**< static id of the DistributionsInterfaceBase object */
std::map<uint32_t,
         DistributionsInterfaceBase
             *> /**< local id of the DistributionsInterfaceBase object */
    DistributionsInterfaceBase::live_objects; /**<
      map relating the ID of the DistributionsInterfaceBase to the
      DistributionsInterfaceBase objects */

/**
 * @brief Rcpp interface for Dnorm as an S4 object. To instantiate
 * from R:
 * dnorm_ <- new(TMBDnormDistribution)
 *
 */
class DnormDistributionsInterface : public DistributionsInterfaceBase {
 public:
  ParameterVector x; /**< observed data */
  ParameterVector expected_values; /**< mean of x for the normal distribution */
  ParameterVector log_sd;   /**< sd of x for the normal distribution */
  Rcpp::NumericVector lpdf_vec; /**< The vector*/

  DnormDistributionsInterface() : DistributionsInterfaceBase() {}

  virtual uint32_t get_id() { return this->id_m; }

  virtual ~DnormDistributionsInterface() {}

  /**
   * @brief Set the unique id for the Observed Data object
   *
   * @param observed_data_id Unique id for the Observed Age Comp Data
   * object
   */
  virtual bool set_observed_data(int observed_data_id) {
    this->interface_observed_data_id_m = observed_data_id;
    return true;
  }

  /**
   * @brief set_distribution_links sets pointers for data observations, random effects, or priors
   * 
   * @param input_type String that sets whether the distribution type is: priors, random_effects, or data.
   * @param ids Vector of unique ids for each linked parameter/s, derived value/s, or observed data vector
   */
  virtual bool set_distribution_links(std::string input_type, Rcpp::IntegerVector ids){
    this->input_type_m = input_type;
    this->key_m.resize(ids.size());
    for(int i; i<ids.size(); i++){
      this->key_m[i] = ids[i];
    }

    return true;
  }

  /**
   * @brief Evaluate normal probability density function, default returns the
   * log of the pdf
   *
   * @tparam T
   * @return log pdf
   */
  virtual double evaluate() {
    fims_distributions::NormalLPDF<double> dnorm;
    dnorm.x.resize(this->x.size());
    dnorm.expected_values.resize(this->expected_values.size());
    dnorm.log_sd.resize(this->log_sd.size());
    for(size_t i=0; i<x.size(); i++){
      dnorm.x[i] = this->x[i].initial_value_m;
    }
    for(size_t i=0; i<expected_values.size(); i++){
      dnorm.expected_values[i] = this->expected_values[i].initial_value_m;
    }
    for(size_t i=0; i<log_sd.size(); i++){
      dnorm.log_sd[i] = this->log_sd[i].initial_value_m;
    }
    return dnorm.evaluate();
}
    /** 
     * @brief finalize function. Extracts derived quantities back to 
     * the Rcpp interface object from the Information object. 
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
            for(size_t i=0; i < this->lpdf_vec.size(); i++) {
                this->lpdf_vec[i] = dnorm->lpdf_vec[i];
            }

        }


    }

    /**
     * @brief Convert the data to json representation for the output.
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
            for(size_t i=0; i < this->lpdf_vec.size() - 1; i++) {
                ss << this->lpdf_vec[i] << ", ";
            }
            ss << this->lpdf_vec[this->lpdf_vec.size() - 1] << "]\n";
        }
        ss << " }\n]";

        return ss.str();
    }


#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_distributions::NormalLPDF<Type>> distribution =
        std::make_shared<fims_distributions::NormalLPDF<Type>>();

    // interface to data/parameter value

    distribution->observed_data_id_m =
        interface_observed_data_id_m;
    distribution->input_type = this->input_type_m;
    distribution->key.resize(this->key_m.size());
    for(size_t i=0; i<this->key_m.size(); i++){
      distribution->key[i] = this->key_m[i];
    }
    distribution->id = this->id_m;
    distribution->x.resize(this->x.size());
    for(size_t i=0; i<this->x.size(); i++){
      distribution->x[i] = this->x[i].initial_value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for(size_t i=0; i<this->expected_values.size(); i++) {
        distribution->expected_values[i] = this->expected_values[i].initial_value_m;
    }
    distribution->log_sd.resize(this->log_sd.size());
    for(size_t i=0; i<this->log_sd.size(); i++){
      distribution->log_sd[i] = this->log_sd[i].initial_value_m;
      if(this->log_sd[i].estimated_m){
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
   * @brief adds the dnorm distribution and its parameters to the TMB model
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

/**
 * @brief Rcpp interface for Dlnorm as an S4 object. To instantiate
 * from R:
 * dlnorm_ <- new(TMBDlnormDistribution)
 *
 */
class DlnormDistributionsInterface : public DistributionsInterfaceBase {
 public:
  ParameterVector x;       /**< observation */
  ParameterVector expected_values; /**< mean of the distribution of log(x) */
  ParameterVector log_sd;   /**< log standard deviation of the distribution of log(x) */
  Rcpp::NumericVector lpdf_vec; /**< The vector */

  DlnormDistributionsInterface() : DistributionsInterfaceBase() {}

  virtual ~DlnormDistributionsInterface() {}

  /**
   * @brief get the id of the Dlnorm distributions interface class object
   */
  virtual uint32_t get_id() { return this->id_m; }

  /**
   * @brief Set the unique id for the Observed Data object
   *
   * @param observed_data_id Unique id for the Observed Age Comp Data
   * object
   */
  virtual bool set_observed_data(int observed_data_id) {
    this->interface_observed_data_id_m = observed_data_id;

    return true;
  }

  /**
   * @brief set_distribution_links sets pointers for data observations, random effects, or priors
   * 
   * @param input_type String that sets whether the distribution type is: priors, random_effects, or data.
   * @param ids Vector of unique ids for each linked parameter/s, derived value/s, or observed data vector
   */
  virtual bool set_distribution_links(std::string input_type, Rcpp::IntegerVector ids){
    this->input_type_m = input_type;
    this->key_m.resize(ids.size());
    for(int i; i<ids.size(); i++){
      this->key_m[i] = ids[i];
    }

    return true;
  }   

  /**
   * @brief Evaluate lognormal probability density function, default returns the
   * log of the pdf
   *
   * @tparam T
   * @return log pdf
   */
  virtual double evaluate() {
    fims_distributions::LogNormalLPDF<double> dlnorm;
    dlnorm.x.resize(this->x.size());
    dlnorm.expected_values.resize(this->expected_values.size());
    dlnorm.log_sd.resize(this->log_sd.size());
    for(size_t i=0; i<x.size(); i++){
      dlnorm.x[i] = this->x[i].initial_value_m;
    }
    for(size_t i=0; i<expected_values.size(); i++){
      dlnorm.expected_values[i] = this->expected_values[i].initial_value_m;
    }
    for(size_t i=0; i<log_sd.size(); i++){
      dlnorm.log_sd[i] = this->log_sd[i].initial_value_m;
    }
    return dlnorm.evaluate();
  }
    /** 
     * @brief finalize function. Extracts derived quantities back to 
     * the Rcpp interface object from the Information object. 
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
            for(size_t i=0; i < this->lpdf_vec.size(); i++) {
                this->lpdf_vec[i] = dlnorm->lpdf_vec[i];
            }

        }


    }

    /**
     * @brief Convert the data to json representation for the output.
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
            for(size_t i=0; i < this->lpdf_vec.size() - 1; i++) {
                ss << this->lpdf_vec[i] << ", ";
            }
            ss << this->lpdf_vec[this->lpdf_vec.size() - 1] << "]\n";
        }
        ss << " }\n]";

        return ss.str();
    }



#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_distributions::LogNormalLPDF<Type>> distribution =
        std::make_shared<fims_distributions::LogNormalLPDF<Type>>();

    // set relative info
    distribution->id = this->id_m;
    distribution->observed_data_id_m =
        interface_observed_data_id_m;
    distribution->input_type = this->input_type_m;
    distribution->key.resize(this->key_m.size());
    for(size_t i=0; i<this->key_m.size(); i++){
      distribution->key[i] = this->key_m[i];
    }
    distribution->x.resize(this->x.size());
    for(size_t i=0; i<this->x.size(); i++){
      distribution->x[i] = this->x[i].initial_value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for(size_t i=0; i<this->expected_values.size(); i++){
      distribution->expected_values[i] = this->expected_values[i].initial_value_m;
    }
    distribution->log_sd.resize(this->log_sd.size());
    for(size_t i=0; i<this->log_sd.size(); i++){
      distribution->log_sd[i] = this->log_sd[i].initial_value_m;
      if(this->log_sd[i].estimated_m){
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
   * @brief adds the dlnorm distribution and its parameters to the TMB model
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

/**
 * @brief Rcpp interface for Dmultinom as an S4 object. To instantiate
 * from R:
 * dmultinom_ <- new(TMBDmultinomDistribution)
 *
 */
// template <typename Type>

class DmultinomDistributionsInterface : public DistributionsInterfaceBase {
 public:
  ParameterVector x; /**< Vector of length K of integers */
  ParameterVector expected_values; /**< Vector of length K, specifying the probability
 for the K classes (note, unlike in R these must sum to 1). */
  Rcpp::NumericVector dims; /**< Dimensions of the number of rows and columns of the multivariate dataset */
  Rcpp::NumericVector lpdf_vec; /**< The vector */

  DmultinomDistributionsInterface() : DistributionsInterfaceBase() {}

  virtual ~DmultinomDistributionsInterface() {}

  virtual uint32_t get_id() { return this->id_m; }

  /**
   * @brief Set the unique id for the Observed Data object
   *
   * @param observed_data_id Unique id for the Observed Age Comp Data
   * object
   */
  virtual bool set_observed_data(int observed_data_id) {
    this->interface_observed_data_id_m = observed_data_id;

    return true;
  }

  /**
   * @brief set_distribution_links sets pointers for data observations, random effects, or priors
   * 
   * @param input_type String that sets whether the distribution type is: priors, random_effects, or data.
   * @param ids Vector of unique ids for each linked parameter/s, derived value/s, or observed data vector
   */
  virtual bool set_distribution_links(std::string input_type, Rcpp::IntegerVector ids){
    this->input_type_m = input_type;
    this->key_m.resize(ids.size());
    for(int i; i<ids.size(); i++){
      this->key_m[i] = ids[i];
    }

    return true;
  }

  /**
   * @brief Evaluate multinom probability density function, default returns the
   * log of the pdf
   *
   * @tparam T
   * @return log pdf
   */
  virtual double evaluate() {
    fims_distributions::MultinomialLPMF<double> dmultinom;
    // Declare TMBVector in this scope
    dmultinom.x.resize(this->x.size());
    dmultinom.expected_values.resize(this->expected_values.size());
    for(size_t i=0; i<x.size(); i++){
      dmultinom.x[i] = this->x[i].initial_value_m;
    }
    for(size_t i=0; i<expected_values.size(); i++){
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
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_distributions::MultinomialLPMF<Type>> distribution =
        std::make_shared<fims_distributions::MultinomialLPMF<Type>>();

    distribution->id = this->id_m;
    distribution->observed_data_id_m =
        interface_observed_data_id_m;
    distribution->input_type = this->input_type_m;
    distribution->key.resize(this->key_m.size());
    for(size_t i=0; i<this->key_m.size(); i++){
      distribution->key[i] = this->key_m[i];
    }
    distribution->x.resize(this->x.size());
    for(size_t i=0; i<this->x.size(); i++){
      distribution->x[i] = this->x[i].initial_value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for(size_t i=0; i<this->expected_values.size(); i++){
      distribution->expected_values[i] = this->expected_values[i].initial_value_m;
    }
    if(this->dims.size()>0){
      distribution->dims.resize(2);
      distribution->dims[0] = this->dims[0];
      distribution->dims[1] = this->dims[1];
    }

    info->density_components[distribution->id] = distribution;

    return true;
  }

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
