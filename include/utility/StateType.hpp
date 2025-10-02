#pragma once

#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <variant>

namespace fsm {
template <class FirstStateType, class... RemainingStateTypes>
using StateVariant = std::variant<FirstStateType, RemainingStateTypes...>;

template <class FirstStateType, class... RemainingStateTypes>
using StateMap =
    std::unordered_map<std::type_index,
                       StateVariant<FirstStateType, RemainingStateTypes...>>;

template <class FirstStateType, class... RemainingStateTypes>
using StateTuple = std::tuple<FirstStateType, RemainingStateTypes...>;
} // namespace fsm