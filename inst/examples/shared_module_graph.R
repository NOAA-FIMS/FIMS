# Shared module graph example.
#
# This example shows how the R specification layer can represent a model as
# reusable modules plus links between modules. The graph is representation-only:
# it is meant to describe sharing before the builder consumes graph specs.

library(FIMS)

years <- 2020:2022
ages <- 1:4

# Step 1: Define reusable submodules once.
shared_growth <- module(
  "growth.shared",
  ewaa_growth(
    ages = ages,
    weights = c(0.2, 0.5, 0.9, 1.1)
  )
)

shared_selectivity <- module(
  "selectivity.shared",
  logistic_selectivity(
    a50 = estimate(3),
    slope = constant(1.1)
  )
)

# Step 2: Define population-specific modules.
recruitment_a <- module(
  "recruitment.a",
  beverton_holt(
    log_rzero = estimate(log(1000)),
    steepness = constant(0.75)
  )
)

recruitment_b <- module(
  "recruitment.b",
  beverton_holt(
    log_rzero = estimate(log(800)),
    steepness = constant(0.72)
  )
)

maturity_a <- module(
  "maturity.a",
  logistic_maturity(a50 = constant(3), slope = constant(1.2))
)

maturity_b <- module(
  "maturity.b",
  logistic_maturity(a50 = constant(4), slope = constant(1.1))
)

# Step 3: Refer to shared modules by id.
population_a <- module(
  "population.a",
  population(
    name = "stock_a",
    ages = ages,
    years = years,
    growth = ref("growth.shared"),
    maturity = ref("maturity.a"),
    recruitment = ref("recruitment.a")
  )
)

population_b <- module(
  "population.b",
  population(
    name = "stock_b",
    ages = ages,
    years = years,
    growth = ref("growth.shared"),
    maturity = ref("maturity.b"),
    recruitment = ref("recruitment.b")
  )
)

fleet_a <- module(
  "fleet.a",
  fleet(
    name = "fishery_a",
    selectivity = ref("selectivity.shared"),
    fishing_mortality = estimate(rep(log(0.1), length(years)))
  )
)

fleet_b <- module(
  "fleet.b",
  fleet(
    name = "fishery_b",
    selectivity = ref("selectivity.shared"),
    fishing_mortality = estimate(rep(log(0.08), length(years)))
  )
)

# Step 4: Convert modules and references into a model graph.
graph <- as_model_graph(list(
  shared_growth,
  shared_selectivity,
  recruitment_a,
  recruitment_b,
  maturity_a,
  maturity_b,
  population_a,
  population_b,
  fleet_a,
  fleet_b
))

# Step 5: Convert the graph to plain tibbles.
tibbles <- model_graph_tibbles(graph)
modules <- tibbles$modules
links <- tibbles$links

# The modules tibble stores each module once.
print(modules[, c("id", "type", "name")])

# The links tibble shows sharing explicitly.
print(links)

# Step 6: Rebuild a graph object from the tibbles.
rebuilt_graph <- model_graph_from_tibbles(
  modules = modules,
  links = links
)

stopifnot(isTRUE(all.equal(
  as.data.frame(graph$links),
  as.data.frame(rebuilt_graph$links),
  check.attributes = FALSE
)))
