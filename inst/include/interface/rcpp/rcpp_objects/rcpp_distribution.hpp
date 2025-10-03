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
  std::shared_ptr<std::vector<uint32_t>> key_m;
  /**
   * @brief The type of density input. The options are prior, re, or data.
   */
  SharedString input_type_m;
  /**
   * @brief Shared string indicating whether to use the mean.
   */
  SharedString use_mean_m = fims::to_string("no");
  /**
   * @brief The map associating the ID of the DistributionsInterfaceBase to the
     DistributionsInterfaceBase objects. This is a live object, which is an
     object that has been created and lives in memory.
   */
  static std::map<uint32_t, std::shared_ptr<DistributionsInterfaceBase>>
      live_objects;
  /**
   * @brief The ID of the observed data object, which is set to -999.
   */
  SharedInt interface_observed_data_id_m = -999;

  /**
   * @brief The log probability density function value.
   */
  double lpdf_value = 0;
  /**
   * @brief The constructor.
   */
  DistributionsInterfaceBase() {
    this->key_m = std::make_shared<std::vector<uint32_t>>();
    this->id_m = DistributionsInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    DistributionsInterfaceBase */
    // DistributionsInterfaceBase::live_objects[this->id_m] = this;
  }

  /**
   * @brief Construct a new Distributions Interface Base object
   *
   * @param other
   */
  DistributionsInterfaceBase(const DistributionsInterfaceBase &other)
      : id_m(other.id_m),
        key_m(other.key_m),
        input_type_m(other.input_type_m),
        use_mean_m(other.use_mean_m),
        interface_observed_data_id_m(other.interface_observed_data_id_m) {}

  /**
   * @brief The destructor.
   */
  virtual ~DistributionsInterfaceBase() {}

  /**
   * @brief Get the ID for the child distribution interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief Sets pointers for data observations, random effects, or priors.
   *
   * @param input_type String that sets whether the distribution type is for
   * priors, random effects, or data.
   * @param ids Vector of unique ids for each linked parameter(s), derived
   * value(s), or observed data vector.
   */
  virtual bool set_distribution_links(std::string input_type,
                                      Rcpp::IntegerVector ids) {
    return false;
  }

  /**
   * @brief Set mean expected value for distribution.
   *
   * @param input_value Value to use for the distribution mean.
   */
  virtual bool set_distribution_mean(double input_value) {
    return false;
  }

  /**
   * @brief Set the unique ID for the observed data object.
   *
   * @param observed_data_id Unique ID for the Observed Age Comp Data
   * object
   */
  virtual bool set_observed_data(int observed_data_id) { return false; }

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
std::map<uint32_t, std::shared_ptr<DistributionsInterfaceBase>>
    DistributionsInterfaceBase::live_objects;

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
   * @brief The expected mean, which would be the mean of x for this
   * distribution.
   */
  ParameterVector expected_mean;
  /**
   * @brief The uncertainty, which would be the standard deviation of x for the
   * normal distribution.
   */
  ParameterVector log_sd;
  /**
   * @brief Vector that records the individual log probability function for each
   * observation.
   */
  RealVector lpdf_vec; /**< The vector*/

  /**
   * @brief The constructor.
   */
  DnormDistributionsInterface() : DistributionsInterfaceBase() {
    DistributionsInterfaceBase::live_objects[this->id_m] =
        std::make_shared<DnormDistributionsInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DistributionsInterfaceBase::live_objects[this->id_m]);
  }

  /**
   * @brief Construct a new Dnorm Distributions Interface object
   *
   * @param other
   */
  DnormDistributionsInterface(const DnormDistributionsInterface &other)
      : DistributionsInterfaceBase(other),
        x(other.x),
        expected_values(other.expected_values),
        log_sd(other.log_sd),
        expected_mean(other.expected_mean),
        lpdf_vec(other.lpdf_vec) {}

  /**
   * @brief The destructor.
   */
  virtual ~DnormDistributionsInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id_m; }

  /**
   * @brief Set the unique ID for the observed data object.
   * @param observed_data_id Unique ID for the observed data object.
   */
  virtual bool set_observed_data(int observed_data_id) {
    this->interface_observed_data_id_m.set(observed_data_id);
    return true;
  }

  /**
   * @brief Set expected mean for distribution.
   * @param input_value Distribution mean.
   */
  virtual bool set_distribution_mean(double input_value) {
    this->expected_mean[0].initial_value_m = input_value;
    this->expected_mean[0].estimation_type_m.set("fixed_effects");
    this->use_mean_m.set(fims::to_string("yes"));
    return true;
  }

  /**
   * @brief Sets pointers for data observations, random effects, or priors.
   *
   * @param input_type String that sets whether the distribution type is for
   * priors, random effects, or data.
   * @param ids Vector of unique ids for each linked parameter(s), derived
   * value(s), or observed data vector.
   */
  virtual bool set_distribution_links(std::string input_type,
                                      Rcpp::IntegerVector ids) {
    this->input_type_m.set(input_type);
    this->key_m->resize(ids.size());
    for (int i = 0; i < ids.size(); i++) {
      this->key_m->at(i) = ids[i];
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
    dnorm.expected_mean.resize(this->expected_mean.size());
    for (size_t i = 0; i < x.size(); i++) {
      dnorm.x[i] = this->x[i].initial_value_m;
    }
    for (size_t i = 0; i < expected_values.size(); i++) {
      dnorm.expected_values[i] = this->expected_values[i].initial_value_m;
    }
    for (size_t i = 0; i < log_sd.size(); i++) {
      dnorm.log_sd[i] = this->log_sd[i].initial_value_m;
    }
    for (size_t i = 0; i < expected_mean.size(); i++) {
      dnorm.expected_mean[i] = this->expected_mean[i].initial_value_m;
    }
    dnorm.use_mean = this->use_mean_m;
    return dnorm.evaluate();
  }

  /**
   * @brief Extracts the derived quantities from `Information` to the Rcpp
   * object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("DnormDistribution  " + fims::to_string(this->id_m) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::density_components_iterator it;

    // search for density component in Information
    it = info->density_components.find(this->id_m);
    // if not found, just return
    if (it == info->density_components.end()) {
      FIMS_WARNING_LOG("DnormDistribution " + fims::to_string(this->id_m) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_distributions::NormalLPDF<double>> dnorm =
          std::dynamic_pointer_cast<fims_distributions::NormalLPDF<double>>(
              it->second);

      this->lpdf_value = dnorm->lpdf;

      for (size_t i = 0; i < this->log_sd.size(); i++) {
        if (this->log_sd[i].estimation_type_m.get() == "constant") {
          this->log_sd[i].final_value_m = this->log_sd[i].initial_value_m;
        } else {
          this->log_sd[i].final_value_m = dnorm->log_sd[i];
        }
      }

      for (size_t i = 0; i < this->expected_mean.size(); i++) {
        if (this->expected_mean[i].estimation_type_m.get() == "constant") {
          this->expected_mean[i].final_value_m = this->expected_mean[i].initial_value_m;
        } else {
          this->expected_mean[i].final_value_m = dnorm->expected_mean[i];
        }
      }

      this->lpdf_vec = RealVector(dnorm->report_lpdf_vec.size());
      if (this->expected_values.size() == 1) {
        this->expected_values.resize(dnorm->expected_values.size());
      }
      if (this->x.size() == 1) {
        size_t nx = dnorm->get_n_x();
        this->x.resize(nx);
      }

      for (R_xlen_t i = 0; i < this->lpdf_vec.size(); i++) {
        this->lpdf_vec[i] = dnorm->report_lpdf_vec[i];
        this->expected_values[i].final_value_m = dnorm->get_expected(i);
        this->x[i].final_value_m = dnorm->get_observed(i);
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

    ss << "{\n";
    ss << " \"module_name\": \"density\",\n";
    ss << " \"module_id\": " << this->id_m << ",\n";
    ss << " \"module_type\": \"normal\",\n";
    ss << " \"observed_data_id\" : " << this->interface_observed_data_id_m
       << ",\n";
    ss << " \"input_type\" : \"" << this->input_type_m << "\",\n";
    ss << " \"density_component\": {\n";
    ss << "  \"lpdf_value\": " << this->lpdf_value << ",\n";
    ss << "  \"value\":[";
    if (this->lpdf_vec.size() == 0) {
      ss << "],\n";
    } else {
      for (R_xlen_t i = 0; i < this->lpdf_vec.size() - 1; i++) {
        ss << this->value_to_string(this->lpdf_vec[i]);
        ss << ", ";
      }
      ss << this->value_to_string(this->lpdf_vec[this->lpdf_vec.size() - 1]);

      ss << "],\n";
    }
    ss << "  \"expected_values\":[";
    if (this->expected_values.size() == 0) {
      ss << "],\n";
    } else {
      for (R_xlen_t i = 0; i < this->expected_values.size() - 1; i++) {
        ss << this->value_to_string(this->expected_values[i].final_value_m)
           << ", ";
      }
      ss << this->value_to_string(
          this->expected_values[this->expected_values.size() - 1]
              .final_value_m);
      ss << "],\n";
    }
    ss << "  \"observed_values\":[";
    if (this->x.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->x.size() - 1; i++) {
        ss << this->x[i].final_value_m << ", ";
      }
      ss << this->x[this->x.size() - 1].final_value_m << "]\n";
    }
    ss << " }}\n";

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

    distribution->observed_data_id_m = interface_observed_data_id_m;
    std::stringstream ss;
    distribution->input_type = this->input_type_m;
    distribution->key.resize(this->key_m->size());
    for (size_t i = 0; i < this->key_m->size(); i++) {
      distribution->key[i] = this->key_m->at(i);
    }
    distribution->id = this->id_m;
    distribution->x.resize(this->x.size());
    for (size_t i = 0; i < this->x.size(); i++) {
      distribution->x[i] = this->x[i].initial_value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for (size_t i = 0; i < this->expected_values.size(); i++) {
      distribution->expected_values[i] =
          this->expected_values[i].initial_value_m;
    }
    distribution->log_sd.resize(this->log_sd.size());
    for (size_t i = 0; i < this->log_sd.size(); i++) {
      distribution->log_sd[i] = this->log_sd[i].initial_value_m;
      if (this->log_sd[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "dnorm." << this->id_m << ".log_sd." << this->log_sd[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(distribution->log_sd[i]);
      }
      if (this->log_sd[i].estimation_type_m.get() == "random_effects") {
        FIMS_ERROR_LOG("standard deviations cannot be set to random effects");
      }
    }
    info->variable_map[this->log_sd.id_m] = &(distribution)->log_sd;

    distribution->use_mean = this->use_mean_m.get();
    distribution->expected_mean.resize(this->expected_mean.size());
    for (size_t i = 0; i < this->expected_mean.size(); i++){
      distribution->expected_mean[i] = this->expected_mean[i].initial_value_m;
      if (this->expected_mean[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "dnorm." << this->id_m << ".expected_mean." << this->expected_mean[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(distribution->expected_mean[i]);
      }
      if (this->expected_mean[i].estimation_type_m.get() == "random_effects") {
        FIMS_ERROR_LOG("expected_mean cannot be set to random effects");
      }
    }
    info->variable_map[this->expected_mean.id_m] = &(distribution)->expected_mean;
    

    info->density_components[distribution->id] = distribution;

    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
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
   * @brief Vector that records the individual log probability function for each
   * observation.
   */
  RealVector lpdf_vec; /**< The vector */

  /**
   * @brief The constructor.
   */
  DlnormDistributionsInterface() : DistributionsInterfaceBase() {
    DistributionsInterfaceBase::live_objects[this->id_m] =
        std::make_shared<DlnormDistributionsInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DistributionsInterfaceBase::live_objects[this->id_m]);
  }

  /**
   * @brief Construct a new Dlnorm Distributions Interface object
   *
   * @param other
   */
  DlnormDistributionsInterface(const DlnormDistributionsInterface &other)
      : DistributionsInterfaceBase(other),
        x(other.x),
        expected_values(other.expected_values),
        log_sd(other.log_sd),
        lpdf_vec(other.lpdf_vec) {}

  /**
   * @brief The destructor.
   */
  virtual ~DlnormDistributionsInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id_m; }

  /**
   * @brief Set the unique ID for the observed data object.
   * @param observed_data_id Unique ID for the observed data object.
   */
  virtual bool set_observed_data(int observed_data_id) {
    this->interface_observed_data_id_m.set(observed_data_id);
    return true;
  }

  /**
   * @brief Sets pointers for data observations, random effects, or priors.
   *
   * @param input_type String that sets whether the distribution type is for
   * priors, random effects, or data.
   * @param ids Vector of unique ids for each linked parameter(s), derived
   * value(s), or observed data vector.
   */
  virtual bool set_distribution_links(std::string input_type,
                                      Rcpp::IntegerVector ids) {
    this->input_type_m.set(input_type);
    this->key_m->resize(ids.size());
    for (int i = 0; i < ids.size(); i++) {
      this->key_m->at(i) = ids[i];
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
    // dlnorm.input_type = "prior";
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
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("LogNormalLPDF  " + fims::to_string(this->id_m) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::density_components_iterator it;

    // search for density component in Information
    it = info->density_components.find(this->id_m);
    // if not found, just return
    if (it == info->density_components.end()) {
      FIMS_WARNING_LOG("LogNormalLPDF " + fims::to_string(this->id_m) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_distributions::LogNormalLPDF<double>> dlnorm =
          std::dynamic_pointer_cast<fims_distributions::LogNormalLPDF<double>>(
              it->second);

      this->lpdf_value = dlnorm->lpdf;

      for (size_t i = 0; i < this->log_sd.size(); i++) {
        if (this->log_sd[i].estimation_type_m.get() == "constant") {
          this->log_sd[i].final_value_m = this->log_sd[i].initial_value_m;
        } else {
          this->log_sd[i].final_value_m = dlnorm->log_sd[i];
        }
      }

      this->lpdf_vec = RealVector(dlnorm->report_lpdf_vec.size());
      if (this->expected_values.size() == 1) {
        this->expected_values.resize(
            this->lpdf_vec.size());  // dlnorm->expected_values.size());
      }
      if (this->x.size() == 1) {
        size_t nx = dlnorm->get_n_x();
        this->x.resize(nx);
      }
      for (R_xlen_t i = 0; i < this->lpdf_vec.size(); i++) {
        this->lpdf_vec[i] = dlnorm->report_lpdf_vec[i];
        this->expected_values[i].final_value_m = dlnorm->get_expected(i);
        this->x[i].final_value_m = dlnorm->get_observed(i);
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

    ss << "{\n";
    ss << " \"module_name\": \"density\",\n";
    ss << " \"module_id\": " << this->id_m << ",\n";
    ss << " \"module_type\": \"log_normal\",\n";
    ss << " \"observed_data_id\" : " << this->interface_observed_data_id_m
       << ",\n";
    ss << " \"input_type\" : \"" << this->input_type_m << "\",\n";
    ss << " \"density_component\": {\n";
    ss << "  \"lpdf_value\": " << this->lpdf_value << ",\n";
    ss << "  \"value\":[";
    if (this->lpdf_vec.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->lpdf_vec.size() - 1; i++) {
        ss << this->value_to_string(this->lpdf_vec[i]) << ", ";
      }
      ss << this->value_to_string(this->lpdf_vec[this->lpdf_vec.size() - 1]);

      ss << "],\n";
    }
    ss << "  \"expected_values\":[";
    if (this->expected_values.size() == 0) {
      ss << "],\n";
    } else {
      for (R_xlen_t i = 0; i < this->expected_values.size() - 1; i++) {
        ss << this->value_to_string(this->expected_values[i].final_value_m)
           << ", ";
      }
      ss << this->value_to_string(
          this->expected_values[this->expected_values.size() - 1]
              .final_value_m);

      ss << "],\n";
    }
    ss << "  \"observed_values\":[";
    if (this->x.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->x.size() - 1; i++) {
        ss << this->x[i].final_value_m << ", ";
      }
      ss << this->x[this->x.size() - 1].final_value_m << "]\n";
    }
    ss << " }}\n";
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
    std::stringstream ss;
    distribution->observed_data_id_m = interface_observed_data_id_m;
    distribution->input_type = this->input_type_m;
    distribution->key.resize(this->key_m->size());
    for (size_t i = 0; i < this->key_m->size(); i++) {
      distribution->key[i] = this->key_m->at(i);
    }
    distribution->x.resize(this->x.size());
    for (size_t i = 0; i < this->x.size(); i++) {
      distribution->x[i] = this->x[i].initial_value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for (size_t i = 0; i < this->expected_values.size(); i++) {
      distribution->expected_values[i] =
          this->expected_values[i].initial_value_m;
    }
    distribution->log_sd.resize(this->log_sd.size());
    for (size_t i = 0; i < this->log_sd.size(); i++) {
      distribution->log_sd[i] = this->log_sd[i].initial_value_m;
      if (this->log_sd[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "dlnorm." << this->id_m << ".log_sd." << this->log_sd[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(distribution->log_sd[i]);
      }
      if (this->log_sd[i].estimation_type_m.get() == "random_effects") {
        FIMS_ERROR_LOG("standard deviations cannot be set to random effects");
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
  RealVector dims;
  /**
   * @brief Vector that records the individual log probability function for each
   * observation.
   */
  RealVector lpdf_vec; /**< The vector */

  /**
   * @brief TODO: document this.
   *
   */
  SharedString notes;

  /**
   * @brief The constructor.
   */
  DmultinomDistributionsInterface() : DistributionsInterfaceBase() {
    DistributionsInterfaceBase::live_objects[this->id_m] =
        std::make_shared<DmultinomDistributionsInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DistributionsInterfaceBase::live_objects[this->id_m]);
  }

  /**
   * @brief Construct a new Dmultinom Distributions Interface object
   *
   * @param other
   */
  DmultinomDistributionsInterface(const DmultinomDistributionsInterface &other)
      : DistributionsInterfaceBase(other),
        x(other.x),
        expected_values(other.expected_values),
        dims(other.dims),
        lpdf_vec(other.lpdf_vec),
        notes(other.notes) {}

  /**
   * @brief The destructor.
   */
  virtual ~DmultinomDistributionsInterface() {}
  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id_m; }

  /**
   * @brief Set the unique ID for the observed data object.
   * @param observed_data_id Unique ID for the observed data object.
   */
  virtual bool set_observed_data(int observed_data_id) {
    this->interface_observed_data_id_m.set(observed_data_id);
    return true;
  }

  /**
   * @brief Sets pointers for data observations, random effects, or priors.
   *
   * @param input_type String that sets whether the distribution type is for
   * priors, random effects, or data.
   * @param ids Vector of unique ids for each linked parameter(s), derived
   * value(s), or observed data vector.
   */
  virtual bool set_distribution_links(std::string input_type,
                                      Rcpp::IntegerVector ids) {
    this->input_type_m.set(input_type);
    this->key_m->resize(ids.size());
    for (int i = 0; i < ids.size(); i++) {
      this->key_m->at(i) = ids[i];
    }
    return true;
  }

  /**
   * @brief Set the note object
   *
   * @param note
   */
  void set_note(std::string note) { this->notes.set(note); }

  /**
   * @brief
   *
   * @return double
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

  void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("DmultinomDistributions  " +
                       fims::to_string(this->id_m) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::density_components_iterator it;

    // search for density component in Information
    it = info->density_components.find(this->id_m);
    // if not found, just return
    if (it == info->density_components.end()) {
      FIMS_WARNING_LOG("DmultinomDistributions " + fims::to_string(this->id_m) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_distributions::MultinomialLPMF<double>> dmultinom =
          std::dynamic_pointer_cast<
              fims_distributions::MultinomialLPMF<double>>(it->second);

      this->lpdf_value = dmultinom->lpdf;

      size_t nx = dmultinom->report_lpdf_vec.size();
      this->lpdf_vec = Rcpp::NumericVector(nx);
      if (this->expected_values.size() != nx) {
        this->expected_values.resize(nx);
      }
      if (this->x.size() != nx) {
        this->x.resize(nx);
      }
      for (R_xlen_t i = 0; i < this->lpdf_vec.size(); i++) {
        this->lpdf_vec[i] = dmultinom->report_lpdf_vec[i];
        this->expected_values[i].final_value_m = dmultinom->get_expected(i);
        if (dmultinom->input_type != "data") {
          this->x[i].final_value_m = dmultinom->get_observed(i);
        }
      }
      if (dmultinom->input_type == "data") {
        dims.resize(2);
        dims[0] = dmultinom->observed_values->get_imax();
        dims[1] = dmultinom->observed_values->get_jmax();
        for (size_t i = 0; i < dims[0]; i++) {
          for (size_t j = 0; j < dims[1]; j++) {
            size_t idx = (i * dims[1]) + j;
            this->x[idx].final_value_m = dmultinom->get_observed(i, j);
          }
        }
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

    ss << "{\n";
    ss << " \"module_name\": \"density\",\n";
    ss << " \"module_id\": " << this->id_m << ",\n";
    ss << " \"module_type\": \"multinomial\",\n";
    ss << "\"observed_data_id\" : " << this->interface_observed_data_id_m
       << ",\n";
    ss << " \"input_type\" : \"" << this->input_type_m << "\",\n";
    ss << " \"density_component\": {\n";
    ss << "  \"lpdf_value\": " << this->lpdf_value << ",\n";
    ss << "  \"value\":[";
    if (this->lpdf_vec.size() == 0) {
      ss << "],\n";
    } else {
      for (R_xlen_t i = 0; i < this->lpdf_vec.size() - 1; i++) {
        ss << this->value_to_string(this->lpdf_vec[i]);
        ss << ", ";
      }
      ss << this->value_to_string(this->lpdf_vec[this->lpdf_vec.size() - 1]);

      ss << "],\n";
    }
    ss << "  \"expected_values\":[";
    if (this->expected_values.size() == 0) {
      ss << "],\n";
    } else {
      for (R_xlen_t i = 0; i < this->expected_values.size() - 1; i++) {
        ss << this->value_to_string(this->expected_values[i].final_value_m)
           << ", ";
      }
      ss << this->value_to_string(
          this->expected_values[this->expected_values.size() - 1]
              .final_value_m);

      ss << "],\n";
    }
    ss << "  \"observed_values\":[";
    if (this->x.size() == 0) {
      ss << "]\n";
    } else {
      for (R_xlen_t i = 0; i < this->x.size() - 1; i++) {
        ss << this->x[i].final_value_m << ", ";
      }
      ss << this->x[this->x.size() - 1].final_value_m << "]\n";
    }
    ss << " }}\n";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    FIMS_INFO_LOG("Adding multinomial to FIMS.");
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_distributions::MultinomialLPMF<Type>> distribution =
        std::make_shared<fims_distributions::MultinomialLPMF<Type>>();

    distribution->id = this->id_m;
    distribution->observed_data_id_m = interface_observed_data_id_m;
    distribution->input_type = this->input_type_m;
    distribution->key.resize(this->key_m->size());
    for (size_t i = 0; i < this->key_m->size(); i++) {
      distribution->key[i] = this->key_m->at(i);
    }
    distribution->x.resize(this->x.size());
    for (size_t i = 0; i < this->x.size(); i++) {
      distribution->x[i] = this->x[i].initial_value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for (size_t i = 0; i < this->expected_values.size(); i++) {
      distribution->expected_values[i] =
          this->expected_values[i].initial_value_m;
    }

    info->density_components[distribution->id] = distribution;
    FIMS_INFO_LOG("Done adding multinomial to FIMS.");
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
