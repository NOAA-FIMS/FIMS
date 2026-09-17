#include <set>
#include <iostream>
#include <memory>
#include "information.hpp"
#include "test_stubs.hpp"

// Standalone regression: object destruction, not just container counts.
int main() {
  using Recruitment = fims_popdy::RecruitmentBase<double>;
  int failures = 0;
  for (bool explicit_clear : {false, true}) {
    for (bool register_process : {false, true}) {
      std::weak_ptr<Recruitment> recruitment_observer, process_observer;
      {
        fims_info::Information<double> info;
        auto recruitment = std::make_shared<fims_popdy::SRBevertonHolt<double>>();
        auto process = std::make_shared<fims_popdy::LogDevs<double>>();
        recruitment->process = process;
        process->recruitment = recruitment;
        info.recruitment_models[recruitment->id] = recruitment;
        if (register_process) info.recruitment_process_models[process->id] = process;
        recruitment_observer = recruitment;
        process_observer = process;
        recruitment.reset();
        process.reset();
        if (explicit_clear) {
          info.Clear();
          info.Clear();  // Clearing an already-cleared owner is safe.
          if (!recruitment_observer.expired() || !process_observer.expired()) ++failures;
        }
      }
      if (!recruitment_observer.expired() || !process_observer.expired()) ++failures;
    }
  }
  // Native population construction can use the recruitment itself as process.
  std::weak_ptr<Recruitment> self_observer;
  {
    fims_info::Information<double> info;
    auto recruitment = std::make_shared<fims_popdy::SRBevertonHolt<double>>();
    recruitment->process = recruitment;
    self_observer = recruitment;
    info.recruitment_models[recruitment->id] = recruitment;
    info.recruitment_models[9999] = nullptr;
    info.recruitment_process_models[9999] = nullptr;
  }
  if (!self_observer.expired()) ++failures;
  // A process-only registry must also release its back-reference cycle.
  std::weak_ptr<Recruitment> orphan_recruitment, orphan_process;
  {
    fims_info::Information<double> info;
    auto recruitment = std::make_shared<fims_popdy::SRBevertonHolt<double>>();
    auto process = std::make_shared<fims_popdy::LogR<double>>();
    recruitment->process = process;
    process->recruitment = recruitment;
    orphan_recruitment = recruitment;
    orphan_process = process;
    info.recruitment_process_models[process->id] = process;
  }
  if (!orphan_recruitment.expired() || !orphan_process.expired()) ++failures;
  std::cout << "Ownership failures: " << failures << '\n';
  return failures ? 1 : 0;
}
