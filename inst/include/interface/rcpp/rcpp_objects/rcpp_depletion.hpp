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
  ParameterVector growth_rate_input;
  /**
   * @brief The population carrying capacity.
   */
  ParameterVector carrying_capacity_input;
  /**
   * @brief The shape parameter that adjusts the curvature of the growth
   * function.
   */
  ParameterVector shape_input;
  /**
   * @brief The depletion level
   */
  ParameterVector depletion_input;
  /**
   * @brief The natural log of the initial depletion.
   */
  ParameterVector init_depletion_input;
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
        growth_rate_input(other.growth_rate_input),
        carrying_capacity_input(other.carrying_capacity_input),
        shape_input(other.shape_input),
        depletion_input(other.depletion_input),
        init_depletion_input(other.init_depletion_input),
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
    PTDep.growth_rate_input.resize(1);
    PTDep.growth_rate_input[0] = this->growth_rate_input[0].initial_value_m;
    PTDep.growth_rate_input.get_transformation() =
      *(this->growth_rate_input.transformation_m);
    PTDep.carrying_capacity.resize(1);
    PTDep.carrying_capacity_input.resize(1);
    PTDep.carrying_capacity_input[0] = 
      this->carrying_capacity_input[0].initial_value_m;
    PTDep.carrying_capacity_input.get_transformation() =
      *(this->carrying_capacity_input.transformation_m);
    PTDep.shape.resize(1);
    PTDep.shape_input.resize(1);
    PTDep.shape_input[0] = this->shape_input[0].initial_value_m;
    PTDep.shape_input.get_transformation() =
      *(this->shape_input.transformation_m);

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

      for (size_t i = 0; i < growth_rate_input.size(); i++) {
        if (this->growth_rate_input[i].estimation_type_m.get() == "constant") {
          this->growth_rate_input[i].final_value_m = 
            this->growth_rate_input[i].initial_value_m;
        } else {
          this->growth_rate_input[i].final_value_m = dep->growth_rate_input[i];
        }
      }

      for (size_t i = 0; i < carrying_capacity_input.size(); i++) {
        if (this->carrying_capacity_input[i].estimation_type_m.get() == "constant") {
          this->carrying_capacity_input[i].final_value_m = 
            this->carrying_capacity_input[i].initial_value_m;
        } else {
          this->carrying_capacity_input[i].final_value_m = 
            dep->carrying_capacity_input[i];
        }
      }

      for (size_t i = 0; i < shape_input.size(); i++) {
        if (this->shape_input[i].estimation_type_m.get() == "constant") {
          this->shape_input[i].final_value_m = this->shape_input[i].initial_value_m;
        } else {
          this->shape_input[i].final_value_m = dep->shape_input[i];
        }
      }

      for (size_t i = 0; i < init_depletion_input.size(); i++) {
        if (this->init_depletion_input[i].estimation_type_m.get() == "constant") {
          this->init_depletion_input[i].final_value_m = this->init_depletion_input[i].initial_value_m;
        } else {
          this->init_depletion_input[i].final_value_m = dep->init_depletion_input[i];
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
    ss << "   \"name\": \"growth_rate_input\",\n";
    ss << "   \"id\":" << this->growth_rate_input.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->growth_rate_input << "},\n";

    ss << "{\n";
    ss << "   \"name\": \"carrying_capacity_input\",\n";
    ss << "   \"id\":" << this->carrying_capacity_input.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->carrying_capacity_input << "}\n";

    ss << "{\n";
    ss << "   \"name\": \"shape_input\",\n";
    ss << "   \"id\":" << this->shape_input.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->shape_input << "}\n";

    ss << "]}";

    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::PellaTomlinsonDepletion<Type> > depletion_module =
        std::make_shared<fims_popdy::PellaTomlinsonDepletion<Type> >();

    // set relative info
    depletion_module->id = this->id;
    std::stringstream ss;

    // set growth_rate
    depletion_module->growth_rate_input.get_transformation() =
      *(this->growth_rate_input.transformation_m);
    depletion_module->growth_rate.resize(1);
    depletion_module->growth_rate_input.resize(1);
    depletion_module->growth_rate_input[0] = this->growth_rate_input[0].initial_value_m;
  
    // Register growth_rate_input as parameter
    for (size_t i = 0; i < this->growth_rate_input.size(); i++) {
      if (this->growth_rate_input[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << "growth_rate_input." << 
          this->growth_rate_input[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->growth_rate_input[i]);
        info->RegisterParameterBounds(this->growth_rate_input[i].min_m,
                                     this->growth_rate_input[i].max_m);
      }
      if (this->growth_rate_input[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << "growth_rate_input." << 
          this->growth_rate_input[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->growth_rate_input[i]);
        info->RegisterRandomEffectBounds(this->growth_rate_input[i].min_m,
                                         this->growth_rate_input[i].max_m);
      }
    }
    info->variable_map[this->growth_rate_input.id_m] = &(depletion_module)->growth_rate_input;

    // set carrying_capacity
    depletion_module->carrying_capacity_input.get_transformation() =
      *(this->carrying_capacity_input.transformation_m);
    depletion_module->carrying_capacity.resize(1);
    depletion_module->carrying_capacity_input.resize(1);
    depletion_module->carrying_capacity_input[0] = this->carrying_capacity_input[0].initial_value_m;
  
    // Register carrying_capacity as parameter
    for (size_t i = 0; i < this->carrying_capacity_input.size(); i++) {
      if (this->carrying_capacity_input[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << "carrying_capacity_input." << 
          this->carrying_capacity_input[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->carrying_capacity_input[i]);
        info->RegisterParameterBounds(this->carrying_capacity_input[i].min_m,
                                      this->carrying_capacity_input[i].max_m);
      }
      if (this->carrying_capacity_input[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << "carrying_capacity_input." << 
          this->carrying_capacity_input[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->carrying_capacity_input[i]);
        info->RegisterRandomEffectBounds(this->carrying_capacity_input[i].min_m,
                                         this->carrying_capacity_input[i].max_m);
      }
    }
    info->variable_map[this->carrying_capacity_input.id_m] = &(depletion_module)->carrying_capacity_input;

    // set shape
    depletion_module->shape_input.get_transformation() =
      *(this->shape_input.transformation_m);
    depletion_module->shape.resize(1);
    depletion_module->shape_input.resize(1);
    depletion_module->shape_input[0] = this->shape_input[0].initial_value_m;

    // Register shape as parameter
    for (size_t i = 0; i < this->shape_input.size(); i++) {
      if (this->shape_input[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << "shape." << this->shape_input[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->shape_input[i]);
        info->RegisterParameterBounds(this->shape_input[i].min_m,
                                      this->shape_input[i].max_m);
      }
      if (this->shape_input[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << "shape_input." << this->shape_input[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->shape_input[i]);
        info->RegisterRandomEffectBounds(this->shape_input[i].min_m,
                                         this->shape_input[i].max_m);
      }
    }
    info->variable_map[this->shape_input.id_m] = &(depletion_module)->shape_input;

    // set depletion
    depletion_module->depletion_input.get_transformation() =
      *(this->depletion_input.transformation_m);
    depletion_module->depletion.resize(this->depletion_input.size());
    depletion_module->depletion_input.resize(this->depletion_input.size());

     // Register depletion as parameter
    for (size_t i = 0; i < this->depletion_input.size(); i++) {
      depletion_module->depletion_input[i] = this->depletion_input[i].initial_value_m;
      if (this->depletion_input[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".depletion_input."
          << this->depletion_input[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->depletion_input[i]);
        info->RegisterParameterBounds(this->depletion_input[i].min_m,
                                      this->depletion_input[i].max_m);
      }
      if (this->depletion_input[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".depletion_input."
          << this->depletion_input[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->depletion_input[i]);
        info->RegisterRandomEffectBounds(this->depletion_input[i].min_m,
                                         this->depletion_input[i].max_m);
      }
    }
    info->variable_map[this->depletion_input.id_m] = &(depletion_module)->depletion_input;


     // set init_depletion
    depletion_module->init_depletion_input.get_transformation() =
      *(this->init_depletion_input.transformation_m);
    depletion_module->init_depletion.resize(1);
    depletion_module->init_depletion_input.resize(1);
    depletion_module->init_depletion_input[0] = this->init_depletion_input[0].initial_value_m;
    
    // Register init_depletion_input as parameter
    for (size_t i = 0; i < this->init_depletion_input.size(); i++) {
      if (this->init_depletion_input[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".init_depletion_input."
            << this->init_depletion_input[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(depletion_module->init_depletion_input[i]);
        info->RegisterParameterBounds(this->init_depletion_input[i].min_m,
                                      this->init_depletion_input[i].max_m);
      }
      if (this->init_depletion_input[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "depletion." << this->id << ".init_depletion_input."
            << this->init_depletion_input[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(depletion_module->init_depletion_input[i]);
        info->RegisterRandomEffectBounds(this->init_depletion_input[i].min_m,
                                          this->init_depletion_input[i].max_m);
      }
    }
    
    info->variable_map[this->init_depletion_input.id_m] = &(depletion_module)->init_depletion_input;

    // setup log_expected_depletion
    depletion_module->log_expected_depletion.resize(this->n_years.get() + 1);
    ss.str("");
    ss << "depletion." << this->id << ".log_expected_depletion";
    for (size_t i = 0; i < static_cast<size_t>(this->n_years.get() + 1); i++) {
      depletion_module->log_expected_depletion[i] = 0;
    }
    info->variable_map[this->log_expected_depletion.id_m] =
        &(depletion_module)->log_expected_depletion;

  
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