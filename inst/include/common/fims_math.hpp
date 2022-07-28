/*! \file fims_math.hpp
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

#include "../interface/interface.hpp"

namespace fims {
#ifdef STD_LIB
/**
 * @brief The exponential function.
 *
 * @param x value to exponentiate. Please use fims::exp<double>(x) if x is an
 * integer.
 * @return the exponentiated value
 */
template <class T>
inline const T exp(const T &x) {
  return std::exp(x);
}

/**
 * @brief The natural log function (base e)
 * @param x the value to take the log of. Please use fims::log<double>(x) if x
 * is an integer.
 * @return
 */
template <class T>
inline const T log(const T &x) {
  return std::log(x);
}
#endif

#ifdef TMB_MODEL
//#include <TMB.hpp>

/**
 * @brief The exponential function.
 * The code cannot be tested using the compilation flag
 * -DTMB_MODEL through CMake and Google Test
 * @param x value to exponentiate. Please use fims::exp<double>(x) if x is an
 * integer.
 * @return the exponentiated value
 */
template <class T>
inline const T exp(const T &x) {
  return exp(x);
}

/**
 * @brief The natural log function (base e)
 * The code cannot be tested using the compilation flag
 * -DTMB_MODEL through CMake and Google Test.
 * @param x the value to take the log of. Please use fims::log<double>(x) if x
 * is an integer.
 * @return the log of the value
 */
template <class T>
inline const T log(const T &x) {
  return log(x);
}

#endif

/**
 * @brief The general logistic function
 *
 * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope (x - median))} \f$
 *
 * @param median the median (inflection point) of the logistic function
 * @param slope the slope of the logistic function
 * @param x the index the logistic function should be evaluated at
 * @return
 */
template <class T>
inline const T logistic(const T &median, const T &slope, const T &x) {
  return (1.0) / (1.0 + exp(-1.0 * slope * (x - median)));
}

/**
 * @brief The general double logistic function
 *
 * \f$ \frac{1.0}{ 1.0 + exp(-1.0 * slope_{asc} (x - median_{asc}))}
 * \left(1-\frac{1.0}{ 1.0 + exp(-1.0 * slope_{desc} (x - median_{desc}))}
 * \right)\f$
 *
 * @param median_asc the median (inflection point) of the ascending limb of the
 * double logistic function
 * @param slope_asc the slope of the ascending limb of the double logistic
 * function
 * @param median_desc the median (inflection point) of the descending limb of
 * the double logistic function, where median_desc > median_asc
 * @param slope_desc the slope of the descending limb of the double logistic
 * function
 * @param x the index the logistic function should be evaluated at
 * @return
 */

template <class T>
inline const T double_logistic(const T &median_asc, const T &slope_asc,
                               const T &median_desc, const T &slope_desc,
                               const T &x) {
  return (1.0) / (1.0 + exp(-1.0 * slope_asc * (x - median_asc))) *
         (1.0 - (1.0) / (1.0 + exp(-1.0 * slope_desc * (x - median_desc))));
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
template <class T>
const T ad_fabs(const T &x, T C = 1e-5) {
  return sqrt((x * x) + C);  //, .5);
}

/**
 * Returns the minimum between a and b in a continuous manner using:
 *
 * (a + b - \ref fims::ad_fabs(a - b))*.5;
 *
 * This is an approximation with minimal error.
 *
 * @param a
 * @param b
 * @param C default = 1e-5
 * @return
 */
template <typename T>
inline const T ad_min(const T &a, const T &b, T C = 1e-5) {
  return (a + b - fims::ad_fabs(a - b, C)) * .5;
}

/**
 * Returns the maximum between a and b in a continuous manner using:
 *
 * (a + b + \ref fims::ad_fabs(a - b)) *.5;
 *
 * This is an approximation with minimal error.
 *
 * @param a
 * @param b
 * @param C default = 1e-5
 * @return
 */
template <typename T>
inline const T ad_max(const T &a, const T &b, T C = 1e-5) {
  return (a + b + fims::ad_fabs(a - b, C)) * static_cast<T>(.5);
}
  
  template<class T>
T gamma(T x);

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
    T ret = (T(1.0) / (sd * pow(T(2.0) * T(M_PI), 0.5))) *
            exp((T(-1.0)*(x - mean)*(x - mean))
            / (T(2.0) * sd * sd));

    if (!ret_log) {
        return ret;
    } else {
        return log(ret);
    }
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

template<class T>
T gamma(T x) { // We require x > 0

    if (x <= 0.0) {
        std::stringstream os;
        os << "Invalid input argument " << x << ". Argument must be positive.";
        throw std::invalid_argument(os.str());
    }

    // Split the function domain into three intervals:
    // (0, 0.001), [0.001, 12), and (12, infinity)

    ///////////////////////////////////////////////////////////////////////////
    // First interval: (0, 0.001)
    //
    // For small x, 1/Gamma(x) has power series x + gamma x^2  - ...
    // So in this range, 1/Gamma(x) = x + gamma x^2 with error on the order of x^3.
    // The relative error over this interval is less than 6e-7.

    const T gamma = 0.577215664901532860606512090; // Euler's gamma constant

    if (x < 0.001)
        return 1.0 / (x * (1.0 + gamma * x));

    ///////////////////////////////////////////////////////////////////////////
    // Second interval: [0.001, 12)

    if (x < 12.0) {
        // The algorithm directly approximates gamma over (1,2) and uses
        // reduction identities to reduce other arguments to this interval.

        T y = x;
        int n = 0;
        bool arg_was_less_than_one = (y < 1.0);

        // Add or subtract integers as necessary to bring y into (1,2)
        // Will correct for this below
        if (arg_was_less_than_one) {
            y += 1.0;
        } else {
            n = static_cast<int> (floor(y)) - 1; // will use n later
            y -= n;
        }

        // numerator coefficients for approximation over the interval (1,2)
        static const T p[] = {
            -1.71618513886549492533811E+0,
            2.47656508055759199108314E+1,
            -3.79804256470945635097577E+2,
            6.29331155312818442661052E+2,
            8.66966202790413211295064E+2,
            -3.14512729688483675254357E+4,
            -3.61444134186911729807069E+4,
            6.64561438202405440627855E+4
        };

        // denominator coefficients for approximation over the interval (1,2)
        static const T q[] = {
            -3.08402300119738975254353E+1,
            3.15350626979604161529144E+2,
            -1.01515636749021914166146E+3,
            -3.10777167157231109440444E+3,
            2.25381184209801510330112E+4,
            4.75584627752788110767815E+3,
            -1.34659959864969306392456E+5,
            -1.15132259675553483497211E+5
        };

        T num = 0.0;
        T den = 1.0;
        int i;

        T z = y - 1;
        for (i = 0; i < 8; i++) {
            num = (num + p[i]) * z;
            den = den * z + q[i];
        }
        T result = num / den + 1.0;

        // Apply correction if argument was not initially in (1,2)
        if (arg_was_less_than_one) {
            // Use identity gamma(z) = gamma(z+1)/z
            // The variable "result" now holds gamma of the original y + 1
            // Thus we use y-1 to get back the orginal y.
            result /= (y - 1.0);
        } else {
            // Use the identity gamma(z+n) = z*(z+1)* ... *(z+n-1)*gamma(z)
            for (i = 0; i < n; i++)
                result *= y++;
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Third interval: [12, infinity)

    if (x > 171.624) {
        // Correct answer too large to display. Force +infinity.
        T temp = std::numeric_limits<double>::max();
        return temp * 2.0;
    }

    return exp(lgamma(x));
}

template<class T>
T lgamma(T x) {// x must be positive

    if (x <= 0.0) {
        std::stringstream os;
        os << "Invalid input argument " << x << ". Argument must be positive.";
        throw std::invalid_argument(os.str());
    }

    if (x < 12.0) {
        return log(fabs(gamma(x)));
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
T LogGammaLanczos(T x) {
    // Log of Gamma from Lanczos with g=5, n=6/7
    // not in A & S 
    static const T coef[6] = {76.18009172947146,
        -86.50532032941677, 24.01409824083091,
        -1.231739572450155, 0.1208650973866179E-2,
        -0.5395239384953E-5};
    T LogSqrtTwoPi = 0.91893853320467274178;
    T denom = x + 1;
    T y = x + 5.5;
    T series = 1.000000000190015;
    for (int i = 0; i < 6; ++i) {
        series += coef[i] / denom;
        denom += 1.0;
    }
    return (LogSqrtTwoPi + (x + 0.5) * log(y) -
            y + log(series / x));
}



template<typename T>
T LogGammaSeries(T z) {
    // A & S 6.1.41 (Stirling's approximation)
    T x1 = (z - 0.5) * log(z);
    T x3 = 0.5 * log(2 * M_PI);

    T x4 = 1 / (12 * z);
    T x5 = 1 / (360 * z * z * z);
    T x6 = 1 / (1260 * z * z * z * z * z);
    T x7 = 1 / (1680 * z * z * z * z * z * z * z);
    // more terms possible
    return x1 - z + x3 + x4 - x5 + x6 - x7;
}

template<typename T>
std::vector<T> lgamma(const std::vector<T>& v) {
    std::vector<T> ret(v.size());
    for (int i = 0; i < v.size(); i++) {
        ret[i] = lgamma(v[i]);
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
template <class T>
T dmultinom(std::vector<T> x, std::vector<T> p, bool ret_log = false) {


    std::vector<T> ret2(x.size());
    std::vector<T> xp1(x.size());
    T sum_p = 0.0;
    //normalize p
    for (int i = 0; i < x.size(); i++) {
        sum_p += p[i];
    }

    for (int i = 0; i < x.size(); i++) {
        p[i] /= sum_p;
        xp1[i] = x[i] + 1.0;
        ret2[i] = x[i] * std::log(p[i]);

    }


    T ret = lgamma(sum(x) + 1.0) - sum(lgamma(xp1)) + sum(ret2);

    if (ret_log) {
        return ret;
    } else {
        return exp(ret);
    }

}


}  // namespace fims

#endif /* FIMS_MATH_HPP */
