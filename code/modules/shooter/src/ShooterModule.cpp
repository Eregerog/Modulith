/*
 * \brief
 * \author Daniel Götz
 */

#include "ShooterModule.h"

#include "ecs/guns/GunComponents.h"
#include "ecs/guns/GunSystem.h"
#include "ecs/ThirdPersonController.h"
#include "ecs/CameraControllerSwitch.h"

// TODO: Automate most of this, expose a function to the user

using namespace modulith;

extern "C++" {

void __declspec(dllexport) __cdecl Initialize(modulith::ModuleResources& module) {

    module.Register<SystemResource<ThirdPersonController>>();
    module.Register<SystemResource<CameraControllerSwitch>>();
    module.Register<SystemResource<GunSystem>>();

    module.Register<ComponentResource<AmmunitionData>>("Ammunition");
    module.Register<ComponentResource<GunData>>("Gun");
    module.Register<ComponentResource<ManualShootingTag>>("ManualShooting");
    module.Register<ComponentResource<IsReloadingTag>>("IsReloading");
    module.Register<ComponentResource<GunAimData>>("GunAim");
    module.Register<ComponentResource<IsAimingTag>>("IsAiming");
    module.Register<ComponentResource<IsShootingTag>>("IsShooting");
    module.Register<ComponentResource<VisualizedAmmunitionData>>("VisualizedAmmunition");
}


void __declspec(dllexport) __cdecl Shutdown(modulith::ModuleResources& module){
}


}
