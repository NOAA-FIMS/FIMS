\name{native_quadra_objective}
\alias{native_quadra_objective}
\title{Evaluate the native joint objective with forward-only Quadra replay}
\usage{native_quadra_objective(fixed, random = numeric())}
\arguments{\item{fixed, random}{Numeric fixed- and random-effect parameter vectors.}}
\value{The scalar joint objective.}
\description{Evaluate the native joint objective with forward-only Quadra replay}
\details{Exact repeated parameter vectors reuse cached tape passes. Rebuilding or clearing the model discards the cache. Objective-only requests skip the reverse derivative sweep.}
