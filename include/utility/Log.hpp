#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <thread>

namespace fsm {
#ifndef NDEBUG
constexpr bool DEBUG_MODE = true;
#else
constexpr bool DEBUG_MODE = false;
#endif

std::mutex mut;

template <typename Obj>
void debug_log(Obj obj, std::string func, std::string msg) {
  if constexpr (DEBUG_MODE) {
    const std::lock_guard<std::mutex> lock(mut);
    std::clog << "[Thread " << std::this_thread::get_id() << " - "
              << typeid(obj).name() << ":" << func << "] " << msg << "\n";
  }
}
} // namespace fsm
