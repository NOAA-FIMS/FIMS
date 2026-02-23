# Glossary {#glossary}

Definitions of mathematical symbols used in FIMS equations.

## Population and Recruitment

- \f$N_{a,y}\f$ — Numbers at age \f$a\f$ in year \f$y\f$ (fished)
- \f$N^U_{a,y}\f$ — Unfished numbers at age \f$a\f$ in year \f$y\f$
- \f$N_{a,0}\f$ — Numbers at age \f$a\f$ in the initial year 0
- \f$N^U_{a,0}\f$ — Unfished numbers at age \f$a\f$ in the initial year 0
- \f$R_0\f$ — Unfished equilibrium recruitment
- \f$R_y\f$ — Recruitment in year \f$y\f$
- \f$\log R_0\f$ — Natural log of unfished equilibrium recruitment
- \f$\log devs\f$ — Natural log recruitment deviations (see log_devs.hpp)

## Mortality

- \f$M\f$ — Natural mortality rate (yr\f$^{-1}\f$)
- \f$F_{f,y}\f$ — Fishing mortality for fleet \f$f\f$ in year \f$y\f$ (yr\f$^{-1}\f$)
- \f$F_{f,a,y}\f$ — Fishing mortality for fleet \f$f\f$ at age \f$a\f$ in year \f$y\f$ (yr\f$^{-1}\f$)
- \f$Z_{a,y}\f$ — Total mortality at age \f$a\f$ in year \f$y\f$, \f$Z = M + F\f$ (yr\f$^{-1}\f$)
- \f$S_{f,a}\f$ — Selectivity for fleet \f$f\f$ at age \f$a\f$
- \f$f_y\f$ — F multiplier in year \f$y\f$

## Biomass

- \f$B_y\f$ — Total biomass in year \f$y\f$
- \f$B^U_y\f$ — Unfished biomass in year \f$y\f$
- \f$SB_y\f$ — Spawning biomass in year \f$y\f$
- \f$SB^U_y\f$ — Unfished spawning biomass in year \f$y\f$
- \f$SB^U_0\f$ — Unfished spawning biomass in the initial year 0
- \f$\text{ratio}_y = SB_y / SB^U_0\f$ — Spawning biomass ratio in year \f$y\f$

## Stock--Recruitment (Beverton--Holt)

- \f$h\f$ — Steepness, recruitment relative to \f$R_0\f$ at 20% of \f$SB^U_0\f$; range (0.2, 1.0]
- \f$\phi_0\f$ — Spawning biomass per recruit in the initial year 0
- \f$SB_{y-1}\f$ — Spawning biomass in the previous year

## Growth

- \f$w_a\f$ — Weight at age \f$a\f$ (mt)

## Catch and Landings

- \f$C_{f,a,y}\f$ — Catch in numbers for fleet \f$f\f$ at age \f$a\f$ in year \f$y\f$
- \f$CW_{f,y}\f$ — Catch weight for fleet \f$f\f$ in year \f$y\f$ (mt)
- \f$CW_{f,a,y}\f$ — Catch weight for fleet \f$f\f$ at age \f$a\f$ in year \f$y\f$ (mt)
- \f$q_{f,y}\f$ — Catchability coefficient for fleet \f$f\f$ in year \f$y\f$

## Index of Abundance

- \f$IN_{f,y}\f$ — Index numbers for fleet \f$f\f$ in year \f$y\f$
- \f$I_{f,a,y}\f$ — Index numbers for fleet \f$f\f$ at age \f$a\f$ in year \f$y\f$
- \f$IW_{f,y}\f$ — Index weight for fleet \f$f\f$ in year \f$y\f$ (mt)
- \f$IWAA_{a,y}\f$ — Index weight at age \f$a\f$ in year \f$y\f$
- \f$INAA_{a,y}\f$ — Index numbers at age \f$a\f$ in year \f$y\f$

## Index Variables

- \f$a\f$ — Age
- \f$y\f$ — Year
- \f$t\f$ — Time step (sometimes used interchangeably with \f$y\f$)
- \f$f\f$ — Fleet

## Statistical

- \f$\sigma\f$ — Standard deviation; sd

## Common Abbreviations

- **NAA** — Numbers at age
- **SSB** — Spawning stock biomass
- **SBPR** — Spawning biomass per recruit
- **EWAA** — Empirical weight at age
- **BH** — Beverton--Holt
- **LPDF** — Log probability density function
- **LPMF** — Log probability mass function
