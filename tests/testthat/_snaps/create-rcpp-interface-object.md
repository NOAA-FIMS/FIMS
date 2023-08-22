# create_fims_rcpp_interface() works when evaluate_parameter is not null

    class DnormDistributionsInterface : public DistributionsInterfaceBase {
     public:
    
      Parameter x;
      Parameter mean;
      Parameter sd;
    
    
      DnormDistributionsInterface() : DistributionsInterfaceBase() {}
    
      virtual ~DnormDistributionsInterface() {}
    
    
      virtual uint32_t get_id() { return this->id; }
    
    
      virtual double evaluate(bool do_log) {
        fims_distributions::Dnorm<double> object;
        object.x = this->x.value;
        object.mean = this->mean.value;
        object.sd = this->sd.value;
        return object.evaluate(bool);
      }
    
    
      virtual bool add_to_fims_tmb(){
       std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE> > d0 =
        fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
    
       std::shared_ptr<fims_distributions::Dnorm<TMB_FIMS_REAL_TYPE> > model0 =
       std::make_shared<fims_distributions::Dnorm<TMB_FIMS_REAL_TYPE> >();
    
    
       model0->id = this->id;
       model0->x= this->x.value;
       if (this->x.estimated) {
            if (this->x.is_random_effect) {
              d0->RegisterRandomEffect(model0->x);
       } else {
          d0->RegisterParameter(model0->x);
       }
    } 
    model0->mean= this->mean.value;
       if (this->mean.estimated) {
            if (this->mean.is_random_effect) {
              d0->RegisterRandomEffect(model0->mean);
       } else {
          d0->RegisterParameter(model0->mean);
       }
    } 
    model0->sd= this->sd.value;
       if (this->sd.estimated) {
            if (this->sd.is_random_effect) {
              d0->RegisterRandomEffect(model0->sd);
       } else {
          d0->RegisterParameter(model0->sd);
       }
    } 
       d0->distribution_models[model0->id]=model0;
    
    
       std::shared_ptr<fims_info::Information<TMB_FIMS_FIRST_ORDER> > d1 =
        fims_info::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();
    
       std::shared_ptr<fims_distributions::Dnorm<TMB_FIMS_FIRST_ORDER> > model1 =
       std::make_shared<fims_distributions::Dnorm<TMB_FIMS_FIRST_ORDER> >();
    
    
       model1->id = this->id;
       model1->x= this->x.value;
       if (this->x.estimated) {
            if (this->x.is_random_effect) {
              d1->RegisterRandomEffect(model1->x);
       } else {
          d1->RegisterParameter(model1->x);
       }
    } 
    model1->mean= this->mean.value;
       if (this->mean.estimated) {
            if (this->mean.is_random_effect) {
              d1->RegisterRandomEffect(model1->mean);
       } else {
          d1->RegisterParameter(model1->mean);
       }
    } 
    model1->sd= this->sd.value;
       if (this->sd.estimated) {
            if (this->sd.is_random_effect) {
              d1->RegisterRandomEffect(model1->sd);
       } else {
          d1->RegisterParameter(model1->sd);
       }
    } 
       d1->distribution_models[model1->id]=model1;
    
    
       std::shared_ptr<fims_info::Information<TMB_FIMS_SECOND_ORDER> > d2 =
        fims_info::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();
    
       std::shared_ptr<fims_distributions::Dnorm<TMB_FIMS_SECOND_ORDER> > model2 =
       std::make_shared<fims_distributions::Dnorm<TMB_FIMS_SECOND_ORDER> >();
    
    
       model2->id = this->id;
       model2->x= this->x.value;
       if (this->x.estimated) {
            if (this->x.is_random_effect) {
              d2->RegisterRandomEffect(model2->x);
       } else {
          d2->RegisterParameter(model2->x);
       }
    } 
    model2->mean= this->mean.value;
       if (this->mean.estimated) {
            if (this->mean.is_random_effect) {
              d2->RegisterRandomEffect(model2->mean);
       } else {
          d2->RegisterParameter(model2->mean);
       }
    } 
    model2->sd= this->sd.value;
       if (this->sd.estimated) {
            if (this->sd.is_random_effect) {
              d2->RegisterRandomEffect(model2->sd);
       } else {
          d2->RegisterParameter(model2->sd);
       }
    } 
       d2->distribution_models[model2->id]=model2;
    
    
       std::shared_ptr<fims_info::Information<TMB_FIMS_THIRD_ORDER> > d3 =
        fims_info::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();
    
       std::shared_ptr<fims_distributions::Dnorm<TMB_FIMS_THIRD_ORDER> > model3 =
       std::make_shared<fims_distributions::Dnorm<TMB_FIMS_THIRD_ORDER> >();
    
    
       model3->id = this->id;
       model3->x= this->x.value;
       if (this->x.estimated) {
            if (this->x.is_random_effect) {
              d3->RegisterRandomEffect(model3->x);
       } else {
          d3->RegisterParameter(model3->x);
       }
    } 
    model3->mean= this->mean.value;
       if (this->mean.estimated) {
            if (this->mean.is_random_effect) {
              d3->RegisterRandomEffect(model3->mean);
       } else {
          d3->RegisterParameter(model3->mean);
       }
    } 
    model3->sd= this->sd.value;
       if (this->sd.estimated) {
            if (this->sd.is_random_effect) {
              d3->RegisterRandomEffect(model3->sd);
       } else {
          d3->RegisterParameter(model3->sd);
       }
    } 
       d3->distribution_models[model3->id]=model3;
    
    
    return true;
    
    
    }
    
    };
    
    //Add the following to the RCpp module definition: rcpp_interface.hpp
    
    Rcpp::class_<DnormDistributionsInterface>("DnormDistributions")
    .constructor()
    .method("get_id",  &DnormDistributionsInterface::get_id)
    .method("evaluate", & DnormDistributionsInterface ::evaluate)
    .field("x", &DnormDistributionsInterface::x)
    .field("mean", &DnormDistributionsInterface::mean)
    .field("sd", &DnormDistributionsInterface::sd);

# create_fims_rcpp_interface() works when evaluate_parameter is null

    class LogisticSelectivityInterface : public SelectivityInterfaceBase {
     public:
    
      Parameter slope;
      Parameter median;
    
    
      LogisticSelectivityInterface() : SelectivityInterfaceBase() {}
    
      virtual ~LogisticSelectivityInterface() {}
    
    
      virtual uint32_t get_id() { return this->id; }
    
    
      virtual bool add_to_fims_tmb(){
       std::shared_ptr<fims_info::Information<TMB_FIMS_REAL_TYPE> > d0 =
        fims_info::Information<TMB_FIMS_REAL_TYPE>::GetInstance();
    
       std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_REAL_TYPE> > model0 =
       std::make_shared<fims::LogisticSelectivity<TMB_FIMS_REAL_TYPE> >();
    
    
       model0->id = this->id;
       model0->slope= this->slope.value;
       if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
              d0->RegisterRandomEffect(model0->slope);
       } else {
          d0->RegisterParameter(model0->slope);
       }
    } 
    model0->median= this->median.value;
       if (this->median.estimated) {
            if (this->median.is_random_effect) {
              d0->RegisterRandomEffect(model0->median);
       } else {
          d0->RegisterParameter(model0->median);
       }
    } 
       d0->selectivity_models[model0->id]=model0;
    
    
       std::shared_ptr<fims_info::Information<TMB_FIMS_FIRST_ORDER> > d1 =
        fims_info::Information<TMB_FIMS_FIRST_ORDER>::GetInstance();
    
       std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_FIRST_ORDER> > model1 =
       std::make_shared<fims::LogisticSelectivity<TMB_FIMS_FIRST_ORDER> >();
    
    
       model1->id = this->id;
       model1->slope= this->slope.value;
       if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
              d1->RegisterRandomEffect(model1->slope);
       } else {
          d1->RegisterParameter(model1->slope);
       }
    } 
    model1->median= this->median.value;
       if (this->median.estimated) {
            if (this->median.is_random_effect) {
              d1->RegisterRandomEffect(model1->median);
       } else {
          d1->RegisterParameter(model1->median);
       }
    } 
       d1->selectivity_models[model1->id]=model1;
    
    
       std::shared_ptr<fims_info::Information<TMB_FIMS_SECOND_ORDER> > d2 =
        fims_info::Information<TMB_FIMS_SECOND_ORDER>::GetInstance();
    
       std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_SECOND_ORDER> > model2 =
       std::make_shared<fims::LogisticSelectivity<TMB_FIMS_SECOND_ORDER> >();
    
    
       model2->id = this->id;
       model2->slope= this->slope.value;
       if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
              d2->RegisterRandomEffect(model2->slope);
       } else {
          d2->RegisterParameter(model2->slope);
       }
    } 
    model2->median= this->median.value;
       if (this->median.estimated) {
            if (this->median.is_random_effect) {
              d2->RegisterRandomEffect(model2->median);
       } else {
          d2->RegisterParameter(model2->median);
       }
    } 
       d2->selectivity_models[model2->id]=model2;
    
    
       std::shared_ptr<fims_info::Information<TMB_FIMS_THIRD_ORDER> > d3 =
        fims_info::Information<TMB_FIMS_THIRD_ORDER>::GetInstance();
    
       std::shared_ptr<fims::LogisticSelectivity<TMB_FIMS_THIRD_ORDER> > model3 =
       std::make_shared<fims::LogisticSelectivity<TMB_FIMS_THIRD_ORDER> >();
    
    
       model3->id = this->id;
       model3->slope= this->slope.value;
       if (this->slope.estimated) {
            if (this->slope.is_random_effect) {
              d3->RegisterRandomEffect(model3->slope);
       } else {
          d3->RegisterParameter(model3->slope);
       }
    } 
    model3->median= this->median.value;
       if (this->median.estimated) {
            if (this->median.is_random_effect) {
              d3->RegisterRandomEffect(model3->median);
       } else {
          d3->RegisterParameter(model3->median);
       }
    } 
       d3->selectivity_models[model3->id]=model3;
    
    
    return true;
    
    
    }
    
    };
    
    //Add the following to the RCpp module definition: rcpp_interface.hpp
    
    Rcpp::class_<LogisticSelectivityInterface>("LogisticSelectivity")
    .constructor()
    .method("get_id",  &LogisticSelectivityInterface::get_id)
    .field("slope", &LogisticSelectivityInterface::slope)
    .field("median", &LogisticSelectivityInterface::median);

