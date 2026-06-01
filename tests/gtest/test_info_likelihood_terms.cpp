#include "gtest/gtest.h"

#include <cmath>
#include <memory>

#include "common/information.hpp"
#include "common/model.hpp"
#include "distributions/kernels/distribution_kernels.hpp"

namespace {

TEST(InformationLikelihoodTerms, StoresAndClearsLikelihoodTerms) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> observed{1.0, 2.0, 3.0};
  fims_likelihood::Parameter<double> sigma(
      "sigma", fims::Vector<double>{std::log(1.0)},
      fims_likelihood::Transform::Log);

  std::shared_ptr<fims_likelihood::LikelihoodTerm<double>> term =
      std::make_shared<fims_likelihood::LikelihoodTerm<double>>(
          fims_likelihood::LikelihoodTermType::Data, "normal_data",
          fims_likelihood::vector_ref(observed),
          fims_likelihood::constant_ref(2.0),
          fims_likelihood::parameter_ref(sigma),
          fims_distributions::kernels::Normal<double>::log_density);

  info->likelihood_terms.push_back(term);

  EXPECT_EQ(info->likelihood_terms.size(), 1);
  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(), -3.756815599614018,
              1e-12);

  info->Clear();

  EXPECT_EQ(info->likelihood_terms.size(), 0);
}

TEST(InformationLikelihoodTerms, EvaluatesTermsByTypeAndTotal) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> prior_parameter{2.0};
  fims::Vector<double> random_effects{3.2};
  fims::Vector<double> observed{1.0, -999.0, 3.0};
  fims::Vector<double> expected{2.0};
  fims_likelihood::ValueRef<double> unit_sd =
      fims_likelihood::constant_ref(1.0);

  std::shared_ptr<fims_likelihood::LikelihoodTerm<double>> prior =
      std::make_shared<fims_likelihood::LikelihoodTerm<double>>(
          fims_likelihood::LikelihoodTermType::Prior, "prior",
          fims_likelihood::vector_ref(prior_parameter),
          fims_likelihood::constant_ref(0.0), unit_sd,
          fims_distributions::kernels::Normal<double>::log_density);

  std::shared_ptr<fims_likelihood::LikelihoodTerm<double>> random_effect =
      std::make_shared<fims_likelihood::LikelihoodTerm<double>>(
          fims_likelihood::LikelihoodTermType::RandomEffect, "random_effect",
          fims_likelihood::vector_ref(random_effects),
          fims_likelihood::constant_ref(2.1), unit_sd,
          fims_distributions::kernels::Normal<double>::log_density);

  std::shared_ptr<fims_likelihood::LikelihoodTerm<double>> data =
      std::make_shared<fims_likelihood::LikelihoodTerm<double>>(
          fims_likelihood::LikelihoodTermType::Data, "data",
          fims_likelihood::vector_ref(observed),
          fims_likelihood::vector_ref(expected), unit_sd,
          fims_distributions::kernels::Normal<double>::log_density);
  data->include = [&observed](size_t i) -> bool { return observed[i] != -999.0; };

  info->likelihood_terms.push_back(prior);
  info->likelihood_terms.push_back(random_effect);
  info->likelihood_terms.push_back(data);

  double prior_value =
      fims_distributions::kernels::Normal<double>::log_density(2.0, 0.0, 1.0);
  double random_effect_value =
      fims_distributions::kernels::Normal<double>::log_density(3.2, 2.1, 1.0);
  double data_value =
      fims_distributions::kernels::Normal<double>::log_density(1.0, 2.0, 1.0) +
      fims_distributions::kernels::Normal<double>::log_density(3.0, 2.0, 1.0);

  EXPECT_NEAR(
      info->EvaluateLikelihoodTerms(fims_likelihood::LikelihoodTermType::Prior),
      prior_value, 1e-12);
  EXPECT_NEAR(info->EvaluateLikelihoodTerms(
                  fims_likelihood::LikelihoodTermType::RandomEffect),
              random_effect_value, 1e-12);
  EXPECT_NEAR(
      info->EvaluateLikelihoodTerms(fims_likelihood::LikelihoodTermType::Data),
      data_value, 1e-12);
  EXPECT_NEAR(info->EvaluateLikelihoodTerms(),
              prior_value + random_effect_value + data_value, 1e-12);
  EXPECT_NEAR(info->EvaluateNegativeLogLikelihoodTerms(
                  fims_likelihood::LikelihoodTermType::Prior),
              -prior_value, 1e-12);
  EXPECT_NEAR(info->EvaluateNegativeLogLikelihoodTerms(
                  fims_likelihood::LikelihoodTermType::RandomEffect),
              -random_effect_value, 1e-12);
  EXPECT_NEAR(info->EvaluateNegativeLogLikelihoodTerms(
                  fims_likelihood::LikelihoodTermType::Data),
              -data_value, 1e-12);
  EXPECT_NEAR(info->EvaluateNegativeLogLikelihoodTerms(),
              -(prior_value + random_effect_value + data_value), 1e-12);
  EXPECT_EQ(data->log_density_values[1], 0.0);

  info->Clear();
}

TEST(InformationLikelihoodTerms, FindsAndEvaluatesTermsBySource) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> prior_parameter{2.0};
  fims_likelihood::ValueRef<double> unit_sd =
      fims_likelihood::constant_ref(1.0);

  std::shared_ptr<fims_likelihood::LikelihoodTerm<double>> prior =
      std::make_shared<fims_likelihood::LikelihoodTerm<double>>(
          fims_likelihood::LikelihoodTermType::Prior, "prior.4000",
          fims_likelihood::vector_ref(prior_parameter),
          fims_likelihood::constant_ref(0.0), unit_sd,
          fims_distributions::kernels::Normal<double>::log_density);
  prior->source_id = 4000;

  std::shared_ptr<fims_likelihood::LikelihoodTerm<double>> data =
      std::make_shared<fims_likelihood::LikelihoodTerm<double>>(
          fims_likelihood::LikelihoodTermType::Data, "data.4000",
          fims_likelihood::vector_ref(prior_parameter),
          fims_likelihood::constant_ref(1.0), unit_sd,
          fims_distributions::kernels::Normal<double>::log_density);
  data->source_id = 4000;

  info->likelihood_terms.push_back(prior);
  info->likelihood_terms.push_back(data);

  EXPECT_EQ(info->FindLikelihoodTerm(4000), prior);
  EXPECT_EQ(info->FindLikelihoodTerm(
                4000, fims_likelihood::LikelihoodTermType::Prior),
            prior);
  EXPECT_EQ(info->FindLikelihoodTerm(
                4000, fims_likelihood::LikelihoodTermType::Data),
            data);
  EXPECT_EQ(info->FindLikelihoodTerm(
                4000, fims_likelihood::LikelihoodTermType::RandomEffect),
            nullptr);
  EXPECT_NEAR(info->EvaluateLikelihoodTerm(
                  4000, fims_likelihood::LikelihoodTermType::Prior),
              fims_distributions::kernels::Normal<double>::log_density(
                  2.0, 0.0, 1.0),
              1e-12);
  EXPECT_NEAR(info->EvaluateNegativeLogLikelihoodTerm(
                  4000, fims_likelihood::LikelihoodTermType::Prior),
              -fims_distributions::kernels::Normal<double>::log_density(
                  2.0, 0.0, 1.0),
              1e-12);
  EXPECT_THROW(info->EvaluateLikelihoodTerm(
                   4000, fims_likelihood::LikelihoodTermType::RandomEffect),
               std::runtime_error);
  EXPECT_THROW(info->EvaluateNegativeLogLikelihoodTerm(
                   4000, fims_likelihood::LikelihoodTermType::RandomEffect),
               std::runtime_error);

  info->Clear();
}

TEST(InformationLikelihoodTerms, ModelEvaluationUsesMirroredTermsWhenEnabled) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> prior_parameter{2.0};
  fims::Vector<double> observed{1.0, 3.0};
  fims::Vector<double> expected{2.0};
  fims_likelihood::ValueRef<double> unit_sd =
      fims_likelihood::constant_ref(1.0);

  std::shared_ptr<fims_likelihood::LikelihoodTerm<double>> prior =
      std::make_shared<fims_likelihood::LikelihoodTerm<double>>(
          fims_likelihood::LikelihoodTermType::Prior, "prior",
          fims_likelihood::vector_ref(prior_parameter),
          fims_likelihood::constant_ref(0.0), unit_sd,
          fims_distributions::kernels::Normal<double>::log_density);

  std::shared_ptr<fims_likelihood::LikelihoodTerm<double>> data =
      std::make_shared<fims_likelihood::LikelihoodTerm<double>>(
          fims_likelihood::LikelihoodTermType::Data, "data",
          fims_likelihood::vector_ref(observed),
          fims_likelihood::vector_ref(expected), unit_sd,
          fims_distributions::kernels::Normal<double>::log_density);

  info->likelihood_terms.push_back(prior);
  info->likelihood_terms.push_back(data);

  std::shared_ptr<fims_model::Model<double>> model =
      fims_model::Model<double>::GetInstance();
  model->fims_information = info;

  EXPECT_EQ(info->use_likelihood_terms, false);
  EXPECT_EQ(model->Evaluate(), 0.0);

  info->use_likelihood_terms = true;

  double expected_nll =
      -fims_distributions::kernels::Normal<double>::log_density(
          2.0, 0.0, 1.0) -
      fims_distributions::kernels::Normal<double>::log_density(
          1.0, 2.0, 1.0) -
      fims_distributions::kernels::Normal<double>::log_density(
          3.0, 2.0, 1.0);

  EXPECT_NEAR(model->Evaluate(), expected_nll, 1e-12);

  info->Clear();
}

TEST(InformationLikelihoodTerms, MirrorsNormalPriorDensityComponent) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> parameter{2.0};
  info->variable_map[100] = &parameter;

  std::shared_ptr<fims_distributions::NormalLPDF<double>> normal_prior =
      std::make_shared<fims_distributions::NormalLPDF<double>>();
  normal_prior->id = 1000;
  normal_prior->input_type = "prior";
  normal_prior->key.resize(1);
  normal_prior->key[0] = 100;
  normal_prior->expected_values.resize(1);
  normal_prior->expected_values[0] = 0.0;
  normal_prior->log_sd.resize(1);
  normal_prior->log_sd[0] = std::log(1.0);
  info->density_components[normal_prior->id] = normal_prior;

  info->SetupPriors();

  EXPECT_EQ(info->likelihood_terms.size(), 1);
  EXPECT_EQ(info->likelihood_terms[0]->type,
            fims_likelihood::LikelihoodTermType::Prior);
  EXPECT_EQ(info->likelihood_terms[0]->source_id, normal_prior->id);
  EXPECT_EQ(info->likelihood_terms[0]->name, "normal_prior.1000");
  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(),
              fims_distributions::kernels::Normal<double>::log_density(
                  2.0, 0.0, 1.0),
              1e-12);

  parameter[0] = 1.0;

  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(),
              fims_distributions::kernels::Normal<double>::log_density(
                  1.0, 0.0, 1.0),
              1e-12);

  info->Clear();
}

TEST(InformationLikelihoodTerms, MirrorsLognormalPriorDensityComponent) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> parameter{2.0};
  info->variable_map[101] = &parameter;

  std::shared_ptr<fims_distributions::LogNormalLPDF<double>> lognormal_prior =
      std::make_shared<fims_distributions::LogNormalLPDF<double>>();
  lognormal_prior->id = 1001;
  lognormal_prior->input_type = "prior";
  lognormal_prior->key.resize(1);
  lognormal_prior->key[0] = 101;
  lognormal_prior->expected_values.resize(1);
  lognormal_prior->expected_values[0] = 0.0;
  lognormal_prior->log_sd.resize(1);
  lognormal_prior->log_sd[0] = std::log(0.5);
  info->density_components[lognormal_prior->id] = lognormal_prior;

  info->SetupPriors();

  EXPECT_EQ(info->likelihood_terms.size(), 1);
  EXPECT_EQ(info->likelihood_terms[0]->type,
            fims_likelihood::LikelihoodTermType::Prior);
  EXPECT_EQ(info->likelihood_terms[0]->source_id, lognormal_prior->id);
  EXPECT_EQ(info->likelihood_terms[0]->name, "lognormal_prior.1001");
  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(),
              fims_distributions::kernels::LogNormal<double>::log_density(
                  2.0, 0.0, 0.5),
              1e-12);

  info->Clear();
}

TEST(InformationLikelihoodTerms, MirrorsNormalRandomEffectDensityComponent) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> random_effects{3.2, 4.1};
  fims::Vector<double> expected{2.1, -1.7};
  info->variable_map[200] = &random_effects;
  info->variable_map[201] = &expected;

  std::shared_ptr<fims_distributions::NormalLPDF<double>> normal_re =
      std::make_shared<fims_distributions::NormalLPDF<double>>();
  normal_re->id = 2000;
  normal_re->input_type = "random_effects";
  normal_re->key.resize(2);
  normal_re->key[0] = 200;
  normal_re->key[1] = 201;
  normal_re->log_sd.resize(1);
  normal_re->log_sd[0] = std::log(1.0);
  info->density_components[normal_re->id] = normal_re;

  info->SetupRandomEffects();

  EXPECT_EQ(info->likelihood_terms.size(), 1);
  EXPECT_EQ(info->likelihood_terms[0]->type,
            fims_likelihood::LikelihoodTermType::RandomEffect);
  EXPECT_EQ(info->likelihood_terms[0]->source_id, normal_re->id);
  EXPECT_EQ(info->likelihood_terms[0]->name, "normal_random_effect.2000");
  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(),
              fims_distributions::kernels::Normal<double>::log_density(
                  3.2, 2.1, 1.0) +
                  fims_distributions::kernels::Normal<double>::log_density(
                      4.1, -1.7, 1.0),
              1e-12);

  random_effects[0] = 4.3;
  expected[0] = 5.6;

  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(),
              fims_distributions::kernels::Normal<double>::log_density(
                  4.3, 5.6, 1.0) +
                  fims_distributions::kernels::Normal<double>::log_density(
                      4.1, -1.7, 1.0),
              1e-12);

  info->Clear();
}

TEST(InformationLikelihoodTerms,
     MirrorsLognormalRandomEffectDensityComponentWithoutJacobian) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> random_effects{2.0};
  info->variable_map[202] = &random_effects;

  std::shared_ptr<fims_distributions::LogNormalLPDF<double>> lognormal_re =
      std::make_shared<fims_distributions::LogNormalLPDF<double>>();
  lognormal_re->id = 2001;
  lognormal_re->input_type = "random_effects";
  lognormal_re->key.resize(1);
  lognormal_re->key[0] = 202;
  lognormal_re->expected_values.resize(1);
  lognormal_re->expected_values[0] = 0.0;
  lognormal_re->log_sd.resize(1);
  lognormal_re->log_sd[0] = std::log(0.5);
  info->density_components[lognormal_re->id] = lognormal_re;

  info->SetupRandomEffects();

  EXPECT_EQ(info->likelihood_terms.size(), 1);
  EXPECT_EQ(info->likelihood_terms[0]->type,
            fims_likelihood::LikelihoodTermType::RandomEffect);
  EXPECT_EQ(info->likelihood_terms[0]->source_id, lognormal_re->id);
  EXPECT_EQ(info->likelihood_terms[0]->name,
            "lognormal_random_effect.2001");
  EXPECT_NEAR(
      info->likelihood_terms[0]->evaluate(),
      fims_distributions::kernels::LogNormal<double>::log_density_log_scale(
          2.0, 0.0, 0.5),
      1e-12);

  info->Clear();
}

TEST(InformationLikelihoodTerms, MirrorsNormalDataDensityComponent) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  std::shared_ptr<fims_data_object::DataObject<double>> observed =
      std::make_shared<fims_data_object::DataObject<double>>(3);
  observed->id = 300;
  observed->data[0] = 1.0;
  observed->data[1] = observed->na_value;
  observed->data[2] = 3.0;
  info->data_objects[observed->id] = observed;

  fims::Vector<double> expected{2.0, 2.0, 2.0};
  info->variable_map[300] = &expected;

  std::shared_ptr<fims_distributions::NormalLPDF<double>> normal_data =
      std::make_shared<fims_distributions::NormalLPDF<double>>();
  normal_data->id = 3000;
  normal_data->input_type = "data";
  normal_data->observed_data_id_m = observed->id;
  normal_data->key.resize(1);
  normal_data->key[0] = 300;
  normal_data->log_sd.resize(1);
  normal_data->log_sd[0] = std::log(1.0);
  info->density_components[normal_data->id] = normal_data;

  bool valid_model = true;
  info->SetDataObjects(valid_model);
  info->SetupData();

  EXPECT_TRUE(valid_model);
  EXPECT_EQ(info->likelihood_terms.size(), 1);
  EXPECT_EQ(info->likelihood_terms[0]->type,
            fims_likelihood::LikelihoodTermType::Data);
  EXPECT_EQ(info->likelihood_terms[0]->source_id, normal_data->id);
  EXPECT_EQ(info->likelihood_terms[0]->name, "normal_data.3000");
  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(),
              fims_distributions::kernels::Normal<double>::log_density(
                  1.0, 2.0, 1.0) +
                  fims_distributions::kernels::Normal<double>::log_density(
                      3.0, 2.0, 1.0),
              1e-12);
  EXPECT_EQ(info->likelihood_terms[0]->log_density_values[1], 0.0);

  observed->data[0] = 2.0;

  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(),
              fims_distributions::kernels::Normal<double>::log_density(
                  2.0, 2.0, 1.0) +
                  fims_distributions::kernels::Normal<double>::log_density(
                      3.0, 2.0, 1.0),
              1e-12);

  info->Clear();
}

TEST(InformationLikelihoodTerms, MirrorsLognormalDataDensityComponent) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  std::shared_ptr<fims_data_object::DataObject<double>> observed =
      std::make_shared<fims_data_object::DataObject<double>>(1);
  observed->id = 301;
  observed->data[0] = 2.0;
  info->data_objects[observed->id] = observed;

  fims::Vector<double> expected{0.0};
  info->variable_map[301] = &expected;

  std::shared_ptr<fims_distributions::LogNormalLPDF<double>> lognormal_data =
      std::make_shared<fims_distributions::LogNormalLPDF<double>>();
  lognormal_data->id = 3001;
  lognormal_data->input_type = "data";
  lognormal_data->observed_data_id_m = observed->id;
  lognormal_data->key.resize(1);
  lognormal_data->key[0] = 301;
  lognormal_data->log_sd.resize(1);
  lognormal_data->log_sd[0] = std::log(0.5);
  info->density_components[lognormal_data->id] = lognormal_data;

  bool valid_model = true;
  info->SetDataObjects(valid_model);
  info->SetupData();

  EXPECT_TRUE(valid_model);
  EXPECT_EQ(info->likelihood_terms.size(), 1);
  EXPECT_EQ(info->likelihood_terms[0]->type,
            fims_likelihood::LikelihoodTermType::Data);
  EXPECT_EQ(info->likelihood_terms[0]->source_id, lognormal_data->id);
  EXPECT_EQ(info->likelihood_terms[0]->name, "lognormal_data.3001");
  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(),
              fims_distributions::kernels::LogNormal<double>::log_density(
                  2.0, 0.0, 0.5),
              1e-12);

  info->Clear();
}

TEST(InformationLikelihoodTerms, MirrorsMultinomialDataDensityComponent) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  std::shared_ptr<fims_data_object::DataObject<double>> observed =
      std::make_shared<fims_data_object::DataObject<double>>(2, 2);
  observed->id = 302;
  observed->data[0] = 3.0;
  observed->data[1] = 7.0;
  observed->data[2] = observed->na_value;
  observed->data[3] = 5.0;
  info->data_objects[observed->id] = observed;

  fims::Vector<double> expected{0.3, 0.7, 0.4, 0.6};
  info->variable_map[302] = &expected;

  std::shared_ptr<fims_distributions::MultinomialLPMF<double>>
      multinomial_data =
          std::make_shared<fims_distributions::MultinomialLPMF<double>>();
  multinomial_data->id = 3002;
  multinomial_data->input_type = "data";
  multinomial_data->observed_data_id_m = observed->id;
  multinomial_data->key.resize(1);
  multinomial_data->key[0] = 302;
  info->density_components[multinomial_data->id] = multinomial_data;

  bool valid_model = true;
  info->SetDataObjects(valid_model);
  info->SetupData();

  EXPECT_TRUE(valid_model);
  EXPECT_EQ(multinomial_data->data_observed_values, observed);
  EXPECT_EQ(multinomial_data->data_expected_values, &expected);
  EXPECT_EQ(info->likelihood_terms.size(), 1);
  EXPECT_EQ(info->likelihood_terms[0]->type,
            fims_likelihood::LikelihoodTermType::Data);
  EXPECT_EQ(info->likelihood_terms[0]->source_id, multinomial_data->id);
  EXPECT_EQ(info->likelihood_terms[0]->name, "multinomial_data.3002");
  fims::Vector<double> first_observed_row{3.0, 7.0};
  fims::Vector<double> first_expected_row{0.3, 0.7};
  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(),
              fims_distributions::kernels::Multinomial<double>::log_density(
                  first_observed_row, first_expected_row),
              1e-12);
  EXPECT_NEAR(info->likelihood_terms[0]->log_density_values[0],
              info->likelihood_terms[0]->log_density_values[1], 1e-12);
  EXPECT_EQ(info->likelihood_terms[0]->log_density_values[2], 0.0);
  EXPECT_EQ(info->likelihood_terms[0]->log_density_values[3], 0.0);

  info->Clear();
}

TEST(InformationLikelihoodTerms, ModelEvaluationMatchesMirroredSetupPaths) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> normal_prior_parameter{2.0};
  fims::Vector<double> lognormal_prior_parameter{2.0};
  fims::Vector<double> normal_random_effects{3.2};
  fims::Vector<double> normal_random_effects_expected{2.1};
  fims::Vector<double> lognormal_random_effects{2.0};
  fims::Vector<double> normal_data_expected{2.0, 2.0, 2.0};
  fims::Vector<double> lognormal_data_expected{0.0};

  info->variable_map[500] = &normal_prior_parameter;
  info->variable_map[501] = &lognormal_prior_parameter;
  info->variable_map[502] = &normal_random_effects;
  info->variable_map[503] = &normal_random_effects_expected;
  info->variable_map[504] = &lognormal_random_effects;
  info->variable_map[505] = &normal_data_expected;
  info->variable_map[506] = &lognormal_data_expected;

  std::shared_ptr<fims_data_object::DataObject<double>> normal_observed =
      std::make_shared<fims_data_object::DataObject<double>>(3);
  normal_observed->id = 500;
  normal_observed->data[0] = 1.0;
  normal_observed->data[1] = normal_observed->na_value;
  normal_observed->data[2] = 3.0;
  info->data_objects[normal_observed->id] = normal_observed;

  std::shared_ptr<fims_data_object::DataObject<double>> lognormal_observed =
      std::make_shared<fims_data_object::DataObject<double>>(1);
  lognormal_observed->id = 501;
  lognormal_observed->data[0] = 2.0;
  info->data_objects[lognormal_observed->id] = lognormal_observed;

  std::shared_ptr<fims_distributions::NormalLPDF<double>> normal_prior =
      std::make_shared<fims_distributions::NormalLPDF<double>>();
  normal_prior->id = 5000;
  normal_prior->input_type = "prior";
  normal_prior->key.resize(1);
  normal_prior->key[0] = 500;
  normal_prior->expected_values.resize(1);
  normal_prior->expected_values[0] = 0.0;
  normal_prior->log_sd.resize(1);
  normal_prior->log_sd[0] = std::log(1.0);
  info->density_components[normal_prior->id] = normal_prior;

  std::shared_ptr<fims_distributions::LogNormalLPDF<double>> lognormal_prior =
      std::make_shared<fims_distributions::LogNormalLPDF<double>>();
  lognormal_prior->id = 5001;
  lognormal_prior->input_type = "prior";
  lognormal_prior->key.resize(1);
  lognormal_prior->key[0] = 501;
  lognormal_prior->expected_values.resize(1);
  lognormal_prior->expected_values[0] = 0.0;
  lognormal_prior->log_sd.resize(1);
  lognormal_prior->log_sd[0] = std::log(0.5);
  info->density_components[lognormal_prior->id] = lognormal_prior;

  std::shared_ptr<fims_distributions::NormalLPDF<double>> normal_re =
      std::make_shared<fims_distributions::NormalLPDF<double>>();
  normal_re->id = 5002;
  normal_re->input_type = "random_effects";
  normal_re->key.resize(2);
  normal_re->key[0] = 502;
  normal_re->key[1] = 503;
  normal_re->log_sd.resize(1);
  normal_re->log_sd[0] = std::log(1.0);
  info->density_components[normal_re->id] = normal_re;

  std::shared_ptr<fims_distributions::LogNormalLPDF<double>> lognormal_re =
      std::make_shared<fims_distributions::LogNormalLPDF<double>>();
  lognormal_re->id = 5003;
  lognormal_re->input_type = "random_effects";
  lognormal_re->key.resize(1);
  lognormal_re->key[0] = 504;
  lognormal_re->expected_values.resize(1);
  lognormal_re->expected_values[0] = 0.0;
  lognormal_re->log_sd.resize(1);
  lognormal_re->log_sd[0] = std::log(0.5);
  info->density_components[lognormal_re->id] = lognormal_re;

  std::shared_ptr<fims_distributions::NormalLPDF<double>> normal_data =
      std::make_shared<fims_distributions::NormalLPDF<double>>();
  normal_data->id = 5004;
  normal_data->input_type = "data";
  normal_data->observed_data_id_m = normal_observed->id;
  normal_data->key.resize(1);
  normal_data->key[0] = 505;
  normal_data->log_sd.resize(1);
  normal_data->log_sd[0] = std::log(1.0);
  info->density_components[normal_data->id] = normal_data;

  std::shared_ptr<fims_distributions::LogNormalLPDF<double>> lognormal_data =
      std::make_shared<fims_distributions::LogNormalLPDF<double>>();
  lognormal_data->id = 5005;
  lognormal_data->input_type = "data";
  lognormal_data->observed_data_id_m = lognormal_observed->id;
  lognormal_data->key.resize(1);
  lognormal_data->key[0] = 506;
  lognormal_data->log_sd.resize(1);
  lognormal_data->log_sd[0] = std::log(0.5);
  info->density_components[lognormal_data->id] = lognormal_data;

  bool valid_model = true;
  info->SetDataObjects(valid_model);
  info->SetupPriors();
  info->SetupRandomEffects();
  info->SetupData();
  info->use_likelihood_terms = true;

  double expected_nll =
      -fims_distributions::kernels::Normal<double>::log_density(
          2.0, 0.0, 1.0) -
      fims_distributions::kernels::LogNormal<double>::log_density(
          2.0, 0.0, 0.5) -
      fims_distributions::kernels::Normal<double>::log_density(
          3.2, 2.1, 1.0) -
      fims_distributions::kernels::LogNormal<double>::log_density_log_scale(
          2.0, 0.0, 0.5) -
      fims_distributions::kernels::Normal<double>::log_density(
          1.0, 2.0, 1.0) -
      fims_distributions::kernels::Normal<double>::log_density(
          3.0, 2.0, 1.0) -
      fims_distributions::kernels::LogNormal<double>::log_density(
          2.0, 0.0, 0.5);

  std::shared_ptr<fims_model::Model<double>> model =
      fims_model::Model<double>::GetInstance();
  model->fims_information = info;

  EXPECT_TRUE(valid_model);
  EXPECT_EQ(info->likelihood_terms.size(), 6);
  EXPECT_NEAR(model->Evaluate(), expected_nll, 1e-12);

  info->Clear();
}

}  // namespace
