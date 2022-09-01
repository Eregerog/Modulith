\page renderer_api Renderer Module API Documentation

*This docs file gives a short overview over the Renderer Module*

The renderer module provides ECS components that can be used by other modules to easily render its entities via the standard renderer.
The modulith::renderer::RenderSystem executed late in the game loop will then render these entities.

*All renderer components require a modulith::GlobalTransformData attached to them in order to be used during rendering. While this can be done manually, it is also automated when there is any other TransformComponent (e.g.) modulith::PositionData, modulith::RotationData, etc. present*

## Defining a camera

In order to render something there needs to be one entity with a modulith::renderer::CameraData and modulith::GlobalTransformData.

Example:

```cpp
using namespace modulith;
using namespace modulith::renderer;

Context& ctx = ...;
ref<EntityManager> ecs = ...;

Entity camera = ecs.CreateEntityWith(
    CameraData(
        /* field of view: */ 60.0f,
        /* dimensions: */ ctx.GetWindow()->GetSize(),
        /* near plane: */ 0.1f,
        /* far plane: */ 1000.0f
    ),
    PositionData(0, 0, 10.0f)
);

```

## Creating light

You will also need some light in your scene. This is created by an entity with a modulith::renderer::PointLightData or modulith::renderer::DirectionalLightData alongside a modulith::GlobalTransformData.

Example:

```cpp
using namespace modulith;
using namespace modulith::renderer;

Context& ctx = ...;
ref<EntityManager> ecs = ...;

Entity sun = ecs.CreateEntityWith(
    DirectionalLightData(
        /* color: */ float3(1.0f, 1.0f, 0.7f),
        /* ambient light factor: */ 0.2f
    ),
    PositionData(0, 0, 10.0f)
);

Entity light = ecs.CreateEntityWith(
    PointLightData(
        /* color: */ float3(1.0f, 0.0f, 0.0f),
        /* range: */ 15.0f    
    ),
    PositionData(0, 10.0f, 0)
);
```

## Rendering a mesh

Lastly, you can then render your mesh by attaching a modulith::renderer::RenderMeshData and a modulith::GlobalTransformData

Example:

```cpp
using namespace modulith;
using namespace modulith::renderer;

Context& ctx = ...;
ref<EntityManager> ecs = ...;
shared<Mesh> meshToRender = ...;
shared<Material> materialForRendering = ...; /* optional, if not provided a fallback material is used */

Entity renderedEntity = ecs.CreateEntityWith(
    RenderMeshData(meshToRender, materialForRendering),
    PositionData(0, 0.5f, 0)       
);
```