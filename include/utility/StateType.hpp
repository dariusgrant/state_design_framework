#pragma once

#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <variant>

namespace fsm {
template <class _T>
const inline size_t state_type_hash_v =
    typeid(std::remove_reference_t<_T> *).hash_code();
} // namespace fsm