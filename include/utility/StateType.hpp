#pragma once

#include <typeindex>
#include <unordered_map>
#include <variant>

namespace fsm {
// Get the type index of pointer to State type.
// This allows classes to forward declare for cyclic dependencies.
template <class State>
auto GetStateTypeIndex =
    []() -> std::type_index { return std::type_index(typeid(State *)); };

template <class... StateTypes>
using StateMap =
    std::unordered_map<std::type_index, std::variant<StateTypes...>>;

template <class... StateTypes>
using StateMapIt = typename StateMap<StateTypes...>::iterator;
} // namespace fsm