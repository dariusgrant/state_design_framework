# August 24, 2024
Let there be two objects: Lightbulb as `B` and Lightswitch as `S`.

`B` has two states: `off` & `illuminate`
`S` has two states: `off` & `on`

`B`'s state is dependent on `S`'s state, where if f(x) => y:
```
|x         |  y         |
-------------------------
|`B->off`| `S->off` |
|`B->on` | `S->on`  |
```

One of the goals of the State Design Framework is to separate the states of both objects independently, while easily allowing them to interface with other without needing to explicitly invoke conditional statements. The current idealolgy is to incorporate callbacks within the framework so that upon entering a state, any object subscribing to the state will be notified.

Timeline for turning light on:
1. `S` defaults to `off`
2. `B` defaults to `off`
3. `B` subscribes to `S->off` where its action is to switch its state to `off`
4. `B` subscribes to `S->on` where its action is to switch its state to `on`
5. `S` takes input to toggle state to `S->on`
6. `S` invokes actions in `S->on`
7. `S` notifies subscribers to `S->on`
8. `B` callback is triggered, resulting in it's state switching to `B->on`

# Inferences
1. Each state contains at least one callback definition for entering the state. Adding an exit callback extends functionality.
2. Each state contains a list of subscribers in which those subscribers must implement the callback definition.

## Psuedocode
```
class State:
    subscribers: list

    decl on_enter_cb(args) -> callback

    func on_enter(args):
        do state-specific actions
        notify_subscribers()

    func add_subscribers(subs):
        for sub in subs:
            subscribers.append(sub)

    func notify_subscribers():
        for each sub in subscribers:
            sub->on_enter_cb(args)

class Bulb:
    states = {Off, Illuminate}
    current_state = Off

    func on_lightswitch_on(args):
        switch state to illuminate

    func on_lightswitch_off(args):
        switch state to off

class Switch:
    states = {Off, On}
    current_state = Off

    func on(args):
        switch current_state to on

    func off(args):
        switch current_state to off
```

# August 25, 2024
I've concluded that the states themselves will need to keep a list of subscribers in order to notify them. However, I'd like to find a way to abstract the conection away from the states and into the object the state resides in, presumable the "StateMachine" object.

## Example
Lightbulb subscribes to Lightswitch where Lightswitch invoke the respective callback based on the state.

## Inferences
- Lightbulb implements the callbacks declared in Lightswitch

## Psuedocode
```
interface LightSwitchSubscriber:
    abstract func lightswitch_on_cb():
        return

class Lightbulb(LightSwitchSubscriber):
    concrete func lightswitch_on_cb():
        turn lightbulb on when callback is invoked

class LightSwitchOnState(State):
    func on_enter(s: LightSwitch):
        turn on power from s
        for each sub in s.subscribers:
            s.lightswitch_on_cb()
        

class LightSwitch:
    subscribers: LightSwitchSubscribers
```

## Created
- src/light_example.cpp: Minimal example of hooking a lightbulb to a lightswitch. It excludes the `State` object as those are implementation details.

## Workflow
1. Create an abstract interface that will be used by the subscribing objects of the publishing object.
```c++
class LightSwitchSubscriber {
    public:
    virtual void lightswitch_on_cb() = 0;
};

// Implement object-specific interface
class Lightbulb : public LightSwitchSubscriber {
    void lightswitch_on_cb() override {
        ...
    }
};
```

2. Implement the interface for the subscribing objects.
```c++
class LightSwitchSubscriber {
    public:
    virtual void lightswitch_on_cb() = 0;
};

// Implement object-specific interface
class Lightbulb : public LightSwitchSubscriber {
    void lightswitch_on_cb() override {
        ...
    }
};
```

3. Create the publishing object that inherits from `Publisher<T>`, where `T` is the subscribing interface class.
```c++
// The base publishing class - apart of lib.
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
    ...
}
```