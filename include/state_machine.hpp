#include <unordered_map>
#include <stdexcept>

template <class Input>
class State {
    private:
    std::unordered_map<Input, State*> transition_map;
    
    public:
    virtual void on_enter() = 0;
    virtual void on_exit() = 0;

    State& add_transition(Input input, State* new_state) {
        transition_map[input] = new_state;
        return *this;
    }

    State* transition(Input input) {
        try {
            return transition_map.at(input);
        } catch (std::out_of_range& e) {
            return this;
        }
    }
};

template <class Input>
class StateMachine {
    private:
    State<Input>* current_state;

    public:
    void set_current_state(State<Input>* state) {
        current_state = state;
    }

    void input(Input input) {
        if(!current_state) {
            throw std::runtime_error("State machine hasn't set initial state.");
        }
        current_state->on_exit();
        current_state = current_state->transition(input);
        current_state->on_enter();
    }
    
};
