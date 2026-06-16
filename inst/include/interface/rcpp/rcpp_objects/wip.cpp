/**
 * @brief Rcpp interface for DSEM recruitment.
 */
class DSEMRecruitmentInterface : public RecruitmentInterfaceBase {
public:
    /** @brief Vector of path coefficients inherited from FIMSObject logic. */
    ParameterVector beta_z;

    DSEMRecruitmentInterface() : RecruitmentInterfaceBase() {
        std::shared_ptr<DSEMRecruitmentInterface> dsem_rec = 
            std::make_shared<DSEMRecruitmentInterface>(*this);
        FIMSRcppInterfaceBase::fims_interface_objects.push_back(dsem_rec);
        RecruitmentInterfaceBase::live_objects[this->id] = dsem_rec;
    }

    DSEMRecruitmentInterface(const DSEMRecruitmentInterface& other)
        : RecruitmentInterfaceBase(other), beta_z(other.beta_z) {}

    virtual ~DSEMRecruitmentInterface() {}
    virtual uint32_t get_id() { return this->id; }

    virtual double evaluate_mean(double spawners, double ssb0) { return 0.0; }
    virtual double evaluate_process(size_t pos) { return 0.0; }

#ifdef TMB_MODEL
    template <typename Type>
    bool add_to_fims_tmb_internal() {
        std::shared_ptr<fims_info::Information<Type>> info = 
            fims_info::Information<Type>::GetInstance();
        std::shared_ptr<fims_popdy::DSEMRecruitment<Type>> recruitment = 
            std::make_shared<fims_popdy::DSEMRecruitment<Type>>();
        
        recruitment->id = this->id;
        recruitment->beta_z.resize(this->beta_z.size());

        for (size_t i = 0; i < this->beta_z.size(); ++i) {
            recruitment->beta_z[i] = static_cast<Type>(this->beta_z[i].initial_value_m);
            std::string label = "beta_z_rec_" + std::to_string(this->id) + "_" + std::to_string(i);
            
            if (this->beta_z[i].estimation_type_m == "fixed_effects") {
                info->RegisterParameter(recruitment->beta_z[i]);
                info->RegisterParameterName(label);
            } else if (this->beta_z[i].estimation_type_m == "random_effects") {
                info->RegisterRandomEffect(recruitment->beta_z[i]);
                info->RegisterRandomEffectName(label);
            }
        }

        // Connect the R-facing ID to the C++ memory location in the global map [Plan]
        info->variable_map[this->beta_z.get_id()] = &recruitment->beta_z;
        info->recruitment_models[this->id] = recruitment;
        return true;
    }

    virtual bool add_to_fims_tmb() {
        this->add_to_fims_tmb_internal<double>();
        this->add_to_fims_tmb_internal<TMBad::ad_aug>();
        return true;
    }
#endif

    virtual void finalize() {
        if (this->finalized) return;
        std::shared_ptr<fims_info::Information<double>> info = 
            fims_info::Information<double>::GetInstance();
        auto it = info->recruitment_models.find(this->id);
        if (it != info->recruitment_models.end()) {
            auto rec = std::dynamic_pointer_cast<fims_popdy::DSEMRecruitment<double>>(it->second);
            for (size_t i = 0; i < this->beta_z.size(); ++i) {
                this->beta_z[i].final_value_m = rec->beta_z[i];
            }
        }
        this->finalized = true;
    }
};