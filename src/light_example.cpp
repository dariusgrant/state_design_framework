#include "../include/publisher.hpp"
#include "../include/state_machine.hpp"
#include <iostream>
#include <unordered_map>

class LightSwitchSubscriber : public Subscriber<LightSwitchSubscriber> {
public:
  LightSwitchSubscriber(const char *name) : Subscriber(name) {}
  virtual void lightswitch_on_cb() = 0;
  virtual void lightswitch_off_cb() = 0;
  virtual void lightswitch_on_cb(int p) = 0;
  virtual void lightswitch_off_cb(int p) = 0;
};

class Lightbulb : public LightSwitchSubscriber {
public:
  Lightbulb() : LightSwitchSubscriber("LightBulb"){};
  void lightswitch_on_cb() override { std::cout << "Lightbulb On\n"; }

  void lightswitch_off_cb() override { std::cout << "Lightbulb Off\n"; }

  void lightswitch_on_cb(int p) override {
    std::cout << "Lightbulb = " << p << "\n";
  }

  void lightswitch_off_cb(int p) override {
    std::cout << "Lightbulb = " << p << "\n";
  }
};

class Radio : public LightSwitchSubscriber {
public:
  Radio() : LightSwitchSubscriber("Radio"){};

  void lightswitch_on_cb() override { std::cout << "Radio On\n"; }

  void lightswitch_off_cb() override { std::cout << "Radio Off\n"; }

  void lightswitch_on_cb(int p) override {
    std::cout << "Radio = " << p << "\n";
  }

  void lightswitch_off_cb(int p) override {
    std::cout << "Radio = " << p << "\n";
  }
};

class LightSwitchState : public State<bool> {};
class OnState : public LightSwitchState {
  void on_enter(bool) override { std::cout << "Switch On\n"; }

  void on_exit(bool) override { std::cout << "Turning Switch Off\n"; }
};

class OffState : public LightSwitchState {
  void on_enter(bool) override { std::cout << "Switch Off\n"; }

  void on_exit(bool) override { std::cout << "Turning Switch On\n"; }
};

class LightSwitchV2 : public Publisher<LightSwitchSubscriber>,
                      FSM<LightSwitchState, bool> {
public:
private:
  // OnState on;
  // OffState off;

public:
  LightSwitchV2(LightSwitchState *initial_state,
                std::unordered_map<LightSwitchState *,
                                   std::unordered_map<bool, LightSwitchState *>>
                    transitions)
      : FSM(initial_state, transitions) {}

  void turn_on() {
    input(true);
    notify_all(&LightSwitchSubscriber::lightswitch_on_cb);
  }

  void turn_off() {
    input(false);
    notify_all(&LightSwitchSubscriber::lightswitch_off_cb);
  }
};

int main() {
  // auto lswitch = LightSwitch();
  OffState offstate;
  OnState onstate;
  std::unordered_map<bool, LightSwitchState *> offstate_transitions{
      {false, &offstate}, {true, &onstate}};
  std::unordered_map<bool, LightSwitchState *> onstate_transitions{
      {false, &offstate}, {true, &onstate}};

  std::unordered_map<LightSwitchState *,
                     std::unordered_map<bool, LightSwitchState *>>
      transitions = {{&offstate, offstate_transitions},
                     {&onstate, onstate_transitions}};
  auto lswitchv2 = LightSwitchV2(&offstate, transitions);
  auto lbulb = Lightbulb();
  auto radio = Radio();
  lswitchv2.add_subscriber(&lbulb);
  lswitchv2.turn_on();
  lswitchv2.turn_off();
  // # TODO test removing single subscriber
  // lswitch.remove_subscriber(lbulb);
  // lswitch.turn_off();
  // lswitch.turn_on();
  // radio.unsubscribe_from(lswitch);
  // lswitch.turn_off();
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