/*
 * File:   rcpp_selectivity.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE
 * file for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_SELECTIVITY_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_SELECTIVITY_HPP

#include "../../../population_dynamics/selectivity/selectivity.hpp"
#include "rcpp_interface_base.hpp"

/**
 * Selectivity Rcpp interface
 */

/**
 * @brief SelectivityInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible Selectivity function
 */
class SelectivityInterfaceBase : public FIMSRcppInterfaceBase {
 public:
  static uint32_t id_g; /**< static id of the recruitment interface base*/
  uint32_t id;          /**< id of the recruitment interface base */
  // live objects in C++ are objects that have been created and live in memory
  static std::map<uint32_t, SelectivityInterfaceBase*>
      live_objects; /**< map associating the ids of
                                SelectivityInterfaceBase to the objects */

  SelectivityInterfaceBase() {
    this->id = SelectivityInterfaceBase::id_g++;
    /* Create instance of map: key is id and value is pointer to
    SelectivityInterfaceBase */
    SelectivityInterfaceBase::live_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~SelectivityInterfaceBase() {}

  /** @brief get the ID of the interface base object
   */
  virtual uint32_t get_id() = 0;

  /**
   * @brief evaluate the function
   *
   */
  virtual double evaluate(double x) = 0;
};

uint32_t SelectivityInterfaceBase::id_g = 1;
std::map<uint32_t, SelectivityInterfaceBase*>
    SelectivityInterfaceBase::live_objects;

/**
 * @brief Rcpp interface for logistic selectivity as an S4 object. To
 * instantiate from R: logistic_selectivity <- new(logistic_selectivity)
 */
class LogisticSelectivityInterface : public SelectivityInterfaceBase {
 public:
  ParameterVector
      inflection_point; /**< the index value at which the response reaches .5 */
  ParameterVector slope; /**< the width of the curve at the inflection_point */

  LogisticSelectivityInterface() : SelectivityInterfaceBase() {}

  virtual ~LogisticSelectivityInterface() {}

  /** @brief returns the id for the logistic selectivity interface */
  virtual uint32_t get_id() { return this->id; }

  /** @brief evaluate the logistic selectivity function
   *   @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual double evaluate(double x) {
    fims_popdy::LogisticSelectivity<double> LogisticSel;
    LogisticSel.inflection_point.resize(1);
    LogisticSel.inflection_point[0] = this->inflection_point[0].initial_value_m;
    LogisticSel.slope.resize(1);
    LogisticSel.slope[0] = this->slope[0].initial_value_m;
    return LogisticSel.evaluate(x);
  }
    /** 
     * @brief finalize function. Extracts derived quantities back to 
     * the Rcpp interface object from the Information object. 
     */
    virtual void finalize() {

        if (this->finalized) {
            //log warning that finalize has been called more than once.
            FIMS_WARNING_LOG("Logistic Selectivity  " + fims::to_string(this->id) + " has been finalized already.");
        }

        this->finalized = true; //indicate this has been called already

        std::shared_ptr<fims_info::Information<double> > info =
                fims_info::Information<double>::GetInstance();


        fims_info::Information<double>::selectivity_models_iterator it;


        //search for maturity in Information
        it = info->selectivity_models.find(this->id);
        //if not found, just return
        if (it == info->selectivity_models.end()) {
            FIMS_WARNING_LOG("Logistic Selectivity " + fims::to_string(this->id) + " not found in Information.");
            return;
        } else {
            std::shared_ptr<fims_popdy::LogisticSelectivity<double> > sel =
                    std::dynamic_pointer_cast<fims_popdy::LogisticSelectivity<double> >(it->second);

            for (size_t i = 0; i < inflection_point.size(); i++) {
                if (this->inflection_point[i].estimated_m) {
                    this->inflection_point[i].final_value_m = sel->inflection_point[i];
                } else {
                    this->inflection_point[i].final_value_m = this->inflection_point[i].initial_value_m;
                }
            }

            for (size_t i = 0; i < slope.size(); i++) {
                if (this->slope[i].estimated_m) {
                    this->slope[i].final_value_m = sel->slope[i];
                } else {
                    this->slope[i].final_value_m = this->slope[i].initial_value_m;
                }

            }

        }
    }

    /**
     * @brief Convert the data to json representation for the output.
     */
    virtual std::string to_json() {
        std::stringstream ss;

        ss << "\"module\" : {\n";
        ss << " \"name\":\"selectivity\",\n";
        ss << " \"type\": \"Logistic\",\n";
        ss << " \"id\": " << this->id << ",\n";

        ss << " \"parameter\": {\n";
        ss << "   \"name\": \"inflection_point\",\n";
        ss << "   \"id\":" << this->inflection_point.id_m << ",\n";
        ss << "   \"type\": \"vector\",\n";
        ss << "   \"values\":" << this->inflection_point << ",\n },\n";

        ss << " \"parameter\": {\n";
        ss << "   \"name\": \"slope\",\n";
        ss << "   \"id\":" << this->slope.id_m << ",\n";
        ss << "   \"type\": \"vector\",\n";
        ss << "   \"values\":" << this->slope << ",\n}\n";


        ss << "}";

        return ss.str();
    }


#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::LogisticSelectivity<Type> > selectivity =
        std::make_shared<fims_popdy::LogisticSelectivity<Type> >();
    std::stringstream ss;
    // set relative info
    selectivity->id = this->id;
        selectivity->inflection_point.resize(this->inflection_point.size());
        for (size_t i = 0; i < this->inflection_point.size(); i++) {
            selectivity->inflection_point[i] = this->inflection_point[i].initial_value_m;
            if (this->inflection_point[i].estimated_m) {
                ss.str("");
                ss << "selectivity.inflection_point ." << this->id << "." << i;
                info->RegisterParameterName(ss.str());
                if (this->inflection_point[i].is_random_effect_m) {
                    info->RegisterRandomEffect(selectivity->inflection_point[i]);
                } else {
                    info->RegisterParameter(selectivity->inflection_point[i]);
                }
            }
        }

    info->variable_map[this->inflection_point.id_m] = &(selectivity)->inflection_point;
        selectivity->slope.resize(this->slope.size());
        for (size_t i = 0; i < this->slope.size(); i++) {
            selectivity->slope[i] = this->slope[i].initial_value_m;
            if (this->slope[i].estimated_m) {
                ss.str("");
                ss << "selectivity.slope." << this->id << "." << i;
                info->RegisterParameterName(ss.str());
                if (this->slope[i].is_random_effect_m) {
                    info->RegisterRandomEffect(selectivity->slope[i]);
                } else {
                    info->RegisterParameter(selectivity->slope[i]);
                }
            }
        }
    info->variable_map[this->slope.id_m] = &(selectivity)->slope;

    // add to Information
    info->selectivity_models[selectivity->id] = selectivity;

    return true;
  }

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    FIMS_INFO_LOG("adding Selectivity object to TMB");
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

/**
 * @brief Rcpp interface for logistic selectivity as an S4 object. To
 * instantiate from R: logistic_selectivity <- new(logistic_selectivity)
 */
class DoubleLogisticSelectivityInterface : public SelectivityInterfaceBase {
 public:
  ParameterVector inflection_point_asc; /**< the index value at which the response
                                     reaches .5 */
  ParameterVector slope_asc; /**< the width of the curve at the inflection_point */
  ParameterVector inflection_point_desc; /**< the index value at which the response
                                      reaches .5 */
  ParameterVector slope_desc; /**< the width of the curve at the inflection_point */


  DoubleLogisticSelectivityInterface() : SelectivityInterfaceBase() {}

  virtual ~DoubleLogisticSelectivityInterface() {}

  /** @brief returns the id for the double logistic selectivity interface */
  virtual uint32_t get_id() { return this->id; }

  /** @brief evaluate the double logistic selectivity function
   *   @param x  The independent variable in the logistic function (e.g., age or
   * size in selectivity).
   */
  virtual double evaluate(double x) {
    fims_popdy::DoubleLogisticSelectivity<double> DoubleLogisticSel;
    DoubleLogisticSel.inflection_point_asc.resize(1);
    DoubleLogisticSel.inflection_point_asc[0] = this->inflection_point_asc[0].initial_value_m;
    DoubleLogisticSel.slope_asc.resize(1);
    DoubleLogisticSel.slope_asc[0] = this->slope_asc[0].initial_value_m;
    DoubleLogisticSel.inflection_point_desc.resize(1);
    DoubleLogisticSel.inflection_point_desc[0] =
        this->inflection_point_desc[0].initial_value_m;
    DoubleLogisticSel.slope_desc.resize(1);
    DoubleLogisticSel.slope_desc[0] = this->slope_desc[0].initial_value_m;
    return DoubleLogisticSel.evaluate(x);
  }
    /** 
     * @brief finalize function. Extracts derived quantities back to 
     * the Rcpp interface object from the Information object. 
     */
    virtual void finalize() {

        if (this->finalized) {
            //log warning that finalize has been called more than once.
            FIMS_WARNING_LOG("Double Logistic Selectivity  " + fims::to_string(this->id) + " has been finalized already.");
        }

        this->finalized = true; //indicate this has been called already

        std::shared_ptr<fims_info::Information<double> > info =
                fims_info::Information<double>::GetInstance();


   
        fims_info::Information<double>::selectivity_models_iterator it;

        //search for maturity in Information
        it = info->selectivity_models.find(this->id);
        //if not found, just return
        if (it == info->selectivity_models.end()) {
            FIMS_WARNING_LOG("Double Logistic Selectivity " + fims::to_string(this->id) + " not found in Information.");
            return;
        } else {
            std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<double> > sel =
                    std::dynamic_pointer_cast<fims_popdy::DoubleLogisticSelectivity<double> >(it->second);


            for (size_t i = 0; i < inflection_point_asc.size(); i++) {
                if (this->inflection_point_asc[i].estimated_m) {
                    this->inflection_point_asc[i].final_value_m = sel->inflection_point_asc[i];
                } else {
                    this->inflection_point_asc[i].final_value_m = this->inflection_point_asc[i].initial_value_m;
                }

            }

            for (size_t i = 0; i < slope_asc.size(); i++) {
                if (this->slope_asc[i].estimated_m) {
                    this->slope_asc[i].final_value_m = sel->slope_asc[i];
                } else {
                    this->slope_asc[i].final_value_m = this->slope_asc[i].initial_value_m;
                }

            }

            for (size_t i = 0; i < inflection_point_desc.size(); i++) {
                if (this->inflection_point_desc[i].estimated_m) {
                    this->inflection_point_desc[i].final_value_m = sel->inflection_point_desc[i];
                } else {
                    this->inflection_point_desc[i].final_value_m = this->inflection_point_desc[i].initial_value_m;
                }

            }

            for (size_t i = 0; i < slope_desc.size(); i++) {
                if (this->slope_desc[i].estimated_m) {
                    this->slope_desc[i].final_value_m = sel->slope_desc[i];
                } else {
                    this->slope_desc[i].final_value_m = this->slope_desc[i].initial_value_m;
                }

            }



        }
    }

    /**
     * @brief Convert the data to json representation for the output.
     */
    virtual std::string to_json() {
        std::stringstream ss;
        
                ss << "\"module\" : {\n";
                ss << " \"name\": \"selectivity\",\n";
                ss << " \"type\": \"DoubleLogistic\",\n";
                ss << " \"id\": " << this->id << ",\n";
        
                ss << " \"parameter\": {\n";
                ss << "   \"name\": \"inflection_point_asc\",\n";
                ss << "   \"id\":" << this->inflection_point_asc.id_m << ",\n";
                ss << "   \"type\": \"vector\",\n";
                ss << "   \"values\":" << this->inflection_point_asc << ",\n},\n";
        
                ss << " \"parameter\": {\n";
                ss << "   \"name\": \"slope_asc\",\n";
                ss << "   \"id\":" << this->slope_asc.id_m << ",\n";
                ss << "   \"type\": \"vector\",\n";
                ss << "   \"values\":" << this->slope_asc << ",\n},\n";
        
                ss << " \"parameter\": {\n";
                ss << "   \"name\": \"inflection_point_desc\",\n";
                ss << "   \"id\":" << this->inflection_point_desc.id_m << ",\n";
                ss << "   \"type\": \"vector\",\n";
                ss << "   \"values\":" << this->inflection_point_desc << ",\n},\n";
        
                ss << " \"parameter\": {\n";
                ss << "   \"name\": \"slope_desc\",\n";
                ss << "   \"id\":" << this->slope_desc.id_m << ",\n";
                ss << "   \"type\": \"vector\",\n";
                ss << "   \"values\":" << this->slope_desc << ",\n}\n";
        
        
                ss << "}";

        return ss.str();
    }


#ifdef TMB_MODEL

  template <typename Type>
  bool add_to_fims_tmb_internal() {
    std::shared_ptr<fims_info::Information<Type> > info =
        fims_info::Information<Type>::GetInstance();

    std::shared_ptr<fims_popdy::DoubleLogisticSelectivity<Type> > selectivity =
        std::make_shared<fims_popdy::DoubleLogisticSelectivity<Type> >();

        std::stringstream ss;
        // set relative info
        selectivity->id = this->id;
        selectivity->inflection_point_asc.resize(this->inflection_point_asc.size());
        for (size_t i = 0; i < this->inflection_point_asc.size(); i++) {
            selectivity->inflection_point_asc[i] = this->inflection_point_asc[i].initial_value_m;
            if (this->inflection_point_asc[i].estimated_m) {
                ss.str("");
                ss << "selectivity.inflection_point_asc." << this->id << "." << i;
                info->RegisterParameterName(ss.str());
                if (this->inflection_point_asc[i].is_random_effect_m) {
                    info->RegisterRandomEffect(selectivity->inflection_point_asc[i]);
                } else {
                    info->RegisterParameter(selectivity->inflection_point_asc[i]);
                }
            }
        }

    info->variable_map[this->inflection_point_asc.id_m] = &(selectivity)->inflection_point_asc;


        selectivity->slope_asc.resize(this->slope_asc.size());
        for (size_t i = 0; i < this->slope_asc.size(); i++) {
            selectivity->slope_asc[i] = this->slope_asc[i].initial_value_m;
            if (this->slope_asc[i].estimated_m) {
                ss.str("");
                ss << "selectivity.slope_asc." << this->id << "." << i;
                info->RegisterParameterName(ss.str());
                if (this->slope_asc[i].is_random_effect_m) {
                    info->RegisterRandomEffect(selectivity->slope_asc[i]);
                } else {
                    info->RegisterParameter(selectivity->slope_asc[i]);
                }
            }
        }
    info->variable_map[this->slope_asc.id_m] = &(selectivity)->slope_asc;

        selectivity->inflection_point_desc.resize(this->inflection_point_desc.size());
        for (size_t i = 0; i < this->inflection_point_desc.size(); i++) {
            selectivity->inflection_point_desc[i] = this->inflection_point_desc[i].initial_value_m;
            if (this->inflection_point_desc[i].estimated_m) {
                ss.str("");
                ss << "selectivity.inflection_point_desc." << this->id << "." << i;
                info->RegisterParameterName(ss.str());
                if (this->inflection_point_desc[i].is_random_effect_m) {
                    info->RegisterRandomEffect(selectivity->inflection_point_desc[i]);
                } else {
                    info->RegisterParameter(selectivity->inflection_point_desc[i]);
                }
            }
        }

    info->variable_map[this->inflection_point_desc.id_m] = &(selectivity)->inflection_point_desc;
        selectivity->slope_desc.resize(this->slope_desc.size());
        for (size_t i = 0; i < this->slope_desc.size(); i++) {
            selectivity->slope_desc[i] = this->slope_desc[i].initial_value_m;
            if (this->slope_asc[i].estimated_m) {
                ss.str("");
                ss << "selectivity.slope_desc." << this->id << "." << i;
                info->RegisterParameterName(ss.str());
                if (this->slope_desc[i].is_random_effect_m) {
                    info->RegisterRandomEffect(selectivity->slope_desc[i]);
                } else {
                    info->RegisterParameter(selectivity->slope_desc[i]);
                }
            }
        }


    info->variable_map[this->slope_desc.id_m] = &(selectivity)->slope_desc;

    // add to Information
    info->selectivity_models[selectivity->id] = selectivity;

    return true;
  }

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    this->add_to_fims_tmb_internal<TMB_FIMS_REAL_TYPE>();
    this->add_to_fims_tmb_internal<TMB_FIMS_FIRST_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_SECOND_ORDER>();
    this->add_to_fims_tmb_internal<TMB_FIMS_THIRD_ORDER>();

    return true;
  }

#endif
};

#endif