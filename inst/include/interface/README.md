## Interface

This directory contains the Rcpp interface files for the FIMS C++ code. The interface files are responsible for exposing the C++ functionality to R, allowing users to call C++ functions and access C++ classes directly from R. The interface files typically include function definitions, class declarations, and other necessary components to facilitate communication between R and C++. By using Rcpp package available within R, we can leverage the performance benefits of C++ while still providing a user-friendly interface for R users.

At the top level, this directory contains just three files, `init.hpp`, `interface.hpp`, and `rcpp_interface.hpp`. The `init.hpp` file is responsible for initializing the Rcpp interface, while the `interface.hpp` file contains the main interface definitions. The `rcpp_interface.hpp` file is a wrapper that allows R to call C++ functions and access C++ classes seamlessly and each of the header files within `rcpp_objects` must be listed in this file.

Within the `rcpp_objects` subdirectory, there are several header files that define the C++ classes and functions that will be exposed to R. Each of these header files corresponds to a specific component or functionality within the FIMS C++ codebase.
