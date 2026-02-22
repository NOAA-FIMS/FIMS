# Glossary {#glossary}

Definitions of mathematical symbols used in FIMS equations.

## Population and Recruitment

- \f$N_{a,y}\f$ — Numbers at age \f$a\f$ in year \f$y\f$ (fished)
- \f$N^U_{a,y}\f$ — Unfished numbers at age \f$a\f$ in year \f$y\f$
- \f$N_{a,0}\f$ — Initial numbers at age \f$a\f$ in the initial year 0
- \f$N^U_{a,0}\f$ — Unfished initial numbers at age \f$a\f$ in the initial year 0
- \f$R_0\f$ — Unfished equilibrium recruitment
- \f$R_y\f$ — Recruitment in year \f$y\f$
- \f$\log R_0\f$ — Natural log of unfished equilibrium recruitment
- \f$\log\_devs\f$ — Natural log recruitment deviations

## Mortality

- \f$M\f$ — Natural mortality rate (yr\f$^{-1}\f$)
- \f$F_{f,y}\f$ — Fleet-specific fishing mortality for fleet \f$f\f$ in year \f$y\f$ (yr\f$^{-1}\f$)
- \f$F_{f,a,y}\f$ — Fishing mortality for fleet \f$f\f$ at age \f$a\f$ in year \f$y\f$ (yr\f$^{-1}\f$)
- \f$Z_{a,y}\f$ — Total mortality at age \f$a\f$ in year \f$y\f$, \f$Z = M + F\f$ (yr\f$^{-1}\f$)
- \f$S_{f,a}\f$ — Selectivity for fleet \f$f\f$ at age \f$a\f$
- \f$f_y\f$ — Year-specific F multiplier in year \f$y\f$

## Biomass

- \f$B_y\f$ — Total biomass in year \f$y\f$
- \f$B^U_y\f$ — Unfished biomass in year \f$y\f$
- \f$SB_y\f$ — Spawning biomass in year \f$y\f$
- \f$SB^U_y\f$ — Unfished spawning biomass in year \f$y\f$
- \f$SB^U_0\f$ — Unfished spawning biomass in the initial year 0
- \f$\text{ratio}_y = SB_y / SB^U_0\f$ — Spawning biomass ratio in year \f$y\f$

## Stock--Recruitment (Beverton--Holt)

- \f$h\f$ — Steepness, recruitment relative to \f$R_0\f$ at 20% of \f$SB^U_0\f$; range (0.2, 1.0]
- \f$\phi_0\f$ — Spawning biomass per recruit (unfished)
- \f$SB_{y-1}\f$ — Spawning output in the previous year

\f[
R_y = \frac{0.8\, R_0\, h\, SB_{y-1}}{0.2\, R_0\, \phi_0\, (1 - h) + SB_{y-1}\, (h - 0.2)}
\f]

## Selectivity and Maturity

- \f$x\f$ — Age or size
- \f$\text{slope}\f$ — Logistic curve slope
- \f$\text{inflection\_point}\f$ — Age or size at 50% selectivity/maturity
- \f$\text{slope\_asc}\f$, \f$\text{slope\_desc}\f$ — Ascending and descending slopes (double logistic)
- \f$\text{inflection\_point\_asc}\f$, \f$\text{inflection\_point\_desc}\f$ — Ascending and descending inflection points

Logistic:
\f[
\frac{1}{1 + \exp(-\text{slope} \cdot (x - \text{inflection\_point}))}
\f]

## Growth

- \f$w_a\f$ — Weight at age \f$a\f$ (mt)

## Catch and Landings

- \f$C_{f,a,y}\f$ — Catch in numbers for fleet \f$f\f$ at age \f$a\f$ in year \f$y\f$
- \f$CW_{f,a,y}\f$ — Catch weight for fleet \f$f\f$ at age \f$a\f$ in year \f$y\f$
- \f$CW_{f,y}\f$ — Total catch weight for fleet \f$f\f$ in year \f$y\f$
- \f$q_{f,y}\f$ — Catchability coefficient for fleet \f$f\f$ in year \f$y\f$

Baranov catch equation:
\f[
C_{f,a,y} = \frac{F_{f,y} \times f_y \times S_{f,a}}{Z_{a,y}} \times N_{a,y} \times (1 - e^{-Z_{a,y}})
\f]

## Index (Survey)

- \f$I_{f,a,y}\f$ — Index numbers for fleet \f$f\f$ at age \f$a\f$ in year \f$y\f$
- \f$IW_{f,y}\f$ — Total index weight for fleet \f$f\f$ in year \f$y\f$
- \f$IN_{f,y}\f$ — Total index numbers for fleet \f$f\f$ in year \f$y\f$
- \f$IWAA_{a,y}\f$ — Index weight at age \f$a\f$ in year \f$y\f$
- \f$INAA_{a,y}\f$ — Index numbers at age \f$a\f$ in year \f$y\f$

## Transformations

- \f$\exp(x)\f$ — Exponential function
- \f$\log(x)\f$ — Natural logarithm
- \f$\text{logit}(x; a, b) = -\log(b-x) + \log(x-a)\f$
- \f$\text{logit}^{-1}(x; a, b) = a + \frac{b-a}{1+\exp(-x)}\f$

## Index Variables

- \f$a\f$ — Age
- \f$y\f$ — Year
- \f$t\f$ — Time step (sometimes used interchangeably with \f$y\f$)
- \f$f\f$ — Fleet

## Statistical

- \f$\sigma\f$ — Standard deviation
- \f$\log\_sd\f$ — Log of standard deviation (used in likelihoods)

## Common Abbreviations

- **NAA** — Numbers at age
- **SSB** — Spawning stock biomass
- **SBPR** — Spawning biomass per recruit
- **EWAA** — Empirical weight at age
- **BH** — Beverton--Holt
- **LPDF** — Log probability density function
- **LPMF** — Log probability mass function
