# state_design_framework
A framework for writing code using the state design pattern as its basis.

# Objects
Below are the objects defined for this framework:
* `AbstractState`
* `FiniteStateMachine`

## `AbstractState`
An `AbstractState` object is the fundamental basis of a state machine.It encapsulates specific behavior (and possibly properties) for the object within the context of what the state represents.

As an example that relates to real life, the state of a baby can determine their behavior. If a baby is in the state of hunger, their behavior may be to cry or yell. Contrastly, if a baby is in the state of contentment, their behavior may be to coo or smile. The state describes the context to what the baby will do (or likely to do for non-deterministic models). 

This can be translated to the state of software components and defining how they should behave when reaching a certain state. For example, an ordering system. The system can be in the state of waiting, processing, sending, and printing. Waiting may have a screensaver of a popular food item or prompt the next user that it's ready for the next order. Processing takes in the user's inputs and calculates the final price of the order being sending. Sending sends the order to the restaurant and confirms successful deliver. Printing prints information related to the order, such as number or id, itemization of order, and other additional info.

The `AbstractState` object defined in this framework takes on the perspective of containing 3 core functions: 
- `on_enter`
- `on_exit`
- `process`

### `on_enter`
`on_enter`: These are user-defined operations on what behavior should occur for the object upon entering and exiting the state.

### `on_exit`
`on_exit`

### `process`
`process` is a user-defined operation in which a `FiniteStateMachine` object and any arbitrary input is passed in as parameters. The intention of this function is to evaluate incoming input into the `AbstractState` object and potentially manipulate the `FiniteStateMachine` object by assigning it's current state to another. This would represent the concept of a state transition process. 



## `FiniteStateMachine`
A `FiniteStateMachine` object encapsulates an specific type of object (denoted as `ObjType`), a set of states that the object can be in, and the current state of the object. The purpose of this object is to facilitate state tracking and transitioning of the object it encapsulates.
