# Entity Component System

Arclight employs an Entity Component System (ECS) architecture, which is very much a work in progress. This allows better use of CPU cache and multithreading.

A more efficent ECS architecture will require the following:

- More advanced job scheduler
  - Yielding
    - Waiting for other jobs to finish
    - Waiting for I/O completion
- Asynchronous I/O

Arclight currently uses EnTT as an ECS system.

### Entities

Entities are no more than integer IDs. They are managed by the **Registry**. Entities contain no code and consist only of components. Entities are added in **World** object.

### Components

Components represent the data associated with Entities. They contain no code and are exclusively data. They are used by **Systems**.

Components are implemented as concepts.

They must:

- Not be decayable (no pointers)
- Have a default constructor
- Have a move constructor

See `Arclight/ECS/Component.h`

### Systems

Systems consist of the code. Through querying the registry, they can obtain the relevant **Components** (data).

Systems are implemented as **Jobs** which call functions and are placed on the **ThreadPool** each tick. A system is any function (can be a class member) that takes a `float` (time elapsed since last tick) and `World&` (reference to current World object) as parameters.

See `Arclight/Core/Application.h`

### Registry

The registry keeps track of all the components and entities. There is one registry per **World**. 

## World

See [World.md](World.md)
