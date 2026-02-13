#include <chrono>
#include <cstddef>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <time.h>
#include <type_traits>
#include <unistd.h>
#include <unordered_map>
#include <variant>

class Node {
public:
  const std::string name;

protected:
  std::unordered_map<std::string, Node *> children;

public:
  Node(std::string name) : name(name), children() {}

  void connect(Node *node) {
    if (!node) {
      return;
    }
    children[node->name] = node;
  }

  void disconnect(Node *node) {
    if (!node) {
      return;
    }
    children.erase(children.find(node->name));
  }

  size_t child_count() const { return children.size(); }

  bool has_child(std::string name) const {
    return children.find(name) != children.end();
  }

  bool has_child(Node *node) const {
    if (!node) {
      return false;
    }
    return has_child(node->name);
  }

  bool is_child() const { return has_child(name); }

  template <class _Node> _Node *get_child(std::string name) {
    static_assert(std::is_convertible_v<_Node *, Node *>);
    auto it = children.find(name);
    if (it == children.end()) {
      return nullptr;
    } else {
      return static_cast<_Node *>(it->second);
    }
  }
};

template <typename... _Inputs>
using InputVariant = std::variant<std::monostate, _Inputs...>;

template <typename _InputVariant, typename _Output, typename _StateNode>
struct StateTransition {
  _Output output;
  _StateNode *state;
  std::optional<_InputVariant> input;

  StateTransition(_Output output, _StateNode *next = nullptr,
                  std::optional<_InputVariant> input = std::nullopt)
      : output(output), state(next), input(input) {}
};

template <typename _InputVariant, typename _Output>
class StateNode : public Node {
public:
  using input_variant_t = _InputVariant;
  using output_t = _Output;
  using state_node_t = StateNode<input_variant_t, output_t>;
  using state_transition_t =
      StateTransition<input_variant_t, output_t, state_node_t>;

public:
  StateNode(std::string name) : Node(name) {}

  // `enter` will be invoked upon a FSM transitioning into this `StateNode`.
  // The input that was used to exit the previous `StateNode` will be the input
  // to this `StateNode`.
  virtual void enter(const input_variant_t &input) {}

  // `exit` will be invoked upon a FSM transitioning out of this `StateNode`.
  // The input is the same of when the previous `process` function was invoked.
  virtual void exit(const input_variant_t &input) {}

  // `process` will be invoked upon a FSM receiving input. It will return a
  // `StateTransition` that has the next state and output after processing.
  virtual state_transition_t process(const input_variant_t &input) = 0;
};

// template <typename _InputVariant, typename _Output>
// class DelayStateNode : public StateNode<_InputVariant, _Output> {
// private:
//   int _delay;

// public:
//   DelayStateNode(std::string name, int seconds = 0)
//       : StateNode<_InputVariant, _Output>(name), _delay(seconds) {}

//   void process(const DelayStateNode::input_variant_t &) override {}
// };

// template <typename _InputVariant, typename _Output>
// class ExitDelayNode : public StateNode<_InputVariant, _Output> {
// private:
//   int _delay;

// public:
//   ExitDelayNode(std::string name, int seconds = 0)
//       : StateNode<_InputVariant, _Output>(name), _delay(seconds) {}

//   void exit(const ExitDelayNode::input_variant_t &) override { sleep(_delay);
//   }
// };

template <typename _InputVariant, typename _Output> class FiniteStateMachine {
  //   static_assert(std::is_same_v<_InputVariant, InputVariant<typename
  //   _InputVariant::input_types_t>>);
public:
  using input_variant_t = _InputVariant;
  using output_t = _Output;
  using state_node_t = StateNode<input_variant_t, output_t>;
  using state_transition_t = typename state_node_t::state_transition_t;

private:
  state_node_t *_current;
  std::chrono::nanoseconds _feedback_rate =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::milliseconds(500));

public:
  FiniteStateMachine(state_node_t *initial,
                     const input_variant_t &arg = input_variant_t())
      : _current(initial) {
    if (!_current) {
      throw std::runtime_error("No initial state set in FSM.\n");
    }
    _current->enter(arg);
  }

  const state_node_t *current_state() const { return _current; }

  [[maybe_unused]] output_t
  transduce(const input_variant_t &input = input_variant_t()) {
    if (!_current) {
      throw std::runtime_error("No current state for FSM\n");
    }

    auto transition = _current->process(input);
    if (transition.state) {
      _transition(transition.state, input);
    }

    if (transition.input.has_value()) {
      return _feedback(transition.input.value());
    }

    return transition.output;
  }

protected:
  void _transition(state_node_t *state, const input_variant_t &input) {
    if (!_current->has_child(state)) {
      throw std::runtime_error("Invalid transition from " + _current->name +
                               " to " + state->name);
    }
    _current->exit(input);
    _current = state;
    _current->enter(input);
  }

  output_t _feedback(input_variant_t &input) {
    auto delay_secs =
        std::chrono::duration_cast<std::chrono::seconds>(_feedback_rate);
    auto delay_ns =
        delay_secs.count() == 0 ? _feedback_rate : _feedback_rate % delay_secs;
    timespec ts{delay_secs.count(), delay_ns.count()};
    while (nanosleep(&ts, &ts)) {
    }
    return transduce(input);
  }
};

// TODO think of how to implement time-based execution
// Ex: FSM polls for input on 1 sec intervals via receive queue
/*
  class TimeFSM:
      Queue q
      func poll(int freq):
        while true:
          if q:
            transduce(q.pop())
          else:
            transduce(now_time())
*/