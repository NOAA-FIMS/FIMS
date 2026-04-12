/**
 * @file types.hpp
 * @brief Centralized definition of core model types, enumerations, and structures.
 * @details This file serves as a lightweight, dependency-free foundation for the 
 * fims namespace.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */

#ifndef FIMS_COMMON_TYPES_HPP
#define FIMS_COMMON_TYPES_HPP

namespace fims {

/**
 * @brief A data structure for transforming parameters
 * 
 * @details The `Transformation` struct defines the type of transformation to 
 * apply to a parameter and any necessary arguments for that transformation. 
 * This allows for flexible parameter transformations within the FIMS framework.
 */
struct Transformation {
  /**
   * @brief An enumeration of supported transformation types.
   * @details Supported transformations include:
   * - `identity`: No transformation, parameter is on the natural scale.
   * - `exp`: Exponential transformation, parameter is on the multiplicative scale.
   * - `log`: Log transformation, parameter is on the log scale.
   * - `logit`: Logit transformation, parameter is on the logit scale.
   * - `square`: Square transformation, parameter is on the squared scale.
   * - `sqrt`: Square root transformation, parameter is on the square root scale.
   */
    enum class Label {
      identity,
      exp,
      log,
      logit,
      square,
      sqrt
    }; 
    /**
     * @brief A structure to hold arguments for transformations that require them.
     * @details For the logit transformation, `lower` and `upper` specify the
     * bounds of the parameter in natural space. 
     */
    struct Args {
      //used by logit
      double lower = 0.0;
      double upper = 1.0;
    };


    Label label = Label::log;
    Args args{};

  };

  static const char* TransformationLabelToString(
      fims::Transformation::Label label) {
    switch (label) {
      case fims::Transformation::Label::identity: return "identity";
      case fims::Transformation::Label::exp: return "exp";
      case fims::Transformation::Label::log: return "log";
      case fims::Transformation::Label::logit: return "logit";
      case fims::Transformation::Label::square: return "square";
      case fims::Transformation::Label::sqrt: return "sqrt";
      default: return "unknown";
    }
  }

  /**
 * @brief A structure to hold distribution type information for priors.
 * @details Supported distributions include:
 * - `Normal`: Normal distribution.
 * - `Lognormal`: Lognormal distribution.
 * - `Gamma`: Gamma distribution.
 * - `InvGamma`: Inverse gamma distribution.
 * - `Multinom`: Multinomial distribution.
 */
struct Distribution {
  /**
   * @brief An enumeration of supported distribution types.
   */
  enum class Label {
    Normal,
    Lognormal,
    Gamma,
    InvGamma
  };

  Label label = Label::Normal;
};

fims::Distribution::Label StringToDistributionLabel(const std::string& name) {
    if (name == "Normal")     return fims::Distribution::Label::Normal;
    if (name == "Lognormal")    return fims::Distribution::Label::Lognormal;
    if (name == "Gamma")    return fims::Distribution::Label::Gamma;
    if (name == "InvGamma") return fims::Distribution::Label::InvGamma;
    throw std::invalid_argument("Unsupported distribution: " + name);
}

} // namespace fims

#endif /* FIMS_COMMON_TYPES_HPP */