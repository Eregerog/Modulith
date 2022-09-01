/*
 * \brief
 * \author Daniel Götz
 */

#include "PhysicsModule.h"
#include <PhysicsComponents.h>
#include "PxPhysicsAPI.h"
#include "PhysicsModuleCore.h"
#include "PhysXSystem.h"
#include "InternalPhysXComponents.h"
#include "PhysicsSystemsGroup.h"
#include "serialization/SerializerResource.h"

#ifndef PVD_HOST
#define PVD_HOST "127.0.0.1"
#endif

physx::PxFoundation* PhysXFoundation = nullptr;
physx::PxPhysics* PhysXPhysics = nullptr;
physx::PxPvd* PhysXVisualDebugger = nullptr;

namespace modulith::physics{
    physx::PxFoundation* GetPhysXFoundation() { return PhysXFoundation; }
    physx::PxPhysics* GetPhysXPhysics() {   return PhysXPhysics; }
}

class PhysicsErrorCallback : public physx::PxErrorCallback{

public:
    void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override {
        LogError("PhysX Error {}: {} ({}, line {})", code, message, file, line);
    }
};

extern "C++" {

using namespace physx;
using namespace modulith;
using namespace modulith::physics;

void __declspec(dllexport) __cdecl Initialize(modulith::ModuleResources& module) {
    module.Register<SystemsGroupResource<PhysicsSystemsGroup, ExecuteAfter<InputSystemsGroup>, ExecuteBefore<TransformSystemsGroup>>>();

    module.Register<SystemResource<PhysXSystem, InGroup<PhysicsSystemsGroup>>>();

    module.Register<ComponentResource<RigidbodyData>>("Rigidbody");
    module.Register<SerializerResource<RigidbodyData>>();
    module.Register<ComponentResource<DisableGravityTag>>("DisableGravity");
    module.Register<SerializerResource<DisableGravityTag, TrivialSerializer<DisableGravityTag>>>();
    module.Register<ComponentResource<EnableContinuousCollisionDetectionTag>>("EnableContinuousCollisionDetection");
    module.Register<SerializerResource<EnableContinuousCollisionDetectionTag, TrivialSerializer<EnableContinuousCollisionDetectionTag>>>();

    module.Register<ComponentResource<BoxColliderData>>("BoxCollider");
    module.Register<SerializerResource<BoxColliderData>>();
    module.Register<ComponentResource<PhysicsContactsData>>("PhysicsContracts");
    module.Register<ComponentResource<CharacterControllerData>>("CharacterController");
    module.Register<SerializerResource<CharacterControllerData>>();

    // Internal Components
    module.Register<ComponentResource<PxRigidDynamicData>>("InternalPxRigidDynamic");
    module.Register<ComponentResource<PxRigidStaticData>>("InternalPxRigidStatic");
    module.Register<ComponentResource<PxControllerData>>("InternalPxController");
    module.Register<ComponentResource<PxShapeData>>("InternalPxShape");

    static PhysicsErrorCallback errorCallback;
    static PxDefaultAllocator gDefaultAllocatorCallback;

    PhysXFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback,
        errorCallback
    );

    Assert(PhysXFoundation, "PhysX Foundation object could not be created!");

    bool recordMemoryAllocations = true;

    // TODO disable PVD in release builds
    PhysXVisualDebugger = PxCreatePvd(*PhysXFoundation);
    auto* pvdTransport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    auto pvdSuccess = PhysXVisualDebugger->connect(*pvdTransport, PxPvdInstrumentationFlag::eALL);

    if(pvdSuccess){
        LogInfo("Successfully connected to PhysXVisualDebugger");
    }
    else{
        LogWarn("Could not connect to the PhysXVisualDebugger");
    }

    PhysXPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *PhysXFoundation, PxTolerancesScale(), recordMemoryAllocations, PhysXVisualDebugger);

    Assert(PhysXPhysics, "PhysX Physics object could not be created!");
}

void __declspec(dllexport) __cdecl Shutdown(modulith::ModuleResources& module) {
    PhysXPhysics->release();
    PhysXVisualDebugger->release();
    PhysXFoundation->release();
}

}
