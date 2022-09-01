/*
 * \brief
 * \author Daniel Götz
 */

#include "EditorModule.h"
#include <modules/resources/ModuleResources.h>
#include <ecs/SelectionSystem.h>
#include <ecs/PropertiesWindowSystem.h>
#include "ecs/ProfilerSystem.h"
#include "ecs/browser/ModuleBrowserSystem.h"
#include "ecs/modules/EditedModuleSystem.h"
#include "ecs/browser/BrowserSystem.h"
#include "ecs/browser/FileBrowserSystem.h"
#include "ecs/resources/SystemResource.h"
#include "resources/SubcontextResource.h"
#include "EntityDebugContext.h"

using namespace modulith;
using namespace modulith::editor;

extern "C++" {

void __declspec(dllexport) __cdecl Initialize(modulith::ModuleResources& module) {
    module.Register<SubcontextResource<EntityDebugContext>>();

    module.Register<SystemResource<SelectionSystem>>();
    module.Register<SystemResource<PropertiesWindowSystem>>();
    module.Register<SystemResource<ModuleBrowserSystem>>();
    module.Register<SystemResource<EditedModuleSystem>>();
    module.Register<SystemResource<BrowserSystem>>();
    module.Register<SystemResource<FileBrowserSystem>>();
    module.Register<SystemResource<ProfilerSystem>>();
}

void __declspec(dllexport) __cdecl Shutdown(modulith::ModuleResources& module){
}

}
