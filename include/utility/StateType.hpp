#pragma once

#include <type_traits>
#include <typeinfo>

namespace fsm {
template <class _T>
const static inline std::size_t state_type_hash_v =
    typeid(std::decay_t<_T> *).hash_code();
} // namespace fsm