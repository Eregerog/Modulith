\page physics_api Physics Module API Documentation

*This docs file gives a short overview over the Physics Module*

The physics module uses Nvidia PhysX to simulate physics. It offers ECS components that allow other modules to define the physical interactions that should take place by modifying the data of these components. 

*Most physics components require a modulith::GlobalTransformData attached to them in order to be used for simulation. While this can be done manually, it is also automated when there is any other TransformComponent (e.g.) modulith::PositionData, modulith::RotationData, etc. present*

## Defining static geometry with collision

First, you likely want some static geometry in your scene. This can be done by using a modulith::physics::BoxColliderData while a modulith::GlobalTransformData is present.

Example:

```cpp
using namespace modulith;
using namespace modulith::physics;

Context& ctx = ...;
ref<EntityManager> ecs = ...;

Entity floor = ecs.CreateEntityWith(
    BoxColliderData(), // the collider automatically assumes the scale of the entity
    ScaleData(100.0f, 0.5f, 100.0f),
    /** some additional components that visualize the floor
);


```

## Creating a rididbody

In order to define an entity with kinematics you will need a modulith::physics::RidigBodyData while a modulith::GlobalTransformData is present.
You can additionally add a modulith::phyiscs::BoxColliderData if the rigidbody should collide,
while a modulith::physics::DisableGravityTag can be used to disable gravity.
For small but fast moving entities it is recommended to add a modulith::physics::EnableContinuousCollisionTag so no collisions are missed. 

Example:

```cpp
using namespace modulith;
using namespace modulith::physics;

Context& ctx = ...;
ref<EntityManager> ecs = ...;

Entity fallingBlock = ecs.CreateEntityWith(
    RigidBodyData(1.0f),
    BoxColliderData(),
    PositionData(0, 0, 10.0f)
    /** some components that visualize the collider **/
);
```

## Detecting collisions

When a modulith::physics::PhysicsContactsData is attached to an entity with a collider,
all its collisions can be queried.
The component will store all the entities this entity has started collision with the last physics update.
Entities that we already collided with before will not be in this list for as long as they are touched.


Example:

```cpp
using namespace modulith;
using namespace modulith::physics;

Context& ctx = ...;
ref<EntityManager> ecs = ...;

Entity detectCollisions = ecs.CreateEntityWith(
    BoxColliderData(),
    PhysicsContactsData(),
    PositionData(0, 0, 0)
    /** some components that visualize the collider **/
);

// later

PhysicsContactsData* contacts = detectCollisions.Get<PhysicsContactsData(ecs);

for(Entity collidedWith : contacts->BeginContact){
    // do something for every entity it started collision with
}
```

## Using character controllers

Lastly, this module also offers a component for easily creating characters controlled by physics:
By using the modulith::physics::CharacterControllerData alongside a modulith::GlobalTransformData.

This character controller can be told how much it should move every frame. It will then use a "collide and slide" algorithm
that was used in games like DOOM in order to determine if and how they should move. Collisions will automatically stop the character.

Example:

```cpp
using namespace modulith;
using namespace modulith::physics;

Context& ctx = ...;
ref<EntityManager> ecs = ...;

Entity doomGuy = ecs.CreateEntityWith(
    CharacterControllerData(/* radius: */ 0.3f, /* height: */ 1.2f),
    PositionData(0, 0.5f, 0)       
);

// to determine the movement:

CharacterControllerData* characterController = doomGuy-Get<CharacterControllerData>(ecs);

// After this, the character will try to move 0.2 meters in the positive z direction the next physics update.
characterController->CurrentDisplacement = float3(0.0f, 0.0f, 0.2f);
```

## PhysX License

The PhysX binaries used by this module fall under the following license:

Copyright (c) 2018 NVIDIA Corporation. All rights reserved. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    Neither the name of NVIDIA CORPORATION nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.