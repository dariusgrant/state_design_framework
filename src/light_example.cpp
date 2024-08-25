#include <iostream>
#include <vector>



class LightSwitchSubscriber {
    public:
    virtual void lightswitch_on_cb() = 0;
};

class Lightbulb : public LightSwitchSubscriber {
    void lightswitch_on_cb() override {
        std::cout << "Lightbulb On\n";
    }
};

class Radio : public LightSwitchSubscriber {
    void lightswitch_on_cb() override {
        std::cout << "Radio On\n";
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
class LightSwitch : public Publisher<LightSwitchSubscriber>{    
    public:
    void on_enter(){
        std::cout << "Switch On\n";
        for (auto& sub : subscribers) {
            sub->lightswitch_on_cb();
        }
    }
};




int main() {
    auto lswitch = LightSwitch();
    auto lbulb = Lightbulb();
    auto radio = Radio();
    lswitch.add_subscriber(&lbulb);
    lswitch.add_subscriber(&radio);
    lswitch.on_enter();
}

/*
> clang++ ./src/light_example.cpp && ./a.out
Switch On
Lightbulb On
*/