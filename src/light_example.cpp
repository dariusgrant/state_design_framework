#include <functional>
#include <iostream>
#include "../include/publisher.hpp"
#include "../include/state_machine.hpp"

class LightSwitchSubscriber {
    public:
    virtual void lightswitch_on_cb() = 0;
    virtual void lightswitch_off_cb() = 0;
    virtual void lightswitch_on_cb(int p) = 0;
    virtual void lightswitch_off_cb(int p) = 0;
};

class Lightbulb : public LightSwitchSubscriber {
    void lightswitch_on_cb() override {
        std::cout << "Lightbulb On\n";
    }

    void lightswitch_off_cb() override {
        std::cout << "Lightbulb Off\n";
    }

    void lightswitch_on_cb(int p) override {
        std::cout << "Lightbulb = " << p << "\n";
    }

    void lightswitch_off_cb(int p) override {
        std::cout << "Lightbulb = " << p << "\n";
    }
};

class Radio : public LightSwitchSubscriber {
    void lightswitch_on_cb() override {
        std::cout << "Radio On\n";
    }

    void lightswitch_off_cb() override {
        std::cout << "Radio Off\n";
    }

    void lightswitch_on_cb(int p) override {
        std::cout << "Radio = " << p << "\n";
    }

    void lightswitch_off_cb(int p) override {
        std::cout << "Radio = " << p << "\n";
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
        notify_all(&LightSwitchSubscriber::lightswitch_on_cb);
        notify_all(&LightSwitchSubscriber::lightswitch_on_cb, 1);
    }

    void turn_off() {
        input(false);
        notify_all(&LightSwitchSubscriber::lightswitch_off_cb);
        notify_all(&LightSwitchSubscriber::lightswitch_off_cb, 0);
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