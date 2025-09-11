/**
 * @file rcpp_depletion.hpp
 * @brief The Rcpp interface to declare different depletion options, e.g.,
 * Pella--Tomlinson. Allows for the use of methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DEPLETION_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DEPLETION_HPP

#include "../../../population_dynamics/depletion/depletion.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp depletion
 * interfaces. This type should be inherited and not called from R directly.
 */
class DepletionInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static id of the DepletionInterfaceBase object.
   */
  static uint32_t id_g;
  /**
   * @brief The local id of the DepletionInterfaceBase object.
   */
  uint32_t id;
  /**
   * @brief The map associating the IDs of DepletionInterfaceBase to the
   * objects. This is a live object, which is an object that has been created
   * and lives in memory.
   */
  static std::map<uint32_t, DepletionInterfaceBase*> live_objects;

  /**
   * @brief The constructor.
   */
  DepletionInterfaceBase() {
    this->id = DepletionInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    DepletionInterfaceBase */
    DepletionInterfaceBase::live_objects[this->id] = this;
  }

  /**
   * @brief Construct a new Depletion Interface Base object
   *
   * @param other
   */
  DepletionInterfaceBase(const DepletionInterfaceBase& other) : id(other.id) {}

  /**
   * @brief The destructor.
   */
  virtual ~DepletionInterfaceBase() {}

  /**
   * @brief Get the ID for the child depletion interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief A method for each child depletion interface object to inherit so
   * each depletion option can have an evaluate_mean() function.
   *
   * @param depletion_ym1 The depletion value from the previous time step.
   * @param catch_ym1 The observed catch from the previous time step.
   */
  virtual double evaluate_mean(double depletion_ym1, double catch_ym1) = 0;
};
// static id of the DepletionInterfaceBase object
uint32_t DepletionInterfaceBase::id_g = 1;
// local id of the DepletionInterfaceBase object map relating the ID of the
// DepletionInterfaceBase to the DepletionInterfaceBase objects
std::map<uint32_t, DepletionInterfaceBase*>
    DepletionInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Pella--Tomlinson depletion to instantiate the
 * object from R: pt_depletion <- methods::new(PTDepletion).
 */
class PellaTomlinsonInterface : public DepletionInterfaceBase {
 public:
  /**
   * @brief The intrinsic growth rate.
   */
  ParameterVector log_r;
  /**
   * @brief The population carrying capacity.
   */
  ParameterVector log_K;
  /**
   * @brief The shape parameter that adjusts the curvature of the growth
   * function.
   */
  ParameterVector log_m;
  /**
   * @brief The depletion level
   */
  ParameterVector pop_depletion;
  /**
   * @brief The logit of the depletion level
   */
  ParameterVector logit_depletion;
  /**
   * @brief The log of the expected depletion level
   */
  ParameterVector log_expected_depletion;
  /**
   * @brief The number of years.
   */
  SharedInt nyears;
  /**
   * @brief The constructor.
   */
  PellaTomlinsonInterface() : DepletionInterfaceBase() {
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        std::make_shared<PellaTomlinsonInterface>(*this));
  }

  /**
   * @brief Construct a new Pella Tomlinson Interface object
   *
   * @param other
   */
  PellaTomlinsonInterface(const PellaTomlinsonInterface& other)
      : DepletionInterfaceBase(other),
        nyears(other.nyears),
        log_r(other.log_r),
        log_K(other.log_K),
        log_m(other.log_m),
        pop_depletion(other.pop_depletion),
        logit_depletion(other.logit_depletion),
        log_expected_depletion(other.log_expected_depletion) {}

  /**
   * @brief The destructor.
   */
  virtual ~PellaTomlinsonInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Evaluate depletion using the Pella--Tomlinson function.
   * @param depletion_ym1 The depletion value from the previous time step.
   * @param catch_ym1 The observed catch from the previous time step.
   */
  virtual double evaluate_mean(double depletion_ym1, double catch_ym1) {
    fims_popdy::PellaTomlinsonDepletion<double> PTDep;
    PTDep.log_r.resize(1);
    PTDep.log_r[0] = this->log_r[0].initial_value_m;
    PTDep.log_K.resize(1);
    PTDep.log_K[0] = this->log_K[0].initial_value_m;
    PTDep.log_m.resize(1);
    PTDep.log_m[0] = this->log_m[0].initial_value_m;
    return PTDep.evaluate_mean(depletion_ym1, catch_ym1);
  }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object from
   * the Information object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Pella-Tomlinson Depletion  " +
                       fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double> > info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::depletion_models_iterator it;

    // search for depletion in Information
    it = info->depletion_models.find(this->id);
    // if not found, just return
    if (it == info->depletion_models.end()) {
      FIMS_WARNING_LOG("Pella-Tomlinson Depletion " +
                       fims::to_string(this->id) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_popdy::PellaTomlinsonDepletion<double> > dep =
          std::dynamic_pointer_cast<
              fims_popdy::PellaTomlinsonDepletion<double> >(it->second);

      for (size_t i = 0; i < log_r.size(); i++) {
        if (this->log_r[i].estimation_type_m.get() == "constant") {
          this->log_r[i].final_value_m = this->log_r[i].initial_value_m;
        } else {
          this->log_r[i].final_value_m = dep->log_r[i];
        }
      }

      for (size_t i = 0; i < log_K.size(); i++) {
        if (this->log_K[i].estimation_type_m.get() == "constant") {
          this->log_K[i].final_value_m = this->log_K[i].initial_value_m;
        } else {
          this->log_K[i].final_value_m = dep->log_K[i];
        }
      }

      for (size_t i = 0; i < log_m.size(); i++) {
        if (this->log_m[i].estimation_type_m.get() == "constant") {
          this->log_m[i].final_value_m = this->log_m[i].initial_value_m;
        } else {
          this->log_m[i].final_value_m = dep->log_m[i];
        }
      }
    }
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * depletion interface with Pella-Tomlinson depletion. It also returns the ID
   * and the parameters. This string is formatted for a json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;
    ss << "{\n";
    ss << " \"name\": \"depletion\",\n";
    ss << " \"type\": \"Pella--Tomlinson\",\n";
    ss << " \"id\": " << this->id << ",\n";

    ss << " \"parameters\": [\n{\n";
    ss << "   \"name\": \"log_r\",\n";
    ss << "   \"id\":" << this->log_r.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->log_r << "},\n";

    ss << "{\n";
    ss << "   \"name\": \"log_K\",\n";
    ss << "   \"id\":" << this->log_K.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->log_K << "}\n";

    ss << "{\n";
    ss << "   \"name\": \"log_m\",\n";
    ss << "   \"id\":" << this->log_m.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->log_m << "}\n";

    ss << "]}";

    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::PellaTomlinsonDepletion<Type> > depletion =
        std::make_shared<fims_popdy::PellaTomlinsonDepletion<Type> >();

    // set relative info
    depletion->id = this->id;
    std::stringstream ss;
    // set log_r
    depletion->log_r.resize(this->log_r.size());
    for (size_t i = 0; i < this->log_r.size(); i++) {
      depletion->log_r[i] = this->log_r[i].initial_value_m;

      if (this->log_r[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_r." << this->log_r[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion->log_r[i]);
      }
      if (this->log_r[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_r." << this->log_r[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion->log_r[i]);
      }
    }
    info->variable_map[this->log_r.id_m] = &(depletion)->log_r;

    depletion->log_K.resize(this->log_K.size());
    for (size_t i = 0; i < this->log_K.size(); i++) {
      depletion->log_K[i] = this->log_K[i].initial_value_m;
      if (this->log_K[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_K." << this->log_K[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion->log_K[i]);
      }
      if (this->log_K[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_K." << this->log_K[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion->log_K[i]);
      }
    }
    info->variable_map[this->log_K.id_m] = &(depletion)->log_K;

    depletion->log_m.resize(this->log_m.size());
    for (size_t i = 0; i < this->log_m.size(); i++) {
      depletion->log_m[i] = this->log_m[i].initial_value_m;
      if (this->log_m[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_m." << this->log_m[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion->log_m[i]);
      }
      if (this->log_m[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_m." << this->log_m[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion->log_m[i]);
      }
    }
    info->variable_map[this->log_m.id_m] = &(depletion)->log_m;

    // set logit_depletion
    depletion->logit_depletion.resize(this->nyears.get()-1);
    for (size_t i = 0; i < this->nyears.get()-1; i++) {
      depletion->logit_depletion[i] = this->logit_depletion[i].initial_value_m;

      if (this->logit_depletion[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".logit_depletion."
           << this->logit_depletion[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion->logit_depletion[i]);
      }
      if (this->logit_depletion[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".logit_depletion."
           << this->logit_depletion[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion->logit_depletion[i]);
      }
    }
    info->variable_map[this->logit_depletion.id_m] = &(depletion)->logit_depletion;

    // set depletion
    depletion->depletion.resize(this->nyears.get());
    for (size_t i = 0; i < this->nyears.get(); i++) {
      depletion->depletion[i] = 0.5;  // initial depletion value
    }
    info->variable_map[this->pop_depletion.id_m] = &(depletion)->depletion;

    // set log_expected_depletion
    depletion->log_expected_depletion.resize(this->nyears.get());
    for (size_t i = 0; i < this->nyears.get(); i++) {
      depletion->log_expected_depletion[i] = log(0.5);
    }
    info->variable_map[this->log_expected_depletion.id_m] =
        &(depletion)->log_expected_depletion;

    // add to Information
    info->depletion_models[depletion->id] = depletion;

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

#endif