  /*! \file test_dnorm_likelihood.hpp 
   * Specifies the negative log-likelihood of 
   * the dnorm distribution given data and parameters. 
   * Creates singleton class that links this .hpp with 
   * the test_dnorm_likelihood.cpp TMB verison of the model
   */
  #ifndef TEST_DNORM_LIKELIHOOD_HPP
  #define TEST_DNORM_LIKELIHOOD_HPP

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
      using DataVector = typename ModelTraits<T>::DataVector;
      public:
      DataVector y; /*!< observation */
      T mean;  /*!< mean of the normal distribution */
      T sd; /*!< standard deviation of the normal distribution, must be strictly positive.*/

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
        int i;
        int n = y.size();
        fims::Dnorm<T> nll_dnorm;
        nll_dnorm.mean = mean;
        nll_dnorm.sd = sd;
        for(i =0; i < n; i++){
          nll_dnorm.x = y[i];
          nll -= nll_dnorm.evaluate(true);
        }
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


  #endif  /* TEST_DNORM_LIKELIHOOD_HPP */


