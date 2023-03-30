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
  uint32_t id;          /**< id of the recruitment interface base */
  static std::map<uint32_t, SelectivityInterfaceBase*>
      selectivity_objects; /**< map associating the ids of
                              SelectivityInterfaceBase to the objects */

  SelectivityInterfaceBase() {
    this->id = SelectivityInterfaceBase::id_g++;
    SelectivityInterfaceBase::selectivity_objects[this->id] = this;
    FIMSRcppInterfaceBase::fims_interface_objects.push_back(this);
  }

  virtual ~SelectivityInterfaceBase() {}

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
  Parameter slope;  /**< the width of the curve at the median */

  LogisticSelectivityInterface() : SelectivityInterfaceBase() {}

  virtual ~LogisticSelectivityInterface() {}

  /** @brief returns the id for the logistic selectivity interface */
  virtual uint32_t get_id() { return this->id; }

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

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_REAL_TYPE> > ls0 =
        std::make_shared<fims::LogisticSelectivity<TMB_FIMS_REAL_TYPE> >();

    // set relative info
    ls0->id = this->id;
    ls0->median = this->median.value;
    if (this->median.estimated) {
      if (this->median.is_random_effect) {
        d0->RegisterRandomEffect(ls0->median);
      } else {
        d0->RegisterParameter(ls0->median);
      }
    }
    ls0->slope = this->slope.value;
    if (this->slope.estimated) {
      if (this->slope.is_random_effect) {
        d0->RegisterRandomEffect(ls0->slope);
      } else {
        d0->RegisterParameter(ls0->slope);
      }
    }

    // add to Information
    d0->selectivity_models[ls0->id] = ls0;

    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_FIRST_ORDER> > ls1 =
        std::make_shared<fims::LogisticSelectivity<TMB_FIMS_FIRST_ORDER> >();

    // set relative info
    ls1->id = this->id;
    ls1->median = this->median.value;
    if (this->median.estimated) {
      if (this->median.is_random_effect) {
        d1->RegisterRandomEffect(ls1->median);
      } else {
        d1->RegisterParameter(ls1->median);
      }
    }
    ls1->slope = this->slope.value;
    if (this->slope.estimated) {
      if (this->slope.is_random_effect) {
        d1->RegisterRandomEffect(ls1->slope);
      } else {
        d1->RegisterParameter(ls1->slope);
      }
    }

    // add to Information
    d1->selectivity_models[ls1->id] = ls1;

    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_SECOND_ORDER> > ls2 =
        std::make_shared<fims::LogisticSelectivity<TMB_FIMS_SECOND_ORDER> >();

    // set relative info
    ls2->id = this->id;
    ls2->median = this->median.value;
    if (this->median.estimated) {
      if (this->median.is_random_effect) {
        d2->RegisterRandomEffect(ls2->median);
      } else {
        d2->RegisterParameter(ls2->median);
      }
    }
    ls2->slope = this->slope.value;
    if (this->slope.estimated) {
      if (this->slope.is_random_effect) {
        d2->RegisterRandomEffect(ls2->slope);
      } else {
        d2->RegisterParameter(ls2->slope);
      }
    }

    // add to Information
    d2->selectivity_models[ls2->id] = ls2;

    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_THIRD_ORDER> > ls3 =
        std::make_shared<fims::LogisticSelectivity<TMB_FIMS_THIRD_ORDER> >();

    // set relative info
    ls3->id = this->id;
    ls3->median = this->median.value;
    if (this->median.estimated) {
      if (this->median.is_random_effect) {
        d3->RegisterRandomEffect(ls3->median);
      } else {
        d3->RegisterParameter(ls3->median);
      }
    }
    ls3->slope = this->slope.value;
    if (this->slope.estimated) {
      if (this->slope.is_random_effect) {
        d3->RegisterRandomEffect(ls3->slope);
      } else {
        d3->RegisterParameter(ls3->slope);
      }
    }

    // add to Information
    d3->selectivity_models[ls3->id] = ls3;

    return true;
  }
};

/**
 * @brief Rcpp interface for logistic selectivity as an S4 object. To
 * instantiate from R: logistic_selectivity <- new(fims$logistic_selectivity)
 */
class DoubleLogisticSelectivityInterface : public SelectivityInterfaceBase {
 public:
  Parameter median_asc; /**< the index value at which the response reaches .5 */
  Parameter slope_asc;  /**< the width of the curve at the median */
  Parameter
      median_desc;      /**< the index value at which the response reaches .5 */
  Parameter slope_desc; /**< the width of the curve at the median */

  DoubleLogisticSelectivityInterface() : SelectivityInterfaceBase() {}

  virtual ~DoubleLogisticSelectivityInterface() {}

  /** @brief returns the id for the double logistic selectivity interface */
  virtual uint32_t get_id() { return this->id; }

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

  /** @brief this adds the parameter values and derivatives to the TMB model
   * object */
  virtual bool add_to_fims_tmb() {
    std::shared_ptr<fims::Information<TMB_FIMS_REAL_TYPE> > d0 =
        fims::Information<TMB_FIMS_REAL_TYPE>::GetInstance();

    std::shared_ptr<fims::DoubleLogisticSelectivity<TMB_FIMS_REAL_TYPE> > ls0 =
        std::make_shared<
            fims::DoubleLogisticSelectivity<TMB_FIMS_REAL_TYPE> >();

    // set relative info
    ls0->id = this->id;
    ls0->median_asc = this->median_asc.value;
    if (this->median_asc.estimated) {
      if (this->median_asc.is_random_effect) {
        d0->RegisterRandomEffect(ls0->median_asc);
      } else {
        d0->RegisterParameter(ls0->median_asc);
      }
    }
    ls0->slope_asc = this->slope_asc.value;
    if (this->slope_asc.estimated) {
      if (this->slope_asc.is_random_effect) {
        d0->RegisterRandomEffect(ls0->slope_asc);
      } else {
        d0->RegisterParameter(ls0->slope_asc);
      }
    }
    ls0->median_desc = this->median_desc.value;
    if (this->median_desc.estimated) {
      if (this->median_desc.is_random_effect) {
        d0->RegisterRandomEffect(ls0->median_desc);
      } else {
        d0->RegisterParameter(ls0->median_desc);
      }
    }
    ls0->slope_desc = this->slope_desc.value;
    if (this->slope_desc.estimated) {
      if (this->slope_desc.is_random_effect) {
        d0->RegisterRandomEffect(ls0->slope_desc);
      } else {
        d0->RegisterParameter(ls0->slope_desc);
      }
    }

    // add to Information
    d0->selectivity_models[ls0->id] = ls0;

    std::shared_ptr<fims::Information<TMB_FIMS_FIRST_ORDER> > d1 =
        fims::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();

    std::shared_ptr<fims::DoubleLogisticSelectivity<TMB_FIMS_FIRST_ORDER> >
        ls1 = std::make_shared<
            fims::DoubleLogisticSelectivity<TMB_FIMS_FIRST_ORDER> >();

    // set relative info
    ls1->id = this->id;
    ls1->median_asc = this->median_asc.value;
    if (this->median_asc.estimated) {
      if (this->median_asc.is_random_effect) {
        d1->RegisterRandomEffect(ls1->median_asc);
      } else {
        d1->RegisterParameter(ls1->median_asc);
      }
    }
    ls1->slope_asc = this->slope_asc.value;
    if (this->slope_asc.estimated) {
      if (this->slope_asc.is_random_effect) {
        d1->RegisterRandomEffect(ls1->slope_asc);
      } else {
        d1->RegisterParameter(ls1->slope_asc);
      }
    }
    ls1->median_desc = this->median_desc.value;
    if (this->median_desc.estimated) {
      if (this->median_desc.is_random_effect) {
        d1->RegisterRandomEffect(ls1->median_desc);
      } else {
        d1->RegisterParameter(ls1->median_desc);
      }
    }
    ls1->slope_desc = this->slope_desc.value;
    if (this->slope_desc.estimated) {
      if (this->slope_desc.is_random_effect) {
        d1->RegisterRandomEffect(ls1->slope_desc);
      } else {
        d1->RegisterParameter(ls1->slope_desc);
      }
    }

    // add to Information
    d1->selectivity_models[ls1->id] = ls1;

    std::shared_ptr<fims::Information<TMB_FIMS_SECOND_ORDER> > d2 =
        fims::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();

    std::shared_ptr<fims::DoubleLogisticSelectivity<TMB_FIMS_SECOND_ORDER> >
        ls2 = std::make_shared<
            fims::DoubleLogisticSelectivity<TMB_FIMS_SECOND_ORDER> >();

    // set relative info
    ls2->id = this->id;
    ls2->median_asc = this->median_asc.value;
    if (this->median_asc.estimated) {
      if (this->median_asc.is_random_effect) {
        d2->RegisterRandomEffect(ls2->median_asc);
      } else {
        d2->RegisterParameter(ls2->median_asc);
      }
    }
    ls2->slope_asc = this->slope_asc.value;
    if (this->slope_asc.estimated) {
      if (this->slope_asc.is_random_effect) {
        d2->RegisterRandomEffect(ls2->slope_asc);
      } else {
        d2->RegisterParameter(ls2->slope_asc);
      }
    }

    ls2->median_desc = this->median_desc.value;
    if (this->median_desc.estimated) {
      if (this->median_desc.is_random_effect) {
        d2->RegisterRandomEffect(ls2->median_desc);
      } else {
        d2->RegisterParameter(ls2->median_desc);
      }
    }
    ls2->slope_desc = this->slope_desc.value;
    if (this->slope_desc.estimated) {
      if (this->slope_desc.is_random_effect) {
        d2->RegisterRandomEffect(ls2->slope_desc);
      } else {
        d2->RegisterParameter(ls2->slope_desc);
      }
    }

    // add to Information
    d2->selectivity_models[ls2->id] = ls2;

    std::shared_ptr<fims::Information<TMB_FIMS_THIRD_ORDER> > d3 =
        fims::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();

    std::shared_ptr<fims::DoubleLogisticSelectivity<TMB_FIMS_THIRD_ORDER> >
        ls3 = std::make_shared<
            fims::DoubleLogisticSelectivity<TMB_FIMS_THIRD_ORDER> >();

    // set relative info
    ls3->id = this->id;
    ls3->median_asc = this->median_asc.value;
    if (this->median_asc.estimated) {
      if (this->median_asc.is_random_effect) {
        d3->RegisterRandomEffect(ls3->median_asc);
      } else {
        d3->RegisterParameter(ls3->median_asc);
      }
    }
    ls3->slope_asc = this->slope_asc.value;
    if (this->slope_asc.estimated) {
      if (this->slope_asc.is_random_effect) {
        d3->RegisterRandomEffect(ls3->slope_asc);
      } else {
        d3->RegisterParameter(ls3->slope_asc);
      }
    }

    ls3->median_desc = this->median_desc.value;
    if (this->median_desc.estimated) {
      if (this->median_desc.is_random_effect) {
        d3->RegisterRandomEffect(ls3->median_desc);
      } else {
        d3->RegisterParameter(ls3->median_desc);
      }
    }
    ls3->slope_desc = this->slope_desc.value;
    if (this->slope_desc.estimated) {
      if (this->slope_desc.is_random_effect) {
        d3->RegisterRandomEffect(ls3->slope_desc);
      } else {
        d3->RegisterParameter(ls3->slope_desc);
      }
    }

    // add to Information
    d3->selectivity_models[ls3->id] = ls3;

    return true;
  }
};

#endif