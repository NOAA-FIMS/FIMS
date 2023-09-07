/*
 * File:   rcpp_maturity.hpp
 *
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE file
 * for reuse information.
 *
 */
#ifndef FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MATURITY_HPP
#define FIMS_INTERFACE_RCPP_RCPP_OBJECTS_RCPP_MATURITY_HPP

#include "../../../population_dynamics/maturity/maturity.hpp"
#include "rcpp_interface_base.hpp"

/****************************************************************
 * maturity Rcpp interface                                   *
 ***************************************************************/

/**
 * @brief MaturityInterfaceBase class should be inherited to
 * define different Rcpp interfaces for each possible maturity function
 * */
class MaturityInterfaceBase : public FIMSRcppInterfaceBase {
public:
    static uint32_t id_g; /**< static id of the recruitment interface base*/
    uint32_t id; /**< id of the recruitment interface base */
    static std::map<uint32_t, MaturityInterfaceBase*>
    maturity_objects; /**< map associating the ids of
                              MaturityInterfaceBase to the objects */

    MaturityInterfaceBase() {
        this->id = MaturityInterfaceBase::id_g++;
        MaturityInterfaceBase::maturity_objects[this->id] = this;
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
    }

    virtual ~MaturityInterfaceBase() {
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

uint32_t MaturityInterfaceBase::id_g = 1;
std::map<uint32_t, MaturityInterfaceBase*>
MaturityInterfaceBase::maturity_objects;

/**
 * @brief Rcpp interface for logistic maturity as an S4 object. To
 * instantiate from R: logistic_maturity <- new(fims$logistic_maturity)
 */
class LogisticMaturityInterface : public MaturityInterfaceBase {
public:
    Parameter median; /**< the index value at which the response reaches .5 */
    Parameter slope; /**< the width of the curve at the median */

    LogisticMaturityInterface() : MaturityInterfaceBase() {
    }

    virtual ~LogisticMaturityInterface() {
    }

    /** @brief returns the id for the logistic maturity interface */
    virtual uint32_t get_id() {
        return this->id;
    }

    /** @brief evaluate the logistic maturity function
     *   @param x  The independent variable in the logistic function (e.g., age or
     * size in maturity).
     */
    virtual double evaluate(double x) {
        fims::LogisticMaturity<double> LogisticMat;

        LogisticMat.median = this->median.value;
        LogisticMat.slope = this->slope.value;
        return LogisticMat.evaluate(x);
    }

#ifdef TMB_MODEL

    template<typename Type>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims::Information<Type> > info =
                fims::Information<Type>::GetInstance();

        std::shared_ptr<fims::LogisticMaturity<Type> > maturity =
                std::make_shared<fims::LogisticMaturity<Type> >();

        // set relative info
        maturity->id = this->id;
        maturity->median = this->median.value;
        if (this->median.estimated) {
            if (this->median.is_random_effect) {
                info->RegisterRandomEffect(maturity->median);
            } else {
                info->RegisterParameter(maturity->median);
            }
        }
        maturity->slope = this->slope.value;
        if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
                info->RegisterRandomEffect(maturity->slope);
            } else {
                info->RegisterParameter(maturity->slope);
            }
        }

        // add to Information
        info->maturity_models[maturity->id] = maturity;

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