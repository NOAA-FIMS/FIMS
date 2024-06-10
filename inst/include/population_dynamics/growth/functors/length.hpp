/*
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project.
 * Refer to the LICENSE file for reuse information.
 *
 * The purpose of this file is to declare the growth functor class
 * which is the base class for all growth functors.
 */
#ifndef POPULATION_DYNAMICS_LENGTH_HPP
#define POPULATION_DYNAMICS_LENGTH_HPP

//#include "../../../interface/interface.hpp"
#include <map>

#include "growth_base.hpp"

namespace fims_popdy {

/**
 *  @brief EWAAgrowth class that returns the EWAA function value.
 */
template <typename Type>
struct Length : public GrowthBase<Type> {
    
    fims::Vector<Type> length_classes;
    fims::Vector<Type> age_means; //estimable
    fims::Vector<Type> age_stddevs; //estimable
    //derived
    fims::Vector<fims::Vector<Type> > alk;
    
    boolean estimated = false;
    
    Length() : GrowthBase<Type>() {}
    
    virtual ~Length() {}
    
    
    /**
     * Updates the ALK if any elements are estimated.
     */
    virtual void prepare(){
        if(estimated)
            this->update_alk();
    }
    
    /**
     * Computes the initial ALK.
     */
    virtual void initialize(){
        this->update_alk();
    }
    
    /**
     * @brief Returns the weight at age a (in kg) from the input vector.
     *
     * @param a  age of the fish, the age vector must start at zero
     */
    virtual const Type evaluate(const double& a) {
        
        return 1.0;
    }
    
    
    //sign function using hyperbolic tangent
    Type sign(Type x) {
        const Type k = 1e3; // A large constant
        return tanh(k * x);
    }
    
    
    /**
     * @brief Differentiable cumulative normal distribution function.
     *
     *@param x
     *@param mean
     *@param stddev
     *
     */
    Type cdf(Type x, Type mean, Type stddev) {
        
        // Constants for the cumulative normal distribution function
        const Type a1 = 0.254829592;
        const Type a2 = -0.284496736;
        const Type a3 = 1.421413741;
        const Type a4 = -1.453152027;
        const Type a5 = 1.061405429;
        const Type p = 0.3275911;
        Type z = (x - mean) / (stddev * sqrt(2.0));
        Type t = 1.0 / (1.0 + p * abs(z));
        Type erf_approx = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-z * z);
        return  0.5 * (1.0 + sign(z) * erf_approx);
    }
    
    /**
     * @brief The Age-Length Key (ALK) is used to estimate the age
     * composition of a fish population based on length measurements.
     *
     * @param lengths, measured lengths.
     * @param age_means, measured mean age at length.
     * @param age_stddevs, measured standard deviation age at length.
     *
     */
    void update_alk() {
        
        size_t num_lengths = this->length_classes.size();
        size_t num_ages = this->age_means.size();
        
        for (size_t i = 0; i < num_lengths; ++i) {
            for (size_t j = 0; j < num_ages; ++j) {
                
                // Probability of length given age j
                this->alk[i][j] = Type(cdf(this->length_classes[i] + 0.5, this->age_means[j], this->age_stddevs[j]) -
                                       cdf(this->length_classes[i] - 0.5, this->age_means[j], this->age_stddevs[j]));
            }
        }
    }
    
};
}  // namespace fims_popdy
#endif
