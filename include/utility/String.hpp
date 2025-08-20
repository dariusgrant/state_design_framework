#include <string>
namespace fsm {
template <typename Value>
std::string key_value_string(std::string key, Value value) {
  if constexpr (std::is_integral_v<Value>) {
    return key + "=" + std::to_string(value);
  } else {
    return key + std::string("=") + value;
  }
}

template <typename Value, typename... Values>
std::string key_value_string(std::string key, Value value, Values... values) {
  return key_value_string(key, value) + key_value_string(values...);
}
} // namespace fsm