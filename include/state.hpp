#include <any>
#include <memory>
#include <string>

template <class obj_t> class state_transition;
template <class obj_t> class finite_state_machine;
/*
An abstract represention of a state of a finite state machine.

  obj_t: The type of object the state will be operated on.
*/
template <class obj_t> class state {
  friend std::hash<state<obj_t>>;

private:
  std::weak_ptr<finite_state_machine<obj_t>> _fsm;
  std::string _name;

public:
  state<obj_t>(std::shared_ptr<finite_state_machine<obj_t>> fsm,
               std::string name)
      : _fsm(fsm), _name(name) {}
  /*
  What should happened when the State is entered/started.
    input: Input of any type.
  */
  virtual void process(std::any input) = 0;
};

template <class obj_t> struct std::hash<state<obj_t>> {
  std::size_t operator()(const state<obj_t> &s) const {
    return std::hash<std::string>{}(s._name);
  }
};
