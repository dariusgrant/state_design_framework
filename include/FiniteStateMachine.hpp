#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
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

template <typename _Input, typename _Output> class StateNode : public Node {
public:
  using state_node_t = StateNode<_Input, _Output>;
  using input_t = _Input;
  using output_t = _Output;

  struct StateTransition {
    output_t output;
    state_node_t *state;

    StateTransition(output_t output, state_node_t *next = nullptr)
        : output(output), state(next) {}
  };

public:
  StateNode(std::string name) : Node(name) {}

  // `enter` will be invoked upon a FSM transitioning into this `StateNode`.
  // The input that was used to exit the previous `StateNode` will be the input
  // to this `StateNode`.
  virtual void enter(const input_t &input) {}

  // `exit` will be invoked upon a FSM transitioning out of this `StateNode`.
  // The input is the same of when the previous `process` function was invoked.
  virtual void exit(const input_t &input) {}

  // `process` will be invoked upon a FSM receiving input. It will return a
  // `StateTransition` that has the next state and output after processing.
  virtual StateTransition process(const input_t &input) = 0;
};

template <typename... _Inputs>
class InputVariant : public std::variant<std::monostate, _Inputs...> {
public:
  using input_types_t = std::tuple<_Inputs...>;
  
};

template <typename _InputVariant, typename _Output> class FiniteStateMachine {
//   static_assert(std::is_same_v<_InputVariant, InputVariant<typename _InputVariant::input_types_t>>);
public:
  using input_variant_t = _InputVariant;
  using output_t = _Output;
  using state_node_t = StateNode<input_variant_t, output_t>;

private:
  state_node_t *_current;

public:
  FiniteStateMachine(state_node_t *initial, const input_variant_t& arg = input_variant_t())
      : _current(initial) {
    if (!_current) {
      throw std::runtime_error("No initial state set in FSM.\n");
    }
    _current->enter(arg);
  }

  const state_node_t *current_state() const { return _current; }

  output_t transduce(const input_variant_t & input = input_variant_t()) {
    if (!_current) {
      throw std::runtime_error("No current state for FSM\n");
    }

    auto [output, state] = _current->process(input);
    if (state) {
      _transition(state, input);
    }
    return output;
  }

protected:
  void _transition(state_node_t *state, const input_variant_t& input) {
    if (!_current->has_child(state)) {
      throw std::runtime_error("Invalid transition from " + _current->name +
                               " to " + state->name);
    }
    _current->exit(input);
    _current = state;
    _current->enter(input);
  }
};