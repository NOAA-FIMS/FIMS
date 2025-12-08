# Class constructors for class `FIMSFit` and associated child classes

Create an object with the class of `FIMSFit` after running a FIMS model.
This is typically done within
[`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)
but it can be create manually by the user if they have used their own
bespoke code to fit a FIMS model.

## Usage

``` r
FIMSFit(
  input,
  obj,
  opt = list(),
  sdreport = list(),
  timing = c(time_total = as.difftime(0, units = "secs")),
  version = utils::packageVersion("FIMS")
)
```

## Arguments

- input:

  Input list as returned by
  [`initialize_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/initialize_fims.md).

- obj:

  An object returned from
  [`TMB::MakeADFun()`](https://rdrr.io/pkg/TMB/man/MakeADFun.html).

- opt:

  An object returned from an optimizer, typically from
  [`stats::nlminb()`](https://rdrr.io/r/stats/nlminb.html), used to fit
  a TMB model.

- sdreport:

  An object of the `sdreport` class as returned from
  [`TMB::sdreport()`](https://rdrr.io/pkg/TMB/man/sdreport.html).

- timing:

  A vector of at least length one, where all entries are of the
  `timediff` class and at least one is named "time_total". This
  information is available in
  [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)
  and added to this argument internally but if you are a power user you
  can calculate the time it took to run your model by subtracting two
  [`Sys.time()`](https://rdrr.io/r/base/Sys.time.html) objects.

- version:

  The version of FIMS that was used to optimize the model. If
  [`fit_fims()`](https://NOAA-FIMS.github.io/FIMS/reference/fit_fims.md)
  was not used to optimize the model, then the default is to use the
  current version of the package that is loaded.

## Value

An object with an S4 class of `FIMSFit` is returned. The object will
have the following slots:

- `input`::

  A list containing the model setup in the same form it was passed.

- `obj`::

  A list returned from
  [`TMB::MakeADFun()`](https://rdrr.io/pkg/TMB/man/MakeADFun.html) in
  the same form it was passed.

- `opt`::

  A list containing the optimized model in the same form it was passed.

- `max_gradient`::

  The maximum gradient found when optimizing the model. The default is
  `NA`, which means that the model was not optimized.

- `report`::

  A list containing the model report from `obj[["report"]]()`.

- `sdreport`::

  An object with the `sdreport` class containing the output from
  `TMB::sdreport(obj)`.

- `timing`::

  The length of time it took to run the model if it was optimized.

- `version`::

  The package version of FIMS used to fit the model or at least the
  version used to create this output, which will not always be the same
  if you are running this function yourself.

- `model_output`::

  The FIMS model output as a JSON string.
