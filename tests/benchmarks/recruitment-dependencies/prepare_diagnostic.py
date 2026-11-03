"""Create an isolated, instrumented FIMS source tree; never edit production code.

Usage: python3 prepare_diagnostic.py REPO OUTPUT
The extra controls intentionally change the model for causal diagnostics.
They must never be used for assessment inference.
"""
import io
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile

repo, output = map(lambda p: Path(p).resolve(), sys.argv[1:])
output.mkdir(parents=True, exist_ok=False)
archive = subprocess.check_output(['git', 'archive', 'HEAD'], cwd=repo)
with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
    tar.extractall(output, filter='data')
for name in subprocess.check_output(['git', 'diff', '--name-only'], cwd=repo,
                                     text=True).splitlines():
    if (repo / name).is_file():
        shutil.copy2(repo / name, output / name)

def replace(path, old, new):
    file = output / path
    text = file.read_text()
    if text.count(old) != 1:
        raise ValueError(f'Expected one match in {path}, found {text.count(old)}')
    file.write_text(text.replace(old, new))

(output / 'inst/include/common/recruitment_dependency_diagnostic.hpp').write_text('''
#pragma once
#include <cstdlib>
#include <string>
namespace fims_diagnostic {
inline bool enabled(const char* name) {
  const char* value = std::getenv(name);
  return value && std::string(value) == "1";
}
template<class Type> inline vector<Type>* plus_states = nullptr;
template<class Type> inline vector<Type> constraints;
}
''')
caa = 'inst/include/models/functors/catch_at_age.hpp'
replace(caa, '#include "fishery_model_base.hpp"',
        '#include "fishery_model_base.hpp"\n#include "common/recruitment_dependency_diagnostic.hpp"')
replace(caa, '''    // Plus group calculation
    if (age == (population->n_ages - 1)) {
      dq_["numbers_at_age"][i_age_year] =''', '''    // Diagnostic ablation: discard plus-group survivors.
    if (age == (population->n_ages - 1) &&
        !fims_diagnostic::enabled("FIMS_DIAG_NO_PLUS")) {
      dq_["numbers_at_age"][i_age_year] =''')
replace(caa, '''              (fims_math::exp(-dq_["mortality_Z"][i_agem1_yearm1 + 1]));
    }
  }''', '''              (fims_math::exp(-dq_["mortality_Z"][i_agem1_yearm1 + 1]));
    }
    const size_t year = i_age_year / population->n_ages;
    if (age == population->n_ages - 1 && year < population->n_years &&
        fims_diagnostic::enabled("FIMS_DIAG_LIFT_PLUS")) {
      auto* states = fims_diagnostic::plus_states<Type>;
      if (states == nullptr || states->size() != population->n_years - 1) {
        throw std::runtime_error("Diagnostic plus-state dimensions mismatch");
      }
      // Scale by R0 for conditioning; constraints have unit diagonal in states.
      const Type scale = fims_math::exp(population->recruitment->log_rzero[0]);
      fims_diagnostic::constraints<Type>[year - 1] =
          (*states)[year - 1] - dq_["numbers_at_age"][i_age_year] / scale;
      dq_["numbers_at_age"][i_age_year] = (*states)[year - 1] * scale;
    }
  }''')
start = (output / caa).read_text().index('size_t i_dev, const Type &phi_0)')
end = (output / caa).read_text().index('\n  /**', start)
text = (output / caa).read_text()
chunk = text[start:end]
old = '''population->recruitment->evaluate_mean(
              dq_["spawning_biomass"][year - 1], phi_0)'''
if chunk.count(old) != 2:
    raise ValueError('Recruitment expectation call sites changed')
chunk = chunk.replace(old, 'expected_recruitment')
chunk = chunk.replace('    if (i_dev == population->n_years)', '''    const Type expected_recruitment =
        fims_diagnostic::enabled("FIMS_DIAG_NO_FEEDBACK")
            ? fims_math::exp(population->recruitment->log_rzero[0])
            : population->recruitment->evaluate_mean(
                  dq_["spawning_biomass"][year - 1], phi_0);
    if (i_dev == population->n_years)''')
(output / caa).write_text(text[:start] + chunk + text[end:])

logdev = 'inst/include/population_dynamics/recruitment/functors/log_devs.hpp'
replace(logdev, '#include "common/fims_vector.hpp"',
        '#include "common/fims_vector.hpp"\n#include "common/recruitment_dependency_diagnostic.hpp"')
replace(logdev, '  virtual const Type evaluate_process(size_t pos) {', '''  virtual const Type evaluate_process(size_t pos) {
    if (fims_diagnostic::enabled("FIMS_DIAG_CENTERED")) {
      // Input coordinates are annual log recruitment. Convert the value used
      // by the existing innovation density back to its implied deviation.
      // FIMS's objective resets every input parameter before each evaluation.
      const Type log_r = this->recruitment->log_recruit_devs[pos];
      this->recruitment->log_recruit_devs[pos] =
          log_r - this->recruitment->log_expected_recruitment[pos];
      return log_r;
    }''')

replace('src/FIMS.cpp', '#include "../inst/include/common/model.hpp"',
        '#include "../inst/include/common/model.hpp"\n#include "../inst/include/common/recruitment_dependency_diagnostic.hpp"')
replace('src/FIMS.cpp', '  PARAMETER_VECTOR(re);', '''  PARAMETER_VECTOR(re);
  PARAMETER_VECTOR(diagnostic_plus_states);
  DATA_VECTOR(diagnostic_plus_lambda);
  DATA_INTEGER(diagnostic_constraints_only);
  if (fims_diagnostic::enabled("FIMS_DIAG_LIFT_PLUS")) {
    if (!fims_diagnostic::enabled("FIMS_DIAG_CENTERED") ||
        fims_diagnostic::enabled("FIMS_DIAG_NO_PLUS") ||
        diagnostic_plus_states.size() != diagnostic_plus_lambda.size()) {
      error("Invalid lifted diagnostic configuration");
    }
    fims_diagnostic::plus_states<Type> = &diagnostic_plus_states;
    fims_diagnostic::constraints<Type>.resize(diagnostic_plus_states.size());
    fims_diagnostic::constraints<Type>.setZero();
  }''')
replace('src/FIMS.cpp', '    nll = model->Evaluate();', '''    nll = model->Evaluate();
    if (fims_diagnostic::enabled("FIMS_DIAG_LIFT_PLUS")) {
      vector<Type> diagnostic_constraints = fims_diagnostic::constraints<Type>;
      REPORT(diagnostic_constraints);
      if (diagnostic_constraints_only) nll = Type(0);
      for (int j = 0; j < diagnostic_constraints.size(); ++j) {
        nll += diagnostic_plus_lambda[j] * diagnostic_constraints[j];
      }
      fims_diagnostic::plus_states<Type> = nullptr;
    }''')
print(output)
