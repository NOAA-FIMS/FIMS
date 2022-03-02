#include <cmath>

template <class Type>
/**
 * Probability density function of the lognormal distribution.
 *
 * @param x A double value.
 * @param meanlog Mean of the distribution on the log scale.
 * @param sdlog Standard deviation of distribution on the log scale.
 * @returns A density value.
 */
Type dlognorm(Type x, Type meanlog, Type sdlog)
{
  Type resid = (log(x) - meanlog) / sdlog;
  Type logres = -log(sqrt(2.0 * M_PI)) - log(sdlog) - Type(0.5) * resid * resid - log(x);
  return logres;
}