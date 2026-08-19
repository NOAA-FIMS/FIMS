# C++ Directory Structure

[TOC]

The C++ code for FIMS is stored in two location, src/ and inst/include/. The files stored in src/ contain the details necessary for implementation so the code can be compiled into a shared library, i.e., a .dll on Windows or a .so on Linux. The files stored in inst/include/ contain header files (.hpp files) that contain class and struct definitions, function prototypes, template declarations and definitions, inline function definitions, and constants and macros. We use .hpp extension instead of .h because .h declares a file a header file but not necessarily a header file that contains C++ code; though the extensions can be used interchangeably. Header files are essential for modular programming in C++, serving as interfaces, declaring functions, classes, and other entities that are defined in the corresponding ../source/*.cpp files or directly within the header itself (for templates and inline functions). They allow the compiler to understand the structure of the code and ensure that different parts of the program can interact correctly. When a .cpp file includes a .hpp file, the compiler gains access to the declarations within the header, enabling it to compile the code that uses those declarations.

## inst/include/

The header files in inst/include/ are organized into subdirectories based on their functionality or the components they represent. This organization helps maintain a clear structure and makes it easier to locate specific headers when working on different parts of the codebase. The subdirectories may include headers related to data structures, algorithms, utilities, and other components that are part of the FIMS C++ codebase.

### inst/include/common/

This directory contains common header files that are used across different parts of the project, i.e., between the interface, the TMB objective function, and the mathematics and population dynamics components of the package. These headers define common data structures, utility functions, and macros that are shared among various components.

The directory contains no additional structure, as all common headers are placed directly within this directory. This flat structure is intended to simplify access to these common headers, making it easier for developers to include them in their code without navigating through multiple subdirectories.

This directory also includes `mainpage.dox` that serves as the entry point for the Doxygen documentation. It provides an overview of the project, its purpose, and its structure.

### inst/include/distributions/

This directory contains a single header file, `distributions.hpp`, that links to all of the header files within the `functors` subdirectory. For example, `lognormal_lpdf.hpp` contains the log probability density (lpdf) for the lognormal distribution. Each distribution must have its own header file, and the header file must be named according to the distribution it contains and contain the class and its fields and return the lpdf.

### inst/include/interface/

This directory contains the native R interface and TMB integration. Files under
`interface/call` declare registered `.Call` entry points and the registries that
own C++ model components. Implementations live in `src/call_*.cpp`; R wrappers
live in `R/*_call.R`. Inputs cross the boundary as `SEXP` values and must be
validated and converted before they are used by the model.

#### inst/include/interface/interface.hpp

The `interface/interface.hpp` header contains common definitions used to build
the model with or without TMB.

#### inst/include/interface/call

This directory contains native routine declarations, R conversion helpers, and
object registries. Native routines are registered in `TMB/init_tmb.hpp`; dynamic
symbol lookup is disabled.

#### inst/include/interface/TMB

This directory contains the TMB integration headers used when FIMS is compiled with TMB support. These files keep the TMB-specific include, configuration, and initialization code separate from the rest of the interface so FIMS can build smaller source files instead of one large translation unit that instantiates many TMB and automatic differentiation templates at once.

The `TMB.h` file is the main wrapper around TMB's core headers. It supports both TMB library precompilation and normal FIMS source compilation by controlling macros such as `WITH_LIBTMB`, `TMB_PRECOMPILE`, `CSKIP`, `IF_TMB_PRECOMPILE`, and `TMB_EXTERN`; this prevents conflicts between TMB's build mode and the code compiled as part of FIMS. This file was copied from the RTMB package, thanks to guidance from Kasper Kristensen.

The `config.h` file provides the TMB/RTMB configuration used by FIMS. It maps
TMB failures to R errors, turns on `TMB_SAFEBOUNDS`, selects the
`TMBAD_FRAMEWORK`, uses a 64-bit index type (`uint64_t`), and configures
thread-safe R support when OpenMP is available.

The `init_tmb.hpp` file handles run-time initialization and registration for the compiled shared object. It provides the `R_init_FIMS` registration callback, supports optional initialization through `FIMS_ONLOAD_INIT_TMB`, and registers TMB C-callables when `TMB_CCALLABLES` is available.

### inst/include/models/

This directory contains the header files for different types of population dynamics models. For example, `catch_at_age.hpp` is the most widely used model from the suite of available FIMS models. Each available model type uses the overarching base class defined in `functors/fishery_model_base.hpp`. A new file must be added in `functors` for each model added to FIMS.

### inst/include/population_dynamics/

This directory contains header files related to the population dynamics specified in FIMS. The population dynamics modules are responsible for modeling the growth, reproduction, and survival of populations over time. It includes classes and functions that define the population structure, life-history traits, and environmental factors that influence population dynamics.

Each folder in `population_dynamics` corresponds to a component of the population dynamics model. Given the complexity of the component, the structure in these folders within `population_dynamics` may differ. At a minimum there will be a .hpp file with the same name as the subfolder, e.g., `fleet/fleet.hpp`. This file will include an `ifndef` directive and `#include` statements. If the folder is empty other than this file, then the remainder of the file will define the component. If there are additional subdirectories along side the .hpp file, the file will end after the include statements that point to each of the files in the functions folder that sits next to this .hpp file.

For the latter scenario, where a `functor` folder exists, inside the `functor` folder will be a .hpp file with `_base` attached to the component name, e.g., `population_dynamics/maturity/functors/maturity_base.hpp`. This `_base.hpp` file will define the base class for the module type. The base class should only need a constructor method and a number of methods (e.g., `evaluate()`) that are not specific to the type of functions available under the subfolders but reused for all objects of that class type. This is important for life-history processes where there is structural uncertainty in the model and users may wish to investigate different forms, e.g., Beverton--Holt versus Ricker stock--recruitment relationships. Often, there will only be one functor in the subfolder but there will still be a subfolder structure. This structure is to allow for future expansion of the module type.

Many times, the files will look very similar to each other such as for logistic maturity and logistic selectivity. In these cases, the files will be identical except for the name of the class and the name of the base class that it references. Both of the files use the same function within `fims_math.hpp` to calculate the logistic function it is only their setup that differs. Therefore, pattern matching can be used to create new files.

### inst/include/utilities/

Currently, this directory contains just a single header file, `fims_json.hpp`. This header file and future header files in this directory are for processes that are not related to the interface, fitting data, or the population dynamics.
