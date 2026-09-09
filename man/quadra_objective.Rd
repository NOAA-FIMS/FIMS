\name{quadra_objective}
\alias{quadra_objective}
\title{Evaluate the joint objective without automatic differentiation}
\usage{quadra_objective(fixed = get_fixed(), random = get_random())}
\arguments{\item{fixed, random}{Numeric fixed- and random-effect parameter vectors, in the order returned by get_fixed() and get_random().}}
\value{The scalar joint negative log likelihood.}
\description{Evaluate the joint objective without automatic differentiation}
\details{Uses the model templated on double without a derivative tape. Call CreateTMBModel() after assembling or changing modules.}
