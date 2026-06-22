/**
 * @file likelihood.hpp
 * @brief Aggregates the experimental likelihood component module.
 *
 * @details This module lives in parallel to the existing distribution LPDF
 * functors. Likelihood components evaluate and return negative
 * log-likelihood contributions directly.
 */
#ifndef FIMS_LIKELIHOOD_HPP
#define FIMS_LIKELIHOOD_HPP

#include "functors/likelihood_component_base.hpp"
#include "functors/gamma_likelihood.hpp"
#include "functors/invgamma_likelihood.hpp"
#include "functors/lognormal_likelihood.hpp"
#include "functors/multinomial_likelihood.hpp"
#include "functors/normal_likelihood.hpp"

#endif /* FIMS_LIKELIHOOD_HPP */
