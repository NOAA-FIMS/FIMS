\name{native_quadra_gradient}
\alias{native_quadra_gradient}
\title{Evaluate the native joint gradient with Quadra}
\usage{native_quadra_gradient(fixed, random = numeric())}
\arguments{\item{fixed, random}{Numeric fixed- and random-effect parameter vectors.}}
\value{A numeric gradient, fixed effects followed by random effects.}
\description{Evaluate the native joint gradient with Quadra}
\details{Exact repeated parameter vectors reuse cached tape passes. Rebuilding or clearing the model discards the cache. Objective-only requests skip the reverse derivative sweep.}
