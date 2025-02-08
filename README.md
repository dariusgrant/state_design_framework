# state_design_framework
A framework for writing code using the state design pattern as its basis.

# Objects
Below are the objects defined for this framework:
* `state`
* `finite_state_machine`

## `state`
A `state` object is the fundamental basis of a state machine. It is bounded to a arbitrary type of object, denoted as `obj_t`. It defines specific behavior for the object in the context of the current state of the object.

As an example that relates to real life, the state of a baby can determine their behavior. If a baby is in the state of hunger, their behavior may be to cry. Contrastly, if a baby is in the state of contentment, their behavior may be to coo or smile. 

This can be translated to the state of software components and defining how they should behave when reaching a certain state. For example, an editor may hold multiple states: view, modify, save, export, etc. Within these states are specific behavior that are constrained to the state's context - viewing cannot exhibit behaviors of modifying, modifying cannot exhibit behaviors of saving, etc.

The `state` object defined in this framework takes on the perspective of containing three functions: `on_enter`, `on_exit`, and `process`. 

`on_enter` and `on_exit` operates on the any object of type `obj_t`. These are user-defined operations on what behavior should occur for the object upon entering and exiting the state.

`process` is a user-defined operation in which a `finite_state_machine` object and any arbitrary input is passed in as parameters. The intention of this function is to evaluate incoming input into the `state` object and potentially manipulate the `finite_state_machine` object by assigning it's current state to another. This would represent the concept of a state transition process. 

## `finite_state_machine`
A `finite_state_machine` object encapsulates an specific type of object (denoted as `obj_t`), a set of states that the object can be in, and the current state of the object. The purpose of of this object is to facilitate state tracking and transitioning of the object it encapsulates.

There are two functions that are available for use: `process` and `set_state`. `process` will send any input passed into the state machine to the current state of the object along with the `finite_state_machine` object itself as a reference.