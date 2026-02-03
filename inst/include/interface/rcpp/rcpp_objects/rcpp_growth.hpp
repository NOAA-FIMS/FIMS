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
   * @brief The map associating the IDs of GrowthInterfaceBase to the objects.
   * This is a live object, which is an object that has been created and lives
   * in memory.
   */
  static std::map<uint32_t, std::shared_ptr<GrowthInterfaceBase>> live_objects;

  /**
   * @brief The constructor.
   */
  GrowthInterfaceBase() {
    this->id = GrowthInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    GrowthInterfaceBase */
    // GrowthInterfaceBase::live_objects[this->id] =
    // std::make_shared<GrowthInterfaceBase>(*this);
  }

  /**
   * @brief Construct a new Growth Interface Base object
   *
   * @param other
   */
  GrowthInterfaceBase(const GrowthInterfaceBase &other) : id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~GrowthInterfaceBase() {}

  /**
   * @brief Get the ID for the child growth interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief A method for each child growth interface object to inherit so
   * each growth option can have an evaluate() function.
   */
  virtual double evaluate(double age) = 0;
};
// static id of the GrowthInterfaceBase object
uint32_t GrowthInterfaceBase::id_g = 1;
// local id of the GrowthInterfaceBase object map relating the ID of the
// GrowthInterfaceBase to the GrowthInterfaceBase objects
std::map<uint32_t, std::shared_ptr<GrowthInterfaceBase>>
    GrowthInterfaceBase::live_objects;

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
  RealVector weights;
  /**
   * @brief Ages (years) for each age class.
   */
  RealVector ages;
  /**
   * @brief An integer specifying the number of years.
   * 
   */
  SharedInt n_years;
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
    GrowthInterfaceBase::live_objects[this->id] =
        std::make_shared<EWAAGrowthInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        std::make_shared<EWAAGrowthInterface>(*this));
  }

  /**
   * @brief Construct a new EWAAGrowthInterface object
   *
   * @param other
   */
  EWAAGrowthInterface(const EWAAGrowthInterface &other)
      : GrowthInterfaceBase(other),
        weights(other.weights),
        ages(other.ages),
        ewaa(other.ewaa),
        initialized(other.initialized) {}

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
   * @brief Create a map of input numeric vectors.
   * @param weights Type vector of weights.
   * @param ages Type vector of ages.
   * @param n_years An integer specifying the number of years.
   * @return std::map<T, T>.
   */
  inline std::map<double, double> make_map(RealVector ages,
                                           RealVector weights,
                                           SharedInt n_years) {
    std::map<int, std::map<double, double>> mymap;
    for (uint32_t y = 0; y < n_years.get(); y++) {
      for (uint32_t i = 0; i < ages.size(); i++) {
        mymap[y][ages[i]] = weights[i];
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

    if (initialized == false) {
      EWAAGrowth.ewaa = make_map(this->ages, this->weights, this->n_years);
      initialized = true;
    } else {
      Rcpp::stop("this empirical weight at age object is already initialized");
    }
    return EWAAGrowth.evaluate(0, age);
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * growth interface with empirical weight at age. It also returns the ID, the
   * rank of 1, the dimensions, age bins, and the calculated values themselves.
   * This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;
    ss << "{\n";
    ss << " \"module_name\": \"Growth\",\n";
    ss << " \"module_type\": \"EWAA\",\n";
    ss << " \"module_id\":" << this->id << ",\n";
    ss << " \"parameters\": [\n{\n";
    ss << " \"name\": null,\n";
    ss << " \"id\": null,\n";
    ss << " \"type\": \"vector\",\n";
    // TODO: change the dimensionality
    ss << " \"dimensionality\": {\n";
    ss << "  \"header\": [\"n_ages\"],\n";
    ss << "  \"dimensions\": [" << this->ages.size() << "]\n},\n";

    ss << " \"values\": [\n";
    for (size_t i = 0; i < weights.size() - 1; i++) {
      ss << "{\n";
      ss << "\"id\": null,\n";
      ss << "\"value\": " << weights[i] << ",\n";
      ss << "\"estimated_value\": " << weights[i] << ",\n";
      ss << "\"uncertainty\": " << 0 << ",\n";
      ss << "\"min\": \"-Infinity\",\n";
      ss << "\"max\": \"Infinity\",\n";
      ss << "\"estimation_type\": \"constant\"\n";
      ss << "},\n";
    }
    ss << "{\n";
    ss << "\"id\": null,\n";
    ss << "\"value\": " << weights[weights.size() - 1] << ",\n";
    ss << "\"estimated_value\": " << weights[weights.size() - 1] << ",\n";
    ss << "\"uncertainty\": " << 0 << ",\n";
    ss << "\"min\": \"-Infinity\",\n";
    ss << "\"max\": \"Infinity\",\n";
    ss << "\"estimation_type\": \"constant\"\n";
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
    ewaa_growth->ewaa = make_map(this->ages, this->weights, this->n_years);  // this->ewaa;
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
#ifdef TMBAD_FRAMEWORK
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
