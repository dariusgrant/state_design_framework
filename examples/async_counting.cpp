// #define NDEBUG
#include "../examples/include/Counter.hpp"
#include "../include/FiniteStateMachine.hpp"
#include "../include/utility/Log.hpp"
#include <chrono>
#include <future>
#include <queue>
#include <thread>

using namespace std::chrono_literals;

int main(int argc, const char **argv) {
  auto c1 = fsm::FiniteStateMachine<int, Counter, MaxCounter>();
  std::queue<std::shared_future<int>> q;
  c1.set_future_object_cb([&q](std::shared_future<int> fut) {
    q.push(fut);
    // std::cout << "User-Defined Future Callback: " << fut.get() << "\n";
  });
  c1.start(0);
  c1.process(1);

  auto f = [&q]() {
    for (auto i = 0; i < 100; ++i) {
      auto queue_empty_start = std::chrono::high_resolution_clock::now();
      size_t count = 0;
      while (q.empty()) {
        ++count;
        fsm::debug_log(q, "empty", std::to_string(count));
        std::this_thread::yield();
        continue;
      }
      auto queue_empty_end = std::chrono::high_resolution_clock::now();
      auto queue_empty_duration = (queue_empty_end - queue_empty_start);
      fsm::debug_log(q, "front", std::to_string(q.front().get()));
      fsm::debug_log(
          nullptr, __func__,
          fsm::key_value_string(
              "Duration", std::chrono::duration_cast<std::chrono::nanoseconds>(
                              queue_empty_duration)
                              .count()));
      q.pop();
    }
  };
  // Async
  auto async_start = std::chrono::high_resolution_clock::now();
  // std::thread jt(f);
  // jt.detach();
  auto async2 = std::async(std::launch::async, f);

  auto async1 = std::async(std::launch::async, [&c1]() {
    while (!c1.is_terminated()) {
      c1.process(1);
      // fsm::debug_log(nullptr, __func__, std::to_string(c1.get()));
      if (c1.get() == 100) {
        return;
      }
    }
  });

  async1.wait();
  auto async_end = std::chrono::high_resolution_clock::now();
  auto async_duration = (async_end - async_start);
  fsm::debug_log(
      nullptr, __func__,
      fsm::key_value_string(
          "Async",
          std::chrono::duration_cast<std::chrono::microseconds>(async_duration)
              .count()));

  fsm::debug_log(nullptr, __func__, "Queue Size: " + std::to_string(q.size()));
  //   fsm::debug_log(nullptr, __func__,
  //                  fsm::key_value_string("Result",
  //                  std::to_string(c1.get())));
}