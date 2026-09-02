/**
 * @file rcpp_growth.hpp
 * @brief The Rcpp interface to declare different types of growth, e.g.,
 * empirical weight-at-age data. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_GROWTH_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_GROWTH_HPP

#include "../../../population_dynamics/growth/growth.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief The growth forms FIMS can build.
 *
 * @details The create_growth_() function takes one of these names from R and
 * builds the matching class: "EWAA" builds an EWAAGrowthInterface.
 * GrowthInterfaceBase is never built on its own; it only holds what all growth
 * forms have in common.
 *
 * These are an enum rather than plain strings so that every place in the C++
 * code that acts on a growth form has to name one of these values, which makes
 * it harder to add a form and forget to handle it somewhere.
 */
enum class GrowthType : uint8_t {
  ewaa = 0
};

/**
 * @brief Convert a type name supplied from R to a GrowthType.
 */
inline GrowthType GrowthTypeFromString(const std::string &name) {
  if (name == "EWAA") return GrowthType::ewaa;
  throw std::invalid_argument(
      "Invalid type: '" + name +
      "'. Valid options are: EWAA.");
}

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp growth
 * interfaces. This type should be inherited and not called from R directly.
 */
class GrowthInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static id of the GrowthInterfaceBase object.
   */
  static uint32_t id_g;
  /**
   * @brief The local id of the GrowthInterfaceBase object.
   */
  uint32_t id;

  /**
   * @brief The constructor.
   */
  GrowthInterfaceBase() { this->id = GrowthInterfaceBase::id_g++; }

  /**
   * @brief Interface objects are not copyable.
   */
  GrowthInterfaceBase(const GrowthInterfaceBase &) = delete;
  GrowthInterfaceBase &operator=(const GrowthInterfaceBase &) = delete;

  /**
   * @brief The destructor.
   */
  virtual ~GrowthInterfaceBase() {}

    /**
   * @brief Set the number of years this growth module spans.
   *
   * @details Defaults to reporting that the type does not use it, so a growth
   * type without a time dimension needs no override.
   */
  virtual void set_n_years(int n_years) {
    Rcpp::stop("This growth module does not use `n_years`.");
  }

  /**
   * @brief A method for each child growth interface object to inherit so
   * each growth option can have an evaluate() function.
   */
  virtual double evaluate(double age) = 0;
};

/**
 * @brief Rcpp interface for EWAAGrowth to instantiate the object from R:
 * ewaa <- methods::new(EWAAGrowth). Where, EWAA stands for empirical weight at
 * age and growth is not actually estimated.
 */
class EWAAGrowthInterface : public GrowthInterfaceBase {
 public:
  /**
   * @brief Weights (mt) for each age class.
   */
  fims::Vector<double> weights;
  /**
   * @brief Ages (years) for each age class.
   */
  fims::Vector<double> ages;
  /**
   * @brief An integer specifying the number of years.
   *
   */
  int n_years = 0;
  /**
   * @brief A map of empirical weight-at-age values allowing multiple modules to
   * access and modify the weights without copying values between modules.
   */
  std::shared_ptr<std::map<int, std::map<double, double>>> ewaa;
  /**
   * @brief Have weight and age vectors been set? The default is false.
   */
  bool initialized = false;

  /**
   * @brief The constructor.
   */
  EWAAGrowthInterface() : GrowthInterfaceBase() {
    this->ewaa = std::make_shared<std::map<int, std::map<double, double>>>();
  }

  /**
   * @brief Interface objects are not copyable.
   */
  EWAAGrowthInterface(const EWAAGrowthInterface &) = delete;
  EWAAGrowthInterface &operator=(const EWAAGrowthInterface &) = delete;

  /**
   * @brief The destructor.
   */
  virtual ~EWAAGrowthInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @copydoc FIMSRcppInterfaceBase::get_numeric_vector
   */
  virtual fims::Vector<double> *get_numeric_vector(const std::string &name) {
    if (name == "weights") return &this->weights;
    if (name == "ages") return &this->ages;
    return nullptr;
  }

  /**
   * @copydoc GrowthInterfaceBase::set_n_years
   */
  virtual void set_n_years(int n_years) { this->n_years = n_years; }

  /**
   * @brief Create a map of input numeric vectors.
   * @param weights Type vector of weights.
   * @param ages Type vector of ages.
   * @param n_years An integer specifying the number of years.
   * @return std::map<T, T>.
   */
  inline std::map<int, std::map<double, double>> make_map(
      const fims::Vector<double> &ages, const fims::Vector<double> &weights,
                                                          int n_years) {
    std::map<int, std::map<double, double>> mymap;
    const size_t n_years_plus_one = static_cast<size_t>(n_years + 1);

    // Reject invalid year counts because map keys are expected to include
    // at least one model year.
    if (n_years < 1) {
      Rcpp::stop("EWAA Error:: n_years must be at least 1");
    }

    // Reject empty vectors because we need at least one age-weight pair.
    if (weights.size() == 0 || ages.size() == 0) {
      Rcpp::stop("EWAA Error:: ages and weights must have at least one value");
    }

    // Accept either:
    // 1) one weight vector by age (shared across years), or
    // 2) a full year-by-age matrix flattened as (n_years + 1) * n_ages.
    if ((weights.size() != ages.size() * n_years_plus_one) &&
        (weights.size() != ages.size())) {
      Rcpp::stop(
          "weights size does not match ages size or ages size times "
          "(n_years + 1), where the plus one is for the beginning "
          "of the year after the terminal year spawning-biomass "
          "calculations. weights size: " +
          std::to_string(weights.size()) +
          " ages size: " + std::to_string(ages.size()) +
          " n_years: " + std::to_string(n_years));
    } else if (weights.size() == ages.size()) {
      // One age-specific vector was provided, so replicate the same
      // weight-at-age values for every year key (0 through n_years).
      for (size_t y = 0; y < n_years_plus_one; y++) {
        for (size_t i = 0; i < ages.size(); i++) {
          mymap[y][ages[i]] = weights[i];
        }
      }
    } else if (weights.size() == ages.size() * n_years_plus_one) {
      // A flattened year-by-age matrix was provided, so map each block of
      // n_ages values to the corresponding year key (0 through n_years).
      for (size_t y = 0; y < n_years_plus_one; y++) {
        for (size_t i = 0; i < ages.size(); i++) {
          mymap[y][ages[i]] = weights[y * ages.size() + i];
        }
      }
    }
    return mymap;
  }

  /**
   * @brief Evaluate the growth using empirical weight at age.
   * @param age The age at of the individual to evaluate weight.
   * @details This can be called from R using ewaagrowth.evaluate(age).
   */
  virtual double evaluate(double age) {
    fims_popdy::EWAAGrowth<double> EWAAGrowth;

    // Build the EWAA map once from R inputs the first time evaluate() is
    // called.
    if (initialized == false) {
      EWAAGrowth.ewaa = make_map(this->ages, this->weights, this->n_years);
      initialized = true;
    } else {
      // Prevent re-initializing this object with a second evaluate() call.
      Rcpp::stop("this empirical weight at age object is already initialized");
    }
    return EWAAGrowth.evaluate(0, age);
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * growth interface with empirical weight at age. It also returns the ID,
   * the rank of 1, the dimensions, age bins, and the calculated values
   * themselves. This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;
    ss << "{\n";
    ss << " \"module_name\": \"Growth\",\n";
    ss << " \"module_type\": \"EWAA\",\n";
    ss << " \"module_id\":" << this->id << ",\n";
    ss << " \"parameters\": [\n{\n";
    ss << " \"name\": \"weight_at_age\",\n";
    ss << " \"id\": null,\n";
    ss << " \"type\": \"vector\",\n";
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [\"n_years+1\",\"n_ages\"],\n";
    ss << "  \"dimensions\": [" << this->n_years + 1 << ","
       << this->ages.size() << "]\n},\n";

    ss << " \"values\": [\n";
    for (size_t i = 0; i < weights.size() - 1; i++) {
      ss << "{\n";
      ss << "\"id\": null,\n";
      ss << "\"value\": " << weights[i] << ",\n";
      ss << "\"estimated_value\": " << weights[i] << ",\n";
      ss << "\"estimation_status\": \"assumed_known\"\n";
      ss << "},\n";
    }
    ss << "{\n";
    ss << "\"id\": null,\n";
    ss << "\"value\": " << weights[weights.size() - 1] << ",\n";
    ss << "\"estimated_value\": " << weights[weights.size() - 1] << ",\n";
    ss << "\"estimation_status\": \"assumed_known\"\n";
    ss << "}\n]\n";
    ss << "}\n]\n}\n";
    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type>> info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::EWAAGrowth<Type>> ewaa_growth =
        std::make_shared<fims_popdy::EWAAGrowth<Type>>();

    // set relative info
    ewaa_growth->id = this->id;
    ewaa_growth->ewaa =
        make_map(this->ages, this->weights, this->n_years);  // this->ewaa;
    // add to Information
    info->growth_models[ewaa_growth->id] = ewaa_growth;

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

#endif
