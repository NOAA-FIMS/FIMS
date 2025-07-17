/**
 * @file rcpp_recruitment.hpp
 * @brief The Rcpp interface to declare different types of recruitment, e.g.,
 * Beverton--Holt stock--recruitment relationship. Allows for the use of
 * methods::new() in R.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_RECRUITMENT_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_RECRUITMENT_HPP

#include "../../../population_dynamics/recruitment/recruitment.hpp"
#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface that serves as the parent class for Rcpp recruitment
 * interfaces. This type should be inherited and not called from R directly.
 */
class RecruitmentInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  /**
   * @brief The static id of the RecruitmentInterfaceBase object.
   */
  static uint32_t id_g;
  /**
   * @brief The local id of the RecruitmentInterfaceBase object.
   */
  uint32_t id;
  /**
   * @brief The process id of the RecruitmentInterfaceBase object.
   */
  SharedInt process_id = -999;
  /**
   * @brief The map associating the IDs of RecruitmentInterfaceBase to the
   * objects. This is a live object, which is an object that has been created
   * and lives in memory.
   */
  static std::map<uint32_t, RecruitmentInterfaceBase*> live_objects;

  /**
   * @brief The constructor.
   */
  RecruitmentInterfaceBase() {
    this->id = RecruitmentInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    RecruitmentInterfaceBase */
    RecruitmentInterfaceBase::live_objects[this->id] = this;
  }

  /**
   * @brief Construct a new Recruitment Interface Base object
   *
   * @param other
   */
  RecruitmentInterfaceBase(const RecruitmentInterfaceBase& other)
      : id(other.id), process_id(other.process_id) {}

  /**
   * @brief The destructor.
   */
  virtual ~RecruitmentInterfaceBase() {}

  /**
   * @brief Get the ID for the child recruitment interface objects to inherit.
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief A method for each child recruitment interface object to inherit so
   * each recruitment option can have an evaluate_mean() function.
   */
  virtual double evaluate_mean(double spawners, double ssbzero) = 0;

  /**
   * @brief A method for each child recruitment process interface object to
   * inherit so each recruitment process option can have a evaluate_process()
   * function.
   */
  virtual double evaluate_process(size_t pos) = 0;
};
// static id of the RecruitmentInterfaceBase object
uint32_t RecruitmentInterfaceBase::id_g = 1;
// local id of the RecruitmentInterfaceBase object map relating the ID of the
// RecruitmentInterfaceBase to the RecruitmentInterfaceBase objects
std::map<uint32_t, RecruitmentInterfaceBase*>
    RecruitmentInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for Beverton--Holt to instantiate from R:
 * beverton_holt <- methods::new(beverton_holt).
 */
class BevertonHoltRecruitmentInterface : public RecruitmentInterfaceBase {
 public:
  /**
   * @brief The number of years.
   */
  SharedInt nyears;
  /**
   * @brief The logistic transformation of steepness (h; productivity of the
   * population), where the parameter is transformed to constrain it between
   * 0.2 and 1.0.
   */
  ParameterVector logit_steep;
  /**
   * @brief The natural log of recruitment at unfished biomass.
   */
  ParameterVector log_rzero;
  /**
   * @brief The natural log of recruitment deviations.
   */
  ParameterVector log_devs;
  /**
   * @brief The recruitment random effect parameter on the natural log scale.
   */
  ParameterVector log_r;
  /**
   * @brief Expectation of the recruitment process.
   */
  ParameterVector log_expected_recruitment;
  /**
   * @brief The estimate of the logit transformation of steepness.
   */
  fims_double estimated_logit_steep;
  /**
   * @brief The estimate of the natural log of recruitment at unfished biomass.
   */
  fims_double estimated_log_rzero;
  /**
   * @brief The estimates of the natural log of recruitment deviations.
   */
  RealVector estimated_log_devs;

  /**
   * @brief The constructor.
   */
  BevertonHoltRecruitmentInterface() : RecruitmentInterfaceBase() {
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        std::make_shared<BevertonHoltRecruitmentInterface>(*this));
  }

  /**
   * @brief Construct a new Beverton--Holt Recruitment Interface object.
   *
   * @param other The passed object to copy.
   */
  BevertonHoltRecruitmentInterface(
      const BevertonHoltRecruitmentInterface& other)
      : RecruitmentInterfaceBase(other),
        nyears(other.nyears),
        logit_steep(other.logit_steep),
        log_rzero(other.log_rzero),
        log_devs(other.log_devs),
        log_r(other.log_r),
        log_expected_recruitment(other.log_expected_recruitment),
        estimated_logit_steep(other.estimated_logit_steep),
        estimated_log_rzero(other.estimated_log_rzero),
        estimated_log_devs(other.estimated_log_devs) {}

  /**
   * @brief The destructor.
   */
  virtual ~BevertonHoltRecruitmentInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Set the unique ID for the recruitment process object.
   * @param process_id Unique ID for the recruitment process object.
   */
  void SetRecruitmentProcessID(uint32_t process_id) {
    this->process_id.set(process_id);
  }

  /**
   * @brief Evaluate recruitment using the Beverton--Holt stock--recruitment
   * relationship.
   * @param spawners Spawning biomass per time step.
   * @param ssbzero The biomass at unfished levels.
   * TODO: Change to sbzero if continuing to use acronyms.
   */
  virtual double evaluate_mean(double spawners, double ssbzero) {
    fims_popdy::SRBevertonHolt<double> BevHolt;
    BevHolt.logit_steep.resize(1);
    BevHolt.logit_steep[0] = this->logit_steep[0].initial_value_m;
    if (this->logit_steep[0].initial_value_m == 1.0) {
      warning(
          "Steepness is subject to a logit transformation. "
          "Fixing it at 1.0 is not currently possible.");
    }
    BevHolt.log_rzero.resize(1);
    BevHolt.log_rzero[0] = this->log_rzero[0].initial_value_m;

    return BevHolt.evaluate_mean(spawners, ssbzero);
  }

  /**
   * @brief Evaluate recruitment process - returns 0 in this module.
   * @param pos Position index, e.g., which year.
   */
  virtual double evaluate_process(size_t pos) { return 0; }

  /**
   * @brief Extracts derived quantities back to the Rcpp interface object from
   * the Information object.
   */
  virtual void finalize() {
    if (this->finalized) {
      // log warning that finalize has been called more than once.
      FIMS_WARNING_LOG("Beverton-Holt Recruitment  " +
                       fims::to_string(this->id) +
                       " has been finalized already.");
    }

    this->finalized = true;  // indicate this has been called already

    std::shared_ptr<fims_info::Information<double> > info =
        fims_info::Information<double>::GetInstance();

    fims_info::Information<double>::recruitment_models_iterator it;

    it = info->recruitment_models.find(this->id);

    if (it == info->recruitment_models.end()) {
      FIMS_WARNING_LOG("Beverton-Holt Recruitment " +
                       fims::to_string(this->id) +
                       " not found in Information.");
      return;
    } else {
      std::shared_ptr<fims_popdy::SRBevertonHolt<double> > recr =
          std::dynamic_pointer_cast<fims_popdy::SRBevertonHolt<double> >(
              it->second);

      for (size_t i = 0; i < this->logit_steep.size(); i++) {
        if (this->logit_steep[i].estimation_type_m.get() == "constant") {
          this->logit_steep[i].final_value_m =
              this->logit_steep[i].initial_value_m;
        } else {
          this->logit_steep[i].final_value_m = recr->logit_steep[i];
        }
      }

      for (size_t i = 0; i < log_rzero.size(); i++) {
        if (log_rzero[i].estimation_type_m.get() == "constant") {
          this->log_rzero[i].final_value_m = this->log_rzero[i].initial_value_m;
        } else {
          this->log_rzero[i].final_value_m = recr->log_rzero[i];
        }
      }

      for (R_xlen_t i = 0; i < this->log_devs.size(); i++) {
        if (this->log_devs[i].estimation_type_m.get() == "constant") {
          this->log_devs[i].final_value_m = this->log_devs[i].initial_value_m;
        } else {
          this->log_devs[i].final_value_m = recr->log_recruit_devs[i];
        }
      }

      for (R_xlen_t i = 0; i < this->log_r.size(); i++) {
        if (this->log_r[i].estimation_type_m.get() == "constant") {
          this->log_r[i].final_value_m = this->log_r[i].initial_value_m;
        } else {
          this->log_r[i].final_value_m = recr->log_r[i];
        }
      }
    }
  }

  /**
   * @brief Converts the data to json representation for the output.
   * @return A string is returned specifying that the module relates to the
   * recruitment interface with Beverton--Holt stock--recruitment relationship.
   * It also returns the ID and the parameters. This string is formatted for a
   * json file.
   */
  virtual std::string to_json() {
    std::stringstream ss;

    ss << "{\n";
    ss << " \"name\": \"recruitment\",\n";
    ss << " \"type\": \"Beverton--Holt\",\n";
    ss << " \"id\": " << this->id << ",\n";

    ss << " \"parameters\": [\n{\n";
    ss << "  \"name\": \"logit_steep\",\n";
    ss << "  \"id\":" << this->logit_steep.id_m << ",\n";
    ss << "  \"type\": \"vector\",\n";
    ss << "  \"values\":" << this->logit_steep << "\n},\n";

    ss << "{\n";
    ss << "   \"name\": \"log_rzero\",\n";
    ss << "   \"id\":" << this->log_rzero.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->log_rzero << "\n },\n";

    ss << "{\n";
    ss << "   \"name\": \"log_devs\",\n";
    ss << "   \"id\":" << this->log_devs.id_m << ",\n";
    ss << "   \"type\": \"vector\",\n";
    ss << "   \"values\":" << this->log_devs << "\n }]\n}";

    return ss.str();
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::SRBevertonHolt<Type> > recruitment =
        std::make_shared<fims_popdy::SRBevertonHolt<Type> >();

    std::stringstream ss;

    // set relative info
    recruitment->id = this->id;
    recruitment->process_id = this->process_id.get();
    // set logit_steep
    recruitment->logit_steep.resize(this->logit_steep.size());
    for (size_t i = 0; i < this->logit_steep.size(); i++) {
      recruitment->logit_steep[i] = this->logit_steep[i].initial_value_m;

      if (this->logit_steep[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "recruitment." << this->id << ".logit_steep."
           << this->logit_steep[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(recruitment->logit_steep[i]);
      }
      if (this->logit_steep[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "recruitment." << this->id << ".logit_steep."
           << this->logit_steep[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(recruitment->logit_steep[i]);
      }
    }
    info->variable_map[this->logit_steep.id_m] = &(recruitment)->logit_steep;

    // set log_rzero
    recruitment->log_rzero.resize(this->log_rzero.size());
    for (size_t i = 0; i < this->log_rzero.size(); i++) {
      recruitment->log_rzero[i] = this->log_rzero[i].initial_value_m;

      if (this->log_rzero[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "recruitment." << this->id << ".log_rzero."
           << this->log_rzero[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(recruitment->log_rzero[i]);
      }
      if (this->log_rzero[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "recruitment." << this->id << ".log_rzero."
           << this->log_rzero[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(recruitment->log_rzero[i]);
      }
    }
    info->variable_map[this->log_rzero.id_m] = &(recruitment)->log_rzero;
    // set log_recruit_devs
    recruitment->log_recruit_devs.resize(this->log_devs.size());
    for (size_t i = 0; i < this->log_devs.size(); i++) {
      recruitment->log_recruit_devs[i] = this->log_devs[i].initial_value_m;

      if (this->log_devs[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "recruitment." << this->id << ".log_devs."
           << this->log_devs[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(recruitment->log_recruit_devs[i]);
      }
      if (this->log_devs[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "recruitment." << this->id << ".log_devs."
           << this->log_devs[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(recruitment->log_recruit_devs[i]);
      }
    }

    info->variable_map[this->log_devs.id_m] = &(recruitment)->log_recruit_devs;

    // set log_r
    recruitment->log_r.resize(this->log_r.size());
    for (size_t i = 0; i < log_r.size(); i++) {
      recruitment->log_r[i] = this->log_r[i].initial_value_m;

      if (this->log_r[i].estimation_type_m.get() == "fixed_effects") {
        ss.str("");
        ss << "recruitment." << this->id << ".log_r." << this->log_r[i].id_m;
        info->RegisterParameterName(ss.str());
        info->RegisterParameter(recruitment->log_r[i]);
      }
      if (this->log_r[i].estimation_type_m.get() == "random_effects") {
        ss.str("");
        ss << "recruitment." << this->id << ".log_r." << this->log_r[i].id_m;
        info->RegisterRandomEffectName(ss.str());
        info->RegisterRandomEffect(recruitment->log_r[i]);
      }
    }

    info->variable_map[this->log_r.id_m] = &(recruitment)->log_r;
    // set log_expected_recruitment
    recruitment->log_expected_recruitment.resize(this->nyears.get() + 1);
    for (size_t i = 0; i < this->nyears.get() + 1; i++) {
      recruitment->log_expected_recruitment[i] = 0;
    }
    info->variable_map[this->log_expected_recruitment.id_m] =
        &(recruitment)->log_expected_recruitment;

    // add to Information
    info->recruitment_models[recruitment->id] = recruitment;

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
 * @brief Rcpp interface for Log--Devs to instantiate from R:
 * log_devs <- methods::new(log_devs).
 */
class LogDevsRecruitmentInterface : public RecruitmentInterfaceBase {
 public:
  /**
   * @brief The constructor.
   */
  LogDevsRecruitmentInterface() : RecruitmentInterfaceBase() {
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        std::make_shared<LogDevsRecruitmentInterface>(*this));
  }

  /**
   * @brief The destructor.
   */
  virtual ~LogDevsRecruitmentInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Evaluate mean - returns empty function for this module.
   * @param spawners Spawning biomass per time step.
   * @param ssbzero The biomass at unfished levels.
   */
  virtual double evaluate_mean(double spawners, double ssbzero) { return 0; }

  /**
   * @brief Evaluate recruitment process using the Log--Devs approach.
   * @param pos Position index, e.g., which year.
   */
  virtual double evaluate_process(size_t pos) {
    fims_popdy::LogDevs<double> LogDevs;

    return LogDevs.evaluate_process(pos);
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::LogDevs<Type> > recruitment_process =
        std::make_shared<fims_popdy::LogDevs<Type> >();

    recruitment_process->id = this->id;

    // add to Information
    info->recruitment_process_models[recruitment_process->id] =
        recruitment_process;

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
 * @brief Rcpp interface for Log--R to instantiate from R:
 * log_r <- methods::new(log_r).
 */
class LogRRecruitmentInterface : public RecruitmentInterfaceBase {
 public:
  /**
   * @brief The constructor.
   */
  LogRRecruitmentInterface() : RecruitmentInterfaceBase() {
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(
        std::make_shared<LogRRecruitmentInterface>(*this));
  }

  /**
   * @brief The destructor.
   */
  virtual ~LogRRecruitmentInterface() {}

  /**
   * @brief Gets the ID of the interface base object.
   * @return The ID.
   */
  virtual uint32_t get_id() { return this->id; }

  /**
   * @brief Evaluate mean - returns empty function for this module.
   * @param spawners Spawning biomass per time step.
   * @param ssbzero The biomass at unfished levels.
   */
  virtual double evaluate_mean(double spawners, double ssbzero) { return 0; }

  /**
   * @brief Evaluate recruitment process using the Log--R approach.
   * @param pos Position index, e.g., which year.
   */
  virtual double evaluate_process(size_t pos) {
    fims_popdy::LogR<double> LogR;

    return LogR.evaluate_process(pos);
  }

#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::LogR<Type> > recruitment_process =
        std::make_shared<fims_popdy::LogR<Type> >();

    recruitment_process->id = this->id;

    // add to Information
    info->recruitment_process_models[recruitment_process->id] =
        recruitment_process;

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
