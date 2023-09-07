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

/****************************************************************
 * Selectivity Rcpp interface                                   *
 ***************************************************************/

/**
 * @brief SelectivityInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible Selectivity function
 * */
class SelectivityInterfaceBase : public FIMSRcppInterfaceBase {
public:
    static uint32_t id_g; /**< static id of the recruitment interface base*/
    uint32_t id; /**< id of the recruitment interface base */
    static std::map<uint32_t, SelectivityInterfaceBase*>
    selectivity_objects; /**< map associating the ids of
                              SelectivityInterfaceBase to the objects */

    SelectivityInterfaceBase() {
        this->id = SelectivityInterfaceBase::id_g++;
        SelectivityInterfaceBase::selectivity_objects[this->id] = this;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
    }

    virtual ~SelectivityInterfaceBase() {
    }

    /** @brief get the ID of the interface base object
     **/
    virtual uint32_t get_id() = 0;

    /**
     * @brief evaluate the function
     *
     */
    virtual double evaluate(double x) = 0;
};

uint32_t SelectivityInterfaceBase::id_g = 1;
std::map<uint32_t, SelectivityInterfaceBase*>
SelectivityInterfaceBase::selectivity_objects;

/**
 * @brief Rcpp interface for logistic selectivity as an S4 object. To
 * instantiate from R: logistic_selectivity <- new(fims$logistic_selectivity)
 */
class LogisticSelectivityInterface : public SelectivityInterfaceBase {
public:
    Parameter median; /**< the index value at which the response reaches .5 */
    Parameter slope; /**< the width of the curve at the median */

    LogisticSelectivityInterface() : SelectivityInterfaceBase() {
    }

    virtual ~LogisticSelectivityInterface() {
    }

    /** @brief returns the id for the logistic selectivity interface */
    virtual uint32_t get_id() {
        return this->id;
    }

    /** @brief evaluate the logistic selectivity function
     *   @param x  The independent variable in the logistic function (e.g., age or
     * size in selectivity).
     */
    virtual double evaluate(double x) {
        fims::LogisticSelectivity<double> LogisticSel;

        LogisticSel.median = this->median.value;
        LogisticSel.slope = this->slope.value;
        return LogisticSel.evaluate(x);
    }

#ifdef TMB_MODEL

    template<typename T>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims::Information<T> > info =
                fims::Information<T>::GetInstance();

        std::shared_ptr<fims::LogisticSelectivity<T> > selectivity =
                std::make_shared<fims::LogisticSelectivity<T> >();

        // set relative info
        selectivity->id = this->id;
        selectivity->median = this->median.value;
        if (this->median.estimated) {
            if (this->median.is_random_effect) {
                info->RegisterRandomEffect(selectivity->median);
            } else {
                info->RegisterParameter(selectivity->median);
            }
        }
        selectivity->slope = this->slope.value;
        if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
                info->RegisterRandomEffect(selectivity->slope);
            } else {
                info->RegisterParameter(selectivity->slope);
            }
        }

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

/**
 * @brief Rcpp interface for logistic selectivity as an S4 object. To
 * instantiate from R: logistic_selectivity <- new(fims$logistic_selectivity)
 */
class DoubleLogisticSelectivityInterface : public SelectivityInterfaceBase {
public:
    Parameter median_asc; /**< the index value at which the response reaches .5 */
    Parameter slope_asc; /**< the width of the curve at the median */
    Parameter
    median_desc; /**< the index value at which the response reaches .5 */
    Parameter slope_desc; /**< the width of the curve at the median */

    DoubleLogisticSelectivityInterface() : SelectivityInterfaceBase() {
    }

    virtual ~DoubleLogisticSelectivityInterface() {
    }

    /** @brief returns the id for the double logistic selectivity interface */
    virtual uint32_t get_id() {
        return this->id;
    }

    /** @brief evaluate the double logistic selectivity function
     *   @param x  The independent variable in the logistic function (e.g., age or
     * size in selectivity).
     */
    virtual double evaluate(double x) {
        fims::DoubleLogisticSelectivity<double> DoubleLogisticSel;

        DoubleLogisticSel.median_asc = this->median_asc.value;
        DoubleLogisticSel.slope_asc = this->slope_asc.value;
        DoubleLogisticSel.median_desc = this->median_desc.value;
        DoubleLogisticSel.slope_desc = this->slope_desc.value;
        return DoubleLogisticSel.evaluate(x);
    }

#ifdef TMB_MODEL 

    template<typename Type>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims::Information<Type> > info =
                fims::Information<Type>::GetInstance();

        std::shared_ptr<fims::DoubleLogisticSelectivity<Type> > selectivity =
                std::make_shared<
                fims::DoubleLogisticSelectivity<Type> >();

        // set relative info
        selectivity->id = this->id;
        selectivity->median_asc = this->median_asc.value;
        if (this->median_asc.estimated) {
            if (this->median_asc.is_random_effect) {
                info->RegisterRandomEffect(selectivity->median_asc);
            } else {
                info->RegisterParameter(selectivity->median_asc);
            }
        }
        selectivity->slope_asc = this->slope_asc.value;
        if (this->slope_asc.estimated) {
            if (this->slope_asc.is_random_effect) {
                info->RegisterRandomEffect(selectivity->slope_asc);
            } else {
                info->RegisterParameter(selectivity->slope_asc);
            }
        }
        selectivity->median_desc = this->median_desc.value;
        if (this->median_desc.estimated) {
            if (this->median_desc.is_random_effect) {
                info->RegisterRandomEffect(selectivity->median_desc);
            } else {
                info->RegisterParameter(selectivity->median_desc);
            }
        }
        selectivity->slope_desc = this->slope_desc.value;
        if (this->slope_desc.estimated) {
            if (this->slope_desc.is_random_effect) {
                info->RegisterRandomEffect(selectivity->slope_desc);
            } else {
                info->RegisterParameter(selectivity->slope_desc);
            }
        }

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