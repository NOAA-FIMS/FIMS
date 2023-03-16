  /*! \file test_dmultinom_distribution.hpp
   * Specifies the negative log-likelihood of
   * the dmultinom distribution given data and parameters.
   * Creates singleton class that links this .hpp with
   * the test_dmultinom_distribution.cpp TMB verison of the model
   */
  #ifndef TEST_FLEET_ACOMP_NLL_HPP
  #define TEST_FLEET_ACOMP_NLL_HPP

  #include "../../../include/population_dynamics/fleet/fleet_nll.hpp"
  #include "../../../include/interface/interface.hpp"
  #include "../../../include/population_dynamics/population/population.hpp"

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
        int n = x.size();
        int year = 4;
        int age = 6;
        fims::Population<T> population;

        int  nyears = 10;
        int nseasons = 1;
        int nages = 10;
        int nfleets = 2;

        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;
        population.id_g = 0;
    population.nyears = nyears;
    population.nseasons = nseasons;
    population.nages = nages;
    for (int i = 0; i < nfleets; i++) {
      auto fleet = std::make_shared<fims::Fleet<T>>();
      population.fleets.push_back(fleet);
    }

    // Does Fmort need to be in side of the year loop like log_q?
    for (int i = 0; i < nfleets; i++) {
      auto fleet = std::make_shared<fims::Fleet<T>>();
      fleet->Initialize(nyears, nages);
      for (int year = 0; year < nyears; year++) {
        fleet->log_Fmort[year] = 0.4;
        fleet->log_q[year] = 0.1;
      }
      fleet->Prepare();
      population.fleets.push_back(fleet);
    }

    for (int i = 0; i < nages; i++) {
      population.log_init_naa[i] = 10.0;
    }
    for (int i = 0; i < nyears * nages; i++) {
      population.log_M[i] = 0.2;
    }

    for (int i = 0; i < (nyears + 1) * nages; i++) {
      population.numbers_at_age[i] = fims::exp(5.0);
    }

    for (int i = 0; i < nages; i++) {
      population.weight_at_age[i] = 0.1;
    }

    population.Prepare();

    // Make a shared pointer to selectivity and fleet because
    // fleet object needs a shared pointer in fleet.hpp
    // (std::shared_ptr<fims::SelectivityBase<Type> > selectivity;)
    // and population object needs a shared pointer in population.hpp
    // (std::vector<std::shared_ptr<fims::Fleet<Type> > > fleets;)
    for (int i = 0; i < population.nfleets; i++) {
      auto selectivity = std::make_shared<fims::LogisticSelectivity<T>>();
      selectivity->median = 7;
      selectivity->slope = 0.5;

      auto fleet = std::make_shared<fims::Fleet<T>>();
      fleet->Initialize(nyears, nages);
      fleet->selectivity = selectivity;
      population.fleets[i] = fleet;
    }

    auto maturity = std::make_shared<fims::LogisticMaturity<T>>();
    maturity->median = 6;
    maturity->slope = 0.15;
    population.maturity = maturity;

    auto recruitment = std::make_shared<fims::SRBevertonHolt<T>>();
    recruitment->steep = 0.75;
    recruitment->rzero = 1000000.0;
    recruitment->recruit_deviations.resize(nyears);
    population.recruitment = recruitment;
      
    

        // calculate catch numbers at age in population module
        // Ian: not sure which of these are needed
        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2, age);
        fims::FleetAgeCompNLL<T> nll_fac;
        nll_fac.catch_numbers_at_age.resize(n);
        nll_fac.nyears = 1;
        nll_fac.nages = 10;
        std::shared_ptr<fims::DataObject<T>> age_comp_data =
        std::make_shared<fims::DataObject<T>>(10, 1);

        nll_fac.observed_agecomp_data = age_comp_data;
        for(int i =0; i < n; i++){
          nll_fac.observed_agecomp_data->at(i) = x[i];
          nll_fac.catch_numbers_at_age[i] = p[i];
        }
        nll -= nll_fac.evaluate();
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


  #endif  /* TEST_DMULTINOM_DISTRIBUTION_HPP */


