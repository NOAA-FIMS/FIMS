/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to declare the growth functor class
 * which is the base class for all growth functors.
 */
#ifndef POPULATION_DYNAMICS_VBGF_HPP
#define POPULATION_DYNAMICS_VBGF_HPP

#include "growth_base.hpp"

namespace fims_popdy {

/**
 *  @brief Von bertalanffy class that returns the length function value.
 */
template <typename Type>
struct VBGF : public GrowthBase<Type> {
    
    fims::Vector<Type> ages; /*< the vector of ages */
    fims::Vector<Type> lengths; /*< the vector of lengths */
    size_t a1; /*< the age at which von Bertalanffy growth starts */
    size_t a2; /*< the age at which von Bertalanffy growth ends */
    Type K; /*< the von Bertalanffy growth rate */
    Type L1; /*< L1 the mean length at age a1 */
    Type L2; /*< L2 the mean length at age a2*/

    bool estimated = false;
    
    VBGF() : GrowthBase<Type>() {}
    
    virtual ~VBGF() {}

    
    /**
     * Computes the initial ALK.
     */
    virtual void Initialize(){
        this->lengths.resize(ages.size()); 
    }

    /**
     * Computes the length at age a from a von Bertalanffy growth function (Schnute parameterization).
     * @param a the age at which to evaluate length
     */
    virtual void CalculateVBGF(size_t a){
        lengths[0] = L1;
        for(size_t a = 1; a < ages.size(); a++){
            lengths[a] = L1+(L2-L1)*((1-exp(-K*(a-a1))/(1-exp(-K*(a2-a1)))));
        }
    }
    
    /**
     * @brief Returns the length at age a (in cm) from the input vector.
     *
     * @param a  age of the fish, the age vector must start at zero
     */
    virtual const Type Evaluate(const double& a) {
        return this->CalculateVBGF(a);
    }
    
};
}  // namespace fims_popdy
#endif