#include "../include/event.hpp"
#include "../include/state_machine.hpp"
#include <algorithm>
#include <any>
#include <assert.h>
#include <functional>
#include <iostream>
#include <iterator>
#include <unordered_set>
#include <vector>

class light_switch {

  // Events - Will it let the world know something)occurred?
  class turn_event : public event<bool> {
  };

private:
  // State
  bool _turned_on = false;
  turn_event _turn_event;

public:
  light_switch() : _turned_on(false), _turn_event() {}

  bool is_turned_on() { return _turned_on; }

  void turn() {
    _turned_on = !_turned_on;
    _turn_event.notify(_turned_on);
  }

  void notify_on_turn_event(turn_event::event_signature callback) {
    _turn_event.subscribe(callback);
  }
};

class light_bulb {
private:
  unsigned int _illumination;

public:
  void charge(unsigned int illumination) { _illumination = illumination; }
};

class wire {
public:
  light_switch ls;
  light_bulb lb;

  wire(light_switch &ls, light_bulb &lb) : ls(ls), lb(lb) {}
};

class test_light_switch {
public:
  void test_construction() {
    auto ls = light_switch();
    assert(ls.is_turned_on() == false);
  }

  void test_turn_event() {
    std::vector<int> vec;
    auto ls = light_switch();
    ls.notify_on_turn_event([&](bool turned_on) { vec.push_back(1); });
    ls.notify_on_turn_event([&](bool turned_on) { vec.push_back(2); });

    ls.turn();
    ls.turn();
    ls.turn();

    assert(vec.size() == 6);
    for (auto it = vec.begin(); it != vec.end(); ++it) {
      auto index = std::distance(vec.begin(), it);
      assert(*it == (index % 2 == 0 ? 1 : 2));
    }
  }
};

int main() {
  auto test_ls = test_light_switch();
  test_ls.test_construction();
  test_ls.test_turn_event();
}