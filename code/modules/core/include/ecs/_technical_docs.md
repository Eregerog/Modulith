\page ecs_specs ECS Technical Documentation

*This docs file gives an in-depth overview of the technical specifications of Modulith's ECS. For an API documentation, refer to the \ref ecs_api "ECS API Docs"*

## Definitions

### Entity
An ``modulith::Entity`` is a handle with only an ID as data an no functionality by its own. It can be used in conjunction with an ``modulith::EntityManager`` to receive data objects, called components.

### Components
Components are data object attached to an entity relative to an entity manager. Only one type of component may be attached to an entity. The data of a component is not stored by an entity, but by the entity manager. 
As a result, an entity can have different components depending on the entity manager.

It is recommended that components do not have any methods. Should methods be implemented, it is recommended that these functions are defined in the header so the compiler can inline them. This is important for cache friendly code (for more, see below)

### Component Manager
Only component types that are registered to the ``modulith::ComponentManager`` can be used as components. 
The Manager is responsible for providing component-type specific methods, such as converting between ``modulith::SignatureIdentifier``s to ``modulith::Signature``s.

### Component Identifier
A ``modulith::ComponentIdentifier`` is a unique object that identifies the *type* of a component. All instances of components with the same type have the same Component Identifier.

### Signature Identifier
A ``modulith::SignatureIdentifier`` is a set of Component Identifiers. For instance a signature identifier can describe all component types that are present on an entity.

### Signature
A ``modulith::Signature`` stores the same information as a signature identifier, namely a set of component identifiers. However, it is implemented as a std::bitset, allowing for fast bitwise operations.
This is possible because each component identifier is given a unique index, which is its index in the bitset.

### Entity Chunk
An ``modulith::EntityChunk`` contains a fixed number of entities that all have the same *Signature*, meaning the same kind of components attached to them. It ensures a cache-friendly memory layout by using its own memory allocation, as described below.
The capacity of a chunk depends on the size of all components - the more data each entity needs for its components the fewer entites can be stored in a chunk. This ensures that entity chunks have a predicable memory size when allocating new ones.

### Entity Manager
An ``modulith::EntityManager`` contains a variable number of EntityChunks to store the data of all entities and their components that are "alive" in the entity manager.
It has components to create or destroy entities, add or remove components and query the status of entities and their components.
When an entity receives or loses components, it changes the entity chunk it is in, potentially allocating a new chunk or deleting an old chunk.

Generally speaking, the destruction an an entity is always deferred until the end of frame, to allow all systems to properly update before the entity is removed.

### Systems Group
A ``modulith::SystemsGroup`` manages a number of Systems and invokes the game event callbacks on them. The systems it manages can be given "happens-after" dependencies to modify their execution order.
There can be multiple Systems groups and "happens-after" dependencies can also be defined between systems groups. However, all systems in one group are always executed before the systems of another group.

Only one systems groups per type can exist.

### Systems
A ``modulith::System`` receives game event callbacks for initialization, every frame, imgui drawing and shutdown. They are supposed to read from and write to the data of entities and components.
The queries in the entity managers should be used to modify components in bulk, rather than creating a system for every entity.
Also, the state of systems should be kept to a minimum and should be reset when ``OnShutdown`` is called. Both of these recommendations exists to maintain cache-friendly memory layout. 

Only one system per type can exist.

### ECS Context

The ``modulith::ECSContext`` is a context object that can be retrieved by using ``Context::GetInstance<ECSContext>()`` which contains the EntityManager and ComponentManager.

## Memory Layout & Allocation

The ECS system is designed to optimze the usage of the *data cache* and *instruction cache* of the CPU when reading or writing the data of many similar entities.

The **data cache is optimized** by allocating the components of similar entities next to each other in memory and using query methods to iterate over them sequentially. This aims to eliminate cache misses while iterating over all entities of a chunk.
Thus, it is optimal to have few chunks filled with entities rather than many chunks with only a few entities.

The **instruction cache is optimized** by separating data and logic and using queries: The code inside a query my be executed hundred of times in succession, preventing cache misses during this time. After than, it will not be needed until the next frame.
  