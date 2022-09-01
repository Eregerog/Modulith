/*
 * \brief
 * \author Daniel Götz
 */

#include "RendererModule.h"
#include "serialization/SerializerResource.h"
#include <RenderSystem.h>
#include "RenderSystemsGroup.h"

using namespace modulith;
using namespace modulith::renderer;


extern "C++" {

void __declspec(dllexport) __cdecl Initialize(modulith::ModuleResources& module) {

    module.Register<SystemsGroupResource<RenderSystemsGroup, ExecuteAfter<LogicSystemsGroup>>>();

    module.Register<SystemResource<RenderSystem, InGroup<RenderSystemsGroup>>>();

    module.Register<ComponentResource<RenderMeshData>>("RenderMesh");
    module.Register<ComponentResource<CameraData>>("Camera");
    module.Register<SerializerResource<CameraData>>();

    module.Register<ComponentResource<PointLightData>>("PointLight");
    module.Register<SerializerResource<PointLightData>>();
    module.Register<ComponentResource<DirectionalLightData>>("DirectionalLight");
    module.Register<SerializerResource<DirectionalLightData>>();
}

void __declspec(dllexport) __cdecl Shutdown(modulith::ModuleResources& module){
}


}
