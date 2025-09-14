#pragma once

#include <cstdlib>
#include <functional>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <variant>

namespace fsm {
// Get the type index of pointer to State type.
// This allows classes to forward declare for cyclic dependencies.
template <class State>
constexpr auto GetStateTypeIndex =
    []() -> std::type_index { return std::type_index(typeid(State *)); };

template <class FirstStateType, class... RemainingStateTypes>
using StateVariantType = std::variant<FirstStateType, RemainingStateTypes...>;

template <class FirstStateType, class... RemainingStateTypes>
using StateMap = std::unordered_map<
    std::type_index, StateVariantType<FirstStateType, RemainingStateTypes...>>;

template <class FirstStateType, class... RemainingStateTypes>
using StateTuple = std::tuple<FirstStateType, RemainingStateTypes...>;
} // namespace fsm