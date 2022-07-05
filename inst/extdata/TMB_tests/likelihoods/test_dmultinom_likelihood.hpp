  /*! \file test_dmultinom_likelihood.hpp 
   * Specifies the negative log-likelihood of 
   * the dmultinom distribution given data and parameters. 
   * Creates singleton class that links this .hpp with 
   * the test_dmultinom_likelihood.cpp TMB verison of the model
   */
  #ifndef TEST_DMULTINOM_LIKELIHOOD_HPP
  #define TEST_DMULTINOM_LIKELIHOOD_HPP

  #include "../../../include/likelihoods/likelihoods.hpp"
  #include "../../../include/interface/interface.hpp"

  namespace fims {

    /**
     * @brief Model class defines and returns negative log-likelihood (nll)
     * 
     * @tparam T 
     */
    template <typename T>
    class Model {
      using Vector = typename ModelTraits<T>::EigenVector;
      public:
      Vector x; /*!< Vector of length K of integers */
      Vector p; /*!< Vector of length K, specifying the probability for the K classes (note, unlike in R these must sum to 1). */

      // Initiate pointer to link .cpp to .hpp
      static Model<T>* instance;
      
      /** @brief Constructor.
       */
      Model(){}

      /**
       * @brief Create new singleton class
       * 
       * @return Model<T>* 
       */
      static Model<T>* getInstance(){
        return Model<T>::instance;
      }

      /**
       * @brief Function that calculates the negative log-likelihood given the data and parameters
       * 
       * @return negative log-likelihood (nll)
       */
      T evaluate(){

        T nll = 0;
        fims::Dmultinom<T> nll_dmultinom;
        nll_dmultinom.x = x;
        nll_dmultinom.p = p;
        nll -= nll_dmultinom.evaluate(true);
        return nll;
      }
    };

    /**
     * @brief Create new instance of Model
     * 
     * @tparam T 
     */
    template<class T>
    Model<T>* Model<T>::instance = new Model<T>();
  }


  #endif  /* TEST_DMULTINOM_LIKELIHOOD_HPP */


