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

This directory contains the Rcpp interface files for the FIMS C++ code. The interface files are responsible for exposing the C++ functionality to R, allowing users to call C++ functions and access C++ classes directly from R. R, while flexible and expressive, can be slow for computationally intensive tasks and lacks direct support for low-level operations. Directly calling C++ from R is non-trivial due to mismatches in data types, memory handling, and function calling conventions. Rcpp addresses these mismatches by providing an abstraction layer that simplifies the interface process, allowing C++ code to be written in a way that's naturally accessible from R. The interface files typically include function definitions, class declarations, and other necessary components to facilitate communication between R and C++. By using the Rcpp package, we can leverage the performance benefits of C++ while still providing a user-friendly interface for R users.

Typically, Rcpp exposes C++ functions to R using `Rcpp::export` placed before the C++ function code. This enables automatic wrapper generation and type conversion. Unfortunately, we cannot use this functionality within a TMB model, and thus, the code here may be slightly more complicated than a typical Rcpp interface. Classes are made available via `RCPP_MODULE`. `RCPP_MODULE` provides a declarative mechanism to register C++ classes and their members (constructors, methods, fields) so they can be accessed from R via Rcpp modules where class constructors and methods are explicitly bound for R access. For templates, direct export isn't supported, but individual instantiations can be wrapped in type-specific functions.

At the top level, this directory contains just one file, `interface/interface.hpp` that contains the main interface definitions. The `interface/rcpp/rcpp_interface.hpp` file is a wrapper that allows R to call C++ functions and access C++ classes seamlessly and each of the header files within `rcpp_objects` must be listed in this file.

Within the `rcpp_objects` subdirectory, there are several header files that define the C++ classes and functions that will be exposed to R. Each of these header files corresponds to a specific component or functionality within the FIMS C++ codebase.

### inst/include/models/

This directory contains the header files for different types of population dynamics models. For example, `catch_at_age.hpp` is the most widely used model from the suite of available FIMS models. Each available model type uses the overarching base class defined in `functors/fishery_model_base.hpp`. A new file must be added in `functors` for each model added to FIMS.

### inst/include/population_dynamics/

This directory contains header files related to the population dynamics specified in FIMS. The population dynamics modules are responsible for modeling the growth, reproduction, and survival of populations over time. It includes classes and functions that define the population structure, life-history traits, and environmental factors that influence population dynamics.

Each folder in `population_dynamics` corresponds to a component of the population dynamics model. Given the complexity of the component, the structure in these folders within `population_dynamics` may differ. At a minimum there will be a .hpp file with the same name as the subfolder, e.g., `fleet/fleet.hpp`. This file will include an `ifndef` directive and `#include` statements. If the folder is empty other than this file, then the remainder of the file will define the component. If there are additional subdirectories along side the .hpp file, the file will end after the include statements that point to each of the files in the functions folder that sits next to this .hpp file.

For the latter scenario, where a `functor` folder exists, inside the `functor` folder will be a .hpp file with `_base` attached to the component name, e.g., `population_dynamics/maturity/functors/maturity_base.hpp`. This `_base.hpp` file will define the base class for the module type. The base class should only need a constructor method and a number of methods (e.g., `evaluate()`) that are not specific to the type of functions available under the subfolders but reused for all objects of that class type. This is important for life-history processes where there is structural uncertainty in the model and users may wish to investigate different forms, e.g., Beverton--Holt versus Ricker stock--recruitment relationships. Often, there will only be one functor in the subfolder but there will still be a subfolder structure. This structure is to allow for future expansion of the module type.

Many times, the files will look very similar to each other such as for logistic maturity and logistic selectivity. In these cases, the files will be identical except for the name of the class and the name of the base class that it references. Both of the files use the same function within `fims_math.hpp` to calculate the logistic function it is only their setup that differs. Therefore, pattern matching can be used to create new files.

### inst/include/utilities/

Currently, this directory contains just a single header file, `fims_json.hpp`. This header file and future header files in this directory are for processes that are not related to the interface, fitting data, or the population dynamics.
