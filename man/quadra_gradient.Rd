\name{quadra_gradient}
\alias{quadra_gradient}
\title{Evaluate the joint gradient with Quadra}
\usage{quadra_gradient(fixed = get_fixed(), random = get_random())}
\arguments{\item{fixed, random}{Numeric fixed- and random-effect parameter vectors, in the order returned by get_fixed() and get_random().}}
\value{A numeric gradient, fixed effects followed by random effects.}
\description{Evaluate the joint gradient with Quadra}
\details{Uses the Quadra first-order tape; the last forward and reverse passes are reused for exactly identical parameters. Call CreateTMBModel() after assembling or changing modules.}
