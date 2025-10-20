#include "../../include/concurrency/Queue.hpp"
#include <algorithm>
#include <cassert>
#include <future>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>

void test_multiple_producer() {
  fsm::Queue<std::pair<std::thread::id, int>> queue;

  std::unordered_map<std::thread::id, std::vector<int>> actual_counts;

  auto produce_func = [&](int x) {
    auto tid = std::this_thread::get_id();
    for (auto i = 0; i < 100; ++i) {
      queue.push({tid, x});
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
    actual_counts[val.first].push_back(val.second);
  }

  for (auto &kv : actual_counts) {
    assert(kv.first != std::thread::id());
    assert(std::all_of(kv.second.begin(), kv.second.end(),
                       [&](auto i) { return i == *kv.second.begin(); }));
  }
}

int main() { test_multiple_producer(); }