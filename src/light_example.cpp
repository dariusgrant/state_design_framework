#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <unordered_map>

class LightSwitchSubscriber {
    public:
    virtual void lightswitch_on_cb() = 0;
    virtual void lightswitch_off_cb() = 0;
};

class Lightbulb : public LightSwitchSubscriber {
    void lightswitch_on_cb() override {
        std::cout << "Lightbulb On\n";
    }

    void lightswitch_off_cb() override {
        std::cout << "Lightbulb Off\n";
    }
};

class Radio : public LightSwitchSubscriber {
    void lightswitch_on_cb() override {
        std::cout << "Radio On\n";
    }

    void lightswitch_off_cb() override {
        std::cout << "Radio Off\n";
    }

};

template <class T>
class Publisher {
    protected:
    std::vector<T*> subscribers;

    public:
    void add_subscriber(T* sub) {
        subscribers.push_back(sub);
    }
};
 
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

class LightSwitch : public Publisher<LightSwitchSubscriber>, StateMachine<bool>{    
    public:
    class OnState : public State<bool> {
        void on_enter() override {
            std::cout << "Switch On\n";
        }

        void on_exit() override {
            std::cout << "Turning Switch Off\n";
        }
    };

    class OffState : public State<bool> {
        void on_enter() override {
            std::cout << "Switch Off\n";
        }

        void on_exit() override {
            std::cout << "Turning Switch On\n";
        }
    };

    private:
    OnState on;
    OffState off;

    public:
    LightSwitch() {
        on.add_transition(true, &on);
        on.add_transition(false, &off);
        off.add_transition(true, &on);
        off.add_transition(false, &off);
        set_current_state(&off);
    }

    void turn_on(){
        input(true);
        for (auto& sub : subscribers) {
            sub->lightswitch_on_cb();
        }
    }

    void turn_off() {
        input(false);
        for (auto& sub : subscribers) {
            sub->lightswitch_off_cb();
        }

    }
};




int main() {
    auto lswitch = LightSwitch();
    auto lbulb = Lightbulb();
    auto radio = Radio();
    lswitch.add_subscriber(&lbulb);
    lswitch.add_subscriber(&radio);
    lswitch.turn_on();
    lswitch.turn_off();
}

/*
> clang++ ./src/light_example.cpp && ./a.out
Turning Switch On
Switch On
Lightbulb On
Radio On
Turning Switch Off
Switch Off
Lightbulb Off
Radio Off
*/