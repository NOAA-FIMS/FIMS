## Distributions

This directory contains a single header file, `distributions.hpp`, that links to all of the header files within the subdirectory `functors`. For example, `lognormal_lpdf.hpp` contains the log probability density (lpdf) for the lognormal distribution. Each distribution must have its own header file, and the header file must be named according to the distribution it contains and contain the class and its fields and return the lpdf.
