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

#include <cmath>

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
   * @brief A map of empirical weight-at-age values allowing multiple modules to
   * access and modify the weights without copying values between modules.
   */
  std::shared_ptr<std::map<double, double>> ewaa;
  /**
   * @brief Have weight and age vectors been set? The default is false.
   */
  bool initialized = false;

  /**
   * @brief The constructor.
   */
  EWAAGrowthInterface() : GrowthInterfaceBase() {
    this->ewaa = std::make_shared<std::map<double, double>>();
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
   * @return std::map<T, T>.
   */
  inline std::map<double, double> make_map(RealVector ages,
                                           RealVector weights) {
    std::map<double, double> mymap;
    for (uint32_t i = 0; i < ages.size(); i++) {
      mymap.insert(std::pair<double, double>(ages[i], weights[i]));
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
      // Check that ages and weights vector are the same length
      if (this->ages.size() != this->weights.size()) {
        Rcpp::stop("ages and weights must be the same length");
      }
      EWAAGrowth.ewaa = make_map(this->ages, this->weights);
      initialized = true;
    } else {
      Rcpp::stop("this empirical weight at age object is already initialized");
    }
    return EWAAGrowth.evaluate(age);
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
    ewaa_growth->ewaa = make_map(this->ages, this->weights);  // this->ewaa;
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

/**
 * @brief Rcpp interface for VonBertalanffyGrowth to instantiate from R:
 * vb <- methods::new(VonBertalanffyGrowth)
 */
class VonBertalanffyGrowthInterface : public GrowthInterfaceBase {
 public:
  // Required parameters
  ParameterVector L1;
  ParameterVector L2;
  ParameterVector K;
  ParameterVector age_L1;
  ParameterVector age_L2;

  // Length-weight conversion params
  ParameterVector a_wl;
  ParameterVector b_wl;
  // Length-at-age SD params
  ParameterVector SDgrowth;
  // Age dimension (for bounds checking)
  SharedInt n_ages = 0;

  VonBertalanffyGrowthInterface() : GrowthInterfaceBase() {
    GrowthInterfaceBase::live_objects[this->id] =
        std::make_shared<VonBertalanffyGrowthInterface>(*this);
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        std::make_shared<VonBertalanffyGrowthInterface>(*this));
  }

  VonBertalanffyGrowthInterface(const VonBertalanffyGrowthInterface& other)
      : GrowthInterfaceBase(other),
        L1(other.L1),
        L2(other.L2),
        K(other.K),
        age_L1(other.age_L1),
        age_L2(other.age_L2),
        a_wl(other.a_wl),
        b_wl(other.b_wl),
        SDgrowth(other.SDgrowth),
        n_ages(other.n_ages) {}

  virtual ~VonBertalanffyGrowthInterface() {}

  virtual uint32_t get_id() { return this->id; }

  virtual double evaluate(double age) {
    if (age < 0.0) {
      Rcpp::stop("Negative age not supported");
    }
    if (this->n_ages.get() <= 0) {
      Rcpp::stop("n_ages not set");
    }
    const double age_round = std::round(age);
    const double tol = 1e-8;
    if (std::fabs(age - age_round) > tol) {
      Rcpp::stop("Non-integer age not supported yet");
    }
    if (age_round >= this->n_ages.get()) {
      Rcpp::stop("Age out of range for growth model");
    }
    fims_popdy::VonBertalanffyGrowth<double> vb;
    if (this->L1.size() < 1 || this->L2.size() < 1 || this->K.size() < 1 ||
        this->age_L1.size() < 1 || this->age_L2.size() < 1 ||
        this->a_wl.size() < 1 || this->b_wl.size() < 1) {
      Rcpp::stop("VonBertalanffyGrowth parameters not set");
    }

    vb.L1 = this->L1[0].initial_value_m;
    vb.L2 = this->L2[0].initial_value_m;
    vb.K  = this->K[0].initial_value_m;
    vb.age_L1 = this->age_L1[0].initial_value_m;
    vb.age_L2 = this->age_L2[0].initial_value_m;

    vb.a_wl = this->a_wl[0].initial_value_m;
    vb.b_wl = this->b_wl[0].initial_value_m;

    return vb.evaluate(age);
  }

virtual std::string to_json() {
  std::stringstream ss;

  ss << "{\n";
  ss << " \"module_name\":\"Growth\",\n";
  ss << " \"module_type\": \"VonBertalanffy\",\n";
  ss << " \"module_id\": " << this->id << ",\n";

  ss << " \"parameters\": [\n";

  // L1
  ss << "{\n";
  ss << "   \"name\": \"L1\",\n";
  ss << "   \"id\":" << this->L1.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->L1.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->L1 << "\n";
  ss << "},\n";

  // K
  ss << "{\n";
  ss << "   \"name\": \"K\",\n";
  ss << "   \"id\":" << this->K.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->K.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->K << "\n";
  ss << "},\n";

  // L2
  ss << "{\n";
  ss << "   \"name\": \"L2\",\n";
  ss << "   \"id\":" << this->L2.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->L2.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->L2 << "\n";
  ss << "},\n";

  // age_L1
  ss << "{\n";
  ss << "   \"name\": \"age_L1\",\n";
  ss << "   \"id\":" << this->age_L1.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->age_L1.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->age_L1 << "\n";
  ss << "},\n";

  // age_L2
  ss << "{\n";
  ss << "   \"name\": \"age_L2\",\n";
  ss << "   \"id\":" << this->age_L2.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->age_L2.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->age_L2 << "\n";
  ss << "},\n";

  // a_wl
  ss << "{\n";
  ss << "   \"name\": \"a_wl\",\n";
  ss << "   \"id\":" << this->a_wl.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->a_wl.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->a_wl << "\n";
  ss << "},\n";

  // b_wl (last)
  ss << "{\n";
  ss << "   \"name\": \"b_wl\",\n";
  ss << "   \"id\":" << this->b_wl.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->b_wl.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->b_wl << "\n";
  ss << "},\n";

  // SDgrowth (SD1, SDA)
  ss << "{\n";
  ss << "   \"name\": \"SDgrowth\",\n";
  ss << "   \"id\":" << this->SDgrowth.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->SDgrowth.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->SDgrowth << "\n";
  ss << "}]\n";

  ss << "}";

  return ss.str();
}
#ifdef TMB_MODEL
template <typename Type>
bool add_to_fims_tmb_internal() {
  std::shared_ptr<fims_info::Information<Type>> info =
      fims_info::Information<Type>::GetInstance();

  std::shared_ptr<fims_popdy::VonBertalanffyGrowthModelAdapter<Type>> vb =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<Type>>();

  vb->id = this->id;

  // For now, use initial values
  if (this->L1.size() < 1 || this->L2.size() < 1 || this->K.size() < 1 ||
      this->age_L1.size() < 1 || this->age_L2.size() < 1 ||
      this->a_wl.size() < 1 || this->b_wl.size() < 1) {
    Rcpp::stop("VonBertalanffyGrowth parameters not set");
  }
  if (this->SDgrowth.size() < 2) {
    Rcpp::stop("SDgrowth must have two values (SD1, SDA)");
  }
  vb->SetVonBertalanffyParameters(
      this->L1[0].initial_value_m,
      this->L2[0].initial_value_m,
      this->K[0].initial_value_m,
      this->age_L1[0].initial_value_m,
      this->age_L2[0].initial_value_m);
  vb->SetLengthWeightParameters(
      this->a_wl[0].initial_value_m,
      this->b_wl[0].initial_value_m);
  vb->SetLengthSdParams(this->SDgrowth[0].initial_value_m,
                        this->SDgrowth[1].initial_value_m);

  info->growth_models[vb->id] = vb;
  return true;
}

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

};  // end class

#endif  // FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_GROWTH_HPP
