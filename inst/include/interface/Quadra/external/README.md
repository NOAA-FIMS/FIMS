# External dependencies

Quadra expects third-party, header-only dependencies to live under `external/`.

Recommended layout:

```text
external/
├── eigen/
│   └── Eigen/
├── had/
└── lbfgspp/
```

The include path should point to the directory that contains the public include
folder. For Eigen, this means the include path should be:

```bash
-Iexternal/eigen
```

not:

```bash
-Iexternal/eigen/Eigen
```

because Quadra code should include Eigen as:

```cpp
#include <Eigen/Dense>
```

Eigen internally includes sibling headers such as:

```cpp
#include "Core"
```

so a partial Eigen checkout can produce errors such as:
