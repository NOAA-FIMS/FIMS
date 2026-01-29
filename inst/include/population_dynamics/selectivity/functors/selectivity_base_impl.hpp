/**
 * @file selectivity_base_impl.hpp
 * @brief Implementation details for `SelectivityBase` template.
 */
#ifndef POPULATION_DYNAMICS_SELECTIVITY_BASE_IMPL_HPP
#define POPULATION_DYNAMICS_SELECTIVITY_BASE_IMPL_HPP

namespace fims_popdy {

template <typename Type>
SelectivityBase<Type>::SelectivityBase() {
  // increment id of the singleton selectivity class
  this->id = SelectivityBase::id_g++;
}

template <typename Type>
SelectivityBase<Type>::~SelectivityBase() = default;

// default id of the singleton selectivity class
template <typename Type>
inline uint32_t SelectivityBase<Type>::id_g = 0;

}  // namespace fims_popdy

#endif /* POPULATION_DYNAMICS_SELECTIVITY_BASE_IMPL_HPP */
