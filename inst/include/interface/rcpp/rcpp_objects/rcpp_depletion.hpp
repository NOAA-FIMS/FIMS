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
  ParameterVector log_growth_rate;
  /**
   * @brief The population carrying capacity.
   */
  ParameterVector log_carrying_capacity;
  /**
   * @brief The shape parameter that adjusts the curvature of the growth
   * function.
   */
  ParameterVector log_shape;
  /**
   * @brief The depletion level
   */
  ParameterVector log_depletion;
  /**
   * @brief The natural log of the initial depletion.
   */
  ParameterVector log_init_depletion;
  /**
   * @brief The log of the expected depletion level
   */
  ParameterVector log_expected_depletion;
  /**
   * @brief The number of years.
   */
  SharedInt n_years;
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
        log_growth_rate(other.log_growth_rate),
        log_carrying_capacity(other.log_carrying_capacity),
        log_shape(other.log_shape),
        log_depletion(other.log_depletion),
        log_init_depletion(other.log_init_depletion),
        log_expected_depletion(other.log_expected_depletion),
        n_years(other.n_years) {}

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
    PTDep.growth_rate.resize(1);
    PTDep.log_growth_rate.resize(1);
    PTDep.log_growth_rate[0] = this->log_growth_rate[0].initial_value_m;
    PTDep.carrying_capacity.resize(1);
    PTDep.log_carrying_capacity.resize(1);
    PTDep.log_carrying_capacity[0] =
        this->log_carrying_capacity[0].initial_value_m;
    PTDep.shape.resize(1);
    PTDep.log_shape.resize(1);
    PTDep.log_shape[0] = this->log_shape[0].initial_value_m;

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

      for (size_t i = 0; i < log_growth_rate.size(); i++) {
        if (this->log_growth_rate[i].estimation_type_m.get() == "constant") {
          this->log_growth_rate[i].final_value_m =
              this->log_growth_rate[i].initial_value_m;
        } else {
          this->log_growth_rate[i].final_value_m = dep->log_growth_rate[i];
        }
      }

      for (size_t i = 0; i < log_carrying_capacity.size(); i++) {
        if (this->log_carrying_capacity[i].estimation_type_m.get() ==
            "constant") {
          this->log_carrying_capacity[i].final_value_m =
              this->log_carrying_capacity[i].initial_value_m;
        } else {
          this->log_carrying_capacity[i].final_value_m =
              dep->log_carrying_capacity[i];
        }
      }

      for (size_t i = 0; i < log_shape.size(); i++) {
        if (this->log_shape[i].estimation_type_m.get() == "constant") {
          this->log_shape[i].final_value_m = this->log_shape[i].initial_value_m;
        } else {
          this->log_shape[i].final_value_m = dep->log_shape[i];
        }
      }

      for (size_t i = 0; i < log_init_depletion.size(); i++) {
        if (this->log_init_depletion[i].estimation_type_m.get() == "constant") {
          this->log_init_depletion[i].final_value_m =
              this->log_init_depletion[i].initial_value_m;
        } else {
          this->log_init_depletion[i].final_value_m =
              dep->log_init_depletion[i];
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
    ss << "   \"name\": \"log_growth_rate\",\n";
    ss << "   \"id\":" << this->log_growth_rate.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->log_growth_rate << "},\n";

    ss << "{\n";
    ss << "   \"name\": \"log_carrying_capacity\",\n";
    ss << "   \"id\":" << this->log_carrying_capacity.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->log_carrying_capacity << "}\n";

    ss << "{\n";
    ss << "   \"name\": \"log_shape\",\n";
    ss << "   \"id\":" << this->log_shape.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->log_shape << "}\n";

    ss << "]}";

    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::PellaTomlinsonDepletion<Type> >
        depletion_module =
            std::make_shared<fims_popdy::PellaTomlinsonDepletion<Type> >();

    // set relative info
    depletion_module->id = this->id;
    std::stringstream ss;

    // set growth_rate
    depletion_module->growth_rate.resize(1);
    depletion_module->log_growth_rate.resize(1);
    depletion_module->log_growth_rate[0] =
        this->log_growth_rate[0].initial_value_m;

    // Register log_growth_rate as parameter
    for (size_t i = 0; i < this->log_growth_rate.size(); i++) {
      if (this->log_growth_rate[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_growth_rate."
           << this->log_growth_rate[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->log_growth_rate[i]);
      }
      if (this->log_growth_rate[i].estimation_type_m.get() ==
          "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_growth_rate."
           << this->log_growth_rate[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->log_growth_rate[i]);
      }
    }
    set_variable_map(&(depletion_module)->log_growth_rate,
                     this->log_growth_rate);

    // set carrying_capacity
    depletion_module->carrying_capacity.resize(1);
    depletion_module->log_carrying_capacity.resize(1);
    depletion_module->log_carrying_capacity[0] =
        this->log_carrying_capacity[0].initial_value_m;

    // Register log_carrying_capacity as parameter
    for (size_t i = 0; i < this->log_carrying_capacity.size(); i++) {
      if (this->log_carrying_capacity[i].estimation_type_m.get() ==
          "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_carrying_capacity."
           << this->log_carrying_capacity[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->log_carrying_capacity[i]);
      }
      if (this->log_carrying_capacity[i].estimation_type_m.get() ==
          "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_carrying_capacity."
           << this->log_carrying_capacity[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->log_carrying_capacity[i]);
      }
    }
    set_variable_map(&(depletion_module)->log_carrying_capacity,
                     this->log_carrying_capacity);

    // set shape
    depletion_module->shape.resize(1);
    depletion_module->log_shape.resize(1);
    depletion_module->log_shape[0] = this->log_shape[0].initial_value_m;

    // Register log_shape as parameter
    for (size_t i = 0; i < this->log_shape.size(); i++) {
      if (this->log_shape[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_shape."
           << this->log_shape[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->log_shape[i]);
      }
      if (this->log_shape[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << "log_shape."
           << this->log_shape[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->log_shape[i]);
      }
    }
    set_variable_map(&(depletion_module)->log_shape, this->log_shape);

    // set depletion
    depletion_module->depletion.resize(this->log_depletion.size());
    depletion_module->log_depletion.resize(this->log_depletion.size());

    // Register log_depletion as parameter
    for (size_t i = 0; i < this->log_depletion.size(); i++) {
      depletion_module->log_depletion[i] =
          this->log_depletion[i].initial_value_m;
      if (this->log_depletion[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".log_depletion."
           << this->log_depletion[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->log_depletion[i]);
      }
      if (this->log_depletion[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".log_depletion."
           << this->log_depletion[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->log_depletion[i]);
      }
    }
    set_variable_map(&(depletion_module)->log_depletion, this->log_depletion);

    // set log_init_depletion
    depletion_module->init_depletion.resize(1);
    depletion_module->log_init_depletion.resize(1);
    depletion_module->log_init_depletion[0] =
        this->log_init_depletion[0].initial_value_m;

    // Register log_init_depletion as parameter
    for (size_t i = 0; i < this->log_init_depletion.size(); i++) {
      if (this->log_init_depletion[i].estimation_type_m.get() ==
          "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".log_init_depletion."
           << this->log_init_depletion[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->log_init_depletion[i]);
      }
      if (this->log_init_depletion[i].estimation_type_m.get() ==
          "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".log_init_depletion."
           << this->log_init_depletion[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->log_init_depletion[i]);
      }
    }
    set_variable_map(&(depletion_module)->log_init_depletion,
                     this->log_init_depletion);

    // setup log_expected_depletion
    depletion_module->log_expected_depletion.resize(this->n_years.get() + 1);
    ss.str("");
    ss << "depletion." << this->id << ".log_expected_depletion";
    for (size_t i = 0; i < static_cast<size_t>(this->n_years.get() + 1); i++) {
      depletion_module->log_expected_depletion[i] = 0;
    }
    set_variable_map(&(depletion_module)->log_expected_depletion,
                     this->log_expected_depletion);

    // add to Information
    info->depletion_models[depletion_module->id] = depletion_module;

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

#endif  // FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_DEPLETION_HPP