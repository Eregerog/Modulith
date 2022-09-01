\page ecs_api ECS API Documentation

*This docs file gives a short overview of Modulith's ECS system.
For a more detailed and technical introduction, refer to the \ref ecs_specs "technical documentation"*

This ECS system is made for communication between different game sub-systems and allowing one subsystem to re-use or extend the functionality of another.
It is divided into three parts: **Entities**, **Components** and **Systems** and follows a strict separation of logic and data.

## ECS Context

In order to get the ``modulith::EntityManager`` to manipulate entities or to query systems, a reference to the Context's ``ECSContext`` is needed.
If you already have a reference to the ``modulith::Context``, the ECSContext can be retrieved as follows: 
```cpp
#include <ecs/ECSContext.h>

// Optional, using namespace for readability
using namespace modulith;

shared<ECSContext> ecsCtx = Context::GetInstance<ECSContext>();

ref<EntityManager> entityManager = ecsCtx->GetEntityManager();
```

## Entities

An ``Entity`` on its own does nothing and has (almost) no data. It is simply an identifier to be used in conjunction with an ``modulith::EntityManager``. The ``modulith::EntityManager`` knows which Entities are currently registered in it. These entities are considered *alive* relative to that ``EntityManager``.

User-defined **Components** can be attached to entities to give them data. **Systems** can query Entities through the ``modulith::EntityManager`` to mutate their component's data.

Entities also have aliases for all of the ``modulith::EntityManager`` entity mutation methods. For example:

```cpp
modulith::ref<modulith::EntityManager> entityManager;
Entity e;

// Instead of writing
entityManager->AddComponent<FooData>(e);
entityManager->RemoveComponent<FooData>(e);
entityManager->Defer([e](auto manager) { manager->AddComponent<FooData>(e); });

// We can write
e.Add<FooData>(entityManager);
e.Remove<FooData>(entityManager);
e.AddDeferred<FooData>(entityManager);
```

## Components

**Components** can be defined by the user in the form of classes or structs and can be attached to an ``Entity``. They are not supposed to have any behaviour / logic and only contain data.
There are two types of components:
1. Data components: These are structs / classes with fields (e.g. they contain data). The recommended naming scheme is ``FooData``.
2. Tag components: These are structs / classes with no fields (e.g. they have no data). The recommended naming scheme is ``FooTag``.

It is best practice to keep the data inside a component to a minimum and split larger components into smaller one's when possible.
Furthermore, having two different components with partially the same data (double-state) should be avoided at all costs. 

When creating a component, that struct / class does not need to inherit from any class. Instead, it simply needs to be registered with the ``modulith::ComponentManager``.
For the recommended way to do so, refer to the component registration documentation below.

### Example

```cpp
// A data component
struct PositionData {
   public float3 Value; 
};

// A tag component
struct IsFloatingTag{};

// Once registered, they can be added to entities using the EntityManager
entityManager->template AddComponent<PositionData>(myEntity, PositionData { float3 { 1, 1, 0 } } );
```

## Systems

**Systems** are classes that receive game-engine lifetime callbacks, such as ``OnInitialize``, ``OnUpdate`` and ``OnShutdown``.
Whereas **Entities** and **Components** do not have any behaviour, a system's callback functions can be used to mutate the Entities and their Components.
However, systems are not supposed to have any persistent state.

In order to mutate Entities and Components and bring about game logic, **Systems** can use the ``modulith::EntityManager`` queries: ``QueryAll`` and ``QueryActive``.

To create one's own system, create a custom class that inherits from ``modulith::System`` and overwrite the desired methods.
To register a system, refer to the system registration below.

### Example

```cpp
#include "ecs/System.h"

class TestSystem : public modulith::System {
public:    
    TestSystem() : modulith::System("Test System") {}

    void OnInitialize() override { }

    void OnUpdate(float deltaTime) override  { }

    void OnImGui(float deltaTime) override { }
        
    void OnShutdown() override { }
};
```

## System Execution Order

The execution order of systems can be greatly customized: First, systems are grouped in one of many ``SystemsGroups``. These groups have an order and all systems within a group receive their callbacks before the systems of the next group. Within a group, the execution order can also be defined. 
There exist the following systems groups by default: ``InputSystemsGroup``, ``TransformSystemsGroup``  and ``LogicSystemsGroup`` which are executed in the given order.

For systems and systems groups ``ExecuteBefore<...>`` and ``ExecuteAfter<...>`` are used to define which system / groups must be executed before or after a given system. This forms a "DirectedAcyclicGraph" that is topologically traversed when sending callbacks.

To create one's own systems group, create a custom class that inherits from ``modulith::SystemsGroup`` and overwrite the ``GetName()`` method.
To register a systems group, refer to the system registration below.

### Example

```cpp
#include "ecs/systems/SystemsGroup.h"

class MyGroup : public modulith::SystemsGroup {    
public:
    std::string GetName() override { return "MySystemsGroup"; }    
};
```

## Queries

In order to bring about gameplay behaviour, systems often times modify entities with a certain amount of components, called a specific signature.
**Queries** in the ``EntityManager`` can be used to get all entities with a desired signature and execute code for each of them.

The restrictions on the signature can be defined with the following structs by giving them any amount of types as template parameters:
- ``Each<...>()`` all given component types must be present on the entity
- ``Any<...>()`` at least on of the given component type must be present on the entity
- ``None<...>()`` none of the given component types must be present on the entity
- ``Has<...>()`` provides information for each component type if it is present on the entity

These objects are then given to the ``QueryActive`` or ``QueryAll`` methods of the entity manager along with the proper function (preferrably as a lambda).
This function will be called for every entity in the entity manager that matches all the restrictions. Its parameters must be as follows:
1. A parameter of type Entity for the current entity the function is called for
2. One parameter for each type given to the ``Each<...>()`` restriction as a reference
3. One parameter for each type given to the ``Any<...>()`` restriction as a pointer. This pointer is ``nullptr`` if the given component type is not present.
4. One boolean for each type given to the ``Has<...>()`` restriction that encodes if the component is present on the current entity

``QueryActive`` automatically excludes entities with the ``DisabledTag`` or ``IndirectlyDisabledTag``, while ``QueryAll`` does not.

Entities cannot be mutated inside the function of a Query. Therefore, all operations must be **Deferred** using the ``Defer`` method in the ``EntityManager`` or suitable aliases on the ``Entity``.

### Example

Here is an example showing all different restrictions:
```cpp
entityManager->QueryAll(Each<Foo>(), Any<Bar, Baz>(), None<Quaz>(), Has<Jazz, Bass>(),
    [](Entity current, Foo& fooComponent, Bar* barComponent,  Baz* bazComponent, bool hasJazz, bool hasBass){
        // called for every entity that matches the restriction
    }
);
```

Here is an example on how queries may be used for game logic:

```cpp
struct LifetimeData{
    float RemainingLifetime;
};

// A system to reduce the lifetime of all entities with such a component
entityManager->QueryActive(Each<LifetimeData>(), [&entityManager, deltaTime](Entity current, LifetimeData& lifetime){
    lifetime.RemainingLifetime -= deltaTime;
    if(lifetime.RemainingLifetime <= 0)
        current.DestroyDeferred(entityManager);
});
```

## Registering Components, Systems and Systems Groups

As with all APIs in Modulith, the ECS also supports registering new components and systems using the module resource system.
For more on the module resources, refer to the documentation on the module system.

### Registering Components

In order to register a component when initializing a module, the ``ComponentResource`` should be used. Apart from the type of the registered component, it also needs the component's name.

**Example**

```cpp
#include "ecs/resources/ComponentResource.h"

struct MyComponent { /* ... */ };
using namespace modulith;

void Initialize(ModuleResources& resources){
    resources.Register<ComponentResource<MyComponent>>("My Component");
}
```

### Registering Systems

In order to register a system when initializing a module, the ``SystemResource`` should be used.
The following information should be provided as the template types:
- The type of the system to register
- Optional: The type of the ``SystemsGroup`` the system will be executed in. Usage: ``InGroup<MySystemsGroup>``. If omitted, systems are automatically placed in the ``LogicSystemsGroup``
- Optional: The types of registered systems in the same group that must execute **before** this system. Usage ``ExecuteAfter<FirstSystemThatMustComeBefore, SecondSystemThatMustComeBefore, ...>``. If omitted, the system order is not specified.
- Optional: The types of registered systems in the same group that must execute **after** this system. Usage ``ExecuteBefore<FirstSystemThatMustComeAfterwards, SecondSystemThatMustComeAfterwards, ...>``. If omitted, the system order is not specified.

**Example**

```cpp
#include "ecs/resources/SystemResource.h"
#include "ecs/systems/DefaultSystemsGroups.h"
using namespace modulith;

class FirstSystem : public System { /* ... */ };
class SecondSystem : public System { /* ... */ };
class SystemInBetween : public System { /* ... */ };

void Initialize(ModuleResources& resources){
    resources.Register<SystemResource<FirstSystem, InGroup<LogicSystemsGroup>>();
    resources.Register<SystemResource<SecondSystem, InGroup<LogicSystemsGroup>, ExecuteAfter<FirstSystem>>();
    resources.Register<SystemResource<SystemInBetween, InGroup<LogicSystemsGroup>, ExecuteAfter<FirstSystem>, ExecuteBefore<SecondSystem>>>();

    // The systems will be executed in the following order: FirstSystem, SystemInBetween, SecondSystem
}
```

### Registering Systems Groups


In order to register a systems group when initializing a module, the ``SystemsGroupResource`` should be used.
The following information should be provided as the template types:
- The type of the systems group to register
- Optional: The types of registered systems group that must execute **before** this group. Usage ``ExecuteAfter<GroupThatMustComeBefore, ...>``. If omitted, the group order is not specified.
- Optional: The types of registered systems group that must execute **after** this group. Usage ``ExecuteBefore<GroupThatMustComeBefore, ...>``. If omitted, the system order is not specified.

**Example**

```cpp
#include "ecs/resources/SystemsGroupResource.h"
using namespace modulith;

class FirstSystemsGroup : public SystemsGroup { /* ... */ };
class SecondSystemsGroup : public SystemsGroup { /* ... */ };
class SystemsGroupInBetween : public SystemsGroup { /* ... */ };

void Initialize(ModuleResources& resources){
    resources.Register<SystemsGroupResource<FirstSystemsGroup>();
    resources.Register<SystemsGroupResource<SecondSystemsGroup, ExecuteAfter<FirstSystemsGroup>>();    
    resources.Register<SystemResource<SystemsGroupInBetween, ExecuteAfter<FirstSystemsGroup>, ExecuteBefore<SecondSystemsGroup>>>();

    // The systems will be executed in the following order: FirstSystemsGroup, SystemsGroupInBetween, SecondSystemsGroup
}
```