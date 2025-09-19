#pragma once

#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <variant>

namespace fsm {
template <class FirstStateType, class... RemainingStateTypes>
using StateVariantType = std::variant<FirstStateType, RemainingStateTypes...>;

template <class FirstStateType, class... RemainingStateTypes>
using StateMap = std::unordered_map<
    std::type_index, StateVariantType<FirstStateType, RemainingStateTypes...>>;

template <class FirstStateType, class... RemainingStateTypes>
using StateTuple = std::tuple<FirstStateType, RemainingStateTypes...>;
} // namespace fsm