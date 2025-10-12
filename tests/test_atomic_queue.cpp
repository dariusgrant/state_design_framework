#include "../include/utility/Concurrency.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

void test_single_consumer_multiple_producer() {
  fsm::AtomicQueue<std::pair<std::thread::id, int>> queue;

  auto produce_func = [&](int x) {
    for (auto i = 0; i < 100; ++i) {
      queue.push({std::this_thread::get_id(), x});
      //   std::this_thread::yield();
    }
  };
  auto t1 = std::async(std::launch::async, produce_func, 0);
  auto t2 = std::async(std::launch::async, produce_func, 1);
  auto t3 = std::async(std::launch::async, produce_func, 2);

  t1.wait();
  t2.wait();
  t3.wait();

  while (!queue.empty()) {
    auto val = queue.pop();
    std::cout << val.first << ": " << val.second << "\n";
  }
}

int main() { test_single_consumer_multiple_producer(); }