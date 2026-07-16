/**
 * @file edm_linear_algebra.hpp
 * @brief Shared linear algebra utilities for EDM prediction algorithms.
 *
 * @details Currently provides:
 *  - GaussianElimination<Type>: solves A x = b in-place via partial-pivot
 *    Gaussian elimination. Used by SMapProjection and GPEdmProjection.
 *
 * All routines are templated on the numeric scalar type so they work with
 * both plain double and TMB AD scalars (CppAD). Index arithmetic and pivot
 * selection use double casts to keep the AD tape free of non-differentiable
 * branches.
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_LINEAR_ALGEBRA_HPP
#define FIMS_EDM_LINEAR_ALGEBRA_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace fims_edm {

/**
 * @brief Solve the linear system A x = b in-place via Gaussian elimination
 *        with partial pivoting.
 *
 * @details Both @p A and @p b are modified during elimination. On exit,
 * @p b contains the solution vector x. The matrix @p A is stored in
 * row-major order as a flat std::vector of size n*n.
 *
 * This implementation avoids std::sort and std::swap on AD scalars so that
 * the loop structure remains traceable by TMB's CppAD tracer. Index
 * operations (pivot selection, row swaps) use double-typed comparisons.
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 * @param A    Flat row-major coefficient matrix (n × n), modified in-place.
 * @param b    Right-hand-side vector (length n), overwritten with solution.
 * @param n    System dimension.
 * @throws std::runtime_error if the matrix is (near-)singular.
 */
template <typename Type>
void GaussianElimination(std::vector<Type>& A, std::vector<Type>& b,
                         size_t n) {
  for (size_t col = 0; col < n; ++col) {
    // --- Partial pivot: find the row with the largest absolute value in
    //     column `col`, at or below the current diagonal. ---
    size_t pivot_row = col;
    double max_val = 0.0;
    for (size_t row = col; row < n; ++row) {
      // Cast to double for pivot comparison only (not in AD trace).
      double val = static_cast<double>(A[row * n + col]);
      if (val < 0.0) val = -val;
      if (val > max_val) {
        max_val = val;
        pivot_row = row;
      }
    }
    if (max_val < 1e-14) {
      throw std::runtime_error(
          "GaussianElimination: matrix is singular or near-singular. "
          "The system may be underdetermined or the library may be too small.");
    }

    // --- Swap current row with pivot row (both A and b). ---
    if (pivot_row != col) {
      for (size_t j = 0; j < n; ++j) {
        Type tmp = A[col * n + j];
        A[col * n + j] = A[pivot_row * n + j];
        A[pivot_row * n + j] = tmp;
      }
      Type tmp_b = b[col];
      b[col] = b[pivot_row];
      b[pivot_row] = tmp_b;
    }

    // --- Eliminate entries below the pivot. ---
    Type pivot_val = A[col * n + col];
    for (size_t row = col + 1; row < n; ++row) {
      Type factor = A[row * n + col] / pivot_val;
      for (size_t j = col; j < n; ++j) {
        A[row * n + j] -= factor * A[col * n + j];
      }
      b[row] -= factor * b[col];
    }
  }

  // --- Back-substitution. ---
  for (size_t i = n; i-- > 0;) {
    Type sum = Type(0);
    for (size_t j = i + 1; j < n; ++j) {
      sum += A[i * n + j] * b[j];
    }
    b[i] = (b[i] - sum) / A[i * n + i];
  }
}

}  // namespace fims_edm

#endif  // FIMS_EDM_LINEAR_ALGEBRA_HPP
