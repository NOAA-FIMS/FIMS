/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to declare the LogisticSelectivity class
 * which implements the logistic function from fims_math in the selectivity
 * module.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP

//#include "../../../interface/interface.hpp"
#include "../../../common/fims_math.hpp"
#include "../../../common/fims_vector.hpp"
#include "selectivity_base.hpp"

namespace fims_popdy {

/**
 *  @brief LogisticSelectivity class that returns the logistic function value
 * from fims_math.
 */
template <typename Type>
struct LogisticSelectivity : public SelectivityBase<Type> {
    fims::Vector<Type> inflection_point; /**< 50% quantile of the value of the quantity of
               interest (x); e.g. age at which 50% of the fish are selected */
    fims::Vector<Type> slope; /**<scalar multiplier of difference between quantity of interest
              value (x) and inflection_point */

    LogisticSelectivity() : SelectivityBase<Type>()
    {
    }

    virtual ~LogisticSelectivity()
    {
    }

    /**
     * @brief Method of the logistic selectivity class that implements the
     * logistic function from FIMS math.
     *
     * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - inflection\_point))} \f]
     *
     * @param x  The independent variable in the logistic function (e.g., age or
     * size in selectivity).
     */
    virtual const Type evaluate(const Type& x)
    {
        return fims_math::logistic<Type>(inflection_point[0], slope[0], x);
    }

    /**
     * @brief Method of the logistic selectivity class that implements the
     * logistic function from FIMS math.
     *
     * \f[ \frac{1.0}{ 1.0 + exp(-1.0 * slope_t (x - {inflection\_point}_t))} \f]
     *
     * @param x  The independent variable in the logistic function (e.g., age or
     * size in selectivity).
     * @param pos Position index, e.g., which year.
     */
    virtual const Type evaluate(const Type& x, size_t pos)
    {
        return fims_math::logistic<Type>(inflection_point.get_force_scalar(pos), slope.get_force_scalar(pos), x);
    }
};

} // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_LOGISTIC_HPP */
