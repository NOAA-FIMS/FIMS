  #ifndef TEST_DLNORM_LIKELIHOOD_HPP
  #define TEST_DLNORM_LIKELIHOOD_HPP

  #include "../../../include/likelihoods/likelihoods.hpp"
  #include "../../../include/interface/interface.hpp"

  namespace fims {
    template <typename T>
    class Model {
      using DataVector = typename ModelTraits<T>::DataVector;
      public:
      DataVector logy;
      T meanlog;
      T sdlog;

      static Model<T>* instance;

      Model(){}

      static Model<T>* getInstance(){
        return Model<T>::instance;
      }

      T evaluate(){

        T nll = 0;
        int i;
        int n = logy.size();
        fims::Dlnorm<T> nll_dlnorm;
        nll_dlnorm.sdlog = sdlog;
        for(i =0; i < n; i++){
          nll -= nll_dlnorm.evaluate(logy[i],meanlog,true);
        }
        return nll;
      }
    };

    template<class T>
    Model<T>* Model<T>::instance = new Model<T>();
  }


  #endif  /* TEST_DLNORM_LIKELIHOOD_HPP */


