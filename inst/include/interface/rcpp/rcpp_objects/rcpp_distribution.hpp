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

#include "rcpp_interface_base.hpp"
#include "../../../distributions/distributions.hpp"

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
   * @brief The total log probability density function value.
   */
  double lpdf_m = 0;
 
  /**
   * @brief Vector that records the individual log probability function 
   * values for each observation.
   */
  RealVector lpdf_vec_m;

  /**
   * @brief The type of likelihood input. The options are DATA, RANDOM_EFFECT, 
   * PRIOR, or PENALTY.
   */
  fims_distributions::Likelihood_Kind likelihood_type_m; //Also an enum class? Is this ok on R end??

  /**
   * @brief The distribution function for the likelihood component. 
   * The options are NORMAL, LOGNORMAL, or MULTINOMIAL.
   */
  fims_distributions::Density_Function distribution_type_m; //Need to make this an enum class maybe??

  /**
   * @brief Fixed input values to calculate a likelihood of occurance for.
   */
  VariableVector observed_values_m;
  
  /**
   * @brief Key to point the observed value at a data source, parameter, 
   * or derived model quantity.
   */
  std::shared_ptr<std::vector<size_t>> observed_key_m;
  
  /**
   * @brief Vector of values to specify a subset of the observed values or 
   * pointed vector to use in the likelihood calculation.
   */
  std::shared_ptr<std::vector<size_t>> observed_subvector_m;

  /**
   * @brief The expected values of the distribution used to calculate 
   * likelihoods.
   */
  VariableVector expected_values_m;
  
  /**
   * @brief Key to point the expected value at a data source, parameter, 
   * or derived model quantity.
   */
  std::shared_ptr<std::vector<size_t>> expected_key_m;
  
  /**
   * @brief Vector of values to specify a subset of the expected values or 
   * pointed vector to use in the likelihood calculation.
   */
  std::shared_ptr<std::vector<size_t>> expected_subvector_m;

  /**
   * @brief The uncertainty values of the distribution used to calculate 
   * likelihoods.
   */
  VariableVector uncertainty_values_m;
  
  /**
   * @brief Key to point the uncertainty value at a data source, parameter, 
   * or derived model quantity.
   */
  std::shared_ptr<std::vector<size_t>> uncertainty_key_m;
  
  /**
   * @brief Vector of values to specify a subset of the uncertainty values or 
   * pointed vector to use in the likelihood calculation.
   */
  std::shared_ptr<std::vector<size_t>> uncertainty_subvector_m;
  
  /**
   * @brief Vector of values to weight the likelihood calculation results.
   */
  RealVector lambda_values_m;

  /**
   * @brief Dimensions of the value vectors used to subset the likelihood 
   * calculations needed for multivariate distributions.
   */
  std::shared_ptr<std::vector<size_t>> dims_m;
  
  /**
   * @brief The map associating the ID of the DistributionsInterfaceBase to the
     DistributionsInterfaceBase objects. This is a live object, which is an
     object that has been created and lives in memory.
   */
  static std::map<uint32_t, std::shared_ptr<DistributionsInterfaceBase>>
      live_objects;


  /**
   * @brief The constructor.
   */
  DistributionsInterfaceBase() {
    this->observed_key_m = std::make_shared<std::vector<size_t>>();
    this->expected_key_m = std::make_shared<std::vector<size_t>>();
    this->uncertainty_key_m = std::make_shared<std::vector<size_t>>();
    this->observed_subvector_m = std::make_shared<std::vector<size_t>>();
    this->expected_subvector_m = std::make_shared<std::vector<size_t>>();
    this->uncertainty_subvector_m = std::make_shared<std::vector<size_t>>();
    this->dims_m = std::make_shared<std::vector<size_t>>();
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
        likelihood_type_m(other.likelihood_type_m),
        distribution_type_m(other.distribution_type_m),
        observed_values_m(other.observed_values_m),
        observed_key_m(other.observed_key_m),
        observed_subvector_m(other.observed_subvector_m),
        expected_values_m(other.expected_values_m),
        expected_key_m(other.expected_key_m),
        expected_subvector_m(other.expected_subvector_m),
        uncertainty_values_m(other.uncertainty_values_m),
        uncertainty_key_m(other.uncertainty_key_m),
        uncertainty_subvector_m(other.uncertainty_subvector_m),
        lambda_values_m(other.lambda_values_m),
        dims_m(other.dims_m),
        lpdf_vec_m(other.lpdf_vec_m),
        lpdf_m(other.lpdf_m) {}

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

/**
 * @brief The Rcpp interface for adding likelihood components R:
 * dist_ <- methods::new(Distribution).
 */
class DistributionsInterface : public DistributionsInterfaceBase {
 public:
  
  /**
   * @brief The constructor.
   */
  DistributionsInterface(const std::string &likelihood_type, 
    const std::string &distribution_type) : DistributionsInterfaceBase() {

      
    DistributionsInterfaceBase::live_objects[this->id_m] =
        std::make_shared<DistributionsInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        DistributionsInterfaceBase::live_objects[this->id_m]);
  }

  /**
   * @brief Construct a new Distributions Interface object
   *
   * @param other
   */
  DistributionsInterface(const DistributionsInterface &other)
      : DistributionsInterfaceBase(other) {}

  /**
   * @brief The destructor.
   */
  virtual ~DistributionsInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id_m; }

  /**
   * @brief Sets the likelihood type of the distribution object.
   * @param likelihood_type The likelihood type to set for the distribution object.
   */
  virtual void set_likelihood_type(std::string likelihood_type) {
    if(likelihood_type == "data") {
      this->likelihood_type_m = fims_distributions::Likelihood_Kind::DATA;
    }else if(likelihood_type == "random") {
      this->likelihood_type_m = fims_distributions::Likelihood_Kind::RANDOM_EFFECT;
    }else if(likelihood_type == "prior") {
      this->likelihood_type_m = fims_distributions::Likelihood_Kind::PRIOR;
    }else if(likelihood_type == "penalty") {
      this->likelihood_type_m = fims_distributions::Likelihood_Kind::PENALTY;
    }else{
      Rcpp::stop("Invalid likelihood type: " + likelihood_type + ". Must be one of 'data', 'random', 'prior', 'penalty'.");
    }
  }

  /**
   * @brief Sets the distribution type of the distribution object.
   * @param distribution_type The distribution type to set for the distribution object.
   */
  virtual void set_distribution_type(std::string distribution_type) {
    if(distribution_type == "normal") {
      this->distribution_type_m = fims_distributions::Density_Function::NORMAL;
    }else if(distribution_type == "lognormal") {
      this->distribution_type_m = fims_distributions::Density_Function::LOGNORMAL;
    }else if(distribution_type == "multinomial") {
      this->distribution_type_m = fims_distributions::Density_Function::MULTINOMIAL;
    }else{
      Rcpp::stop("Invalid distribution type: " + distribution_type + ". Must be one of 'normal', 'lognormal', 'multinomial'.");
    }
  }


  /**
   * @brief Set the values for the likelihood evaluation.
   * @param observed_values Vector of observed values for likelihood evaluation.
   * @param expected_values Vector of expected values for likelihood evaluation.
   * @param uncertainty_values Vector of uncertainty values for likelihood evaluation.
   */

  virtual bool set_values(Rcpp::Nullable<Rcpp::NumericVector> observed_values = R_NilValue, 
                          Rcpp::Nullable<Rcpp::NumericVector> expected_values = R_NilValue, 
                          Rcpp::Nullable<Rcpp::NumericVector> uncertainty_values = R_NilValue) {
    if(observed_values!= R_NilValue) {
      Rcpp::NumericVector observed_values(observed_values);
      this->observed_values_m.resize(observed_values.size());
      for (size_t i = 0; i < observed_values.size(); i++) {
        this->observed_values_m[i].initial_value_m = observed_values[i];
      }
    }
    if(expected_values!= R_NilValue) {
      Rcpp::NumericVector expected_values(expected_values);
      this->expected_values_m.resize(expected_values.size());
      for (size_t i = 0; i < expected_values.size(); i++) {
        this->expected_values_m[i].initial_value_m = expected_values[i];
      }
    }
    if(uncertainty_values!= R_NilValue) {
      Rcpp::NumericVector uncertainty_values(uncertainty_values);
      this->uncertainty_values_m.resize(uncertainty_values.size());
      for (size_t i = 0; i < uncertainty_values.size(); i++) {
        this->uncertainty_values_m[i].initial_value_m = uncertainty_values[i];
      }
    }
    return true;
  }

  /**
   * @brief Set the values for the likelihood evaluation.
   * @param observed_id Fims variable id link to the observed values for likelihood evaluation.
   * @param expected_id Fims variable id link to the expected values for likelihood evaluation.
   * @param uncertainty_id Fims variable id link to the uncertainty values for likelihood evaluation.
   */

  virtual bool set_id_links(Rcpp::Nullable<int> observed_id = R_NilValue, 
                            Rcpp::Nullable<int> expected_id = R_NilValue, 
                            Rcpp::Nullable<int> uncertainty_id = R_NilValue) {
    if(observed_id!= R_NilValue) {
      int observed_id_value(observed_id);
      this->observed_key_m->resize(1);
      this->observed_key_m->at(0) = observed_id_value;
    }
    if(expected_id!= R_NilValue) {
      int expected_id_value(expected_id);
      this->expected_key_m->resize(1);
      this->expected_key_m->at(0) = expected_id_value;
    }
    if(uncertainty_id!= R_NilValue) {
      int uncertainty_id_value(uncertainty_id);
      this->uncertainty_key_m->resize(1);
      this->uncertainty_key_m->at(0) = uncertainty_id_value;
    }
    return true;
  }

  /**
   * @brief Set subvector index for the likelihood evaluation.
   * @param observed_subvector Index to specify specific observed values from the 
   * linked vector to include in the likelihood evaluation.
   * @param expected_subvector Index to specify specific expected values from the 
   * linked vector to include in the likelihood evaluation.
   * @param uncertainty_subvector Index to specify specific uncertainty values from the 
   * linked vector to include in the likelihood evaluation.
   */

  virtual bool set_subvector_indices(Rcpp::Nullable<Rcpp::IntegerVector> observed_subvector = R_NilValue, 
                                     Rcpp::Nullable<Rcpp::IntegerVector> expected_subvector = R_NilValue, 
                                     Rcpp::Nullable<Rcpp::IntegerVector> uncertainty_subvector = R_NilValue) {
    if(observed_subvector!= R_NilValue) {
      Rcpp::IntegerVector observed_subvector(observed_subvector);
      this->observed_subvector_m->resize(observed_subvector.size());
      for(size_t i = 0; i < observed_subvector.size(); i++) {
        this->observed_subvector_m->at(i) = static_cast<size_t>(observed_subvector[i]);
      }
    }
    if(expected_subvector!= R_NilValue) {
      Rcpp::IntegerVector expected_subvector(expected_subvector);
      this->expected_subvector_m->resize(expected_subvector.size());
      for(size_t i = 0; i < expected_subvector.size(); i++) {
        this->expected_subvector_m->at(i) = static_cast<size_t>(expected_subvector[i]); 
      }
    }
    if(uncertainty_subvector!= R_NilValue) {
      Rcpp::IntegerVector uncertainty_subvector(uncertainty_subvector);
      this->uncertainty_subvector_m->resize(uncertainty_subvector.size());
      for(size_t i = 0; i < uncertainty_subvector.size(); i++) {
        this->uncertainty_subvector_m->at(i) = static_cast<size_t>(uncertainty_subvector[i]); 
      }
    }
    return true;
  }

  /**
   * @brief Evaluate probability density function (pdf). The natural log
   * of the pdf is returned.
   * @return The natural log of the probability density function (pdf) is
   * returned.
   */
  virtual double evaluate() {



    fims_distributions::DensityComponentBase<double> generic_density;
    generic_density.observed_values.resize(this->observed_values_m.size());
    for (size_t i = 0; i < this->observed_values_m.size(); i++) {
      generic_density.observed_values[i] = this->observed_values_m[i].initial_value_m;
    }
    generic_density.expected_values.resize(this->expected_values_m.size());
    for (size_t i = 0; i < this->expected_values_m.size(); i++) {
      generic_density.expected_values[i] = this->expected_values_m[i].initial_value_m;
    }
    generic_density.uncertainty_values.resize(this->uncertainty_values_m.size());
    for (size_t i = 0; i < this->uncertainty_values_m.size(); i++) {
      generic_density.uncertainty_values[i] = this->uncertainty_values_m[i].initial_value_m;
    }
    
    
    return generic_density.evaluate();
  }

  /**
   * @brief Extracts the derived quantities from `Information` to the Rcpp
   * object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("DistributionInterface  " + fims::to_string(this->id_m) +
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
      FIMS_WARNING_LOG("DistributionInterface " + fims::to_string(this->id_m) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_distributions::DensityComponentBase<double>> dgeneric =
          std::dynamic_pointer_cast<fims_distributions::DensityComponentBase<double>>(
              it->second);

      this->lpdf_value = dgeneric->lpdf;

      size_t n_x = static_cast<size_t>(dgeneric->check_input_sizes());

      // If input observed is a scalar, resize to n_x and fill with the scalar
      // value
      if (this->observed_values_m.size() == 0) {
        this->observed_values_m.resize(n_x);
        if ((*dgeneric->observed_pointer).size() == 1) {
          // If input pointer is a scalar, resize to n_x and fill with the scalar
          // value
          auto tmp = (*dgeneric->observed_pointer).at(0); 
          for (size_t i = 0; i < n_x; ++i) {
            this->observed_values_m[i].estimation_type_m = std::string("derived"); 
            this->observed_values_m[i].final_value_m = tmp[0]; 
          }
        }else if ((*dgeneric->observed_pointer).size() == n_x) {
          for (size_t i = 0; i < n_x; ++i) {
            this->observed_values_m[i].estimation_type_m = std::string("derived"); 
            this->observed_values_m[i].final_value_m = (*dgeneric->observed_pointer).at(i)[0]; 
          }
        } else {
          FIMS_WARNING_LOG(
              "observed_pointer size does not match number of observations and is not "
              "scalar.");
        }
      }else if (this->observed_values_m.size() == 1) {
        // If input uncertainty is a scalar, resize to n_x and fill with the scalar
        // value
        auto tmp = this->observed_values_m[0];  // copy the one observed value
        this->observed_values_m.resize(n_x);
        for (size_t i = 0; i < n_x; ++i) {
          this->observed_values_m[i] = tmp;  // copies all fields in Param
          if (this->observed_values_m[i].estimation_type_m.get() == "constant") {
            this->observed_values_m[i].final_value_m = this->observed_values_m[i].initial_value_m;
          } else {
            this->observed_values_m[i].final_value_m = dgeneric->observed_values.get_force_scalar(i);
          }
        }
      }else if (this->observed_values_m.size() != n_x){
          // Handle error
          FIMS_WARNING_LOG(
              "observed_values size does not match number of observations and is not "
              "scalar.");
      }

      // If input expected is a scalar, resize to n_x and fill with the scalar
      // value
      if (this->expected_values_m.size() == 0) {
        this->expected_values_m.resize(n_x);
        if ((*dgeneric->expected_pointer).size() == 1) {
          // If input pointer is a scalar, resize to n_x and fill with the scalar
          // value
          auto tmp = (*dgeneric->expected_pointer).at(0); 
          for (size_t i = 0; i < n_x; ++i) {
            this->expected_values_m[i].estimation_type_m = std::string("derived"); 
            this->expected_values_m[i].final_value_m = tmp[0]; 
          }
        }else if ((*dgeneric->expected_pointer).size() == n_x) {
          for (size_t i = 0; i < n_x; ++i) {
            this->expected_values_m[i].estimation_type_m = std::string("derived"); 
            this->expected_values_m[i].final_value_m = (*dgeneric->expected_pointer).at(i)[0]; 
          }
        } else {
          FIMS_WARNING_LOG(
              "expected_pointer size does not match number of observations and is not "
              "scalar.");
        }
      }else if (this->expected_values_m.size() == 1) {
        // If input expected is a scalar, resize to n_x and fill with the scalar
        // value
        auto tmp = this->expected_values_m[0];  // copy the one expected param
        this->expected_values_m.resize(n_x);
        for (size_t i = 0; i < n_x; ++i) {
          this->expected_values_m[i] = tmp;  // copies all fields in Param
          if (this->expected_values_m[i].estimation_type_m.get() == "constant") {
            this->expected_values_m[i].final_value_m = this->expected_values_m[i].initial_value_m;
          } else {
            this->expected_values_m[i].final_value_m = dgeneric->expected_values.get_force_scalar(i);
          }
        }
      }else if (this->expected_values_m.size() != n_x){
          // Handle error
          FIMS_WARNING_LOG(
              "expected_values size does not match number of observations and is not "
              "scalar.");
      }

      // If input uncertainty is a scalar, resize to n_x and fill with the scalar
      // value
      if (this->uncertainty_values_m.size() == 0) {
        this->uncertainty_values_m.resize(n_x);
        if ((*dgeneric->uncertainty_pointer).size() == 1) {
          // If input pointer is a scalar, resize to n_x and fill with the scalar
          // value
          auto tmp = (*dgeneric->uncertainty_pointer).at(0); 
          for (size_t i = 0; i < n_x; ++i) {
            this->uncertainty_values_m[i].estimation_type_m = std::string("derived"); 
            this->uncertainty_values_m[i].final_value_m = tmp[0]; 
          }
        }else if ((*dgeneric->uncertainty_pointer).size() == n_x) {
          for (size_t i = 0; i < n_x; ++i) {
            this->uncertainty_values_m[i].estimation_type_m = std::string("derived"); 
            this->uncertainty_values_m[i].final_value_m = (*dgeneric->uncertainty_pointer).at(i)[0]; 
          }
        } else {
          FIMS_WARNING_LOG(
              "uncertainty_pointer size does not match number of observations and is not "
              "scalar.");
        }
      }else if (this->uncertainty_values_m.size() == 1) {
        // If input uncertainty is a scalar, resize to n_x and fill with the scalar
        // value
        auto tmp = this->uncertainty_values_m[0];  // copy the one uncertainty param
        this->uncertainty_values_m.resize(n_x);
        for (size_t i = 0; i < n_x; ++i) {
          this->uncertainty_values_m[i] = tmp;  // copies all fields in Param
          if (this->uncertainty_values_m[i].estimation_type_m.get() == "constant") {
            this->uncertainty_values_m[i].final_value_m = this->uncertainty_values_m[i].initial_value_m;
          } else {
            this->uncertainty_values_m[i].final_value_m = dgeneric->uncertainty_values.get_force_scalar(i);
          }
        }
      }else if (this->uncertainty_values_m.size() != n_x){
          // Handle error
          FIMS_WARNING_LOG(
              "uncertainty_values size does not match number of observations and is not "
              "scalar.");
      }

      this->lpdf_vec_m = RealVector(n_x);
      for (size_t i = 0; i < this->lpdf_vec.size(); i++) {
        this->lpdf_vec_m[i] = dgeneric->lpdf_vec[i];
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
    ss << "  \"lpdf_value\": " << sanitize_val(this->lpdf_value) << ",\n";
    ss << "  \"value\":[";
    if (this->lpdf_vec.size() == 0) {
      ss << "],\n";
    } else {
      for (size_t i = 0; i < this->lpdf_vec.size() - 1; i++) {
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
      for (size_t i = 0; i < this->expected_values.size() - 1; i++) {
        ss << this->value_to_string(this->expected_values[i].final_value_m)
           << ", ";
      }
      ss << this->value_to_string(
          this->expected_values[this->expected_values.size() - 1]
              .final_value_m);
      ss << "],\n";
    }
    ss << "  \"log_sd_values\":[";
    if (this->log_sd.size() == 0) {
      ss << "],\n";
    } else {
      for (R_xlen_t i = 0; i < static_cast<R_xlen_t>(this->log_sd.size()) - 1;
           i++) {
        ss << this->value_to_string(this->log_sd[i].final_value_m) << ", ";
      }
      ss << this->value_to_string(
                this->log_sd[this->log_sd.size() - 1].final_value_m)
         << "],\n";
    }
    ss << "  \"observed_values\":[";
    if (this->observed_values.size() == 0) {
      ss << "]\n";
    } else {
      for (size_t i = 0; i < this->observed_values.size() - 1; i++) {
        ss << this->observed_values[i].final_value_m << ", ";
      }
      ss << this->observed_values[this->observed_values.size() - 1]
                .final_value_m
         << "]\n";
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
    distribution->observed_values.resize(this->observed_values.size());
    for (size_t i = 0; i < this->observed_values.size(); i++) {
      distribution->observed_values[i] =
          this->observed_values[i].initial_value_m;
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
    for (size_t i = 0; i < this->expected_mean.size(); i++) {
      distribution->expected_mean[i] = this->expected_mean[i].initial_value_m;
      if (this->expected_mean[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "dnorm." << this->id_m << ".expected_mean."
           << this->expected_mean[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(distribution->expected_mean[i]);
      }
      if (this->expected_mean[i].estimation_type_m.get() == "random_effects") {
        FIMS_ERROR_LOG("expected_mean cannot be set to random effects");
      }
    }
    info->variable_map[this->expected_mean.id_m] =
        &(distribution)->expected_mean;

    info->density_components[distribution->id] = distribution;

    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();

    return true;
  }

#endif
};

/**
 * @brief The Rcpp interface for Dnorm to instantiate from R:
 * dnorm_ <- methods::new(DnormDistribution).
 */
class DnormDistributionsInterface : public DistributionsInterfaceBase {
 public:
  
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
        observed_values(other.observed_values),
        expected_values(other.expected_values),
        uncertainty_values(other.uncertainty_values),
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
   * @copydoc DistributionsInterfaceBase::set_distribution_links
   */
  virtual bool set_distribution_links(std::string input_type,
                                      Rcpp::IntegerVector ids) {
    this->input_type_m.set(input_type);
    this->key_m->resize(ids.size());
    for (R_xlen_t i = 0; i < ids.size(); i++) {
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
    dnorm.observed_values.resize(this->observed_values.size());
    dnorm.expected_values.resize(this->expected_values.size());
    dnorm.uncertainty_values.resize(this->uncertainty_values.size());
    for (size_t i = 0; i < this->observed_values.size(); i++) {
      dnorm.observed_values[i] = this->observed_values[i].initial_value_m;
    }
    for (size_t i = 0; i < this->expected_values.size(); i++) {
      dnorm.expected_values[i] = this->expected_values[i].initial_value_m;
    }
    
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

      size_t n_x = dnorm->get_n_x();

      // If input log_sd is a scalar, resize to n_x and fill with the scalar
      // value
      if (this->log_sd.size() != n_x) {
        // If log_sd size == 1 (scalar), repeat the entry
        if (this->log_sd.size() == 1) {
          auto tmp = this->log_sd[0];  // copy the one log_sd param
          this->log_sd.resize(n_x);
          for (size_t i = 0; i < n_x; ++i) {
            this->log_sd[i] = tmp;  // copies all fields in Param
          }
        } else {
          // Handle error
          FIMS_WARNING_LOG(
              "log_sd size does not match number of observations and is not "
              "scalar.");
        }
      }
      for (size_t i = 0; i < n_x; i++) {
        if (this->log_sd[i].estimation_type_m.get() == "constant") {
          this->log_sd[i].final_value_m = this->log_sd[i].initial_value_m;
        } else {
          this->log_sd[i].final_value_m = dnorm->log_sd.get_force_scalar(i);
        }
      }

      for (size_t i = 0; i < this->expected_mean.size(); i++) {
        if (this->expected_mean[i].estimation_type_m.get() == "constant") {
          this->expected_mean[i].final_value_m =
              this->expected_mean[i].initial_value_m;
        } else {
          this->expected_mean[i].final_value_m = dnorm->expected_mean[i];
        }
      }

      this->lpdf_vec = RealVector(n_x);
      if (this->expected_values.size() == 1) {
        this->expected_values.resize(n_x);
      }
      if (this->observed_values.size() == 1) {
        this->observed_values.resize(n_x);
      }

      for (size_t i = 0; i < this->lpdf_vec.size(); i++) {
        this->lpdf_vec[i] = dnorm->lpdf_vec[i];
        this->expected_values[i].final_value_m = dnorm->get_expected(i);
        this->observed_values[i].final_value_m = dnorm->get_observed(i);
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
    ss << "  \"lpdf_value\": " << sanitize_val(this->lpdf_value) << ",\n";
    ss << "  \"value\":[";
    if (this->lpdf_vec.size() == 0) {
      ss << "],\n";
    } else {
      for (size_t i = 0; i < this->lpdf_vec.size() - 1; i++) {
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
      for (size_t i = 0; i < this->expected_values.size() - 1; i++) {
        ss << this->value_to_string(this->expected_values[i].final_value_m)
           << ", ";
      }
      ss << this->value_to_string(
          this->expected_values[this->expected_values.size() - 1]
              .final_value_m);
      ss << "],\n";
    }
    ss << "  \"log_sd_values\":[";
    if (this->log_sd.size() == 0) {
      ss << "],\n";
    } else {
      for (R_xlen_t i = 0; i < static_cast<R_xlen_t>(this->log_sd.size()) - 1;
           i++) {
        ss << this->value_to_string(this->log_sd[i].final_value_m) << ", ";
      }
      ss << this->value_to_string(
                this->log_sd[this->log_sd.size() - 1].final_value_m)
         << "],\n";
    }
    ss << "  \"observed_values\":[";
    if (this->observed_values.size() == 0) {
      ss << "]\n";
    } else {
      for (size_t i = 0; i < this->observed_values.size() - 1; i++) {
        ss << this->observed_values[i].final_value_m << ", ";
      }
      ss << this->observed_values[this->observed_values.size() - 1]
                .final_value_m
         << "]\n";
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
    distribution->observed_values.resize(this->observed_values.size());
    for (size_t i = 0; i < this->observed_values.size(); i++) {
      distribution->observed_values[i] =
          this->observed_values[i].initial_value_m;
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
    for (size_t i = 0; i < this->expected_mean.size(); i++) {
      distribution->expected_mean[i] = this->expected_mean[i].initial_value_m;
      if (this->expected_mean[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "dnorm." << this->id_m << ".expected_mean."
           << this->expected_mean[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(distribution->expected_mean[i]);
      }
      if (this->expected_mean[i].estimation_type_m.get() == "random_effects") {
        FIMS_ERROR_LOG("expected_mean cannot be set to random effects");
      }
    }
    info->variable_map[this->expected_mean.id_m] =
        &(distribution)->expected_mean;

    info->density_components[distribution->id] = distribution;

    return true;
  }

  /**
   * @brief Adds the parameters to the TMB model.
   * @return A boolean of true.
   */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();

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
  VariableVector observed_values;
  /**
   * @brief The expected values, which would be the mean of log(x) for this
   * distribution.
   */
  VariableVector expected_values;
  /**
   * @brief The uncertainty, which would be the natural logarithm of the
     standard deviation (sd) of log(x) for this distribution. The natural log
     of the standard deviation is necessary because the exponential link
     function is applied to the log transformed standard deviation to insure
     standard deviation is positive.
   */
  VariableVector log_sd;
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
        observed_values(other.observed_values),
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
    for (R_xlen_t i = 0; i < ids.size(); i++) {
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
    dlnorm.observed_values.resize(this->observed_values.size());
    dlnorm.expected_values.resize(this->expected_values.size());
    dlnorm.log_sd.resize(this->log_sd.size());
    // dlnorm.input_type = "prior";
    for (size_t i = 0; i < this->observed_values.size(); i++) {
      dlnorm.observed_values[i] = this->observed_values[i].initial_value_m;
    }
    for (size_t i = 0; i < this->expected_values.size(); i++) {
      dlnorm.expected_values[i] = this->expected_values[i].initial_value_m;
    }
    for (size_t i = 0; i < this->log_sd.size(); i++) {
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

      size_t n_x = dlnorm->get_n_x();

      if (this->log_sd.size() != n_x) {
        // If log_sd size == 1 (scalar), repeat the entry
        if (this->log_sd.size() == 1) {
          auto tmp = this->log_sd[0];  // copy the one log_sd param
          this->log_sd.resize(n_x);
          for (size_t i = 0; i < n_x; ++i) {
            this->log_sd[i] = tmp;  // copies all fields in Param
          }
        } else {
          // Handle error
          FIMS_WARNING_LOG(
              "log_sd size does not match number of observations and is not "
              "scalar.");
        }
      }

      for (size_t i = 0; i < n_x; i++) {
        if (this->log_sd[i].estimation_type_m.get() == "constant") {
          this->log_sd[i].final_value_m = this->log_sd[i].initial_value_m;
        } else {
          this->log_sd[i].final_value_m = dlnorm->log_sd.get_force_scalar(i);
        }
      }

      this->lpdf_vec = RealVector(n_x);
      if (this->expected_values.size() == 1) {
        this->expected_values.resize(n_x);
      }
      if (this->observed_values.size() == 1) {
        this->observed_values.resize(n_x);
      }
      for (size_t i = 0; i < this->lpdf_vec.size(); i++) {
        this->lpdf_vec[i] = dlnorm->lpdf_vec[i];
        this->expected_values[i].final_value_m = dlnorm->get_expected(i);
        this->observed_values[i].final_value_m = dlnorm->get_observed(i);
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
    ss << "  \"lpdf_value\": " << sanitize_val(this->lpdf_value) << ",\n";
    ss << "  \"value\":[";
    if (this->lpdf_vec.size() == 0) {
      ss << "],\n";
    } else {
      for (size_t i = 0; i < this->lpdf_vec.size() - 1; i++) {
        ss << this->value_to_string(this->lpdf_vec[i]) << ", ";
      }
      ss << this->value_to_string(this->lpdf_vec[this->lpdf_vec.size() - 1]);

      ss << "],\n";
    }
    ss << "  \"expected_values\":[";
    if (this->expected_values.size() == 0) {
      ss << "],\n";
    } else {
      for (size_t i = 0; i < this->expected_values.size() - 1; i++) {
        ss << this->value_to_string(this->expected_values[i].final_value_m)
           << ", ";
      }
      ss << this->value_to_string(
          this->expected_values[this->expected_values.size() - 1]
              .final_value_m);

      ss << "],\n";
    }
    ss << "  \"log_sd_values\":[";
    if (this->log_sd.size() == 0) {
      ss << "],\n";
    } else {
      for (R_xlen_t i = 0; i < static_cast<R_xlen_t>(this->log_sd.size()) - 1;
           i++) {
        ss << this->value_to_string(this->log_sd[i].final_value_m) << ", ";
      }
      ss << this->value_to_string(
                this->log_sd[this->log_sd.size() - 1].final_value_m)
         << "],\n";
    }
    ss << "  \"observed_values\":[";
    if (this->observed_values.size() == 0) {
      ss << "]\n";
    } else {
      for (size_t i = 0; i < this->observed_values.size() - 1; i++) {
        ss << this->observed_values[i].final_value_m << ", ";
      }
      ss << this->observed_values[this->observed_values.size() - 1]
                .final_value_m
         << "]\n";
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
    distribution->observed_values.resize(this->observed_values.size());
    for (size_t i = 0; i < this->observed_values.size(); i++) {
      distribution->observed_values[i] =
          this->observed_values[i].initial_value_m;
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
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();

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
  VariableVector observed_values;
  /**
   * @brief The expected values, which should be a vector of length K where
   * each value specifies the probability of class k. Note that, unlike in R,
   * these probabilities must sum to 1.0.
   */
  VariableVector expected_values;
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
        observed_values(other.observed_values),
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
    for (R_xlen_t i = 0; i < ids.size(); i++) {
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
    dmultinom.observed_values.resize(this->observed_values.size());
    dmultinom.expected_values.resize(this->expected_values.size());
    for (size_t i = 0; i < observed_values.size(); i++) {
      dmultinom.observed_values[i] = this->observed_values[i].initial_value_m;
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

      size_t n_x = dmultinom->lpdf_vec.size();
      this->lpdf_vec = Rcpp::NumericVector(n_x);
      if (this->expected_values.size() != n_x) {
        this->expected_values.resize(n_x);
      }
      if (this->observed_values.size() != n_x) {
        this->observed_values.resize(n_x);
      }
      for (size_t i = 0; i < this->lpdf_vec.size(); i++) {
        this->lpdf_vec[i] = dmultinom->lpdf_vec[i];
        this->expected_values[i].final_value_m = dmultinom->get_expected(i);
        if (dmultinom->input_type != "data") {
          this->observed_values[i].final_value_m = dmultinom->get_observed(i);
        }
      }
      if (dmultinom->input_type == "data") {
        dims.resize(2);
        dims[0] = dmultinom->dims[0];
        dims[1] = dmultinom->dims[1];
        for (size_t i = 0; i < dims[0]; i++) {
          for (size_t j = 0; j < dims[1]; j++) {
            size_t idx = (i * dims[1]) + j;
            this->observed_values[idx].final_value_m = dmultinom->get_observed(
                static_cast<size_t>(i), static_cast<size_t>(j));
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
    ss << "  \"lpdf_value\": " << sanitize_val(this->lpdf_value) << ",\n";
    ss << "  \"value\":[";
    if (this->lpdf_vec.size() == 0) {
      ss << "],\n";
    } else {
      for (size_t i = 0; i < this->lpdf_vec.size() - 1; i++) {
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
      for (size_t i = 0; i < this->expected_values.size() - 1; i++) {
        ss << this->value_to_string(this->expected_values[i].final_value_m)
           << ", ";
      }
      ss << this->value_to_string(
          this->expected_values[this->expected_values.size() - 1]
              .final_value_m);

      ss << "],\n";
    }
    // no log_sd_values for multinomial
    ss << "  \"observed_values\":[";
    if (this->observed_values.size() == 0) {
      ss << "]\n";
    } else {
      for (size_t i = 0; i < this->observed_values.size() - 1; i++) {
        ss << this->observed_values[i].final_value_m << ", ";
      }
      ss << this->observed_values[this->observed_values.size() - 1]
                .final_value_m
         << "]\n";
    }
    ss << " }}\n";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
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
    distribution->observed_values.resize(this->observed_values.size());
    for (size_t i = 0; i < this->observed_values.size(); i++) {
      distribution->observed_values[i] =
          this->observed_values[i].initial_value_m;
    }
    // set relative info
    distribution->expected_values.resize(this->expected_values.size());
    for (size_t i = 0; i < this->expected_values.size(); i++) {
      distribution->expected_values[i] =
          this->expected_values[i].initial_value_m;
    }

    info->density_components[distribution->id] = distribution;
    return true;
  }

  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMBAD_FIMS_TYPE>();

    return true;
  }

#endif
};

#endif
