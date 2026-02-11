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

#include <algorithm>
#include <cmath>

#include "../../../population_dynamics/growth/growth.hpp"
#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
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
  ParameterVector length_at_ref_age_1;
  ParameterVector length_at_ref_age_2;
  ParameterVector growth_coefficient_K;
  ParameterVector reference_age_for_length_1;
  ParameterVector reference_age_for_length_2;

  // Length-weight conversion params
  ParameterVector length_weight_a;
  ParameterVector length_weight_b;
  // Length-at-age SD params
  ParameterVector length_at_age_sd_at_ref_ages;
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
        length_at_ref_age_1(other.length_at_ref_age_1),
        length_at_ref_age_2(other.length_at_ref_age_2),
        growth_coefficient_K(other.growth_coefficient_K),
        reference_age_for_length_1(other.reference_age_for_length_1),
        reference_age_for_length_2(other.reference_age_for_length_2),
        length_weight_a(other.length_weight_a),
        length_weight_b(other.length_weight_b),
        length_at_age_sd_at_ref_ages(other.length_at_age_sd_at_ref_ages),
        n_ages(other.n_ages) {}

  virtual ~VonBertalanffyGrowthInterface() {}

  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object from
   * the Information object.
   */
  virtual void finalize() {
    if (this->finalized) {
      FIMS_WARNING_LOG("VonBertalanffy Growth " + fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true;

    std::shared_ptr<fims_info::Information<double>> info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::growth_models_iterator it;
    it = info->growth_models.find(this->id);
    if (it == info->growth_models.end()) {
      FIMS_WARNING_LOG("VonBertalanffy Growth " + fims::to_string(this->id) +
                       " not found in Information.");
      return;
    }

    std::shared_ptr<fims_popdy::VonBertalanffyGrowthModelAdapter<double>> vb =
        std::dynamic_pointer_cast<
            fims_popdy::VonBertalanffyGrowthModelAdapter<double>>(it->second);
    if (!vb) {
      FIMS_WARNING_LOG("Growth model type mismatch for id " +
                       fims::to_string(this->id));
      return;
    }

    auto set_final = [](ParameterVector& pv, const fims::Vector<double>& src,
                        bool log_scale) {
      for (size_t i = 0; i < pv.size(); i++) {
        if (pv[i].estimation_type_m.get() == "constant") {
          pv[i].final_value_m = pv[i].initial_value_m;
        } else {
          double v = src[i];
          if (log_scale) {
            v = fims_math::exp(v);
          }
          pv[i].final_value_m = v;
        }
      }
    };

    set_final(this->length_at_ref_age_1, vb->LengthAtRefAge1Vector(), true);
    set_final(this->length_at_ref_age_2, vb->LengthAtRefAge2Vector(), true);
    set_final(this->growth_coefficient_K, vb->GrowthCoefficientKVector(), true);
    set_final(this->reference_age_for_length_1,
              vb->ReferenceAgeForLength1Vector(), false);
    set_final(this->reference_age_for_length_2,
              vb->ReferenceAgeForLength2Vector(), false);
    set_final(this->length_weight_a, vb->LengthWeightAVector(), true);
    set_final(this->length_weight_b, vb->LengthWeightBVector(), true);
    set_final(this->length_at_age_sd_at_ref_ages, vb->LengthAtAgeSdAtRefAgesVector(), true);
  }

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
    double min_age = 0.0;
    double max_age = static_cast<double>(this->n_ages.get() - 1);
    if (this->reference_age_for_length_1.size() > 0 &&
        this->reference_age_for_length_2.size() > 0) {
      const double a1 =
          this->reference_age_for_length_1[0].initial_value_m;
      const double a2 =
          this->reference_age_for_length_2[0].initial_value_m;
      if (a2 <= a1) {
        Rcpp::stop(
            "VonBertalanffyGrowth reference_age_for_length_2 must be > "
            "reference_age_for_length_1");
      }
      min_age = std::min(a1, a2);
      max_age = std::max(a1, a2);
    }
    if (age_round < min_age - tol || age_round > max_age + tol) {
      Rcpp::warning(
          "Age outside [reference_age_for_length_1, reference_age_for_length_2]; "
          "extrapolating growth curve.");
    }
    fims_popdy::VonBertalanffyGrowth<double> vb;
    if (this->length_at_ref_age_1.size() < 1 ||
        this->length_at_ref_age_2.size() < 1 ||
        this->growth_coefficient_K.size() < 1 ||
        this->reference_age_for_length_1.size() < 1 ||
        this->reference_age_for_length_2.size() < 1 ||
        this->length_weight_a.size() < 1 ||
        this->length_weight_b.size() < 1) {
      Rcpp::stop("VonBertalanffyGrowth parameters not set");
    }

    vb.length_at_ref_age_1 = this->length_at_ref_age_1[0].initial_value_m;
    vb.length_at_ref_age_2 = this->length_at_ref_age_2[0].initial_value_m;
    vb.growth_coefficient_K  = this->growth_coefficient_K[0].initial_value_m;
    vb.reference_age_for_length_1 = this->reference_age_for_length_1[0].initial_value_m;
    vb.reference_age_for_length_2 = this->reference_age_for_length_2[0].initial_value_m;

    vb.length_weight_a = this->length_weight_a[0].initial_value_m;
    vb.length_weight_b = this->length_weight_b[0].initial_value_m;

    return vb.evaluate(age);
  }

virtual std::string to_json() {
  std::stringstream ss;

  ss << "{\n";
  ss << " \"module_name\":\"Growth\",\n";
  ss << " \"module_type\": \"VonBertalanffy\",\n";
  ss << " \"module_id\": " << this->id << ",\n";

  ss << " \"parameters\": [\n";

  // length_at_ref_age_1
  ss << "{\n";
  ss << "   \"name\": \"length_at_ref_age_1\",\n";
  ss << "   \"id\":" << this->length_at_ref_age_1.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->length_at_ref_age_1.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->length_at_ref_age_1 << "\n";
  ss << "},\n";

  // growth_coefficient_K
  ss << "{\n";
  ss << "   \"name\": \"growth_coefficient_K\",\n";
  ss << "   \"id\":" << this->growth_coefficient_K.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->growth_coefficient_K.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->growth_coefficient_K << "\n";
  ss << "},\n";

  // length_at_ref_age_2
  ss << "{\n";
  ss << "   \"name\": \"length_at_ref_age_2\",\n";
  ss << "   \"id\":" << this->length_at_ref_age_2.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->length_at_ref_age_2.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->length_at_ref_age_2 << "\n";
  ss << "},\n";

  // reference_age_for_length_1
  ss << "{\n";
  ss << "   \"name\": \"reference_age_for_length_1\",\n";
  ss << "   \"id\":" << this->reference_age_for_length_1.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->reference_age_for_length_1.size()
     << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->reference_age_for_length_1 << "\n";
  ss << "},\n";

  // reference_age_for_length_2
  ss << "{\n";
  ss << "   \"name\": \"reference_age_for_length_2\",\n";
  ss << "   \"id\":" << this->reference_age_for_length_2.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->reference_age_for_length_2.size()
     << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->reference_age_for_length_2 << "\n";
  ss << "},\n";

  // length_weight_a
  ss << "{\n";
  ss << "   \"name\": \"length_weight_a\",\n";
  ss << "   \"id\":" << this->length_weight_a.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->length_weight_a.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->length_weight_a << "\n";
  ss << "},\n";

  // length_weight_b (last)
  ss << "{\n";
  ss << "   \"name\": \"length_weight_b\",\n";
  ss << "   \"id\":" << this->length_weight_b.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->length_weight_b.size() << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->length_weight_b << "\n";
  ss << "},\n";

  // length_at_age_sd_at_ref_ages (SD1, SDA)
  ss << "{\n";
  ss << "   \"name\": \"length_at_age_sd_at_ref_ages\",\n";
  ss << "   \"id\":" << this->length_at_age_sd_at_ref_ages.id_m << ",\n";
  ss << "   \"type\": \"vector\",\n";
  ss << "   \"dimensionality\": {\n";
  ss << "    \"header\": [null],\n";
  ss << "    \"dimensions\": [" << this->length_at_age_sd_at_ref_ages.size()
     << "]\n";
  ss << "   },\n";
  ss << "   \"values\":" << this->length_at_age_sd_at_ref_ages << "\n";
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

  // Build parameter vectors and register if estimable
  if (this->length_at_ref_age_1.size() < 1 ||
      this->length_at_ref_age_2.size() < 1 ||
      this->growth_coefficient_K.size() < 1 ||
      this->reference_age_for_length_1.size() < 1 ||
      this->reference_age_for_length_2.size() < 1 ||
      this->length_weight_a.size() < 1 ||
      this->length_weight_b.size() < 1) {
    Rcpp::stop("VonBertalanffyGrowth parameters not set");
  }
  if (this->length_at_age_sd_at_ref_ages.size() < 2) {
    Rcpp::stop("length_at_age_sd_at_ref_ages must have two values");
  }

  std::stringstream ss;
  auto load_and_register = [&](ParameterVector& pv,
                               fims::Vector<Type>& target,
                               const std::string& base_name,
                               bool log_scale) {
    target.resize(pv.size());
    for (size_t i = 0; i < pv.size(); i++) {
      double v = pv[i].initial_value_m;
      if (log_scale) {
        if (v <= 0.0) {
          Rcpp::stop((base_name + " must be > 0").c_str());
        }
        v = fims_math::log(v);
      }
      target[i] = static_cast<Type>(v);
      if (pv[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "Growth." << this->id << "." << base_name << "."
           << pv[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(target[i]);
      }
      if (pv[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "Growth." << this->id << "." << base_name << "."
           << pv[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(target[i]);
      }
    }
    info->variable_map[pv.id_m] = &target;
  };

  load_and_register(this->length_at_ref_age_1, vb->LengthAtRefAge1Vector(),
                    "length_at_ref_age_1", true);
  load_and_register(this->length_at_ref_age_2, vb->LengthAtRefAge2Vector(),
                    "length_at_ref_age_2", true);
  load_and_register(this->growth_coefficient_K, vb->GrowthCoefficientKVector(),
                    "growth_coefficient_K", true);
  load_and_register(this->reference_age_for_length_1,
                    vb->ReferenceAgeForLength1Vector(),
                    "reference_age_for_length_1", false);
  load_and_register(this->reference_age_for_length_2,
                    vb->ReferenceAgeForLength2Vector(),
                    "reference_age_for_length_2", false);
  load_and_register(this->length_weight_a, vb->LengthWeightAVector(),
                    "length_weight_a", true);
  load_and_register(this->length_weight_b, vb->LengthWeightBVector(),
                    "length_weight_b", true);
  load_and_register(this->length_at_age_sd_at_ref_ages, vb->LengthAtAgeSdAtRefAgesVector(),
                    "length_at_age_sd_at_ref_ages", true);

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
