\page vertical_slice Vertical Slice Walkthrough
 
*In this example, we create a custom module that uses the ECS of modulith to render geometry in the scene and manipulate it using user input.*
*This should serve as an inspiration and head start to create gameplay code from. It is recommended to **first read the \ref quickstart "Quickstart Guide"** and refer to the Documentation on the \ref ecs_api "ECS API" as needed to understand the concepts at play.*


**There is a bug in some CLion versions that will prompt an error when compiling the module while it is loaded in the engine. If this happens during this tutorial, just compile it again and that time it should work.**


## 1 Setup

**All code snippets start with a using namespace modulith; to enable syntax highlighting. However, these should not be part of the code**

Same in the quickstart, follow the steps to create a module and open its project in the IDE. It should have the "Core" and "Renderer" modules as dependencies.

First, we want to create a *system* for the ECS. Instead of doing so manually, the Editor module partially automates this process: 
When our newly created module is "Active for Editing", we can navigate to the "Scripts" tab in the "Browser" window, select the "src" folder found under "Sources" and create a new System in it using the buttom at the top.

In this example we'll name this the "ExampleSystem" and override the ``OnInitialize()`` and ``OnUpdate()`` methods. Once you click "Create" a header and cpp file will be created.

Before we forget, we'll need to register the newly created system manually. This can be done in the *ModuleName*Module.cpp file's ``OnInitialize()`` method with the following line of code and including the header of the newly created module:

```cpp
using namespace modulith;

void OnInitialize(ModuleResources& module){
    module.Register<SystemResource<ExampleSystem>>();
}
```

We should also refresh CMake for IDE so that it knows about the newly created files.
After that open up the cpp file in the Editor and start editing the ``OnInitialize()`` method.

## 2 Creating the first entity

**In most cases, no additional files need to be included because the "*ModuleName*Module.h" file already contains most of Modulith's API headers.**
**But when needed, all necessary includes are written at the beginning at each code snippet for simplicity, but must be placed outside the methods and at the top of the file**

To create entities and interact with the ECS we'll need to get the ``ECSContext`` and its ``EntityManager``. To do so we need to and write the following line of code in our **system's** ``OnInitialize()`` method:

```cpp
using namespace modulith;

// Inside the System's OnInitialize():
shared<ECSContext> ecsContext = Context::GetInstance<ECSContext>();
ref<EntityManager> entityManager = ecsContext->GetEntityManager();
```

Now we can use the entity manager to create our first entity and attach data to it. Let's start simple and create an object with a name and position.

```cpp
using namespace modulith;

Entity myEntity = entityManager->CreateEntityWith(NameData("First Entity"), PositionData(float3(0,5,0)));
```

Now we can build our created module using the proper profile and reload the module in the editor. Inside the "Entity Info" window under "Hierarchy" we should now find an entity named "First Entity". 
Clicking on it will also reveal its components in the "Properties" window. Note that some additional components have been added automatically.

---

At this point, our initialize code should look like the following (*types were replaced with auto where possible for readability*):
```cpp
using namespace modulith;

// Inside the System's OnInitialize():
auto ecsContext = Context::GetInstance<ECSContext>();
auto entityManager = ecsContext->GetEntityManager();

auto myEntity = entityManager->CreateEntityWith(NameData("First Entity"), PositionData(float3(0,5,0)));
```

## 3 Creating a rendered entity

If we want an entity to be visible it needs to have a ``RenderMeshData`` component with a mesh and material, as well as a ``PositionData``.
Additionally a light and a camera need to be in the scene.

Here is how we can create all three entities in the ``OnInitialize()`` method:

```cpp
#include <RenderComponents.h>
#include "renderer/RenderContext.h"

using namespace modulith;
using namespace modulith::renderer;

// Inside the System's OnInitialize():
shared<Mesh> cubeMesh = Mesh::CreateCube();
Entity cube = entityManager->CreateEntityWith(NameData("Cube"), PositionData(), RenderMeshData(cubeMesh));

Entity sun = entityManager->CreateEntityWith(NameData("Sun"), PositionData(), DirectionalLightData(float3(1.0f, 0.7f, 0.9f), 0.8f));

int2 windowDimensions = Context::GetInstance<RenderContext>()->GetWindow()->GetSize();
Entity camera = entityManager->CreateEntityWith(NameData("Camera"), PositionData(0, 0, 10), CameraData(65.0f, windowDimensions, 0.1f, 1000.0f));
```

First we create a mesh and an entity to render that mesh on. We skip the Material for now, since a fallback material will be used by default.
Next, the sun entity is created that will be our light source. 
Lastly, the camera will be created with a slight offset position. Since the camera looks in the negative z direction, we move it into the positive z direction so it can see the cube.

Note that both the Cube and Sun were given a PositionData, even if no position is provided and these entities are placed at the origin. This is necessary since only entities with a ``GlobalTransformData`` component are considered a valid rendered object, camera or light. Adding a ``PositionData`` is an easy way to ensure the ``GlobalTransformData`` component will be added automatically.


If we build our module and reload it in the engine, we should be able to see a small pink cube.

However, to be able to modify how our cube looks we'll have to use a custom material. 
To make things simple, we'll use the ``StandardMaterial`` with the compatible "PhongShader" which implements simple Phong lighting. We should modify the lines that create our cube to look as follows:
```cpp
using namespace modulith;
using namespace modulith::renderer;

// Inside the System's OnInitialize():
shared<Mesh> cubeMesh = Mesh::CreateCube();
shared<Shader> phongShader = Context::GetInstance<RenderContext>()->Shaders()->Load(Address() / "shaders" / "PhongShader.glsl");
shared<StandardMaterial> standardMaterial = std::make_shared<StandardMaterial>(phongShader, float4(1, 1, 1, 1), 0, 0);
Entity cube = entityManager->CreateEntityWith(NameData("Cube"), PositionData(), RenderMeshData(cubeMesh, standardMaterial));
``` 

In the code above, we additionally load the material's shader and create a material with a white color (the float4) and no specular effects. This material is then given to the ``RenderMeshData``.

By re-compiling and re-loading the module the cube should now have turned white.

Lastly, let's give the camera and sun a different transform in worldspace so we can better see the cube in all 3 dimensions.  This code replaces the code that created the sun and camera entities.

```cpp
using namespace modulith;
using namespace modulith::renderer;

// Inside the System's OnInitialize():
RotationData rotation = RotationData();
rotation.SetLookAt(float3(1, 0.5f, 1)); // The light is pointed diagonally down
Entity sun = entityManager->CreateEntityWith(
    NameData("Sun"),
    rotation,
    DirectionalLightData(float3(1.0f, 0.7f, 0.9f), 0.0f)
);

int2 windowDimensions = ctx.GetWindow()->GetSize();
Entity camera = entityManager->CreateEntityWith(
    NameData("Camera"),
    PositionData(0, 5, 10),
    RotationData(-25, float3(1,0,0)),
    CameraData(65.0f, windowDimensions, 0.1f, 1000.0f)
);
```

---


At this point, our initialize code should look like the following (*types were replaced with auto where possible for readability*):
```cpp
#include <RenderComponents.h>
#include "renderer/RenderContext.h"

using namespace modulith;
using namespace modulith::renderer;

// Inside the System's OnInitialize():
auto ecsContext = Context::GetInstance<ECSContext>();
auto entityManager = ecsContext->GetEntityManager();

auto myEntity = entityManager->CreateEntityWith(NameData("First Entity"), PositionData(float3(0,5,0)));

// Create the cube
auto cubeMesh = Mesh::CreateCube();
shared<Shader> phongShader = Context::GetInstance<RenderContext>()->Shaders()->Load(Address() / "shaders" / "PhongShader.glsl");
auto standardMaterial = std::make_shared<StandardMaterial>(phongShader, float4(1, 1, 1, 1), 0, 0);
auto cube = entityManager->CreateEntityWith(NameData("Cube"), PositionData(), RenderMeshData(cubeMesh, standardMaterial));

// Create the Light Source
auto rotation = RotationData();
rotation.SetLookAt(float3(1, 0.5f, 1)); // The light is pointed diagonally down
auto sun = entityManager->CreateEntityWith(
    NameData("Sun"),
    rotation,
    DirectionalLightData(float3(1.0f, 0.7f, 0.9f), 0.0f)
);

// Create the Camera
auto windowDimensions = Context::GetInstance<RenderContext>()->GetWindow()->GetSize();
auto camera = entityManager->CreateEntityWith(
    NameData("Camera"),
    PositionData(0, 5, 10),
    RotationData(-25, float3(1,0,0)),
    CameraData(65.0f, windowDimensions, 0.1f, 1000.0f)
);
```

## 4. Using components and queries

If we want to modify our entities over time we could remember references when we create them and change the data of their components in ``OnUpdate(...)``. However, it is generally recommended to create components with data and have systems modify *any* entity with such a component.
This has both the benefits of increased performance and also allows us to re-use components for other use-cases.

To demonstrate this, we will make the cube rotate around its own axis automatically and then re-use it later on.

To begin, we want to create a *component* for the ECS. Instead of doing so manually, the Editor module also partially automates this process: 
We can navigate to the "Scripts" tab in the "Browser" window, select the "src" folder found under "Sources" and create a new Component in it using the buttom at the top.

Simply give the Component the name "AutoRotateData", tick the option "Create Serializer" and then click on "Create". 
Like with systems, this component must also be registered in the *ModuleName*Module.cpp file's ``Initialize()`` method using the following code:

```cpp
#include "AutoRotateData.h"
using namespace modulith;

void OnInitialize(ModuleResources& module){
    // previous code that registered the System
    module.Register<ComponentResource<AutoRotateData>>("Auto Rotate Data");
}
```

Once again, we should also refresh CMake for IDE so that it knows about the newly created file.
Afterwards we can navigate to the created file and add data to our created component. To make an entity rotate around its own axis, we'll need the axis, the rotation in degrees per second and the current rotation. All of these are implemented as simple fields of the struct:

```cpp
struct VERTICALSLICE_API AutoRotateData {
    AutoRotateData() = default;

    AutoRotateData(const modulith::float3& rotationAxis, float degreesPerSecond)
        : RotationAxis(rotationAxis), DegreesPerSecond(degreesPerSecond) {}

    modulith::float3 RotationAxis = modulith::float3(0,1,0);
    float DegreesPerSecond = 0.0f;
    float CurrentDegrees = 0.0f;
};

// This was autogenerated an we'll ignore it for now
namespace modulith{
    template<>
    struct Serializer<AutoRotateData> {

        // TODO for User: Don't forget to manually register this serializer in the VerticalSliceModule.cpp file once implemented

        [[nodiscard]] static AutoRotateData Construct() {
            // TODO for User: This should construct your component with default values
            return AutoRotateData();
        }

        [[nodiscard]] static SerializedObject Serialize(const AutoRotateData& component) {
            // TODO for User: This should create a SerializedObject that represents all the data in your component
            return SerializedObject();
        }

        [[nodiscard]] static std::optional<AutoRotateData> TryDeserialize(const SerializedObject& so) {
            // TODO for User: This should parse the serialized object and construct your component from the values, if possible
            return std::nullopt;
        }
    };
}
```

We will ignore the autogenerated Serializer for now and come back to it later. It won't cause us any trouble as long as our component has a trivial constructor, e.g. ``AutoRotateData() = default;``

Next, we'll attach the component to the Cube, alongside a ``RotationData``, which we'll need later. We could modify the ``CreateEntityWith`` function where it is created, but to show some other methods we'll add the component after the creation in the ``OnInitialize()`` method this time. Note that is generally recommended to make as many modifications to an entity at once and not split them up like in this example.

```cpp
#include "AutoRotateData.h"
using namespace modulith;

// Inside the System's OnInitialize():
cube.Add(entityManager, AutoRotateData(float3(1,1,1), 36));
cube.Add(entityManager, RotationData());
```

Now there is only one thing left: Add behaviour to this new component - without that nothing will happen because components are mere data objects.
We want to update the rotation of the cube every frame, based on the data of the AutoRotateData. If we had a reference to both of these and delta time, the code would look as follows:

```cpp
using namespace modulith;

// Theoretical code we would write in the System's OnUpdate(float deltaTime):
RotateData& rotation = // ...
AutoRotateData& autoRotate = // ...

rotation.SetRotation(autoRotate.CurrentDegrees, autoRotate.RotationAxis);
autoRotate.CurrentDegrees += autoRotate.DegreesPerSecond * deltaTime;
```

While the code above does not build and is not finished, the thought experiment will help us with the next step: In order to execute the code every frame, it needs to be placed inside ``OnUpdate(...)`` of our system. And in order to get the rotation and auto rotation data, we'll need a Query:

```cpp
using namespace modulith;

// Inside the System's OnUpdate(float deltaTime):
Context::GetInstance<ECSContext>()->GetEntityManager()->QueryActive(
    Each<RotationData, AutoRotateData>(),
    [deltaTime](Entity entity, RotationData& rotation, AutoRotateData& autoRotate){
        rotation.SetRotation(autoRotate.CurrentDegrees, autoRotate.RotationAxis);
        autoRotate.CurrentDegrees += autoRotate.DegreesPerSecond * deltaTime;
    }
);
```

We use the ``Each<RotationData, AutoRotateData>()`` to specify that we want to following code to be executed on every entity with our component and RotationData.
Next follows a lambda that is executed for the data of every suitable entity and gets references to both components. 
For it, we need to capture the deltaTime variable given to us by ``OnUpdate`` to be able to use it inside the lambda.

If we re-build and re-load the module, we should see that our cube now rotates around its own axis.

---

At this point, the code of our *SystemName*.cpp should look like the following (*types were replaced with auto where possible for readability*, *our system is named ExampleSystem*):

```cpp
#pragma once

#include "ExampleSystem.h"
#include "RenderComponents.h"
#include "AutoRotateData.h"
#include "renderer/RenderContext.h"

using namespace modulith;
using namespace modulith::renderer;

void ExampleSystem::OnInitialize() {
    auto ecsContext = Context::GetInstance<ECSContext>();
    auto entityManager = ecsContext->GetEntityManager();
    
    auto myEntity = entityManager->CreateEntityWith(NameData("First Entity"), PositionData(float3(0,5,0)));
    
    // Create the cube
    auto cubeMesh = Mesh::CreateCube();
shared<Shader> phongShader = Context::GetInstance<RenderContext>()->Shaders()->Load(Address() / "shaders" / "PhongShader.glsl");
    auto standardMaterial = std::make_shared<StandardMaterial>(phongShader, float4(1, 1, 1, 1), 0, 0);
    auto cube = entityManager->CreateEntityWith(NameData("Cube"), PositionData(), RenderMeshData(cubeMesh, standardMaterial));
    
    // Create the Light Source
    auto rotation = RotationData();
    rotation.SetLookAt(float3(1, 0.5f, 1)); // The light is pointed diagonally down
    auto sun = entityManager->CreateEntityWith(
        NameData("Sun"),
        rotation,
        DirectionalLightData(float3(1.0f, 0.7f, 0.9f), 0.0f)
    );
    
    // Create the Camera
    auto windowDimensions = Context::GetInstance<RenderContext>()->GetWindow()->GetSize();
    auto camera = entityManager->CreateEntityWith(
        NameData("Camera"),
        PositionData(0, 5, 10),
        RotationData(-25, float3(1,0,0)),
        CameraData(65.0f, windowDimensions, 0.1f, 1000.0f)
    );

    cube.Add(entityManager, AutoRotateData(float3(1,1,1), 36));
    cube.Add(entityManager, RotationData());
}

void ExampleSystem::OnUpdate(float deltaTime) {
    Context::GetInstance<ECSContext>()->GetEntityManager()->QueryActive(
        Each<RotationData, AutoRotateData>(),
        [deltaTime](Entity entity, RotationData& rotation, AutoRotateData& autoRotate){
            rotation.SetRotation(autoRotate.CurrentDegrees, autoRotate.RotationAxis);
            autoRotate.CurrentDegrees += autoRotate.DegreesPerSecond * deltaTime;
        }
    );
}
```

## 5. Adding serialization to the Component

As you may have noticed, when selecting an entity in the editor's Entity Info window you may look at and edit the data of many of its components. However, this will not yet be possible for out ``AutoRotateData`` attached to the cube: It doesn't have a serializer yet.

To change that, let's go back to the AutoRotateData.h file in the IDE and modify the autogenerated serializer. It has 3 functions we need to implement:
- First, a function that constructs the component with reasonable default values
- Next, a function that creates a SerializedObject from our component
- And last, a function that attempts to create a new component from our SerializedObject.

A ``modulith::SerializedObject`` is a tuple of a name and a value.
 It is used internally by the engine to represent data and its member values.
  This is possible because the value of a ``modulith::SerializedObject`` may be a vector of other ``modulith::SerializedObject`` - it allows for recursive definitions of a component and the values of its fields.

Here is a sample implementation of the Serializer:

```cpp
namespace modulith{
    template<>
    struct Serializer<AutoRotateData> {

        [[nodiscard]] static AutoRotateData Construct() {
            return AutoRotateData();
        }

        [[nodiscard]] static SerializedObject Serialize(const AutoRotateData& component) {
            return SerializedObject("Root", SerializedSubObjects(std::initializer_list<SerializedObject>{
                SerializedObject("Rotation Axis", component.RotationAxis),
                SerializedObject("Degrees Per Second", component.DegreesPerSecond),
                SerializedObject("Current Degrees", component.CurrentDegrees)
            }));
        }

        [[nodiscard]] static std::optional<AutoRotateData> TryDeserialize(const SerializedObject& so) {
            std::optional<modulith::float3> rotationAxis = so.TryGet<modulith::float3>("Rotation Axis");
            std::optional<float> degreesPerSecond = so.TryGet<float>("Degrees Per Second");
            std::optional<float> currentDegrees = so.TryGet<float>("Current Degrees");

            if(rotationAxis.has_value() && degreesPerSecond.has_value() && currentDegrees.has_value()){
                AutoRotateData result = AutoRotateData(rotationAxis.value(), degreesPerSecond.value());
                result.CurrentDegrees = currentDegrees.value();
                return result;
            }
            return std::nullopt;
        }
    };
}
```

Since the AutoRotateData already has a trivial constructor with reasonable default values, we can simply call that in the ``Construct()`` function.

Next, in the ``Serialize()`` function, we create a ``modulith::SerializedObject`` that has multiple ``modulith::SerializedObject``s as its values, each of them for one of the fields of our component.
The names we choose here needn't be the same as the names of the fields - instead these names are displayed in the editor.

Lastly, in the ``TryDeserialize()`` we try to get the serialized subobjects with the names we used in the previous method. We also try to immediately cast the value of that subobject to the needed type. If both succeed the returned ``std::optional`` will have a value, otherwise ``std::nullopt`` is returned.
That is why we check if all our variables have a value before we construct the component.

Before we move on, the code above can be expressed in a shorter manner by using ``auto``, omitting many of the constructor's names and also ommiting the ``.has_value()`` since optionals implicitly cast to boolean.
Here is ow that code would look:

```cpp
namespace modulith {
    template<>
    struct Serializer<AutoRotateData> {

        [[nodiscard]] static AutoRotateData Construct() {
            return AutoRotateData();
        }

        [[nodiscard]] static SerializedObject Serialize(const AutoRotateData& component) {
            return SerializedObject("Root", {
                    {"Rotation Axis",      component.RotationAxis},
                    {"Degrees Per Second", component.DegreesPerSecond},
                    {"Current Degrees",    component.CurrentDegrees}
            });
        }

        [[nodiscard]] static std::optional<AutoRotateData> TryDeserialize(const SerializedObject& so) {
            auto rotationAxis = so.TryGet<modulith::float3>("Rotation Axis");
            auto degreesPerSecond = so.TryGet<float>("Degrees Per Second");
            auto currentDegrees = so.TryGet<float>("Current Degrees");

            if (rotationAxis && degreesPerSecond && currentDegrees) {
                AutoRotateData result = AutoRotateData(rotationAxis.value(), degreesPerSecond.value());
                result.CurrentDegrees = currentDegrees.value();
                return result;
            }
            return std::nullopt;
        }
    };
}
```

As with any new object, whether it is a ``modulith::System``, a component or a ``modulith::Serializer`` we'll need to register it.
So we need to add the following line to our *ModuleName*Module.cpp's ``OnInitialize()`` function:

```cpp
    module.Register<SerializerResource<AutoRotateData>>();
```

This simply works for as long as the header with the Serializer is included in our cpp file, which in this case, it already is.

Now we can re-compile the module and re-load it in the Editor. When selecting the ``Cube`` entity in the ``Entity Info`` window (in Hierarchy View) we will see the Components in the Properties Tab.
When we click on the ``AutoRotateData``, we should be able to see and edit any of its properties.