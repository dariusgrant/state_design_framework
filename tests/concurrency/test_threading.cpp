/*
Testing of objects listed in `include/concurrency/Threading.hpp`
*/

#include "../../include/concurrency/Threading.hpp"
#include <atomic>
#include <cassert>
#include <chrono>
#include <future>
#include <initializer_list>
#include <iostream>
#include <ostream>
#include <pthread.h>
#include <sched.h>
#include <syncstream>
#include <thread>

struct bundle {
  fsm::ThreadOwnership &ownership;
  int &x;
};

void *free_inc(void *params) {
  auto b = reinterpret_cast<bundle *>(params);
  for (auto i = 0; i < 100; ++i) {
    b->ownership.wait_until_yield();
    std::osyncstream(std::cout)
        << "[" << std::this_thread::get_id() << "] - New Owner\n";
    ++b->x;
    std::osyncstream(std::cout)
        << "[" << std::this_thread::get_id() << "] - x = " << b->x << "\n";
    b->ownership.release_ownership();
  }

  return nullptr;
}

std::vector<pthread_t>
create_threads(int count, fsm::ThreadOwnership &thread_ownership, int &x) {
  std::vector<pthread_t> res;
  for (auto i = 0; i < count; ++i) {
    pthread_t p_t1;
    pthread_attr_t p_t1_attr;
    pthread_attr_init(&p_t1_attr);
    pthread_attr_setschedpolicy(&p_t1_attr, SCHED_OTHER);
    auto b = bundle(thread_ownership, x);
    pthread_create(&p_t1, &p_t1_attr, &free_inc, &b);
    res.push_back(p_t1);
    std::cout << "Created thread " << i << "\n";
  }
  return res;
}

void test_sequential_increment() {
  fsm::ThreadOwnership thread_ownership;
  int x = 0;
  auto inc = [&]() {
    // auto s = fsm::ScopedThreadOwnership();
    for (auto i = 0; i < 100; ++i) {
      thread_ownership.wait_until_yield();
      ++x;
      thread_ownership.release_ownership();
    }
  };

  auto threads = create_threads(5, thread_ownership, x);
  for (auto t : threads) {
    pthread_join(t, nullptr);
  }

  // auto set_thread_schedule_policy = [](std::initializer_list<pthread_t>
  // threads,
  //                                      int policy) {
  //   int p = 0;
  //   for (auto t : threads) {
  //     std::cout << "Thread " << t << "\n";
  //     pthread_attr_t attr;
  //     pthread_getattr_np(t, &attr);
  //     pthread_attr_getschedpolicy(&attr, &p);
  //     std::cout << "\tInitial Schedule Policy: " << p << "\n";
  //     pthread_attr_setschedpolicy(&attr, policy);
  //     pthread_attr_getschedpolicy(&attr, &p);
  //     std::cout << "\tUpdated Schedule Policy: " << p << "\n";
  //   }
  // };
  // auto t1 = std::thread(inc);
  // auto t2 = std::thread(inc);
  // auto t3 = std::thread(inc);
  // auto t4 = std::thread(inc);
  // auto t5 = std::thread(inc);
  // set_thread_schedule_policy({t1.native_handle(), t2.native_handle(),
  //                             t3.native_handle(), t4.native_handle(),
  //                             t5.native_handle()},
  //                            SCHED_RR);

  // t1.join();
  // t2.join();
  // t3.join();
  // t4.join();
  // t5.join();

  std::cout << "[" << std::this_thread::get_id() << "] " << x << "\n";
  assert(x == 500);
}

int main() { test_sequential_increment(); }