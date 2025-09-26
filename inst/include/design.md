# Design of FIMS

TODO: add doi links to all citations

[TOC]

Design choices for the Fisheries Integrated Modeling System (FIMS) are collaborative and often involve tradeoffs. The path to these decisions are documented below to help both current and future developers understand why things are the way they are. In the beginning, FIMS was designed to have enough complexity to adequately test a very standard population model. For this reason, we implemented the minimum structure needed to run the model described in [Li et al. 2021](https://spo.nmfs.noaa.gov/content/fishery-bulletin/comparison-4-primary-age-structured-stock-assessment-models-used-united). Thus, the single model available in FIMS could run an age-structured integrated assessment model with two fleets (one survey, one fishery), where each fleet provided data aggregated across sexes. Since then, FIMS has evolved to be true to its name, i.e., "System", where the back-end code can accommodate more models than just a catch-at-age model and the estimation of random effects is possible.

## Model families

In FIMS, we refer to different model types as "model families" where a catch-at-age model is one model family. A surplus production model is an example of a different model family. Each of the available model families in FIMS are described below.

### Catch-at-age model

#### Length structure

<!-- Notes and references in [M2 development notes folder](https://drive.google.com/drive/folders/1Wh9Exh_kMlo97Z-r-Luj1_mjB4XOnRh4) -->

Length-composition data are often readily sampled from fish populations because they are relatively quick to obtain and the measurements do not require further processing in a lab after their collection, whereas rings on otoliths need to be read after being collected before an age can be determined. Furthermore, lengths can be collected from discarded as well as retained fish because they do not require killing the sample to obtain the information. Thus, length data are almost always more readily available from sampling programs than age data. Although, there is ongoing work to be able to sample ages from live fish (TODO: citation).

Within an assessment model, length data has the potential to provide information on fish growth, fleet selectivity, and potentially recruitment depending on how easily cohorts can be discerned using binned length data. There are some data-limited assessments that only fit to length data and do not include any age data (TODO: citation) but fitting both length and age data is ideal for many reasons. For example, selectivity is often a length-based process that cannot adequately be described with ages alone.

TODO: Ask Jenny B. to write a paragraph about the type of information that is contained within length data and what it means with respect to life history, energetic/metabolic theory, and whatever else is pertinent.

There are several approaches available to fit length data in an assessment model. Below we outline each of the methods and weigh the pros and cons for including each particular method in FIMS.

| Approach | FIMS version | Pros | Cons |
|----------------|----------------|----------------|------------------------|
| Empirical length at age | NA | Fast, allows functional forms of length selection | No uncertainty in length at age and no fishery impacts on length-at-age structure |
| Age–to-length transition matrix | 0.3.0 | Allows for length-based selectivity | Does not allow for internal estimation of growth or fishery impact on length-at-age structure |
| State-space |  | State-space flexibility | Current practices in USA is focused on parametric modeling, SAM does not incorporate length in current form |
| Estimated age–length transition matrix |  | Allows for selectivity at length, tracks numbers- and age-at-length information | Slow due to dense matrices, does not track length-specific fishing impacts |
| Full age–length structure |  | Does not assume fixed age-length key with a potential to scale complexity/runtime | Could be computationally intensive based on bin size |
Table: Potential methods for fitting to length data in an assessment model.

##### Empirical length at age

Data

Empirical length-at-age data are observations of the relationship between the length of an organism and its age. Specifically, length data are coerced into a matrix of annual lengths for given age bin without regard to theory or logic, i.e., they are raw measurements without any associated uncertainty or model. The process of using annual length-at-age measurements in an assessment model is not particularly common but users might be familiar with the process given that empirical weight-at-age data are often used instead of estimating growth. Using empirical length-at-age data would allow for the estimation of length-based selectivity but it would not allow for the estimation of growth within FIMS. Thus, adding empirical length at age to FIMS would not necessarily slow down model run times.

Calculating empirical length-at-age data for input to the assessment model would likely employ the same process as empirical weight-at-age data where annual average weights for each age are calculated external to FIMS using data from all from measured fish. For years and ages where there are no measurements, borrowing rules can be used to fill in the missing values, e.g., similar ages within the year or similar years within the age. More recently, generalized linear mixed effects models have proven useful for estimating weight-at-age data and do not require the use of borrowing rules to fill in missing values (TODO: cite the pacific hake assessment or pollock) and similar models could be used for length at age.

Method

Within the assessment model, length-based selectivity would be incorporated into the model by specifying selectivity-at-age as a function of length at age, \f[ s_a = s\left[f\left(L_a\right)\right] \f] where \f$ f() \f$ could be a functional form (e.g., logistic, double-normal, etc.). TODO: More information here on the method.

Caveats

* Inability to incorporate uncertainty in the length-at-age distribution.
* Assumes no variability in length-at-age processes.
* Does not allow length-based mortality to influence the length structure of the population.

##### Age–to-length transition matrix

The use of an age-to-length transition matrix to fit length data within an age-structured assessment model is commonly used for models presented to the North Pacific Fisheries Management Council and is sometimes referred to as the Alaska Method.

Data

A age-to-length transition matrices are calculated external to the assessment model as passed as fixed input to the model. Each age category is represented with a row and each length category is represented with a column. Methods are needed to calculate the age-to-length transition matrix outside of the assessment model. TODO: Provide more information on how these matrices are calculated.

Method

Within the assessment model, the age-to-length transition matrix \f$\phi\f$ is used to convert ages to lengths and facilitates the fitting of marginal length-composition data. The population dynamics remain age based like a pure statistical catch-at-age model. Predicted proportions at age are converted to predicted proportions at length using the dot product of the predicted proportions at age and \f$\phi\f$. In theory, \f$\phi\f$ should be year \f$y\f$ and fleet \f$f\f$ specific but the current implementation in FIMS only allows for a single population-level matrix to be provided.

The yearly, fleet-specific predicted catch-at-age is calculated using element-wise multiplication

\f[ \hat{C}_{y,f,a}=F_{y,f,a}N_{y,a}\frac{(1-\text{exp}(-Z_{y,a}))}{Z_{y,a}} \f]

where \f$N_{y,a}\f$ is the abundance at age and \f$Z_{y,a}\f$ is the total mortality for year \f$y\f$ and age \f$a\f$ and \f$F_{y,f,a}\f$ is the fishing mortality for year \f$y\f$, fleet \f$f\f$, and age \f$a\f$.

The predicted abundance at age for a survey is calculated similarly but takes into account catchability, see `CalculateIndexNumbersAA()` in `models/functors/catch_at_age.hpp`.

For a fishing fleet, the predicted proportions at age (i.e., marginal age compositions) are obtained using:

\f[ \hat{p}_{y,f,a}=\frac{\hat{C}_{y,f,a}}{\sum\limits_{a}{\hat{C}_{y,f,a}}} \f]

Predicted proportions at age are converted to predicted proportions at length (i.e., marginal length compositions) using the dot product of \f$ \hat{p}_{y,f,a} \f$ and the age-to-length transition matrix \f$ \phi_{a,l} \f$:

\f[ \hat{p}_{y,f,l}=\hat{p}_{y,f,a}\cdot\phi_{y,f,l,a} \f]

Predicted survey age and length marginal compositions are obtained in an identical fashion using the age-to-length transition matrix specified for that survey but with the expected abundance rather than catch.

Caveats

* FIMS does not currently include length-based selectivity but this is being developed in a branch. Length-based selectivity could be transformed into age-based selectivity using the transpose of the age-to-length transition matrix. When marginal length compositions are fit, this would require an extra dot product calculation in the conversion from \f$ \hat{p}_{y,f,a} \f$ to \f$ \hat{p}_{y,f,l} \f$. Note that Stock Synthesis does the same calculations but using element-wise multiplication and summing over lengths or ages to get marginal age or length compositions, respectively. In Stock Synthesis, these calculations happen regardless of whether age or length-based selectivity is used. It is unclear if the element-wise multiplication approach in Stock Synthesis or the dot product approach have substantial impacts on model run time or if the model compiler will treat these approaches as equivalent.
* Does not allow for the estimation of growth because the age-to-length transition matrix is not estimated internally.

##### State space

State-space assessment models allow for the estimation of both the observed and unobserved processes (TODO: citation), which allows for collapsing the constructs of fishing mortality and selectivity into a single process. TODO: put this in context for fisheries, i.e., that you do not have to estimate selectivity because every process is in terms of age, e.g., F-at-age.

Two well-known state-space assessment frameworks are the State-Space Assessment Model (SAM), which is used in several ICES stock assessments (Nielsen and Berg, 2014; Berg and Nielsen, 2016) and the Woods Hole Assessment Model (WHAM; Stock and Miller, 2021). TODO: statement about these models and how the equation is is the same but in log space.

\f[ \text{log}\hat{C}_{y,f,a}=\text{log}F_{y,f,a}-\text{log}Z_{y,f,a}+\text{log}N_{y,a}+\text{log}(1-\text{exp}(-Z_{y,a})) \f]

... then extend just like Alaska method.

\f[ \hat{C}_{y,l}=\hat{C}_{y,a}\cdot\phi_{y,l,a} \f]

TODO: Decode this statement. Similarly could convert to \f$ F_{y,f,l} \f$ if needed:

\f[ F_{y,f,l}=F_{y,f,a}\cdot\phi_{y,f,l,a} \f]

Caveats

* A drawback of state-space assessment models is the inability to use parametric selectivity functions that are prevalent in the U.S.A.
* It is currently unknown how well \f$\phi\f$ can be estimated internally given the flexibility of a full state-space model. We do not currently know currently the trade offs for efficiency. <!-- Research Idea -->

##### Estimated age-to-length transition matrix

The age–to-length transition matrix can be estimated within the assessment model instead of specified as a fixed input. This method has a proven track record for estimating growth (TODO: citation) by fitting to numerous types of size-based data (e.g., marginal length-composition and conditional age-at-length data) and allowing for the estimation of length-based and/or age-based selectivity functions. The capacity to model length- and age-based selectivity effects simultaneously has been found to significantly improve fits to the composition data for red snapper in the southeast U.S.A. This method of being able to estimate the age–to-length transition matrix can be found in Stock Synthesis 3 (SS3; Methot and Wetzel, 2013) and was recently implemented in the ["growth"](https://github.com/GiancarloMCorrea/wham/tree/growth) branch of the Wood's Hole Assessment Model (WHAM; Stock and Miller, 2021; Correa et al., 2023).

Internally estimating the age-to-length transition matrix will also allow us to include a range of parametric, semi-parametric, and non-parametric growth functions using random effects (Correa et al., 2023).

Data

Both marginal age- and length-composition data as well as conditional age-at-length data are informative for the internal estimation of growth.

Method

Predicted catch-at-age and -length for year \f$ y \f$ and fishery \f$ f \f$ are calculated using element-wise multiplication

\f[
\hat{C}_{y,f,l,a}=\phi_{y,f,l,a}S_{y,f,l}S_{y,f,a}F_{y,f}N_{y,a}\frac{(1-\text{exp}(-Z_{y,a}))}{Z_{y,a}}
\f]

where \f$ N_{y,a} \f$ is the abundance-at-age \f$ a \f$ during year \f$ y \f$, \f$ Z_{y,a} \f$ is the total age-specific mortality in year \f$ y \f$, \f$ F_{y,f} \f$ is the fleet-specific (\f$ f \f$) annual fishing mortality, \f$ \phi_{y,f,l,a} \f$ is the fleet-specific age-length transition matrix (i.e., the proportion of fish of age \f$ a \f$ in length bin \f$ l \f$) for year \f$ y \f$, and \f$ S_{y,f,l} \f$ and \f$ S_{y,f,a} \f$ are the selectivity-at-length and selectivity-at-age, respectively. When an age-based selectivity \f$ S_{y,f,a} \f$ is not specified by the user then it is set to a matrix of 1s of the appropriate dimensions. Similarly when length based selectivity \f$ S_{y,f,l} \f$ is not specified it is set to a matrix of 1s. This structure allows users to utilize different selectivity forms between fleets as well as combinations of age and size selectivity patterns in a single fleet which can be useful, for instance to combine a gear-based, length-specific selectivity with an age-based selectivity due to age-specific migration.

Note that \f$ Z_{y,a} \f$ is age-based and relies on the calculation of total age-based fishing mortality \f$ F_{y,a} \f$, which in turn relies on age-based fishery selectivity. Therefore, when selectivity-at-length is used, it must first be converted to selectivity-at-age using the dot product of selectivity-at-length and the transpose of the age-length transition matrix:

\f[
S_{y,f,a}=S_{y,f,l}\cdot\phi_{y,f,l,a}^\top
\f]

This implied fleet-specific selectivity-at-age is used to obtain \f$ F_{y,f,a} \f$, which can be summed across fleets and combined with natural mortality to obtain \f$ Z_{y,a} \f$ for use in the predicted catch-at-age and -length equation above.

The predicted abundance index-at-age and -length for year \f$ y \f$ and index \f$ i \f$ is obtained using

\f[
\hat{I}_{y,i,l,a}=\phi_{y,i,l,a}S_{y,i,l}S_{y,i,a}N_{y,a}\text{exp}(-f_{y,a}Z_{y,a})
\f]

where \f$ f_{y,a} \f$ is the fraction of the year when the survey was conducted.

The predicted catch-at-age \f$ \hat{C}_{y,f,a} \f$ and catch-at-length \f$ \hat{C}_{y,f,l} \f$ is calculated as

\f[
\hat{C}_{y,f,a}=\sum_{l}\hat{C}_{y,f,l,a}
\f]

and \f[ \hat{C}_{y,f,l}=\sum_{a}\hat{C}_{y,f,l,a} \f]These are converted to predicted proportions-at-age and proportions-at-length (i.e., marginal age and length compositions, respectively):

\f[
\hat{p}_{y,f,a}=\frac{\hat{C}_{y,f,a}}{\sum\limits_{a}{\hat{C}_{y,f,a}}}
\f]

and

\f[
\hat{p}_{y,f,l}=\frac{\hat{C}_{y,f,l}}{\sum\limits_{l}{\hat{C}_{y,f,l}}}
\f]

Predicted marginal age and length marginal compositions for surveys are obtained in an identical fashion.

Caveats

* The standard normal cumulative density function used to generate the age-to-length transition matrix will result in a dense matrix, and when the dense matrix is multiplied by other quantities it will require a large amount of computation and result in long estimation times. Correa et al. (2023) tried to use `Eigen::SparseMatrix` to alleviate this issue in [WHAM](https://github.com/GiancarloMCorrea/wham/blob/3af37de1566f3a99b967901e540d81ef70f613a0/src/helper_functions.hpp#L1465) but it did not help. To speed up optimization once growth is estimated, methods should be developed to make the \f$\phi\f$ matrices sparse to reduce computation time. <!-- Research Idea -->
* One significant limitation of this approach is that while length-based selectivity patterns can be functionally specified in Stock Synthesis they do not actually impact the length-at-age structure of the population which is calculated from a normal distribution around mean size at age in each year/age calculation step. This results in the model requiring much if not all of the computational burden of a age-and-length conditioned model without many of the benefits. Length-specific effects such as over-depletion of fast growing individuals are hypothesized to be a potential source of the frequent conflict between length- and age-composition data in many assessments. This potential problem can be partially addressed in Stock Synthesis through the use of growth morphs which can represent sub-populations with differing growth rates. However, adding multiple growth morphs on top of the existing length-at-age redistribution function will even further slow model run times.

##### Full age--length structure

An alternate path for FIMS could be to specify abundance at age and length allowing for both age- and length-specific selectivity and growth-related fishery impacts such as depletion of fast-growing individuals. This scenario would track abundance at age and length explicitly rather than through a fixed age-to-length translation matrix.

\f[
\hat{C}_{y,f,l,a}=S_{y,f,l}S_{y,f,a}F_{y,f}N_{y,a,l}\frac{(1-\text{exp}(-Z_{y,a,l}))}{Z_{y,a,l}}
\f]

It will instead require abundance at size and age to be incorporated through a transfer array \f$tran_{a1,l1,a2,l2}\f$ used to specify the proportional movement between length bins for each age step.

\f[
\ N_{y+1,a2,l2}={\sum\limits_{{l}}} tran_{a1,l1,l2}S_{y,f,l}S_{y,f,a}F_{y,f}N_{y,a,l}\frac{\text{exp}(-Z_{y,a,l})}{Z_{y,a,l}}
\f]

This transfer array would specify the source of individuals for every length and age bin that is tracked. These values could be input as a fixed empirical array or calculated based on a parametric rule. A dense formulation assuming all possible transfer combinations between length bins at each age would likely be too slow for practical implementation requiring \f$\text{n_length_bins} \times \text{n_length_bins} \times \text{n_age_bins}\f$ computations. Computational efficiency could then be achieved by reducing the number of tracked bins and the number of bins between which abundance transfers occur to achieve sparsity. As an example, a Stock Synthesis growth-morph style could be used by having a limited number of length bins at each age representing different growth curves where individuals only grow along their growth curve, and thus, the transfer proportion between bins would be 1 for a single pair and zero for all others, which could be ignored, producing a sparse calculation of \f$\text{n_length_bins} \times \text{n_age_bins}\f$ calculations. If only a limited number of length bins are tracked in the population module, an interpolation approach will be needed to calculate abundance by length at age in composition bins that do not match the population bins. If growth is not estimated, then many functions could be used but if growth is estimated a dynamic approach adaptable to variable population bin values will be needed that does not require if statements. Bezier curve interpolation may work for this but needs investigation. <!-- Research Idea -->

Alternatively, in a state-space context the model could possibly be represented something like the following:

\f[ \text{log}\hat{C}_{y,f,a,l}=\text{log}F_{y,f,a,l}-\text{log}Z_{y,f,a,l}+\text{log}N_{y,a,l}+\text{log}(1-\text{exp}(-Z_{y,a,l})) \f]

\f[ N_{t2,a2,l2}= {\sum\limits_{{a1,l1}}} trans_{t2,a1,l1,a2,l2} \ N_{t1,a1,l1}(\text{exp}(-Z_{y1,a1,l1})) \f]

Where numbers at age \f$a2\f$ and length \f$l2\f$ at time \f$t2\f$ are calculated as a sum over a subset of ages \f$a1\f$ and lengths \f$l1\f$ at time \f$t1\f$ adjusted for mortality during the period from \f$t1\f$ to \f$t2\f$ and the proportion transitioning from \f$a1\f$ and \f$l1\f$ to \f$a2\f$ and \f$l2\f$ defined in the \f$trans_{t2,a1,l1,a2,l2}\f$ array.

This method has the benefit of being able to represent true age and length specific dynamics, while also being able to collapse down to a pure age- or length-structured model if the number of length or age bins was reduced to 1 such as is proposed in the fixed empirical length-at-age approach described above. Explicit age- and length-conditioned models, such as GADGET, have been developed in the past; however, they are often not fisheries focused and usually suffer from long run times. Options for achieving sparsity and reduced run-time include variable age/length bins such that only high frequency bins are calculated. For example, there are no 100 cm age zero fish and no 1 cm age 10 fish so we could assume a priori that they are 0 based on some external threshold. For this sparse approach to work in a model where length is estimated, an approach for adaptive length bins would be needed to ensure that the full size structure is captured by the model.

#### Indexing system components

As the complexity of the catch-at-age model in FIMS grows so will the number of system components that need indexed and how each component interacts. The following three goals should be kept in mind while reading this section

* FIMS Needs to allow for a multi-sex population structure that includes hermaphrodites.
* The structure needs to be easily generalizable to new levels of modeling complexity, including multiple areas.
* FIMS needs to maintain maximum sparsity as dimensionality increases to optimize performance.

Some use cases that have been thought of include using subpopulations to allow for predators and prey in the same model, males and females to be separate sub-populations, density-dependent mortality between age groups, sub-populations within an area, etc. Additional thought needs to be put into what would happen if each sup-population or data source has different years of data, would the extra years be filled with NAs or would we allow vector lengths to differ? There would also need to be some way to specify the type of link between sub-populations and method to operate on a one-step lag.

There are several approaches available to indexing system components in an assessment model. Below we outline each of the methods and weigh the pros and cons for including each particular method in FIMS.

| Approach | Pros | Cons |
|------------------|------------------|-----------------------------------|
| Populations are subpopulations | Subpopulation is easily interpretable and follows structure we are familiar with, computationally simple | Dynamics between sub-populations occur at a different time step than those that can occur between ages within a sub-population, limits the applicability of density-dependent processes such as cannibalism, less flexible than specifying fully flexible population segments since each population must have age structure, does not allow for dynamic estimation of the female/male ratio |
| Dynamic subpopulations | Subpopulations do not need age structure and interact in real time, and are very flexible and extensible | Bookkeeping might mean replicating complex structures anyway |
| Biological subpopulations | Easier biological interoperability | Harder to extend to new segments |
Table: Potential methods for indexing system components in an assessment model.

##### Populations as subpopulations

In this scenario the existing population module becomes a sub-population. In this case each sub-population contains age/time structure with its own class. Sub-populations would be generalizable to different partitions, such as sex, area, and species, with a common set of functions that govern (1) interactions between sub-populations and (2) movement between sub-populations. Currently, each population within FIMS has members unique to each population. These members include parameters, derived quantities, and fixed values such as year and age dimensions.

If this were to be implemented in FIMS, the following would be requirements of the implementation:

* No increase in complexity for the single-population case
* Easy to share parameters across populations
* Easy to aggregate derived quantities across populations
* Fleets operate on one or more populations
* Must define the metrics for success, where comparisons to the multi-population version of the Woods Hole Assessment Model and Stock Synthesis could be made?
* Reference points are specific to each population

The design of populations as subpopulations within FIMS would require

* TMB `map` function to mirror parameters if sub-populations need to share
* Can store pointer to the linked population within the Information class and reference it within the other population
* Fleet-specific methods (`CalculateCatch`, etc.) will need to move from `Population` to `Fleet`
* Add `Fleet->Evaluate` and associated call to `model.hpp`

To enable one fleet to act on multiple populations, we will need to calculate catch outside of the population loop. Currently, we loop through fleets within a population because Population->Prepare loops through Fleet->Prepare, CalculateMortality loops through fleets to sum up \f$F\f$, CalculateCatch sums up catch weight at age and also expected catch, and CalculateIndex, CalculateCatchNAA, CalculateCatchWAA report expected index, catch numbers at age, and catch weight at age, respectively. Additionally, the prepare functions set values to zero where we would not want to reinitialize for each population.

##### Unique units as subpopulations

In this scenario a sub-population would be created for every unique sub-unit of the population, for instance all individuals of a single age/size/sex/area/species with whatever level of resolution the user chooses to specify. In this scenario all dynamics of the model are specified as transitions and interactions between these sub-units. Each sub-population would have it's own unique parameter values that could be specified to be fixed for that sub-population (such as growth or mortality rate), or dynamically updated as individuals transfer into the sub-population (such as average age or size), where the updated value could be an abundance weighted average of current and incoming individual values.

A standard age structured model would have \f$n+1\f$ subpopulations for the number of ages in the population and a plus group. Transition dynamics would then be 100% movement of individuals from one age bin to the next at each annual time step with the age zero bin being filled via a recruitment function and the plus group applying mortality and 1 year of aging to the plus group and then averaging that age and size with the incoming individuals from the last tracked age bin based on their relative abundances.

A purely length structured model could be produced by using \f$n\f$ lengths subpopulations and defining transition dynamics between those length bins at each time step. From these a fully age/length specific model could be expanded if desired and sparsity maintained by only having length bins for the reasonably abundance lengths at each age. Using this type of structure it may also be possible to represent fishing and survey fleets as subpopulations as their interactions are analogous to predation in a multispecies model.

Once could even explicitly model mature and immature individuals as separate subpopulations to enable different growth and fishery selectivity impacts. [Berger et al. (2024)](https://doi.org/10.1016/j.fishres.2024.107008) shows how this method breaks down because areas implemented as a population can be too restrictive if there is much adult movement.

##### Biologically-identified sub-populations

Biologically-identified sub-populations are used in Stock Synthesis. Each sub-populations has the following characteristics: sex, growth morph, settlement timing (within year), and platoon (for implementation of size-based survivorship). For each biology x settlement timing, fish are distributed across sexes and across platoons at birth. All reside in a numbers-at-age matrix \f$natage(t,p,g,a)\f$ where \f$t\f$ is year and season, \f$p\f$ is area, \f$g\f$ is biology, and \f$a\f$ is age. A fleet operating in area \f$p\f$ affects all biological groups in that area according to their selectivity and catchability. Recruits are distributed across populations and areas at birth. When fish of a given \f$g\f$ move between areas, they are still the same \f$g\f$ biologically. This is a bit limiting as it means that growth rate cannot be area-specific in Stock Synthesis. Although a fast-growing morph could recruit into one area and a slow growing morph into another area and they can move among the areas.

Alternatively in FIMS, growth could be area-specific and, when fish move between areas, the mean size of fish in each destination area would be the weighted mean of resident and immigrant fish. This means that mean size-at-age would be very dynamic and cause lots of computations.

### Surplus production model

Surplus production models (SPMs) are used for the management of many data-moderate populations and as a diagnostic tool for age-structured assessment models. **The goal is to be able to use FIMS to run an SPM using the same data input structure as a statistical catch-at-age FIMS model (FIMSframe) and a new SPM module, to produce estimates of biomass, harvest rate, and management reference points.** Current best practice recommendations for SPMs ([Kokkalis et al. 2024](https://doi.org/10.1016/j.fishres.2024.107010)) include the use of

1. state-space models (e.g., [SPiCT](https://github.com/DTUAqua/spict) and [JABBA](https://github.com/jabbamodel/JABBA/tree/master))  
2. Bayesian frameworks or penalized likelihood with priors for MLE
3. no priors on stock-specific model parameters (carrying capacity and maximum sustainable yield) when fitting data to an SPM using MLE  
4. uninformative priors when fitting data to an SPM using Bayesian methods  
5. convergence checks, residuals, prior-posterior distribution comparisons, retrospective analysis, hindcasting analysis (sequentially removing data and testing the model's predictive ability for removed data), and jitter analysis as diagnostics  
6. stochastic reference points that include correction factors that depend on estimated error from biomass process when shape parameter is \> 1 (if m \< 1, use deterministic reference points)

SPMs combine all aspects of a population's growth, recruitment, and mortality into one production function and assume that the effect is the same on all parts of the modeled population. This is a major difference between SPMs and statistical-catch-at-age models, which separate population dynamic relationships and age-classes. Note, because there is no age- or size-specific information, there are no age- or size-dependent processes in the model and the portion of the population that is being modeled is the portion that is reflected by the indices of abundance. SPMs strive to describe the population dynamics over time based on the stock's response to fishing pressure (reflected in the index of abundance or catch per unit effort, CPUE, trends) and the historical catches, which provide an indication of the scale of the population.

#### Equations

The goal is to estimate the unobserved quantity of the total population biomass for a given year. To do this, we rely on observed fishery catch data and indices of abundance, such as catch-per-unit-effort from a fishery or a relative index of abundance from a fishery-independent survey. There is a strong assumption that the indices of abundance provide a good idea of how the population responds to fishing pressure over time so the trend of the indices should represent the trend of the population’s biomass over time. While indices of abundance inform the trend of the population over time, catch data informs the scale of the population. In a surplus production model, the total population biomass in year \f$t\f$ is calculated by:
\f[
\begin{align}   
\nonumber B_{0} &= B_{init},\quad t = 0  \\
\nonumber B_{t+1} &= B_{t} + rB_{t}(1-\frac{B_{t}}{K})-C_{t},\quad t = 1…T     
\end{align}
\f]
where \f$t\f$ \= year, \f$B\f$ \= biomass, \f$B_{init}\f$ \= initial biomass, \f$K\f$ \= carrying capacity, \f$r\f$ \= intrinsic growth rate of the population, and \f$C\f$ \= catch. Biomass in year \f$t+1\f$ is dependent on biomass in year \f$t\f$, the amount of production (recruitment, growth, and death) in year \f$t\f$, and catch in year \f$t\f$. If data is available from the assumed start of the fishery, \f$B_{init} = K\f$, otherwise, initial biomass can be estimated based on initial depletion (\f$\psi\f$): 
\f[  
B_{init} = K\psi
\f]
To estimate the unobserved quantity of biomass, we can fit the model to the indices of abundance. Based on the assumptions of the model, biomass is related to the indices of abundance by a catchability, \f$q\f$, parameter:   
\f[
\hat{I}_{t,f} = \frac{C_{t,f}}{E_{t,f}} = q_{f}B_{t}  
\f]
where \f$\hat{I}_{t,f}\f$, is the predicted index of abundance in year \f$t\f$ for fleet \f$f\f$, and \f$q_{f}\f$ is the catchability coefficient (the amount of biomass or catch taken/one unit of effort) for fleet \f$f\f$. It is assumed that the index of abundance is a good representation of the population’s trend and response to fishing pressure over the timeseries. Therefore, the model tries to minimize the difference between the observed index values (\f$I_{t,f}\f$) and the predicted index values (\f$\hat{I}_{t,f}\f$) by finding the values for \f$B_{t}\f$ and \f$q_{f}\f$ that best fit the observed index values.   
The production function (represented by \f$rB_{t}(1-B_{t}K\f$) in the biomass equation) can be parameterized in several ways. 


### Schaefer model 
### Schaefer model 
\f$
f(B_{t}) = rB_{t}(1-\frac{B_{t}}{K})
\f$
### Fox model
\f$
f(B_{t}) = log(K)rB_{t}(1-\frac{log(B_{t})}{log(K)})
\f$
### **Pella-Tomlinson model**
\f$
f(B_{t}) = \frac{r}{m-1}B_{t}(1-(\frac{B_{t}}{K})^{m-1})  
\f$
f(B_{t}) = rB_{t}(1-\frac{B_{t}}{K})
\f$
### Fox model
\f$
f(B_{t}) = log(K)rB_{t}(1-\frac{log(B_{t})}{log(K)})
\f$
### **Pella-Tomlinson model**
\f$
f(B_{t}) = \frac{r}{m-1}B_{t}(1-(\frac{B_{t}}{K})^{m-1})  
\f$

where \f$m\f$ is the shape parameter that determines the \f$B/K\f$ ratio where maximum surplus production is attained at. If \f$m = 2\f$, the model reduces down to the Schaefer model, if \f$m \approx 1\f$, the model reduces to the Fox model but there is no exact solution if \f$m = 1\f$. **We decided to use the Pella-Tomlinson implementation of the production function because it is the most flexible model, the shape parameter at 2 will give a Schaefer model and at 1 will give the Fox.**


##### State-space formulation

State-space models are a type of hierarchical model that allows the natural variability in the environment (process error, \f$\sigma^_{2}\f$) to be modeled separately from the error associated with observed data (observation error, \f$\tau^{2}\f$). To help with computational estimation, the model can be re-written in terms of depletion, \f$P_{t}\f$,  where \f$P_{t} = B_{t}K\f$ is an unobserved state. A Bayesian state-space formulation (Meyer and Millar, 1999) can be written as:  
\f[
\begin{align}
\nonumber P_{0} &= \psi, \quad t = 0\\  
\nonumber P_{t+1} &= P_{t}+\frac{r}{m-1}P_{t}(1-P_{t}^{m-1}) - \frac{C_{t}}{K}, \quad   t=1,...T \\ 
\nonumber P_{t} | P_{t}, \sigma^{2} &\sim lognormal(ln(P_{t}), \sigma_{t}^{2}), \quad   t=0,....T  
\end{align}
\f]

and the depletion is then fit to index of abundance assuming a lognormal distribution:
\f[ 
I_{t,f}| P_{t},K,q_{f},\sigma^{2} \sim lognormal(ln[q_{f}P_{t}K], \tau^{2}) \quad  t=1,...T  
\f]
where \f$\psi\f$ is initial depletion (can be assumed to be 1 or estimated), and depletion in year \f$t\f$ (\f$P_{t}\f$) is log-normally distributed with a mean of \f$ln(P_{t})\f$ and log-normal process error variance (\f$\sigma^{2}\f$) and the expected index of abundance value is lognormally distributed with a mean of \f$ln[q_{f}P_{t}K]\f$ and log-normal observation variance of \f$\tau^{2}\f$. Annual biomass can then be calculated as:   
\f[
B_{t}= P_{t}K
\f]

##### Derived Quantities and Reference Points

Annual harvest rate ($H_{t}$) is calculated by:   
\f$
H_{t} = \frac{C_{t}}{B_{t}}  
\f$
A penalty should be added to ensure that harvest rate does not go above 1.0, because while this may be possible mathematically, it is not possible biologically (cannot have more catch than biomass in a given year).   
In the Pella-Tomlinson parameterization, the shape parameter, \f$m\f$ can be directly linked to biomass at maximum sustainable yield, \f$B_{MSY}\f$, by the ratio of \f$\frac{B_{MSY}}{K}\f$ by:

\f$  
\frac{B_{MSY}}{K} = m^{\frac{-1}{m-1}},  
\f$

therefore $B_{MSY}$ can be calculated as:

\f$  
B_{MSY} = Km^{\frac{-1}{m-1}}.  
\f$

The fishing mortality at maximum sustainable yield (MSY) can be calculated as:  

\f$
F_{MSY}=\frac{r}{m-1}(1-\frac{1}{m}).  
\f$

And MSY is given as: 

\f$  
MSY=F_{MSY}B_{MSY}
\f$

#### Input Requirements

##### Data

* Time series of catch (as complete as possible)
* Time series of index of relative abundance (fishery-independent or CPUE) with measure of uncertainty (annual CVs)

##### Parameters

In a Bayesian framework, priors for all parameters are needed. Basic parameters and their related attributes are given in the following table:

| Name | Description  | Distribution | Indexed by |
| :---: | :---- | :---- | :---: |
| r | Intrinsic population growth rate | Lognormal | run |
| K | Population carrying capacity | Lognormal | run |
| m | Pella-Tomlinson shape | Lognormal | run |
|  | Initial depletion | Lognormal or Beta | run |
| q | Catchability | Lognormal | fleet |
|  | Process error | Inverse gamma | year |
|  | Observation error | Inverse gamma | fleet and year |

Note: Here is a running [list of parameters](https://docs.google.com/spreadsheets/d/1SnlXcfL90w6lEbPx1eRVRBXAEP3_97tzE1Oa_n2ivpM/edit?gid=2063265688#gid=2063265688) (names and abbreviations) that are currently in FIMS. Naming conventions and names are currently being discussed and will be modified after M2.

##### Model Setup

To run in a Bayesian framework, users will also need to specify some settings for the Markov Chain Monte Carlo (MCMC) sampling including:

* Number of chains  
* Number of cores (for parallel processing)  
* Number of iterations  
* Number of burn-in (draws thrown away from the beginning of each chain)  
* Thinning (rate at which sample draws are kept in the output, e.g., 10, keep every 10th draw)  
* Seed (to ensure reproducibility)

All sampling functionality will be done using `tmbstan()` or some other sampling function from a separate R package.

#### SPM modules added to code base

The code for running an SPM will be added into surplus\_production\_model.hpp and include the following method calls.

```c

  // Sample the distributions. (N.B. The sampling will most
  // likely be elsewhere.)
  sampleDistributions(
      num_years, num_fleets,
      sample_r, sample_K, sample_m, sample_psi, sample_q,
      process_error_sigma, observation_error_tau);

  // Calculate the depletion vector
  calculateDepletionP(
      num_years, obs_catch, sample_r,
      sample_K, sample_m, sample_psi,
      process_error_sigma, depletion_P);

  // Calculated the expected abundance and then minimize 
  // for the estimated parameters
  calculateExpectedAbundance(
      num_years, num_fleets, depletion_P, abundance_observed_I,
      sample_r, sample_K, sample_m, sample_psi, sample_q,
      process_error_sigma, observation_error_tau, abundance_expected_I);

  // Calculate minimum estimated parameters for 
  // number of years and fleets. (N.B. This will be done in
  // nlimb() or in tmbStan.)
  calculateEstimatedParameters(
num_years, num_fleets,
       abundance_observed_I, abundance_expected_I,
       sample_r, sample_K, sample_m, sample_psi, sample_q,
       process_error_sigma, observation_error_tau,
       estimated_r, estimated_K, estimated_m, estimated_psi,
       estimated_q, estimated_sigma, estimated_tau);

TODO: The following 3 modules need to work on every draw.

  // Calculate F(MSY) and B(MSY)
  calculateReferencePoints(
       final_r, final_K, final_m, FMSY,BMSY,MSY);

  // Convert depletion P to biomass B
  calculateExpectedBiomass(
       num_years, final_K, depletion_P, biomass_expected_B);

  // Calculate harvest rate H
  calculateHarvestRate(
       num_years, obs_catch, biomass_expected_B, harvest_rate_H);

```

#### Proposed wrapper functions for running an SPM with FIMS

We propose to create wrapper functions (currently being implemented by FIMS Wrapper Function group for SCAA) that the user can call to build and fit a surplus production model.

Proposed FIMS SPM usage (similar to JABBA):

```r
# User knows what's required (and appropriate units/scale) by running: ?FIMS::build_spm
?FIMS::fit_spm

# Define inputs here
fishery_catch <- x
fishing_fleet_indices <- x
survey_fleet_indices <- x

initial_values_[1..num_chains] <- list(
  population_growth_r,
  carrying_capacity_K,
  shape_m,
  initial_depletion_psi,
  catchability_q,
  observation_error_tau,
  process_error_sigma
)

initial_values_full <- list(
  initial_values_1,
  initial_values_2,
  …
  initial_values_[num_chains]
)

prior_distributions <- list(
  population_growth_r,
  carrying_capacity_K, 
  shape_m,
  initial_depletion_psi,
  catchability_q,
  observation_error_tau,
  process_error_sigma
)
mcmc_settings <- list(
  number_chains,
  number_cores,
  number_iterations,
  length_burn_in,
  thinning_rate,
  seed
)

# Combine fishing and survey fleet data in the same tables.
# Formatted as FIMSframe with, for example, first column "Fleet" 
# denoting "Fishing" or "Survey".
indices_of_abundance:
    fishing_fleet_indices = …,
    survey_fleet_indices = …,

# Build the SPM object from the previously defined inputs
spm_object = FIMS::build_spm(
  catch = fishery_catch,
  indices_of_abundance = indices_of_abundance,
  initial_values = initial_values,
  prior_distributions = prior_distributions
)

# Run the model
spm_output = FIMS::fit_spm(
  spm_object = spm_object,
  optimization = BAYESIAN | MLE,
  markov_chain_monte_carlo = mcmc_settings
)
```

#### Running an SPM

Based on the current [GOA Pollock case study](https://github.com/NOAA-FIMS/case-studies/blob/3e8f058d636ccc1737e17a063e7df4ec03c8106e/content/AFSC-GOA-pollock.qmd#L229), running a FIMS model with `tmbstan()` can be done as follows. Note that code for implementing priors is under development and can be added once it is complete.

```r
parameters <- list(p = get_fixed())
map <- parameters
obj3 <- MakeADFun(
  data = list(),
  parameters,
  DLL = "FIMS",
  silent = TRUE,
  map = map
)
fit <- tmbstan(
  obj3,
  chains = 1,
  cores = 1,
  open_progress = FALSE,
  init = "last.par.best",
  control = list(max_treedepth = 10)
)
```

#### Outputs

The current output of FIMS is a JSON file with all parameter estimates. The R interface group is currently working on writing R code to take the output and separate it into 2 "tibbles", one with "estimates" and one with "fits" (R code can be written later so no need to focus too much on that now). SPM output should follow the same format, making sure to include all information that would be needed. Expected outputs from an SPM would include: 

* "Estimates" tibble:   
  * Posterior distributions of annual biomass and harvest rate  
  * Posterior distributions of all parameters   
  * Posterior distributions of reference points   
* "Fits" tibble:   
  * Expected values for CPUE/indices of abundance 

Additional information to include with the values for bookkeeping would include: 

* Time  
* Prior (for MLE format we use initial value)  
* Label (for estimates, e.g. b, f, etc.)  
* Unit   
* Distribution   
* Type  
* Name (for fits, e.g. "survey1")

FIMS uses tables in a "long" format, so for each parameter (or year of a timeseries), there would be the same number of rows as draws that were saved in the output. For example, if a user ran a model with the following settings: 

* 3 chains  
* 1,000 burn-in (or sometimes called warmup)  
* 10,000 iterations  
* 10 thinning interval

((10,000-1,000)/10)*3 = 2,700,  
they would have a total of 2,700 draws. An example of what the output table would look like is: 

| Label | Time | Initial  | Estimate  | … (additional columns) |
| :---- | :---- | :---- | :---- | :---- |
| biomass | 2000 | NA | 12000 |  |
| biomass | 2000 | NA | 12002 |  |
| …row 2,700 | 2000 | NA | 12010 |  |
| biomass | 2001 | NA | 11800 |  |
| biomass | 2001 | NA | 11950 |  |
| …row 5,400 | 2001 | NA | 11865 |  |

#### Comparison of requirements for models along the continuum

|  | SPM | Delay Difference | ASPM | SCAA |
| :---- | ----- | ----- | ----- | ----- |
| **Data Inputs** |  |  |  |  |
| Catch  | X | X | X | X |
| Index of   Abundance/CPUE | X | X | X | X |
| Length Comp |  |  |  | X |
| Age Comp |  |  |  | X |
| **Parameters** |  |  |  |  |
| Catchability, q | X | X | X | X |
| Observation   error | X | X | X | X |
| Natural   Mortality, M  |  | X | X | X |
| Growth |  | X | X | X |
| Weight-at-Age |  | X | X | X |
| Steepness |  | X | X | X |
| Initial   Recruitment, R0   |  | X   | X | X   |
| Initial   depletion | X |  | X | X |
| Maturity |  |  | X | X |
| Max age |  |  | X | X |
| Selectivity |  |  | X | X |
| Process error | X |  |  | X |
| Intrinsic growth   rate, r | X |  |  |  |
| Carrying   capacity, K | X |  |  |  |
| Shape, m | X |  |  |  |
| Proportion   Female |  |  |  | X |

### Future models


## Projections

Successful fisheries management depends upon the implementation of regulations that achieve optimal sustainable yields. Additionally, control rules for determining future catch in federally managed US fisheries cannot exceed a 50% probability of overfishing (Code of Federal Regulations, 50 CFR Ch. VI:600.310, 2012). Age-structured stock assessments, in particular, achieve these goals by:

1. fitting models to historical data such as landings, age/size compositions, and indices of abundance to estimate population dynamics, fisheries selectivity, and fishing effort;  
2. making assumptions about future population dynamics and fisheries selectivity to estimate the equilibrium fishing effort required to achieve target reference points such as maximum sustainable yield (MSY) or spawning biomass depletion levels; and  
3. projecting short-term catch limits that would be achieved by fishing at the target found in (2).

Many approaches to (2) have been developed and implemented in current platforms used to conduct stock assessments. These approaches differ based on the extent of the simplifying assumptions made for future catches, fleet dynamics, environmental conditions, and biological parameters. Increasing the number of assumptions may speed up the calculations but estimates may suffer in their accuracy. Common simplifying assumptions include assuming constant selectivity and relative fishing mortality rate between fleets, which often allows an analytical solution to equilibrium yield without running long \~100 year projections.

While useful in some cases, these analytical solutions given simplifying assumptions often perform poorly in cases where nonstationarity is likely, such as multi-area models or when management actions induce changes in selectivity over time. In these cases, reference points can be more accurately estimated by projecting future fishing effort subject to management constraints for enough years that an equilibrium population status is achieved. Future fishing effort can then be scaled to a level that achieves the targeted population status, i.e., (3).  
The approach of projecting forward to an equilibrium state for estimating reference points and catch limits will be accomplished in FIMS by extending the model forward in time with assumed environmental conditions and priors that constrain parameters of interest such as future fishing mortality rates. This generalized structure will enable flexibility for the diversity of projection concerns posed by regional fishery management councils.

### Projections

The equations and C++ code that specify the population dynamics in FIMS for the model-fitting process are the same code that specifies the projections. Using the same code for projections decreases the amount of code that must be maintained and minimizes the chances for inconsistencies between the fitting and projection processes. Historically, this has not always been the default for assessment software; many platforms had limited functionality such that users had to write their own projection modules to meet management needs or did not offer projections at all (see examples of this in the Background section). Users can either extend their time series for projections during the fitting process or they can use `predict()` on a fitted model to get information about future scenarios. The `predict()` function can be more efficient than refitting the model from initial values for each future scenario that managers wish to gain information about. Additionally, users can set up multiple calls to `predict()` using {purrr} or a similar package like {furrr} to run several future scenarios at once in parallel or run the same scenario on more than one fitted model.

#### Background

Several assessment methods that predate FIMS have projection modules that are implemented externally to the main code base. The following list is just a subset of those that are available:

* SSfuture was written in C++ to perform projections for models fit using Stock Synthesis because the functionality provided within the forecast file does not allow for the flexibility needed for the management of ISC Albacore and IATTC Bigeye.  
* [Decision Support Tool](https://github.com/nathanvaughan1/DST) was written by Nathan Vaughan to allow for future patterns to be different from historical patterns to overcome a limitation of Stock Synthesis.  
* [SSMSE](https://github.com/nmfs-ost/SSMSE) was written by Kathryn Doering and Nathan Vaughan to facilitate manipulating the forecast file of Stock Synthesis to allow for management strategy evaluations (MSEs)
* AFSC [standard projection model](https://github.com/afsc-assessments/spmR) was written by Jim Ianelli and accounts for uncertainty through recruitment draws; the framework can look at correlations between population processes and projects that forward   
* [AGEPRO](https://github.com/PIFSCstockassessments/AGEPRO) was written by Jon Brodziak

Projections in FIMS were designed to limit the need for users to write external models to project a fitted FIMS model. Additionally, the following limitations of other platforms were key in decisions that were made for FIMS:

* TODO: fill out a list of limitations about other frameworks  
* 

TODO: Reference or summarize the material that Joel Rice is creating regarding how other platforms handle projections.

#### Methods

Users will be able to specify projections deliberately by modifying the data and parameters that go into a FIMS model, which we refer to as the "power-user method," or using `predict()`, the "wrapper-function method." Regardless, projections will accommodate additional data, e.g., catch time series, environmental time series, etc., and priors on parameters or derived quantities from the projected time steps. It is much easier to wrap one's head around what future data look like, e.g., managers have specified catch limits for the upcoming two years for each fleet that must be removed from the total biomass. It is slightly harder to imagine how priors work on future parameters. 

##### Data

The data frame used to fit the model can be extended to include data in any future year.

##### Priors

Users can either fix future parameters at a specified value, i.e., the ultimately informative prior, or can place priors on future parameters or derived quantities. For example, you may know that managers want to fish the population at 40% and there is only one fleet so you can either fix future F parameters at 0.4 or place a strong prior on them for the years in the model that are projected to achieve this desired rate of fishing mortality. It is slightly less intuitive to think about how one would want to place a prior on a derived quantity. In the previous scenario but with two fleets, managers might want to continue to fish the population at 40% but they might want to change the allocations of the quota throughout time between the two fleets. Thus, the proportion of total catch caught by each fleet, which is a derived quantity, can be constrained within each year by utilizing a time-varying prior. The ability to place priors on parameters and derived quantities allows for

* **Incorporation of expert opinion:** Stakeholder knowledge or scientific consensus on future trends can be directly integrated.  
* **Sensitivity analysis:** Different assumptions about future conditions can be explored by varying prior specifications.  
* **Constraining outcomes:** Priors can help guide projections towards plausible ranges, especially when data is sparse for future periods.

#### Code

##### predict()

Projections will be run using `predict()` , which is a generic S3 method used to make predictions from various model fitting functions. We chose to use this generic function rather than one named "projection" to increase the consistency of FIMS with other modeling frameworks. For example, when you run a linear model in R you can use the [predict function](https://www.rdocumentation.org/packages/stats/versions/3.6.2/topics/predict.lm) to predict the outcome to new values, and you can also do this for models fit using {sdmTMB} using [`predict.sdmTMB`](https://pbs-assess.github.io/sdmTMB/reference/predict.sdmTMB.html). Note that the ".sdmTMB" just denotes the method, users only interact with `predict()`, i.e., the FIMS function in the code base will be `predict.FIMSFit` but users will never know. This generic function allows for the implementation of methods based on the class of the object passed to its first argument named `object`. We will use standard arguments used by `predict.lm` , e.g., `object`, `newdata`, and `type`, all arguments also used by `predict.sdmTMB`, as well as additional arguments.  
TODO: implement `predict()` has yet to be implemented in FIMS.  
Below is pseudo code on how predict could be used for FIMS.

```r
# Create parameters
fleet1 <- survey1 <- list(
  selectivity = list(form = "LogisticSelectivity"),
  data_distribution = c(
    Landings = "DlnormDistribution",
    AgeComp = "DmultinomDistribution",
    LengthComp = "DmultinomDistribution"
  )
)
names(survey1[["data_distribution"]])[1] <- "Index"
parameters <- FIMSFrame(data1) |>
  create_default_parameters(
    fleets = list(fleet1 = fleet1, survey1 = survey1)
  )

# Run the  model with optimization
fit <- parameters |>
  initialize_fims(data = FIMSFrame(data1)) |>
  fit_fims(optimize = TRUE)

# Project the model

newparams <- parameters |> 
  modify_parameters() |> 
  modify_priors()

predict(
  fit,
  newdata,# could include future catch
  newparams, # includes parameters and prior specs
  type = c('link', 'response')[2],
  se.fit = FALSE,
  do.bias.correct = FALSE,
  bias.correct.control = FALSE
) 
```

* TODO(Today): what should be arguments in predict versus modifiers using |\>, e.g., modify\_priors()  
* Number of arguments \- combine configuration into a tibble? Example from [indexwc](https://github.com/pfmc-assessments/indexwc/blob/main/data-raw/configuration.csv)  
* Set years where take out removals in projection years, functionality for years specific percentages (e.g. OFL out each year)  
* Arguments related to running MCMC sampling (e.g. in sdmTMB mcmc\_samples argument)  
* Consider time-varying with respect to reference points, how would be address these?  
* Re form flexible for speed and turning random effects on/off \- not high priority  
* Se.fit can capture a lot of options, eg. CI and prediction intervals, data model uncertainty  
* Object that shows what options were in predict functions so they can be saved \- can this be added to the JSON output?  
* In sdmtmb, tmb object reinitialized with parameters, redo model configuration settings, use simulate function  
* tinyvast has a more efficient way to set up predictions  
  * Here's the prediction in tinyvast R: [https://github.com/vast-lib/tinyVAST/blob/50580ec2cc77e9becfaef80f516688f9a01dd7e1/R/predict.R\#L63](https://github.com/vast-lib/tinyVAST/blob/50580ec2cc77e9becfaef80f516688f9a01dd7e1/R/predict.R#L63)  
  * Here's the TMB side: [https://github.com/vast-lib/tinyVAST/blob/50580ec2cc77e9becfaef80f516688f9a01dd7e1/src/tinyVAST.cpp\#L1139](https://github.com/vast-lib/tinyVAST/blob/50580ec2cc77e9becfaef80f516688f9a01dd7e1/src/tinyVAST.cpp#L1139)  
* Need to consider period of biology, selectivity period, recruitment regimes to base projections off of

##### Predict functions from other R packages:

* [lme4:](https://rdrr.io/cran/lme4/man/predict.merMod.html)
* [glmmTMB:](https://www.rdocumentation.org/packages/glmmTMB/versions/1.1.11/topics/predict.glmmTMB)  
* [sdmTMB](https://rdrr.io/github/seananderson/sdmTMB/man/predict.sdmTMB.html):  
* [WHAM](https://github.com/timjmiller/wham/blob/47506172e9a55376137501608e645ddae8a574e9/vignettes/ex03_projections.Rmd#L144):  

##### Extending the input data

Projections in FIMS started as a proof of concept to see if the time series of a fitted model could be extended without changing the parameter estimates of the model without those additional years. Thus, it was natural to allow users to continue this proof of concept to perform their projections at the same time as fitting their model. The caution with this method is that the model will often not be started at ideal initial conditions and thus may take longer to fit than if the same specifications are passed to `predict()` because the wrapper function starts the estimation at the best estimates rather than user-specified initial values.

### Reference Points

Assessment scientists have used reference points for over 50 years to provide a scientific evaluation of the status of a given population and reference points are emerging as one of the most widespread and effective bases for modern fisheries management ([Pew Charitable Trust, 2016](https://www.pew.org/-/media/assets/2019/07/harvest-strategies/hs_reference-points_update.pdf)). For simple models, e.g., Surplus Production Models, some reference points are estimable just like any other population-dynamics parameter. But for age- and length-structured models, reference points must be calculated through optimization of derived quantities such as catch and cannot be explicitly estimated as model parameters.

#### Background

TODO: Text on the background of reference points and how they are equilibrium-based reference points.

##### Biological Reference Points

Biological reference points (e.g., maximum sustainable yield (MSY), spawning stock biomass at MSY (Bmsy), fishing mortality at MSY (Fmsy)) will be calculated based on the estimated population dynamics parameters from the model and their projected trajectories.

##### Management Reference Points

Management reference points, often based on policy objectives, will also be derivable. These might include:

* **Target reference points:** Desired states for the fishery (e.g., target biomass, target fishing mortality).  
* **Limit reference points:** Thresholds that should not be exceeded to avoid undesirable outcomes (e.g., minimum spawning stock biomass).

##### Historical Methods

##### Grid-Search

##### Optimization

##### Long-Term Projection

### Questions

1. How do you specify a projection without any catch because starting with Stock Synthesis v.3.30.16 users are able to specify future dynamics without any catch. 

## Random effects and Bayesian estimation

### Background

FIMS, while developed to be portable between statistical optimization software, is currently depending on the software, Template Model Builder (Kristensen et. al. 2015), otherwise known as TMB. This package evaluates the Laplace approximation to integrate out random effects, after which, optimization of the marginal likelihood occurs outside of TMB in R using available minimization software packages. The current dependency on TMB allows for fast random effects estimation, and due to its associated R package tmbstan, allows for easy integration with Bayesian modeling via STAN.  

FIMS architecture needs to be expanded to include State Space and Bayesian capabilities. This expansion requires a refactor of the architecture underlying the distribution components of the model that are summed to produce the negative log-likelihood (nll). In general, the proposed expansion will allow a generalized framework for time-varying parameters allowing for random effects, and the ability to include Bayesian priors on parameters.

This development will also structure the distribution functions so that it is straightforward for the user to select from a choice of distributions for a given dataset, random effect, or Bayesian prior. Development will allow for a library of probability distributions that can be used across FIMS. Each distribution will contain a method for calculating one-step-ahead residuals and a method for simulating from the distribution.

#### Review of prior options in other software and their uses

Stock Synthesis priors:
- Many options for all parameters, see current options [here](https://nmfs-ost.github.io/ss3-doc/SS330_User_Manual_release.html#parameter-priors).

ASAP priors:
- Aggregate catch and indices, lognormal
- Deviations from SR curve (BH assumed, although often set with steepness=1), lognormal
- Fmult in first year, lognormal
- Fmult deviations, lognormal random walk, typically unconstrained
- Catchability, lognormal
- Catchability deviations, lognormal random walk, not used very often
- N at age in first year, lognormal from input guesses or an exponential decline using M and F from first year
- Selectivity parameters, lognormal, from either values at age or parameters of logistic or double logistic (note, this doesn’t totally make sense because selectivity can only be between zero and one)
- Catch and indices at age assume multinomial with input ESS (not sure if this counts as a prior or not)
- No priors are linked to create multivariate priors (e.g., prior on catch and catchability are independent with no covariance)
- M is input as a matrix, no estimation of M allowed in ASAP
- Weights at age are empirical and input as matrices, no estimation of growth allowed in ASAP

WHAM generally does not use priors. It estimates a fixed parameter on a scale from negative to positive infinity and uses transformations to get a desired constraint, estimates a random effect, or treats the parameter as known. There are many more options for dealing with age composition in WHAM than in ASAP: e.g., multinomial, Dirichlet-multinomial, logistic-normal, and multivariate-tweedie, with variations for handling year, age combinations with observed zeros. WHAM has multiple ways to estimate linkages among random effects: e.g., IID, autoregressive across age, year, or both, and mixed with autoregressive across year for recruitment and 2dar1 for remaining ages in NAA. WHAm allows estimation of M. WHAM also allows incorporation of environmental covariates in a statistically sound manner using random effects.

SAM does not use priors. It also does not use age composition estimation generally. It treats indices at age and catch at age as multivariate log-normal random variables. It also estimates F at age over time as a multivariate log-normal randomwalk with covariances among the ages and over time. 

FIMS should allow the use of priors for cases where random effects are not desired or possible, e.g., due to large blocks of missing data. A model without priors is a priori preferable to one with priors, but sometimes priors are needed in order to produce reasonable results.

### Architecture & Organization

Architectural changes will be grouped into four stages: 

1. The development of a hierarchical class structure for distributions (**complete**)
2. A generalized framework for adding random effects and Bayesian priors (**complete**) 
3. Building up capacity which will add new distributions, osa residuals, and simulation functionality (wip)
4. Adding multivariate functionality for random effects and Bayesian priors.

### Decision Points

FIMS will develop a library of likelihoods so that multiple distributions will be available, not only for random effect processes, but also for data components, such as indices and compositions. Given this flexibility, FIMS will require the ability to compare and select best likelihoods from a set. Such a comparision is not always straightforward as some lieklihoods are not caparable, e.g. with AIC. FIMS will initially depend on TMB distribution functions but will eventually expand the fims math section to include explicitly written distributions. 

FIMS will implement full state space capabiliities, which will allow processes such as recruitment, selectivity, numbers at age, etc. to be time varying. This will be accomplished by relying on an autoregressive distributions, such as AR1 and randomwalk, or by depending on environmental linkages. Correlation between two dimensional processes (e.g. correlation between age and year in maturity, etc.) will be captured using 2dAR1 distributions. Time variation will require a general framework on the R side to specify processes on any parameter. Initial development will focus on adding an AR1 process on recruitment, afterwhich, the framework will be applied to other proccesses requiring time variation. 

FIMS will also implement fully Bayesian models. A key step to accomplishing this will be the ability to add both univariate and multivariate priors to any parameter or parameter set. Multivariate priors will allow for the handling of correlations between parameters. Priors will need to be flexible with the option to handlge Jacobian transformations. The R interface will need to be developed to handle a generalized framework for adding priors to parameters and improvements will be needed in the output to handle reporting and accessing other stan R libraries for visualizing Bayesian output.   

FIMS will add additional features to likelihood functions for calculating one-step-ahead (osa) residuals and for simulating data. OSA structure will require a flexible format for assembling the data vector going into the the osa component as this framework will need to work as users add or remove data, include unobserved data entries (i.e. NAs), or add "ghost" data. Additionally, users will need to choose which composition bin to drop for multivariate constrained likelihoods (e.g. multinomial, Dirichlet-Multinomial, Dirichlet, etc.). Data will need to be defined using the "Type" specifier so that values can be added to the tape. This definition is needed as TMB estimates data as random effects in osa calculations. Simulation will rely initially on TMB defined random distribution functions and TMB specific code macros. 

The following user case studies are provided to help guide developers in the architecture and design of a generic framework for random effects and bayesian priors. 

#### Univariate priors

```{r, eval = FALSE}
## Basic normal prior on M
population$SetPriors(pars='logM', mu=log(.2), sigma=.1,
                     family='normal', log=FALSE)

## Process errors when we have random effects also could have priors

## process error priors: half-normal
population$SetPriors(pars='sigmaR', mu=0, sigma=1,
                     family='normal', log=FALSE)
## or gamma to keep it away from boundary
population$SetPriors(pars='sigmaR', shape=2.15, scale=0.17,
                     family='gamma', log=FALSE)
```

In addition to population-level parameters there are those indexed by fleet (and later space or other sub-unit)

```{r, eval = FALSE}
## fleet specific ones like Q or selex
population$SetPriors(pars='log_q', mu=0, sigma=.5,
                     family='normal', fleet=1,
                     log=FALSE)
population$SetPriors(pars='log_q', mu=0, sigma=1,
                     family='normal', fleet=2,
                     log=FALSE)
```

#### Multivariate priors

The most likely source for multivariate priors is from meta-analyses. E.g., one could pull priors from the FishLife package for two growth parameters. This could include other things like M and steepness as well.

```{r, eval = FALSE}

library(FishLife)
library(mvtnorm)
params <- matrix(c('Loo', 'K'), ncol=2)
x <- Search_species(Genus="Hippoglossoides")$match_taxonomy
y <- Plot_taxa(x, params=params)

## multivariate normal in log space for two growth parameters
mu <- y[[1]]$Mean_pred[params]
Sigma <- y[[1]]$Cov_pred[params, params]

## log density in R
dmvnorm(x=c(3,-2), mean=mu, sigma=Sigma, log=TRUE)

## Would look something like this, specify which parameter names,
## the mean and covariance, and that it's in log space

population$SetGrowth(ewaa_growth$get_id())
population$SetPriors( pars=params, mu=mu, Sigma=Sigma,
        family=’multivariate_normal’, log=TRUE)
```

Jacobian adjustments during integration, see this [comment](https://github.com/NOAA-FIMS/FIMS/issues/431#issuecomment-1930450473) for links and background

```{r, eval = FALSE}
## how to deal with Jacobians?
success <- CreateTMBModel(optimization=TRUE)
parameters <- list(p = get_fixed())
obj <- MakeADFun(data = list(), parameters, DLL = "FIMS", silent = TRUE)
obj$fn() ## does not include Jacobian adjustments from priors

## can still be optimized but will include Jacobians so the MLE
## is really the posterior mode
success <- CreateTMBModel(optimization=FALSE)
parameters <- list(p = get_fixed())
obj <- MakeADFun(data = list(), parameters, DLL = "FIMS", silent = TRUE)
obj$fn() ## does include Jacobian adjustments from priors
```

MCMC integration of a model
The R package ‘tmbstan’ provides the most straightforward way to generate posterior samples for a FIMS model. It is demonstrated briefly below.
```{r, eval = FALSE}
## Try MCMC integration with tmbstan. Also see new package

## StanEstimators which may work better
library(tmbstan)
library(shinystan)
obj <- MakeADFun(data = list(), parameters, DLL = "FIMS", silent=TRUE, map=map)
opt <- fit_tmb(obj, getsd=FALSE, newtonsteps=0, control=list(trace=0))
fit <- tmbstan(obj, init='last.par.best', control=list(max_treedepth=10))
launch_shinystan(fit)

## get posterior samples from parameters
df <- as.data.frame(fit)
df <- df[,-ncol(df)] # drop lp__
## for each posterior draw, report to get SSB
postreps <- list()
for(ii in 1:nrow(df)){
  if(ii %% 10==0) print(ii)
  postreps[[ii]] <- obj$rep(df[ii,])
}
## This is the posterior distribution of SSB
ssbpost <- lapply(postreps, function(x) data.frame(year=years, ssb=x$ssb[[1]][-55]))%>%
  bind_rows %>% mutate(rep=rep(1:nrow(df), each=54))
```

#### Cross validation for model selection

Cross validation techniques require the model to report the log-likelihoods for each data point, even when left out of the model (e.g., by setting it to NA). One example is PSIS-LOO for Bayesian models, as demonstrated below. But the fundamental need here is for the report function to return all individual log-likelihood values for a given set of parameters. Exact cross validation means to leave out sets of data and rerun the model. How to choose which groupings of data to leave out depends on the goals. See examples of frequentist versions in papers that do it randomly, or leave out a whole year [here](https://doi.org/10.1016/j.fishres.2011.04.017) and [here](https://doi.org/10.1016/j.fishres.2021.106071). A Bayesian implementation is done in [this paper](https://doi.org/10.1093/icesjms/fsab165).

```{r, eval = FALSE}
## for each posterior draw, get point-wise log-likelihoods from
## the report
df <- as.data.frame(fit)
postreps <- list()
for(ii in 1:nrow(df)){
  if(ii %% 10==0) print(ii)
  loglikes[[ii]] <- get_pointwise_loglikelihoods(obj$rep(df[ii,]))
}

## drop empty columns, and the cols with only zeroes
loglikes <- loglikes[,-ncol(loglikes)] %>% as.matrix
loglikes <- loglikes[,-which(apply(loglikes,2, max)==0)]
stopifnot(!any(is.na(loglikes)))
library(loo)
id <- rep(1:cores,each=nrow(tmp)/cores)
neff <- relative_eff(loglikes, chain_id=id)
loo1 <- loo(x=loglikes, r_eff=neff)
print(loo1)
plot(loo1)
```

#### Process errors for hierarchical models

A variety of types of random effects structures can be applied to different components of the FIMS model. 

Types to consider:
1. iid standard normal: recruitment
2. AR(1), random-walk: univariate smoothers on parameters (selectivity, M, etc.), or data inputted for covariate smoothing in covariate-linked assessments.
3. 2D AR(1), age-cohort-year 2D AR(1) [see [this paper](https://doi.org/10.1016/j.fishres.2023.106755)], SPDE spatial

```{r, eval = FALSE}
## Need to specify which components have random effects, the
## structure type, initialize them, and whether they're esitmated
recruitment$logit_steep$is_random_effect <- FALSE
recruitment$logit_steep$random_effect_type <- "AR1"
recruitment$logit_steep$process_pars_init <- c(1,0)
## E.g., estimate correlation but not variance term
recruitment$logit_steep$process_pars_estimated <- c(FALSE,TRUE)

## multivariate on a single population process
population$estimate_M <- TRUE
population$add_random_effects(par='log_M', type='2d-AR1',
                              init=c(1,0,0), estimated=c(0,1,1))
```




### Hierarchical Class Structure for Distributions

Build library of probability distributions and necessary architecture to connect to FIMS

* UnivariateDistributionBase and MultivariateDistributionBase each with child Distribution classes (e.g. NormalLPDF, MultinomialLPMF, MvnormLPDF, etc.)
    * LPDF: log probability density function (continuous distributions)\
    * LPMF: log probability mass function (discrete distributions)\
    * Base classes will have an evaluate function which takes input ***observed_value*** (can be data, parameters, or random effects) and ***expected_value*** (can be a derived value in the model, fixed or estimated)\
    * Naming convention: prefer log PDF/PMF over "log-likelihood" as it is more generic. In a Bayesian context, a prior is conceptually different from a likelihood\
* Each child Distribution class will have an evaluate function which takes local parameters and returns a log PDF/PMF.\
* The child class evaluate functions will also includes code to calculate the OSA residual and simulation function available through TMB\
* model.hpp is modified to loop over all instantiated Distribution classes and sum the negative values together to produce a joint negative log likelihood
    * Note: When simulating from a model, simulation needs to happen in the order of model hierarchy (i.e. priors first, then random effects, then data).\
    * Functions need to report out an nll value for each data point

### A generalized framework for adding random effects and Bayesian priors

1.  Build infrastructure to include priors and random effects using a generalized framework

* Rcpp interface links user input with members of nll functions for the following three cases. The goal is to develop a generic interface that can handle all three cases:\

1. Data Case: Rcpp interface links user input to ***observed_value*** in Distribution functions and points the ***expected_value*** to the correct derived value in the model. User sets initial values of parameters and Rcpp interface adds the parameters to the list of parameters estimated by the model
2. Prior Case: Rcpp interface links the ***observed_value*** in Distribution functions to a parameter/s in the model. User sets ***expected_value*** and Distribution specific parameters and fixes values so they are not estimated by the model
3. Random Case: Rcpp interface links the ***observed_value*** in Distribution functions to a process in the model and fixes the ***expected value*** at 0. User sets the initial value of Distribution specific parameters and these get added to the list of parameters estimated by the model
    * Functions are designed to be generic to handle the following cases (see [FIMS NLL Examples](https://docs.google.com/document/d/1X1NwjQlLrKGIXZgkMfJ29Kp1lFreKiiTTQxyoicgqxc/edit))
        * Scalar prior

        * Multivariate prior for multiple parameters within the same module (e.g., Multivariate prior on Linf and K)

        * Univariate random effect

        * Multivariate random effect for a single process (e.g., log_M across years and ages)

        * Multivariate prior for two parameters across different modules - doesn't need to be constrained to two (e.g., mortality and steepness)

        * Multivariate random effect for two processes within the same module\

        * Multivariate random effect for two processes across different modules

            
3. Discussion Points

* One class per distribution with flags for osa and simulation flags for data and re/priors
* wrt simulations, order matters and needs to follow the natural hierarchy of the model (i.e., 1. Priors 2. Random effects 3. Data)
* Interface - generic approach but be mindful of different use cases
    * Multivariate where each parameter/process comes from a different module
    * Varying data types (i.e., scalar sd vs vector of cvs for dnorm)
* From WHAM: 2dAR1 with recruitment and NAA - these should **not** be linked; solution was to apply 2dAR1 to ages 2+ and recruitment treated differently - consider if NAA should be an additional module on top of recruitment
    * What complexities occur with NAA (random effect, movement) - if more than two use cases then justification to create new module
* OSA requires a lot of input code on the R side to prepare for OSA wrt multinomial
    * Need to throw out one of the age/year bins - need to set NA so the osa calculation skips this value ([WHAM approach](https://github.com/timjmiller/wham/blob/master/R/set_osa_obs.R#L280))
    * Is this something we develop during M2?
    * What does SAM do?
    * Can this be done internally in C++?
* Need to be mindful of sparsity - e.g., Recruitment with an AR1 process is dense if the random effect is the devs but is sparse if the random effect is logR.

1. Proposed Tests:

* RE test
* tmbstan test
* MLE test with penalties

### Model Specification

Ideas are being explored using the [ModularTMBExample](https://github.com/NOAA-FIMS/ModularTMBExample/tree/FIMS-v0100-nll-refactor-2)

#### Negative Log Density Functions

UnivariateBase

* Normal(x, mu, sd) **completed**
* LogNormal: often written as Normal(log(x), mu, sd) - log(x) **completed**
* Gamma(x, 1/cv^2, cv^2mean), cv: coefficient of variation, mean > 0, typically use a exp() to keep the mean positive
* NegativeBinomial: used for tagging data - parameterization can be a bit tricky, research best one to use for tagging data; discrete data
* Tweedie(x, mean, disp, power), mean >0 (typically exp()), disp > 0 (typically exp()), 1 < power < 2 (scaled logit transformed); used for zero-inflated continuous data (hurdle data)

MultivariateBase

* Multinomial **completed**
    * If calculating OSA residual, data need to be true counts
* Dirichlet Multinomial
* Logistic Normal - performs better than DM at large sample sizes and DM performs better than LN at small sample sizes
    * Relies on the multivariate normal (e.g., [here](https://github.com/timjmiller/wham/blob/master/src/age_comp_osa.hpp#L271))
    * Will work better with OSA compared to multinomial - often comp data are not true integers
* Multivariate Tweedie
* AR1
* Multivariate Normal - required to implement the Logistic Normal
* 2dAR1
* GMRF

Highest priority
Mid priority
Low priority

#### R User Interface {#r-user-interface}

**Generic SetNLL()**

**Case: Univariate Prior (e.g. Normal prior on M)**

```r

logM_nll <- new(NormalNLL)
logM_nll$type <- "prior" # (choices are random_effect, prior, or data) 
logM_nllf$ mu f$value <- log(0.2) 
logM_nllf$ mu f$is_estimated <- FALSE 
logM_nll$log_sd <- log(0.1)
logM_nllf$ sd f$is_estimated <- FALSE

SetNLL(module = 'population', module_id = population\$get_id(),\
member_name = log_M, nll = logM_nll)

# Alternate approach or higher level helper function:
population$SetPriors(
  pars = "log_M",
  mu = log(0.2),
  sigma = 0.1,
  family = "normal",
  log = FALSE
)
SetPriors(
  module = "population",
  module_id = population$get_id(),
  pars = 'log_M',
  mu = log(0.2),
  sigma = 0.1,
  family = 'normal',
  log = FALSE
) {
new_nll <- lookup_function(
  #looks up family and returns new() based on NLL call from family argument
new_nll$type <- "prior"
new_nllf$ mu f$value <- mu
new_nllf$ mu f$is_estimated <- FALSE
new_nll$log_sd <- log(sigma)
new_nllf$ sd f$is_estimated <- FALSE
new_nll$module_name <- module
new_nll$module_id <- module_id
new_nll$member_name <- pars
}
```

**Case: Univariate Random Effect**\

```r
# AR1 random effect

logitSteep_nll <- new(AR1NLL)

logitSteep_nll$type <- random_effect
logitSteep_nllf$ logit_phi f$value <- 0 
logitSteep_nllf$ logit_phi f$is_estimated <- TRUE 
logitSteep_nllf$ log_var f$value <- 1 
logitSteep_nllf$ log_var f$is_estimated <- FALSE

recruitment <- new(BevertonHolt)
SetNLL(recruitment, logit_steep, logitSteep_nll)

#or recruitment itself can be a random effect

recruitment <- new(AR1NLL)
```

**Case: Multivariate Prior - Single Parameter**

```r
# 2dAR1 on M, multivariate dimensions match parameter dimensions\
logM_nll <- new(2dAR1NLL)
logM_nll$type <- "random_effect"
logM_nll$logit_phi1 <- 1
logM_nll$logit_phi2 <- 0
logM_nll$log_var <- 0
logM_nll$logit_phi1$is_estimated <- FALSE
logM_nll$logit_phi2$is_estimated <- TRUE
logM_nllf$ log_var f$is_estimated <- TRUE

population <- new(Population)
SetNLL(population, logM, logM_nll) - or - population$SetNLL(logM, logM_nll)
```

**Case: Multivariate Prior - Multiple Parameters**\
library(FishLife)\
library(mvtnorm)\
params \<- matrix(c('Loo', 'K'), ncol=2)\
x \<- Search_species(Genus="Hippoglossoides")$match\_taxonomy y \<- Plot\_taxa(x, params=params) \#\# multivariate normal in log space for two growth parameters mu \<- y\[\[1\]\]\f$ Mean_pred \f$\f$ params \f$$\
Sigma \<- y$$\[1$$]$Cov\_pred\[params, params\] \#\# log density in R dmvnorm(x=c(3,-2), mean=mu, sigma=Sigma, log=TRUE) \>growth\_nll \<- new(MVnormMLL) \>growth\_nll$type \<- 'prior'\
\>growth_nll$mean \<- mu \>growth\_nll$Cov \<- Sigma\
\>growth_nll\f$ mean \f$is_estimated \<- FALSE\
\>growth_nll\f$ Cov \f$is_estimated \<- FALSE

\>growth \<- new(EWAA)\
\> SetNLL(list(growth), list(L_inf, K), growth_nll)

**Case: Multivariate Random Effect - Different Modules**

**Case: Data**\
catch_nll \<- new(LognormalNLL)\
catch_nll\$log_sd \<- cv_vector\
catch_at_age_nll \<- new(MultinomialNLL)

SetNLL(fleet, catch, catch_nll)\
SetNLL(fleet, catch_at_age, catch_at_age_nll)

#### Rcpp

//Cases:\
single module, x is scalar\
single module, x is vector\
single module, x is matrix\
multiple modules, x is vector\
multiple modules, x is matrix

SetNLL(module_id, x, nll){\
module$\f$ module\_id \f$$ -\> name_same_as(x) = x\
nll -\> x = module$\f$ module\_id \f$$ -\> name_same_as(x)\
}

SetMVNLL(list(m1id, m2id), )

#### C++ Classes

UnivariateBase

* Observed_value:

    * Data: input by the user\
    * Random effect: calculated within a module\
    * Prior: set to a parameter in the model\
    * Needs to be able to handle both scalar and vector\

* expected_value:

    * Data: calculated within population\
    * Random effect: fixed at zero\
    * Prior: set and fixed by the user\

* evaluate(observed_value, expected_value, do_log = true)

    NormalPDF

* sd: initiated through user interface\

* osa_flag: if data, implements the osa calculate\

* simulation_flag\

* evaluate()

    * Loop over the length of observed_value and evaluate the nll for dnorm\
    * Calculate osa residuals\
    * Need to implement the cdf method for all distributions?

Notes

* NLL-Population linkage unclear
    * Setting random effect or prior on parameters that are already in population will be somewhat straightforward\
    * Setting random effects/priors on values in population that are set up as derived quantities are less clear. For example, does NAA need to become its own module? [This calculation](https://github.com/NOAA-FIMS/FIMS/blob/18f96a81d02021a55c9f91a66485e7250a20cb5a/inst/include/population_dynamics/population/population.hpp#L240) gets confusing if NAA is an AR1 or 2dAR1.


## R output

### User Interface

The following sections explain the (1) current state of FIMS model output, (2) future output from a FIMS model, and (3) future FIMS-specific functions to work with that output.

#### Current output from a FIMS model

FIMS lacks a wrapper function to run a FIMS model and the user must call makeADFUN and an optimizer.  
Questions (to ponder SILENTLY):

1. Will users always manually call `TMB::MakeADFun()`?  
2. What would a wrapper function look like?  
3. Returning the FIMS results included in this design document should be accessible to both power users that call optim themselves and new users that utilize a future FIMS wrapper.

#### Future output from a FIMS model

The output from a FIMS model is an S4 object with a class of FIMS and has the following elements, each of which are described in detail below:

* estimates
* fits
* tmb  
* sdreport
* call 
* timestamp
* version

##### estimates (tibble)

A tibble (i.e., a data frame with more features) of estimates of parameters (both fixed and estimated) and derived quantities. For example, a time series of natural mortality and a time series of recruitment deviations are examples of returned parameters that will be in the data frame. Whereas, a time series of spawning biomass and a time series of numbers at age by year are examples of returned derived quantities. 

We expect there to be a common set of derived quantities reported. It may be possible for users to specify additional derived quantities to track, which may increase run times and be difficult to implement (?; this is how I remember the discussion)  
Questions (to ponder SILENTLY):

1. What kind of time allotment is made for calculating variances of derived quantities, will this be a limitation?  
2. How do you turn on variance estimates of derived quantities if we do not want to automatically calculate them for every derived quantity?  
3. How will labels be made for the names of parameters and derived quantities, what will the separator be, should they be all lower-case letters, should multiple types of separators be used, etc? 

Columns

* label: the name of the parameter or derived quantity  
* time: the modeled time period that the value pertains to  
* initial: the initial value use to start the optimization procedure   
* estimate: estimated parameter value, which would be the MLE estimate or the value used for a given MCMC iteration  
* uncertainty: Estimated uncertainty, reported as a standard deviation  
* likelihood: the likelihood component for that parameter given the prior, NA for derived quantities  
* gradient: the gradient component for that parameter, NA for derived quantities  
* estimated: TRUE/FALSE indicator of if the parameter was estimated (and not fixed), with NA for derived quantities

##### fits (tibble)

A tibble with all of the information needed to calculate residuals, such as the data used to fit the model, expected values, likelihoods, data weighting metric, and distribution used.  
Columns:

* All utilized columns from the input data, e.g., type, name, value, …  
* expected: the expected value based on model fits  
* likelihood: the likelihood component for that data point  
* weight: the weight applied to the data point  
* distribution: the distribution used to fit the data point

An example [tibble](https://tibble.tidyverse.org/) returned in the model fits slot, where four additional columns are added onto the input  FIMSFrame object. Note that some columns of FIMSFrame (age, datastart, and dateend) are not shown in the image below just for brevity of this document but will be in the actual model object.  
   **type  name    value unit  uncertainty … expected likelihood weight distribution**  
   **\<chr\> \<chr\>    \<dbl\> \<chr\> \<dbl\>  …   \<dbl\>      \<dbl\>    \<dbl\> \<chr\>**        
 **1 index survey1 0.00680 ""   0.2  …    0.00680      0.001      1 lognormal**    
 **2 index survey1 0.00617 ""   0.2  …    0.00617      0.001      1 lognormal**    
 **3 index survey1 0.00598 ""   0.2  …    0.00598      0.001      1 lognormal**    
 **4 index survey1 0.00591 ""   0.2  …    0.00591      0.001      1 lognormal**    
 **5 index survey1 0.00721 ""   0.2  …    0.00721      0.001      1 lognormal**  
**\# i 200 more rows**  
**\# i Use \`print(n \= ...)\` to see more rows**

##### tmb

The returned object from `TMB::MakeADFun()`. 

* **names(obj)**  
*  **\[1\] "par"  "fn" "gr" "he" "hessian" "method" "retape" "env" "report" "simulate"** 	 

##### sdreport (list)

The returned object from `TMB::sdreport()`.

###### call (call)

The returned object from `match.call()`, which specifies the full name and specifications of all of the arguments passed to the function call allowing for repeatability of the model-fitting process.

Example of returned call from running a linear model via the `lm()` command in R.  
**lm(weight \~ age, method \= "qr")$call**  
**\# lm(formula \= weight \~ age, method \= "qr")**

##### timestamp (vector)

A vector of length two with the start and end timestamps of when the model was started and when the model was done fitting to allow for calculating run times. All timestamps are in UTC. The timestamps will be overwritten if the model is updated. Each element of the returned object is generated using `as.POSIXlt(Sys.time(), tz = "UTC")`. 

Example of the timestamp vector in the returned object  
**\[1\] "2024-03-19 11:42:26 UTC" "2024-03-19 13:42:26 UTC"**

##### version (list)

The package version (package\_version; numeric\_version) for FIMS. A hidden list of length three containing integers that denote the version of FIMS that was used to run the model, e.g., `1.1.9` is stored as `$ : int [1:3] 1 1 9`. 

#### Functions for working with FIMS output

There will be multiple helper functions that can work with the returned object to provide things such as calculated residuals and figures. These functions will be a combination of FIMS-specific functions and general methods written to augment existing functions such as `tidy()` and `summary()`. Many of the functions will be inspired by [tidy models](https://www.tidymodels.org). Additional useful resources for tidy models include: [broom](https://broom.tidymodels.org) and [tmwr workflows](https://www.tmwr.org/workflows).

* `summary()`  
* `tidy()`  
* `plot()`  
* `check_convergence()` helps interpret messages like "relative convergence (4)"  
* `get_status()` shows if a parameter estimate is close to bound

### Requirements

Elements from the FIMS requirements planning process that we felt were in scope for M2 of the R interface development. The fifth column explains how each item was incorporated into the design document.  
From [FIMS Requirements Full](https://docs.google.com/spreadsheets/d/1impCdPPob8IPdoiFDpJpe7-Nbdjbz_Uq05_4BWooY4g/edit#gid=1133225771)

| Topic | Topic details | Feature | How this is incorporated in the design document |
| :---- | :---- | :---- | :---- |
| Output | Format | Consistent formatting & output as a table | We are developing consistent output formatting as part of the output design. Helper functions can be used to reformat output results.  |
| Output Products | Executive summary of key assessment results | Executive summary of key assessment results (See Appendix B of Next Gen SAIP for an example) which Tables and Figures don't pertain to the management. | This can be created using a helper function with the output object. Discuss this with Sam Schiano (contractor in OST), who is working on a project to standardize assessment reporting. |
| User experience | Ease of use | save inputs and outputs | A common output object including the model call made will help users save inputs and outputs. |
| User experience | Ease of use | rapidly explore outputs | A common output object formatting with helper functions will help rapidly exploring the output. |
| Model Comparison | Tabular summaries of parameter estimates and derived quantities | User-defined reporting of parameter estimates, derived quantities, likelihood components for comparison of alternative model configurations | The output object will attempt to report everything the user needs rather than having user-defined outputs. Helper functions can allow users to structure the output in a particular way. If creating this output takes too long, we can consider user-defined reporting in the future to speed up model running and development. |
| Diagnostics | Visual fits to data: Goodness of fit/residual plots | Residual plots, time series of plots of observed vs. predicted values, RMSE estimates, information criteria | Plots can be created using helper functions with the common output object. |
| model convergence | Tests of convergence for optimization algorithms | Hessian, plots of parameter estimates and bounds, final gradient, parameter estimates | Much of this is included in the \`estimates\`, \`fits\`, and \`tmb\` objects within the returned output. There are no such things as bounds within a FIMS model and thus they are not reported. |
| Output Products | Raw model output (e.g. numbers at age for all years/areas/etc., all parameters with variances and covariances) | Output files required by analyst to produce management advice, including input for forecasting analyses | All parameters and derived quantities are included in the `estimates` section of the output along with their variances if calculated. |
| Output Products | Timestamped | Timestamped \- Versions easily identifiable, changes between versions transparent | The time that the model was started and stopped is available in `timestamp` and the package version used to estimate the model is in `version`. |

## References

Berger, A. M., Barceló, C., Goethel, D. R., Hoyle, S. D., Lynch, P. D., McKenzie, J., Dunn, A., Punt, A. E., Methot, R. D., Hampton, J., Porch, C. E., McGarvey, R., Thorson, J. T., A'mar, A. T., Deroba, J. J., Þór Elvarsson, B., Holmes, S. J., Howell, D., Langseth, B. J., Marsh, C., Maunder, M. N., Mormede, S., and Rasmussen, S. 2024. Synthesizing the spatial functionality of contemporary stock assessment software to identify future needs for next generation assessment platforms. *Fisheries Research*, 275, 107008. 10.1016/j.fishres.2024.107008.

Correa, G. M., Monnahan, C. C., Sullivan, J. Y., Thorson, J. T., and Punt, A. E. 2023. Modelling time-varying growth in state-space stock assessments. *ICES Journal of Marine Science*, 80(7), 2036--2049.

Francis, R. I. C. C. 2016. Growth in age-structured stock assessment models. *Fisheries Research*, 180, 77--86.

Kraak, S. B. M., Haase, S., Minto, C., and Santos, J. 2019. The Rosa Lee phenomenon and its consequences for fisheries advice on changes in fishing mortality or gear selectivity. *ICES Journal of Marine Science*, 76(7), 2179--2192.

Methot, R. D., and Wetzel, C. R. 2013. Stock synthesis: a biological and statistical framework for fish stock assessment and fishery management. *Fisheries Research*, 142: 86--99.

Þór Elvarsson, B., Woods, P. J., Björnsson, H., Lentin, J., and Thordarson, G. 2018. Pushing the limits of a data challenged stock: A size- and age-structured assessment of ling (*Molva molva*) in Icelandic waters using Gadget. *Fisheries Research*, 207 95--109.

Punt, A. E., Dunn, A., Þór Elvarsson, B., Hampton, J., Hoyle, S. D., Maunder, M. N., Methot, R. D., and Nielsen, A. 2020. Essential features of the next-generation integrated fisheries stock assessment package: A perspective. *Fisheries Research*, 229, 105617.
