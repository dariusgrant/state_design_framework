#include "../include/FiniteStateMachine.hpp"
#include <cassert>
#include <iostream>
#include <string>

using int_int_state = State<int, int>;
class S1;
class S2;

class S1 : public int_int_state {
public:
  output_state_pair_t operator()() { return {-1, state_hash_v<S1>}; }
  output_state_pair_t operator()(int input) {
    return {input + 1, state_hash_v<S2>};
  }
};

class S2 : public int_int_state {
public:
  output_state_pair_t operator()() { return {-1, state_hash_v<S2>}; }
  output_state_pair_t operator()(int input) {
    return {input + 2, state_hash_v<S1>};
  }
};

class S3;

using int_string_state = State<int, std::string>;
class S3 : public int_string_state {
public:
  output_state_pair_t operator()() { return {"", state_hash_v<S3>}; }
  output_state_pair_t operator()(int input) {
    return {std::to_string(input), state_hash_v<S3>};
  }
};

int main(int, const char **) {
  S1 s1;
  assert((s1() == std::make_pair(-1, state_hash_v<S1>)));
  assert((s1(1) == std::make_pair(2, state_hash_v<S2>)));

  S2 s2;
  assert((s2() == std::make_pair(-1, state_hash_v<S2>)));
  assert((s2(2) == std::make_pair(4, state_hash_v<S1>)));

  FiniteStateMachine<int, int, S1> fsm1;
  assert(fsm1.state_count == 1);
  assert((fsm1.is_current_state<S1>()));
  assert((fsm1.current_state_hash() == state_hash_v<S1>));
  assert(fsm1.is_current_state_valid());

  assert(fsm1.step() == -1);
  assert(fsm1.is_current_state_valid());

  assert((fsm1.is_current_state<S1>()));
  assert(fsm1.is_current_state_valid());

  assert(fsm1.step(5) == 6);
  assert(!(fsm1.is_current_state<S1>()));
  assert(fsm1.current_state_hash() == 0);
  assert(!fsm1.is_current_state_valid());

  FiniteStateMachine<int, int, S1, S2> fsm2;
  assert(fsm2.state_count == 2);
  assert((fsm2.is_current_state<S1>()));
  assert(fsm2.is_current_state_valid());

  assert(fsm2.step() == -1);
  assert(fsm2.is_current_state_valid());

  assert((fsm2.is_current_state<S1>()));
  assert(fsm2.is_current_state_valid());

  assert(fsm2.step(5) == 6);
  assert((fsm2.is_current_state<S2>()));
  assert(fsm2.is_current_state_valid());

  assert(fsm2.step() == -1);
  assert((fsm2.is_current_state<S2>()));
  assert(fsm2.is_current_state_valid());

  assert(fsm2.step(6) == 8);
  assert((fsm2.is_current_state<S1>()));
  assert(fsm2.is_current_state_valid());

  FiniteStateMachine<int, std::string, S3> fsm3;
  std::cout << fsm3.step(fsm2.step(8));

  auto cascade_fsm = CascadingFiniteStateMachine(fsm2, fsm3);
  cascade_fsm.step(5);
}