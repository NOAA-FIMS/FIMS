/**
 * @file model.hpp
 * @brief TODO: provide a brief description.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_COMMON_MODEL_HPP
#define FIMS_COMMON_MODEL_HPP

#include <future>
#include <memory>

#include "information.hpp"

namespace fims_model {

    /**
     * @brief Model class. FIMS objective function.
     */
    template <typename Type>
    class Model { // may need singleton
    public:
        static std::shared_ptr<Model<Type> >
        fims_model; /**< Create a shared fims_model as a pointer to Model*/
        std::shared_ptr<fims_info::Information<Type> >
        fims_information; /**< Create a shared fims_information as a pointer to
                           Information*/

#ifdef TMB_MODEL
        bool do_tmb_reporting = true;
        ::objective_function<Type> *of;
#endif

        // constructor

        virtual ~Model() {
        }

        /**
         * Returns a single Information object for type Type.
         *
         * @return singleton for type Type
         */
        static std::shared_ptr<Model<Type> > GetInstance() {
            if (Model<Type>::fims_model == nullptr) {
                Model<Type>::fims_model = std::make_shared<fims_model::Model<Type> >();
                Model<Type>::fims_model->fims_information =
                        fims_info::Information<Type>::GetInstance();
            }
            return Model<Type>::fims_model;
        }

        /**
         * @brief Evaluate. Calculates the joint negative log-likelihood function.
         */
        const Type Evaluate() {
            // jnll = negative-log-likelihood (the objective function)
            Type jnll = static_cast<Type>(0.0);


            int n_fleets = fims_information->fleets.size();
            int n_pops = fims_information->populations.size();

            // Create vector lists to store output for reporting
#ifdef TMB_MODEL
            // vector< vector<Type> > creates a nested vector structure where
            // each vector can be a different dimension. Does not work with ADREPORT
            // fleets
            vector<vector<Type> > landings_w(n_fleets);
            vector<vector<Type> > landings_n(n_fleets);
            vector<vector<Type> > landings_exp(n_fleets);
            vector<vector<Type> > landings_naa(n_fleets);
            vector<vector<Type> > landings_waa(n_fleets);
            vector<vector<Type> > landings_nal(n_fleets);
            vector<vector<Type> > index_w(n_fleets);
            vector<vector<Type> > index_n(n_fleets);
            vector<vector<Type> > index_exp(n_fleets);
            vector<vector<Type> > index_naa(n_fleets);
            vector<vector<Type> > index_nal(n_fleets);
            vector<vector<Type> > comp_naa(n_fleets);
            vector<vector<Type> > comp_nal(n_fleets);
            vector<vector<Type> > comp_pnaa(n_fleets);
            vector<vector<Type> > comp_pnal(n_fleets);
            vector<vector<Type> > F_mort(n_fleets);
            vector<vector<Type> > q(n_fleets);
            // populations
            vector<vector<Type> > naa(n_pops);
            vector<vector<Type> > ssb(n_pops);
            vector<vector<Type> > total_landings_w(n_pops);
            vector<vector<Type> > total_landings_n(n_pops);
            vector<vector<Type> > biomass(n_pops);
            vector<vector<Type> > log_recruit_dev(n_pops);
            vector<vector<Type> > recruitment(n_pops);
            vector<vector<Type> > M(n_pops);
            vector<Type> nll_components(this->fims_information->density_components.size());
#endif
            // Loop over densities and evaluate joint negative log densities for priors
            typename fims_info::Information<Type>::density_components_iterator d_it;
            nll_components.fill(0);
            int nll_components_idx = 0;
            size_t n_priors = 0;
            FIMS_INFO_LOG("Begin evaluating prior densities.")
            for (d_it = this->fims_information->density_components.begin();
                    d_it != this->fims_information->density_components.end(); ++d_it) {
                std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*d_it).second;
#ifdef TMB_MODEL
                d->of = this->of;
#endif
                if (d->input_type == "prior") {
                    nll_components[nll_components_idx] = -d->evaluate();
                    jnll += nll_components[nll_components_idx];
                    n_priors += 1;
                    nll_components_idx += 1;
                }
            }



            // Loop over populations and evaluate recruitment component

            typename fims_info::Information<Type>::population_iterator p_it;


            for (p_it = this->fims_information->populations.begin();
                    p_it != this->fims_information->populations.end(); ++p_it) {
                //(*p_it).second points to the Population module
                std::shared_ptr<fims_popdy::Population<Type> > p = (*p_it).second;

                // Prepare recruitment
                p->recruitment->Prepare();
                FIMS_INFO_LOG("Recruitmnt successfully prepared.")

            }

            // Loop over densities and evaluate joint negative log-likelihoods for random effects
            this->fims_information->SetupRandomEffects();
            size_t n_random_effects = 0;
            for (d_it = this->fims_information->density_components.begin();
                    d_it != this->fims_information->density_components.end(); ++d_it) {
                std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*d_it).second;
#ifdef TMB_MODEL
                d->of = this->of;
#endif
                if (d->input_type == "random_effects") {
                    nll_components[nll_components_idx] = -d->evaluate();
                    jnll += nll_components[nll_components_idx];
                    n_random_effects += 1;
                    nll_components_idx += 1;
                }
            }


            // Loop over and evaluate populations
            for (p_it = this->fims_information->populations.begin();
                    p_it != this->fims_information->populations.end(); ++p_it) {
                //(*p_it).second points to the Population module
                std::shared_ptr<fims_popdy::Population<Type> > p = (*p_it).second;
                // link to TMB objective function
#ifdef TMB_MODEL
                p->of = this->of;
#endif
                // Evaluate population
                FIMS_INFO_LOG("Begin evaluation for population " + fims::to_string(p->id));
                p->Evaluate();
                FIMS_INFO_LOG("Population successfully evaluated");
            }

            typename fims_info::Information<Type>::fleet_iterator f_it;
            // Loop over fleets/surveys, and evaluate age comp and index expected values
            for (f_it = this->fims_information->fleets.begin();
                    f_it != this->fims_information->fleets.end(); ++f_it) {
                //(*f_it).second points to each individual Fleet module
                std::shared_ptr<fims_popdy::Fleet<Type> > f = (*f_it).second;
#ifdef TMB_MODEL
                f->of = this->of;
#endif
                FIMS_INFO_LOG("Begin evalulation for fleet "+fims::to_string(f->id));
                f->evaluate_age_comp();
                if (f->nlengths > 0) {
                  f->evaluate_length_comp();
                }
                FIMS_INFO_LOG("Begin evalulation of landings for fleet "+fims::to_string(f->id));
                f->evaluate_landings();
                FIMS_INFO_LOG("Begin evalulation of index for fleet "+fims::to_string(f->id));
                f->evaluate_index();
            }
            this->fims_information->SetupData();
            // Loop over and evaluate data joint negative log-likelihoods
            int n_data = 0;
            for (d_it = this->fims_information->density_components.begin();
                    d_it != this->fims_information->density_components.end(); ++d_it) {
                std::shared_ptr<fims_distributions::DensityComponentBase<Type> > d = (*d_it).second;
#ifdef TMB_MODEL
                d->of = this->of;
                //d->keep = this->keep;
#endif
                if (d->input_type == "data") {
                    nll_components[nll_components_idx] = -d->evaluate();
                    jnll += nll_components[nll_components_idx];
                    n_data += 1;
                    nll_components_idx += 1;
                }
            }

            // initiate population index for structuring report out objects
            int pop_idx = 0;
            for (p_it = this->fims_information->populations.begin();
                    p_it != this->fims_information->populations.end(); ++p_it) {
                std::shared_ptr<fims_popdy::Population<Type> > p = (*p_it).second;
#ifdef TMB_MODEL
                naa(pop_idx) = vector<Type>(p->numbers_at_age);
                ssb(pop_idx) = vector<Type>(p->spawning_biomass);
                total_landings_w(pop_idx) = vector<Type>(p->total_landings_weight);
                total_landings_n(pop_idx) = vector<Type>(p->total_landings_numbers);
                log_recruit_dev(pop_idx) =
                        vector<Type>(p->recruitment->log_recruit_devs);
                recruitment(pop_idx) = vector<Type>(p->expected_recruitment);
                biomass(pop_idx) = vector<Type>(p->biomass);
                M(pop_idx) = vector<Type>(p->M);
#endif
                pop_idx += 1;
            }

            // initiate fleet index for structuring report out objects
            int fleet_idx = 0;
            for (f_it = this->fims_information->fleets.begin();
                    f_it != this->fims_information->fleets.end(); ++f_it) {
                std::shared_ptr<fims_popdy::Fleet<Type> > f = (*f_it).second;
#ifdef TMB_MODEL
                landings_w(fleet_idx) = f->landings_weight;
                landings_n(fleet_idx) = f->landings_numbers;
                landings_exp(fleet_idx) = f->landings_expected;
                landings_naa(fleet_idx) = f->landings_numbers_at_age;
                landings_waa(fleet_idx) = f->landings_weight_at_age;
                landings_nal(fleet_idx) = f->landings_numbers_at_length;
                index_w(fleet_idx) = f->index_weight;
                index_n(fleet_idx) = f->index_numbers;
                index_exp(fleet_idx) = f->index_expected;
                index_naa(fleet_idx) = f->index_numbers_at_age;
                index_nal(fleet_idx) = f->index_numbers_at_length;
                comp_naa(fleet_idx) = f->composition_numbers_at_age;
                comp_nal(fleet_idx) = f->composition_numbers_at_length;
                comp_pnaa(fleet_idx) = f->proportion_numbers_at_age;
                comp_pnal(fleet_idx) = f->proportion_numbers_at_length;
                F_mort(fleet_idx) = f->Fmort;
                q(fleet_idx) = f->q;
#endif
                fleet_idx += 1;
            }

            // Reporting
#ifdef TMB_MODEL
    if (do_tmb_reporting) {
            //FIMS_REPORT_F(rec_nll, of);
            //FIMS_REPORT_F(age_comp_nll, of);
            //FIMS_REPORT_F(index_nll, of);
            FIMS_REPORT_F(jnll, of);
            FIMS_REPORT_F(naa, of);
            FIMS_REPORT_F(ssb, of);
            FIMS_REPORT_F(log_recruit_dev, of);
            FIMS_REPORT_F(recruitment, of);
            FIMS_REPORT_F(biomass, of);
            FIMS_REPORT_F(M, of);
            FIMS_REPORT_F(total_landings_w, of);
            FIMS_REPORT_F(total_landings_n, of);
            FIMS_REPORT_F(landings_w, of);
            FIMS_REPORT_F(landings_n, of);
            FIMS_REPORT_F(landings_exp, of);
            FIMS_REPORT_F(landings_naa, of);
            FIMS_REPORT_F(landings_waa, of);
            FIMS_REPORT_F(landings_nal, of);
            FIMS_REPORT_F(index_w, of);
            FIMS_REPORT_F(index_n, of);
            FIMS_REPORT_F(index_exp, of);
            FIMS_REPORT_F(index_naa, of);
            FIMS_REPORT_F(index_nal, of);
            FIMS_REPORT_F(comp_naa, of);
            FIMS_REPORT_F(comp_nal, of);
            FIMS_REPORT_F(comp_pnaa, of);
            FIMS_REPORT_F(comp_pnal, of);
            FIMS_REPORT_F(F_mort, of);
            FIMS_REPORT_F(q, of);
            FIMS_REPORT_F(nll_components, of);

            /*ADREPORT using ADREPORTvector defined in
             * inst/include/interface/interface.hpp:
             * function collapses the nested vector into a single vector
             */
            vector<Type> NAA = ADREPORTvector(naa);
            vector<Type> Biomass = ADREPORTvector(biomass);
            vector<Type> SSB = ADREPORTvector(ssb);
            vector<Type> LogRecDev = ADREPORTvector(log_recruit_dev);
            vector<Type> FMort = ADREPORTvector(F_mort);
            vector<Type> Q = ADREPORTvector(q);
            vector<Type> ExpectedLandings = ADREPORTvector(landings_exp);
            vector<Type> ExpectedIndex = ADREPORTvector(index_exp);
            vector<Type> ExpectedLandingsAtAge = ADREPORTvector(landings_naa);
            vector<Type> ExpectedLandingsAtLength = ADREPORTvector(landings_nal);
            vector<Type> ExpectedIndexAtAge = ADREPORTvector(index_naa);
            vector<Type> ExpectedIndexAtLength = ADREPORTvector(index_nal);
            vector<Type> ExpectedCompositionAtAge = ADREPORTvector(comp_naa);
            vector<Type> ExpectedCompositionAtLength = ADREPORTvector(comp_nal);
            vector<Type> ExpectedCompositionAtAgeProportion =
                    ADREPORTvector(comp_pnaa);
            vector<Type> ExpectedCompositionAtLengthProportion =
                    ADREPORTvector(comp_pnal);

            ADREPORT_F(NAA, of);
            ADREPORT_F(Biomass, of);
            ADREPORT_F(SSB, of);
            ADREPORT_F(LogRecDev, of);
            ADREPORT_F(FMort, of);
            ADREPORT_F(Q, of);
            ADREPORT_F(ExpectedLandings, of);
            ADREPORT_F(ExpectedIndex, of);
            ADREPORT_F(ExpectedLandingsAtAge, of);
            ADREPORT_F(ExpectedLandingsAtLength, of);
            ADREPORT_F(ExpectedIndexAtAge, of);
            ADREPORT_F(ExpectedIndexAtLength, of);
            ADREPORT_F(ExpectedCompositionAtAge, of);
            ADREPORT_F(ExpectedCompositionAtLength, of);
            ADREPORT_F(ExpectedCompositionAtAgeProportion, of);
            ADREPORT_F(ExpectedCompositionAtLengthProportion, of);        
    }
#endif

            return jnll;
        }
    };

    // Create singleton instance of Model class
    template <typename Type>
    std::shared_ptr<Model<Type> > Model<Type>::fims_model =
    nullptr; // singleton instance
} // namespace fims_model

#endif /* FIMS_COMMON_MODEL_HPP */
