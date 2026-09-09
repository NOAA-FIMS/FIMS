\name{quadra_objective}
\alias{quadra_objective}
\title{Evaluate the joint objective with forward-only tape replay}
\usage{quadra_objective(fixed = get_fixed(), random = get_random())}
\arguments{\item{fixed, random}{Numeric fixed- and random-effect parameter vectors, in the order returned by get_fixed() and get_random().}}
\value{The scalar joint negative log likelihood.}
\description{Evaluate the joint objective with forward-only tape replay}
\details{Replays the compact tape forward without a reverse derivative sweep. Exact repeated parameter vectors reuse cached results. Call CreateTMBModel() after assembling or changing modules.}
