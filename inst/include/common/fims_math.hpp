/** \file fims_math.hpp
 */
// note: To document a global C function, typedef, enum or preprocessor
// definition you must first document the file that contains it

/*
 * File:   fims_math.hpp
 *
 * This File is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the
 * source folder for reuse information.
 *
 */
#ifndef FIMS_MATH_HPP
#define FIMS_MATH_HPP

// note: this is modeling platform specific, must be controlled by
// preprocessing macros
//#include "def.hpp"
#include <cmath>
#include <random>
#include <sstream>

#include "../interface/interface.hpp"
#include "fims_vector.hpp"

namespace fims_math {
#ifdef STD_LIB

/**
 * @brief The exponential function.
 *
 * @param x value to exponentiate. Please use fims_math::exp<double>(x) if x is
 * an integer.
 * @return the exponentiated value
 */
template <class Type>
inline const Type exp(const Type &x) {
  return std::exp(x);
}

/**
 * @brief The natural log function (base e)
 * @param x the value to take the log of. Please use fims_math::log<double>(x)
 * if x is an integer.
 * @return
 */
template <class Type>
inline const Type log(const Type &x) {
  return std::log(x);
}

template <class Type>
inline const Type cos(const Type &x) {
  return std::cos(x);
}

template <class Type>
inline const Type sqrt(const Type &x) {
  return std::sqrt(x);
}

template <class Type>
inline const Type pow(const Type &x, const Type &y) {
  return std::pow(x, y);
}
#endif

#ifdef TMB_MODEL
// #include <TMB.hpp>

/**
 * @brief The exponential function.
 * The code cannot be tested using the compilation flag
 * -DTMB_MODEL through CMake and Google Test
 * @param x value to exponentiate. Please use fims_math::exp<double>(x) if x is
 * an integer.
 * @return the exponentiated value
 */
template <class Type>
inline const Type exp(const Type &x) {
  using ::exp;
  return exp(x);
}

template <>
inline const double exp(const double &x) {
  return std::exp(x);
}

/**
 * @brief The natural log function (base e)
 * The code cannot be tested using the compilation flag
 * -DTMB_MODEL through CMake and Google Test.
 * @param x the value to take the log of. Please use fims_math::log<double>(x)
 * if x is an integer.
 * @return the log of the value
 */
template <class Type>
inline const Type log(const Type &x) {
  return log(x);
}

template <>
inline const double log(const double &x) {
  return std::log(x);
}

template <class Type>
inline const Type cos(const Type &x) {
    return cos(x);
}

template <>
inline const double cos(const double &x) {
    return std::cos(x);
}

template <class Type>
inline const Type sqrt(const Type &x) {
    return sqrt(x);
}

template <>
inline const double sqrt(const double &x) {
    return std::sqrt(x);
}

template <class Type>
inline const Type pow(const Type &x, const Type &y) {
    return pow(x, y);
}

template <>
inline const double pow(const double &x, const double &y) {
    return std::pow(x, y);
}

#endif

/**
 * @brief The general logistic function
 *
 * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - inflection_point))} \f$
 *
 * @param inflection_point the inflection point of the logistic function
 * @param slope the slope of the logistic function
 * @param x the index the logistic function should be evaluated at
 * @return
 */
template <class Type>
inline const Type logistic(const Type &inflection_point, const Type &slope,
                           const Type &x) {
  return (1.0) / (1.0 + exp(-1.0 * slope * (x - inflection_point)));
}

/**
 * @brief A logit function for bounding of parameters
 *
 * \f$ -\mathrm{log}(b-x) + \mathrm{log}(x-a) \f$
 * @param a lower bound
 * @param b upper bound
 * @param x the parameter in bounded space
 * @return the parameter in real space
 *
 */
template <class Type>
inline const Type logit(const Type &a, const Type &b, const Type &x) {
  return -fims_math::log(b - x) + fims_math::log(x - a);
}

/**
 * @brief An inverse logit function for bounding of parameters
 *
 * \f$ a+\frac{b-a}{1+\mathrm{exp}(-\mathrm{logit}(x))}\f$
 * @param a lower bound
 * @param b upper bound
 * @param logit_x the parameter in real space
 * @return the parameter in bounded space
 *
 */
template <class Type>
inline const Type inv_logit(const Type &a, const Type &b, const Type &logit_x) {
  return a + (b - a) / (1.0 + fims_math::exp(-logit_x));
}

/**
 * @brief The general double logistic function
 *
 * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope_{asc} (x - inflection_point_{asc}))}
 * \left(1-\frac{1.0}{ 1.0 + exp(-1.0 * slope_{desc} (x -
 * inflection_point_{desc}))} \right)\f$
 *
 * @param inflection_point_asc the inflection point of the ascending limb of the
 * double logistic function
 * @param slope_asc the slope of the ascending limb of the double logistic
 * function
 * @param inflection_point_desc the inflection point of the descending limb of
 * the double logistic function, where inflection_point_desc >
 * inflection_point_asc
 * @param slope_desc the slope of the descending limb of the double logistic
 * function
 * @param x the index the logistic function should be evaluated at
 * @return
 */

template <class Type>
inline const Type double_logistic(const Type &inflection_point_asc,
                                  const Type &slope_asc,
                                  const Type &inflection_point_desc,
                                  const Type &slope_desc, const Type &x) {
  return (1.0) / (1.0 + exp(-1.0 * slope_asc * (x - inflection_point_asc))) *
         (1.0 -
          (1.0) / (1.0 + exp(-1.0 * slope_desc * (x - inflection_point_desc))));
}

/**
 *
 * Used when x could evaluate to zero, which will result in a NaN for
 * derivative values.
 *
 * Evaluates:
 *
 * \f$ (x^2+C)^.5 \f$
 *
 * @param x value to keep positive
 * @param C default = 1e-5
 * @return
 */
template <class Type>
const Type ad_fabs(const Type &x, Type C = 1e-5) {
  return sqrt((x * x) + C);
}

/**
 *
 * Returns the minimum between a and b in a continuous manner using:
 *
 * (a + b - fims_math::ad_fabs(a - b))*.5;
 * Reference: \ref fims_math::ad_fabs()
 *
 * This is an approximation with minimal error.
 *
 * @param a
 * @param b
 * @param C default = 1e-5
 * @return
 */

template <typename Type>
inline const Type ad_min(const Type &a, const Type &b, Type C = 1e-5) {
  return (a + b - fims_math::ad_fabs(a - b, C)) * 0.5;
}

/**
 * Returns the maximum between a and b in a continuous manner using:
 *
 * (a + b + fims_math::ad_fabs(a - b)) *.5;
 * Reference: \ref fims_math::ad_fabs()
 * This is an approximation with minimal error.
 *
 * @param a
 * @param b
 * @param C default = 1e-5
 * @return
 */
template <typename Type>
inline const Type ad_max(const Type &a, const Type &b, Type C = 1e-5) {
  return (a + b + fims_math::ad_fabs(a - b, C)) * static_cast<Type>(.5);
}

    /**
     * \ingroup NormalDistribution
     * 
     * @brief The probability density function for a standard normal distribution.
     *
     * \f$
     * f(x) = \frac{1}{\sqrt{2\pi\theta^2}}exp(-\frac{(x-\mu)^2}{2\theta^2})
     * \f$<br>
     * <b>Where,</b><br>
     *  \f$\ \mu \f$  is the mean.<br>
     *  \f$\ \theta\f$ is the standard deviation.<br>
     * 
     *  @image html http://upload.wikimedia.org/wikipedia/commons/7/74/Normal_Distribution_PDF.svg
     * 
     *  <br> <b>Source:</b> http://en.wikipedia.org/wiki/Normal_distribution<br>
     * 
     * 
     * 
     * @author Matthew Supernaw
     * @date June 6, 2013
     *  
     * @param x
     * @param mean
     * @param sd
     * @return relative likelihood for this random variable to have value x.
     */
    template<class T>
    const T dnorm(const T &x, const T &mean, const T &sd, bool ret_log = false) {

        // Check if the standard deviation is valid
        if (sd <= static_cast<T> (0)) {
            throw std::invalid_argument("Standard deviation must be positive.");
        }

        // Constants
        const T log_sqrt_2pi = fims_math::log(static_cast<T>(2) * M_PI) / static_cast<T>(2);

        // Compute z-score
        T z = (x - mean) / sd;

    // Compute the log of the density
    T log_density = -static_cast<T>(0.5) * z * z - fims_math::log(sd) - log_sqrt_2pi;

        // Return log density if requested
        if (ret_log) {
            return log_density;
        }

        // Otherwise, return the density
        return fims_math::exp(log_density);
    }

    /**
     * @brief Normal distribution random sampling.
     *
     * @param mean The mean of the distribution \( \mu \).
     * @param sd The standard deviation of the distribution \( \sigma \).
     * @param gen Random number generator.
     * @return T Random sample from the Normal distribution.
     */
    template<typename T>
    T rnorm(T mean, T sd) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<double> dist(0.0, 1.0);

        T u1 = static_cast<T> (dist(gen));
        T u2 = static_cast<T> (dist(gen));

        T z0 = fims_math::sqrt(-2.0 * log(u1)) * fims_math::cos(2.0 * M_PI * u2);

        return z0 * sd + mean;
    }

    /**
     * @ingroup LogNormal
     * 
     * @brief The probability distribution function for a log-normal distribution.
     * 
     * \f$
     * f(x) = \frac{1}{x\sqrt{2\pi}\theta}e^{-\frac{(ln x - \mu)^{2}}{2\theta^{2}}}
     * \f$
     * 
     * <br><b>Where,</b><br>
     * \f$\mu\f$ is the log of the mean.<br>
     * \f$\theta\f$ is the log of the standard deviation.<br>
     * 
     * @image html http://upload.wikimedia.org/wikipedia/commons/thumb/8/80/Some_log-normal_distributions.svg/500px-Some_log-normal_distributions.svg.png
     * 
     * <b><br>Source:</b>http://en.wikipedia.org/wiki/Log-normal_distribution<br><br>
     * 
     * 
     * @param x
     * @param meanLog
     * @param sdLog
     * @return 
     */
    template<class T>
    const T dlnorm(const T &x, const T &meanLog, const T &sdLog, bool ret_log = false) {
        if (x > T(0)) {
            T ret = (T(1) / (x * sdLog * pow(T(2) * T(M_PI), 0.5))) *
                    exp(T(-1)*((std::log(x) - meanLog)*
                    (log(x) - meanLog)) / (T(2) * sdLog));

            if (!ret_log) {
                return ret;
            } else {
                return log(ret);
            }
        }
        return T(0);
    }

    /**
     * @brief Computes the Gamma function using the Lanczos approximation.
     * 
     * \f[
     * \Gamma(z) \approx \sqrt{2 \pi} \left( z + \frac{g + 0.5}{z} \right)^{z + 0.5} e^{-(z + g + 0.5)} A_g(z)
     * \f]
     * 
     * Where \( A_g(z) \) is a series with coefficients.
     * 
     * @param z Input value.
     * @return T Value of the Gamma function at z.
     */
    template<typename T>
    T gamma(T z) {
        if (z <= 0) {
            throw std::invalid_argument("z must be greater than 0");
        }

        static const std::vector<T> coef = {
            676.5203681218851, -1259.1392167224028,
            771.32342877765313, -176.61502916214059,
            12.507343278686905, -0.13857109526572012,
            9.9843695780195716e-6, 1.5056327351493116e-7
        };

        static const T sqrt_two_pi = fims_math::sqrt(2 * M_PI);

        T x = 0.99999999999980993;
        for (size_t i = 0; i < coef.size(); ++i) {
            x += coef[i] / (z + i + 1);
        }

        T t = z + coef.size() - 0.5;
        return sqrt_two_pi * fims_math::pow(t, z + 0.5) * fims_math::exp(-t) * x;
    }

    template<class T>
    T lgamma(T x) {// x must be positive

        if (x <= 0.0) {
            std::stringstream os;
            os << "Invalid input argument " << x << ". Argument must be positive.";
            throw std::invalid_argument(os.str());
        }

        if (x < 12.0) {
            return fims_math::log(fims_math::ad_fabs(fims_math::gamma(x)));
        }

        // Abramowitz and Stegun 6.1.41
        // Asymptotic series should be good to at least 11 or 12 figures
        // For error analysis, see Whittiker and Watson
        // A Course in Modern Analysis (1927), page 252

        static const T c[8] = {
            1.0 / 12.0,
            -1.0 / 360.0,
            1.0 / 1260.0,
            -1.0 / 1680.0,
            1.0 / 1188.0,
            -691.0 / 360360.0,
            1.0 / 156.0,
            -3617.0 / 122400.0
        };
        T z = 1.0 / (x * x);
        T sum = c[7];
        for (int i = 6; i >= 0; i--) {
            sum *= z;
            sum += c[i];
        }
        T series = sum / x;


        static const T halfLogTwoPi = 0.91893853320467274178032973640562;
        T logGamma = (x - 0.5) * log(x) - x + halfLogTwoPi + series;
        return logGamma;
    }

    template<typename T>
    T gamma_p(T a, T x) {
        if (x < 0 || a <= 0) {
            return T(NAN); // Not a Number if input is invalid
        }

        if (x == 0) {
            return 0.0; // P(a, 0) is always 0
        }

        T result;
        if (x < a + 1.0) {
            // Series representation
            T sum = 1.0 / a;
            T term = 1.0 / a;
            for (int n = 1; n < 100; ++n) { // Number of iterations for series expansion
                term *= x / (a + n);
                sum += term;
                if (term < 1e-10) {
                    break;
                }
            }
            result = sum * exp(-x + a * log(x) - fims_math::lgamma(a));
        } else {
            // Use the continued fraction representation
            T b = x + 1.0 - a;
            T c = 1.0 / 1e-30;
            T d = 1.0 / b;
            T h = d;
            for (int i = 1; i < 100; ++i) {
                T an = -i * (i - a);
                b += 2.0;
                d = an * d + b;
                if (d == 0) d = 1e-30;
                c = b + an / c;
                if (c == 0) c = 1e-30;
                d = 1.0 / d;
                T del = d * c;
                h *= del;
                if (fims_math::ad_fabs(del - 1.0) < 1e-10) {
                    break;
                }
            }
            result = 1.0 - fims_math::exp(-x + a * fims_math::log(x) - fims_math::lgamma(a)) * h;
        }
        return result;
    }

    template<typename T>
    T LogGammaLanczos(const T& x) {
        // Log of Gamma from Lanczos with g=5, n=6/7
        // not in A & S 
        static const T coef[6] = {76.18009172947146,
            -86.50532032941677, 24.01409824083091,
            -1.231739572450155, 0.1208650973866179E-2,
            -0.5395239384953E-5};
        T LogSqrtTwoPi = T(0.91893853320467274178);
        T denom = x + T(1.0);
        T y = x + T(5.5);
        T series = T(1.000000000190015);
        for (int i = 0; i < 6; ++i) {
            series += coef[i] / denom;
            denom += 1.0;
        }
        return (LogSqrtTwoPi + (x + 0.5) * fims_math::log(y) -
                y + fims_math::log(series / x));
    }

    template<typename T>
    fims::Vector<T> LogGammaLanczos(const fims::Vector<T>& v) {
        fims::Vector<T> ret(v.size());
        for (int i = 0; i < v.size(); i++) {
            ret[i] = LogGammaLanczos(v[i]);
        }
        return ret;
    }

    template<typename T>
    T LogGammaSeries(const T& z) {
        // A & S 6.1.41 (Stirling's approximation)
        T x1 = (z - 0.5) * log(z);
        T x3 = 0.5 * fims_math::log(2.0 * M_PI);

        T x4 = 1 / (12 * z);
        T x5 = 1 / (360 * z * z * z);
        T x6 = 1 / (1260 * z * z * z * z * z);
        T x7 = 1 / (1680 * z * z * z * z * z * z * z);
        // more terms possible
        return x1 - z + x3 + x4 - x5 + x6 - x7;
    }

    template<typename T>
    fims::Vector<T> LogGammaSeries(const fims::Vector<T>& v) {
        fims::Vector<T> ret(v.size());
        for (int i = 0; i < v.size(); i++) {
            ret[i] = LogGammaSeries(v[i]);
        }
        return ret;
    }

    template<typename T>
    std::vector<T> lgamma(const std::vector<T>& v) {
        std::vector<T> ret(v.size());
        for (int i = 0; i < v.size(); i++) {
            ret[i] = lgamma(v[i]);
        }
        return ret;
    }

    template<typename T>
    fims::Vector<T> lgamma(const fims::Vector<T>& v) {
        fims::Vector<T> ret(v.size());
        for (int i = 0; i < v.size(); i++) {
            ret[i] = lgamma(v[i]);
        }
        return ret;
    }

    template<typename T>
    T lgamma_ad(T z) {
        if (z <= 0) {
            throw std::invalid_argument("z must be greater than 0");
        }

        static const T g = 7;
        static const std::vector<T> coef = {
            0.99999999999980993, 676.5203681218851,
            -1259.1392167224028, 771.32342877765313,
            -176.61502916214059, 12.507343278686905,
            -0.13857109526572012, 9.9843695780195716e-6,
            1.5056327351493116e-7
        };

        T x = coef[0];
        for (size_t i = 1; i < coef.size(); ++i) {
            x += coef[i] / (z + i - 1.0);
        }

        T t = z + g - 0.5;
        return (fims_math::log(2.0 * M_PI) / 2.0) + ((z - 0.5) * fims_math::log(t)) - t + fims_math::log(x);
    }

    template<typename T>
    fims::Vector<T> lgamma_ad(const fims::Vector<T>& v) {
        fims::Vector<T> ret(v.size());
        for (int i = 0; i < v.size(); i++) {
            ret[i] = lgamma_ad(v[i]);
        }
        return ret;
    }

    template<class T>
    T sum(const std::vector<T>& v) {
        T ret = 0.0;
        for (int i = 0; i < v.size(); i++) {
            ret += v[i];
        }
        return ret;
    }

    template<class T>
    T sum(const fims::Vector<T>& v) {
        T ret = 0.0;
        for (int i = 0; i < v.size(); i++) {
            ret += v[i];
        }
        return ret;
    }

    /**
     * Multinomial Probability Density function. p is internally normalized to sum 1.
     * 
     * @brief 
     * 
     * @param x
     * @param p
     * @param ret_log
     * @return 
     */
    template<typename T>
    T dmultinom(const fims::Vector<T>& x, fims::Vector<T> p, bool ret_log = false) {
        if (x.size() != p.size()) {
            throw std::invalid_argument("Size of x and p must be the same.");
        }

        T sum_x = fims_math::sum(x);
        T log_factorial_sum_x = fims_math::lgamma_ad(sum_x + 1.0); // log(sum(x)!)

        T log_factorial_x = 0;
        T sum_x_log_p = 0;

        for (size_t i = 0; i < x.size(); ++i) {
            if (p[i] <= 0 || p[i] >= 1) {
                throw std::invalid_argument("Probabilities must be between 0 and 1, exclusive.");
            }
            log_factorial_x += fims_math::lgamma_ad(x[i] + 1); // log(x_i!)
            sum_x_log_p += x[i] * fims_math::log(p[i]); // x_i * log(p_i)
        }
        T ret = log_factorial_sum_x - log_factorial_x + sum_x_log_p;

        if (ret_log) {
            return ret;
        } else {
            return fims_math::exp(ret);
        }
    }

    template <typename T>
    fims::Vector<int> rmultinom(const fims::Vector<T>& prob) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> dist(prob.begin(), prob.end());

        size_t n = prob.size();
        fims::Vector<int> result(prob.size(), 0);
        for (int i = 0; i < n; ++i) {
            int index = dist(gen);
            ++result[index];
        }

        return result;
    }



}  // namespace fims_math

#endif /* FIMS_MATH_HPP */
