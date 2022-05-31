  #ifndef TEST_DNORM_LIKELIHOOD_HPP
  #define TEST_DNORM_LIKELIHOOD_HPP

  #include "../../../inst/include/likelihoods/likelihoods.hpp"
  #include "../../../inst/include/interface/interface.hpp"

  namespace fims {
    template <typename T>
    class Model {
      using DataVector = typename ModelTraits<T>::DataVector;
      public:
      DataVector y;
      T mean;
      T sd;

      static Model<T>* instance;

      Model(){}

      static Model<T>* getInstance(){
        return Model<T>::instance;
      }

      T evaluate(){

        T nll = 0;
        int i;
        int n = y.size();
        fims::Dnorm<T> nll_dnorm;
        nll_dnorm.sd = sd;
        for(i =0; i < n; i++){
          nll -= nll_dnorm.evaluate(y[i],mean,true);
        }
        return nll;
      }
    };

    template<class T>
    Model<T>* Model<T>::instance = new Model<T>();
  }


  #endif  /* TEST_DNORM_LIKELIHOOD_HPP */


