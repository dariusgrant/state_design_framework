#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>

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

template <typename _Output> class StateNode : public Node {
public:
  using state_node_t = StateNode<_Output>;
  using output_t = _Output;

  struct StateTransition {
    output_t output;
    state_node_t *next;

    StateTransition(output_t output, state_node_t *next)
        : output(output), next(next) {}
  };

public:
  StateNode(std::string name) : Node(name) {}

  // Default implementation for state processing - NOOP. Returns this node.
  virtual StateTransition process(void *) = 0;

  StateNode *get_child(std::string name) {
    auto child = Node::get_child<StateNode>(name);
    return child ? child : nullptr;
  }
};

template <typename _Output> class FiniteStateMachine {
public:
  using state_node_t = StateNode<_Output>;

private:
  state_node_t *_current;

public:
  template <typename _Arg = std::nullptr_t>
  FiniteStateMachine(state_node_t *initial, _Arg arg = _Arg(),
                     _Output *output = nullptr)
      : _current(initial) {
    if (!_current) {
      return;
    }
    auto transition = _current->process(arg);
    if (output) {
      *output = transition.output;
    }
    if (transition.next) {
      _transition(transition.next);
    }
  }

  const state_node_t *current_state() const { return _current; }

  template <typename _T> _Output transduce(_T &input) {
    if (!_current) {
      throw std::runtime_error("No current state for FSM\n");
    }

    auto transition = _current->process(&input);
    if (transition.next) {
      _transition(transition.next);
    }
    return transition.output;
  }

protected:
  void _transition(state_node_t *state) {
    if (_current && !_current->has_child(state)) {
      throw std::runtime_error("Invalid transition from " + _current->name +
                               " to " + state->name);
    }
    _current = state;
  }
};