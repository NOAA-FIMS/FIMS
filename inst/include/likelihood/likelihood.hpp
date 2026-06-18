#include <Rcpp.h>
#include <vector>
#include <memory>
#include <cmath>
#include <string>

// =============================================================================
// 1. Data Structures & Interfaces
// =============================================================================

// The shared bridge containing derived quantities from the population model
template <typename Type>
struct DerivedQuantities {
    std::vector<Type> expected_catch;
    std::vector<Type> expected_biomass;
};

// Abstract Base Class for any Likelihood, Prior, or Penalty component
template <typename Type>
class LikelihoodInterface {
public:
    virtual ~LikelihoodInterface() {}
    virtual Type evaluate() = 0;
    virtual std::string get_name() = 0;
};

// =============================================================================
// 2. Core Modules (Model Family & Likelihood Subclasses)
// =============================================================================

// The Producer: Simulates population dynamics and populates the shared bridge
template <typename Type>
class ModelFamily {
private:
    std::shared_ptr<DerivedQuantities<Type>> derived_data;

public:
    ModelFamily(std::shared_ptr<DerivedQuantities<Type>> shared_derived) 
        : derived_data(shared_derived) {}

    // Simulates population trajectories using a parameter value (like log_R0)
    void run_population_dynamics(Type log_R0) {
        Type R0 = std::exp(log_R0);
        // Scaling mock population trajectory based on R0
        derived_data->expected_biomass = {R0 * 10.0, R0 * 12.0, R0 * 14.0};
        derived_data->expected_catch   = {R0 * 2.0,  R0 * 2.5,  R0 * 3.0};
    }
};

// The Consumer: Fits observed data to expected data via the shared bridge
template <typename Type>
class CatchLikelihood : public LikelihoodInterface<Type> {
private:
    std::shared_ptr<DerivedQuantities<Type>> derived_data;
    std::vector<Type> observed_data;
    Type sigma;

public:
    CatchLikelihood(std::shared_ptr<DerivedQuantities<Type>> shared_derived, 
                    const std::vector<Type>& observations, Type s)
        : derived_data(shared_derived), observed_data(observations), sigma(s) {}

    virtual Type evaluate() override {
        Type nll = 0.0;
        const auto& expected = derived_data->expected_catch;

        for (size_t i = 0; i < observed_data.size() && i < expected.size(); ++i) {
            Type residual = observed_data[i] - expected[i];
            nll += 0.5 * (std::log(2.0 * M_PI * sigma * sigma) + (residual * residual) / (sigma * sigma)); 
        }
        return nll;
    }

    virtual std::string get_name() override { return "Catch Data Likelihood"; }
};

// The Consumer: Applies statistical penalties directly to parameters (Priors or Random Effects)
template <typename Type>
class ParameterPrior : public LikelihoodInterface<Type> {
private:
    Type parameter_value; // Stores the evaluated state of the parameter
    double mean;
    double sd;
    std::string name;

public:
    ParameterPrior(Type value, double m, double s, std::string n) 
        : parameter_value(value), mean(m), sd(s), name(n) {}

    virtual Type evaluate() override {
        Type residual = parameter_value - mean;
        return 0.5 * (std::log(2.0 * M_PI * sd * sd) + (residual * residual) / (sd * sd));
    }

    virtual std::string get_name() override { return "Prior/RE Penalty (" + name + ")"; }
};

// =============================================================================
// 3. Rcpp Module Bindings (Instantiated with double)
// =============================================================================

typedef DerivedQuantities<double> DerivedQuantitiesD;
typedef ModelFamily<double> ModelFamilyD;
typedef LikelihoodInterface<double> LikelihoodInterfaceD;
typedef CatchLikelihood<double> CatchLikelihoodD;
typedef ParameterPrior<double> ParameterPriorD;

RCPP_MODULE(fims_hierarchical_module) {
    using namespace Rcpp;

    // Expose the shared data container
    class_<DerivedQuantitiesD>("DerivedQuantities")
        .constructor()
        .field("expected_biomass", &DerivedQuantitiesD::expected_biomass)
        .field("expected_catch", &DerivedQuantitiesD::expected_catch)
        ;

    // Expose the Population Engine
    class_<ModelFamilyD>("ModelFamily")
        .constructor<std::shared_ptr<DerivedQuantitiesD>>()
        .method("run_population_dynamics", &ModelFamilyD::run_population_dynamics)
        ;

    // Expose the Base Interface so R can hold collection references
    class_<LikelihoodInterfaceD>("LikelihoodInterface")
        .method("evaluate", &LikelihoodInterfaceD::evaluate)
        .method("get_name", &LikelihoodInterfaceD::get_name)
        ;

    // Expose Data Likelihood (inheriting from interface)
    class_<CatchLikelihoodD>("CatchLikelihood")
        .derives<LikelihoodInterfaceD>("LikelihoodInterface")
        .constructor<std::shared_ptr<DerivedQuantitiesD>, std::vector<double>, double>()
        ;

    // Expose Parameter Prior / Random Effect Component (inheriting from interface)
    class_<ParameterPriorD>("ParameterPrior")
        .derives<LikelihoodInterfaceD>("LikelihoodInterface")
        .constructor<double, double, double, std::string>()
        ;
}
