#pragma once

#include <Eigen/Dense>

#include <algorithm>
#include <iostream>

namespace quadra {
namespace laplace {
namespace diagnostics {

inline void print_du_dtheta_summary(const Eigen::MatrixXd &dU) {
#ifdef QUADRA_DEBUG_DU_DTHETA_NORMS
  std::cout << "Quadra dU diagnostic\n";

  std::cout << "  dU_col_norms = ";
  for (Eigen::Index j = 0; j < dU.cols(); ++j) {
    std::cout << dU.col(j).norm();
    if (j + 1 < dU.cols())
      std::cout << " ";
  }
  std::cout << "\n";

  std::cout << "  dU_col_maxabs = ";
  for (Eigen::Index j = 0; j < dU.cols(); ++j) {
    std::cout << dU.col(j).cwiseAbs().maxCoeff();
    if (j + 1 < dU.cols())
      std::cout << " ";
  }
  std::cout << "\n";

  std::cout << "  dU_first_rows =";
  const Eigen::Index nprint = std::min<Eigen::Index>(5, dU.rows());
  for (Eigen::Index r = 0; r < nprint; ++r) {
    std::cout << "\n    row " << r << ": ";
    for (Eigen::Index j = 0; j < dU.cols(); ++j) {
      std::cout << dU(r, j);
      if (j + 1 < dU.cols())
        std::cout << " ";
    }
  }
  std::cout << "\n";
#else
  (void)dU;
#endif
}

inline void
print_theta_only_vs_total_logdet_gradient(const Eigen::VectorXd &theta_only,
                                          const Eigen::VectorXd &total) {
#ifdef QUADRA_DEBUG_LOGDET_THETA_ONLY_VS_TOTAL
  std::cout << "Quadra logdet Hdot diagnostic\n";
  std::cout << "  theta_only_logdet_grad = " << theta_only.transpose() << "\n";
  std::cout << "  total_logdet_grad      = " << total.transpose() << "\n";
  std::cout << "  implicit_u_contribution= " << (total - theta_only).transpose()
            << "\n";
#else
  (void)theta_only;
  (void)total;
#endif
}

inline void
print_hdot_exact_vs_fd_trace(const Eigen::VectorXd &exact_trace,
                             const Eigen::VectorXd &fd_trace,
                             const Eigen::VectorXd &rel_hdot_matrix_err) {
#ifdef QUADRA_DEBUG_HDOT_EXACT_VS_FD_TRACE
  std::cout << "Quadra Hdot exact-vs-FD trace diagnostic\n";
  std::cout << "  exact_total_logdet_grad = " << exact_trace.transpose()
            << "\n";
  std::cout << "  fd_total_logdet_grad    = " << fd_trace.transpose() << "\n";
  std::cout << "  exact_minus_fd          = "
            << (exact_trace - fd_trace).transpose() << "\n";
  std::cout << "  rel_Hdot_matrix_err     = " << rel_hdot_matrix_err.transpose()
            << "\n";
#else
  (void)exact_trace;
  (void)fd_trace;
  (void)rel_hdot_matrix_err;
#endif
}

inline void print_gradient_parts(const Eigen::VectorXd &joint_grad,
                                 const Eigen::VectorXd &logdet_grad,
                                 const Eigen::VectorXd &total_grad) {
#ifdef QUADRA_DEBUG_LAPLACE_GRADIENT_PARTS
  std::cout << "Quadra gradient parts\n";
  std::cout << "  joint_grad  = " << joint_grad.transpose() << "\n";
  std::cout << "  logdet_grad = " << logdet_grad.transpose() << "\n";
  std::cout << "  total_grad  = " << total_grad.transpose() << "\n";
#else
  (void)joint_grad;
  (void)logdet_grad;
  (void)total_grad;
#endif
}

inline void
print_logdet_gradient_comparison(const Eigen::VectorXd &exact_logdet_grad,
                                 const Eigen::VectorXd &fd_logdet_grad) {
#ifdef QUADRA_DEBUG_LAPLACE_GRADIENT_PARTS
  std::cout << "Quadra logdet gradient parts\n";
  std::cout << "  logdet_grad      = " << exact_logdet_grad.transpose() << "\n";
  std::cout << "  logdet_fd_grad   = " << fd_logdet_grad.transpose() << "\n";
  std::cout << "  logdet_grad diff = "
            << (exact_logdet_grad - fd_logdet_grad).transpose() << "\n";
#else
  (void)exact_logdet_grad;
  (void)fd_logdet_grad;
#endif
}

} // namespace diagnostics
} // namespace laplace
} // namespace quadra
