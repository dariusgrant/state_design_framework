#include "../../include/state.hpp"
#include <any>

class simple_calculate {
private:
  int sum;

public:
  void add(int x) { sum += x; }
  void subtract(int x) { sum -= x; }
  void reset() { sum = 0; }
};

class add_state : public state<int> {
  void on_enter(int &obj) { ++obj; }

  void on_exit(int &obj) {}

  void process(finite_state_machine<int> &fsm, std::any input) {
    auto input_str = std::any_cast<std::string>(input);
    if (input_str == "-") {
      fsm.set_state("subtract");
    }
  }
};

void a() {}
int main() {}