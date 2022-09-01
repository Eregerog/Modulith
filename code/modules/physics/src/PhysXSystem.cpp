/*
 * \brief
 * \author Daniel Götz
 */

#include "PhysicsModule.h"
#include <PhysicsComponents.h>
#include "PhysXSystem.h"
#include "InternalPhysXComponents.h"
#include "PhysXUtils.h"

using namespace physx;
using namespace modulith;

namespace modulith::physics{

    PxFilterFlags CollisionFilterShader(
        PxFilterObjectAttributes attributes0, PxFilterData filterData0,
        PxFilterObjectAttributes attributes1, PxFilterData filterData1,
        PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize
    ) {
        // let triggers through
        if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) {
            pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
            return PxFilterFlag::eDEFAULT;
        }
        // generate contacts for all that were not filtered above
        pairFlags = PxPairFlag::eCONTACT_DEFAULT;

        pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
        pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
        pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
        pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;

        return PxFilterFlag::eCALLBACK;
    }

    void PhysXSystem::OnInitialize() {

        auto physics = GetPhysXPhysics();
        Assert(physics,
            "Could not retrieve the PhysX Physics object. Make sure that the module has been initialized by the same process the systems are executed upon!");

        PxSceneDesc sceneDesc(physics->getTolerancesScale());
        sceneDesc.
            gravity = PxVec3(0.0f, -9.81f, 0.0f);
        auto dispatcher = PxDefaultCpuDispatcherCreate(2);
        sceneDesc.
            cpuDispatcher = dispatcher;
        sceneDesc.filterCallback = &_filterCallback;
        sceneDesc.filterShader = &CollisionFilterShader;
        sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
        _scene = physics->createScene(sceneDesc);

        _controllerManager = PxCreateControllerManager(*_scene);

        _defaultMaterial = physics->createMaterial(0.1f, 0.1f, 0.6f);
    }


    void PhysXSystem::OnUpdate(float deltaTime) {
        auto& profiler = Context::Instance().GetProfiler();
        auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

        profiler.BeginMeasurement("PhysX System: Update Components");
        updateComponents(ecs, profiler);
        profiler.EndMeasurement();

        profiler.BeginMeasurement("PhysX System: Update Physics World");
        stepSimulation(ecs, deltaTime);
        profiler.EndMeasurement();

        profiler.BeginMeasurement("PhysX System: Update Contacts");
        updateContacts(ecs);
        profiler.EndMeasurement();

        profiler.BeginMeasurement("PhysX System: Update Transformations");
        updateTransforms(ecs);
        profiler.EndMeasurement();
    }

    void PhysXSystem::OnShutdown() {
        _controllerManager->release();
        _controllerManager = nullptr;
        _scene->release();
        _scene = nullptr;
    }


    void PhysXSystem::updateComponents(const ref<EntityManager>& ecs, Profiler& profiler) {

        auto* physics = GetPhysXPhysics();


        // Init Rigidbodies
        {
            ecs->QueryAll(
                Each<GlobalTransformData, RigidbodyData>(),
                Any(),
                None<PxRigidDynamicData, PxRigidStaticData>(),
                Has<DisableGravityTag, EnableContinuousCollisionDetectionTag>(),
                [ecs, &physics, this](
                    auto entity, auto& transform, auto& rigidbody, bool disableGravity, bool enableCCD
                ) {
                    PxRigidDynamic* body = physics->createRigidDynamic(ToPxTransform(transform));
                    Assert(body, "The creation of the dynamic RigidBody for entity {0} failed", entity)

                    auto mass = rigidbody.Mass;

                    body->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, enableCCD);
                    body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, disableGravity);

                    ecs->Defer(
                        [entity, this, body, mass](auto ecs) {
                            auto actorWrapper = PxActorWrapper(&_scene, body, entity);
                            body->setMass(mass);
                            ecs->AddComponent(entity, PxRigidDynamicData(std::move(actorWrapper)));
                        }
                    );
                }
            );

            ecs->QueryAll(
                Each<GlobalTransformData, BoxColliderData>(),
                None<RigidbodyData, PxRigidDynamicData, PxRigidStaticData>(),
                [ecs, &physics, this](
                    auto entity, auto& transform, auto& collider
                ) {
                    PxRigidStatic* body = physics->createRigidStatic(ToPxTransform(transform));
                    body->userData = new Entity(entity);
                    Assert(body, "The creation of the static RigidBody for entity {0} failed", entity)

                    ecs->Defer(
                        [entity, this, body](auto ecs) {
                            auto actorWrapper = PxActorWrapper(&_scene, body, entity);
                            ecs->AddComponent(entity, PxRigidStaticData(std::move(actorWrapper)));
                        }
                    );
                }
            );
        }

        // Init Colliders
        {
            ecs->QueryAll(
                Each<GlobalTransformData, BoxColliderData>(),
                Any<PxRigidDynamicData, PxRigidStaticData, PxControllerData>(),
                None<PxShapeData>(),
                [ecs, &material = _defaultMaterial, &physics](
                    auto entity, auto& transform, auto& collider, auto* pxDynamic, auto* pxStatic, auto* pxController
                ) {
                    auto size = collider.Size;
                    auto scale = transform.Scale();
                    size.x *= scale.x * 0.5f;
                    size.y *= scale.y * 0.5f;
                    size.z *= scale.z * 0.5f;

                    PxShape* shape = physics->createShape(PxBoxGeometry(size.x, size.y, size.z), *material);

                    shape->setLocalPose(PxTransform(PxVec3(collider.Offset.x,collider.Offset.y,collider.Offset.z)));

                    PxRigidActor* actor = ActorFromEither(pxDynamic, pxStatic, pxController);
                    Assert(actor != nullptr, "Actor is null, that's bad!")
                    actor->attachShape(*shape);

                    ecs->Defer([entity, shape](auto manager){ manager->template AddComponent<PxShapeData>(entity, shape); });

                }
            );
        }

        //Update rigidbodies
        {
            ecs->QueryAll(
                Each<RigidbodyData, PxRigidDynamicData>(),
                [](auto entity, auto& rigidbody, PxRigidDynamicData& physXRigidbody) {
                   // physXRigidbody.GetRigidBody()->clearForce();
                    if (rigidbody.AppliedForce.x != 0 || rigidbody.AppliedForce.y != 0 || rigidbody.AppliedForce.z != 0)
                        physXRigidbody.GetRigidBody()->addForce(
                            PxVec3(
                                rigidbody.AppliedForce.x,
                                rigidbody.AppliedForce.y,
                                rigidbody.AppliedForce.z
                            ));
                    rigidbody.AppliedForce.x = 0;
                    rigidbody.AppliedForce.y = 0;
                    rigidbody.AppliedForce.z = 0;
                }
            );


            ecs->QueryAll(
                Each<PxRigidDynamicData>(), Any(), None(),
                Has<DisableGravityTag, EnableContinuousCollisionDetectionTag>(),
                [](auto entity, PxRigidDynamicData& rigidbody, bool disableGravity, bool enableCCD) {
                    rigidbody.GetRigidBody()->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, enableCCD);
                    rigidbody.GetRigidBody()->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, disableGravity);
                }
            );

            ecs->QueryAll(
                Each<CharacterControllerData, PositionData>(), None<PxControllerData, WithParentData>(),
                [ecs, this](auto entity, auto& controller, auto& position) {
                    PxCapsuleControllerDesc desc;
                    desc.radius = controller.Radius;
                    desc.height = controller.Height;
                    desc.upDirection = PxVec3(0, 1, 0);
                    desc.position = PxExtendedVec3(position.Value.x, position.Value.y + (controller.Height / 2.0f + controller.Radius), position.Value.z);
                    desc.material = _defaultMaterial;

                    Assert(desc.isValid(),
                        "The description for the to be created character controller on entity {} is invalid",
                        entity);

                    auto pxController = _controllerManager->createController(desc);
                    Assert(pxController, "The creation of the character controller for entity {0} failed", entity)

                    ecs->Defer([entity, pxController, this](auto ecs){ ecs->template AddComponent<PxControllerData>(entity, &_controllerManager, pxController, entity); });
                }
            );
        }


    }

    void PhysXSystem::stepSimulation(const ref<EntityManager>& ecs, float deltaTime) {

        deltaTime = std::min(deltaTime, 1.0f);

        ecs->QueryActive(
            Each<CharacterControllerData, PxControllerData>(),
            Any(), None(), Has<DisableGravityTag>(),
            [deltaTime, gravity = _scene->getGravity()](
                auto entity, auto& characterController, PxControllerData& pxController, bool disableGravity
            ) {
                auto velocity = characterController.Velocity;
                auto displacement =
                    characterController.CurrentDisplacement + velocity * deltaTime;

                auto collisionFlags = pxController.Controller->move(
                    PxVec3(displacement.x, displacement.y, displacement.z),
                    0.001f,
                    deltaTime,
                    PxControllerFilters()
                );

                velocity += float3(gravity.x, gravity.y, gravity.z) * (disableGravity ? 0.0f : 1.0f) * deltaTime;

                if (collisionFlags.isSet(PxControllerCollisionFlag::eCOLLISION_DOWN) && velocity.y < 0)
                    velocity.y = 0;

                if (collisionFlags.isSet(PxControllerCollisionFlag::eCOLLISION_UP) && velocity.y > 0)
                    velocity.y = 0;

                characterController.Velocity = velocity;
                characterController.CurrentDisplacement = float3(0.0f);

                if (!characterController.TouchedGround && collisionFlags.isSet(PxControllerCollisionFlag::eCOLLISION_DOWN))
                    characterController.TouchedGround = true;
            }
        );

        _accumulator +=
            deltaTime;

        while (_accumulator >= _stepSize) {
            _accumulator -= _stepSize;

            _scene->
                simulate(_stepSize);
            PxU32 err;
            _scene->fetchResults(true, &err);
            Assert(!err, "There has been an error while stepping the physics simulation. Error code {}", err)
        }
    }

    void PhysXSystem::updateContacts(const ref<EntityManager>& ecs) {

        ecs->QueryActive(
            Each<PhysicsContactsData>(), Any<PxRigidDynamicData, PxRigidStaticData, PxControllerData>(), None(),
            [&filterCallback = _filterCallback, ecs](
                auto entity, PhysicsContactsData& contacts, auto* dynamicBody, auto* staticBody, auto* pxController
            ) {
                auto actor = ActorFromEither(dynamicBody, staticBody, pxController);
                auto allContacts = filterCallback.ContactsOf(ecs, static_cast<PxActor*>(actor));
                contacts.Count = allContacts.size();
                contacts.BeginContact = allContacts;
            }
        );

        _filterCallback.ClearForNextFrame();
    }


    void PhysXSystem::updateTransforms(const ref<EntityManager>& ecs) {

        ecs->QueryActive(
            Each<PxRigidDynamicData, GlobalTransformData, LocalTransformData>(),
            Any<PositionData, RotationData>(), None(),
            [](
                auto entity, PxRigidDynamicData& pxRigidBody, auto& globalTransform, auto& localTransform,
                auto* position, auto* rotation
            ) {

                auto currTransform = pxRigidBody.GetRigidBody()->getGlobalPose();

                auto currGPosition = float3(currTransform.p.x, currTransform.p.y, currTransform.p.z);
                // order of ctor is different
                auto currGRotation = quat(currTransform.q.w, currTransform.q.x, currTransform.q.y, currTransform.q.z);

                auto newWorldSpaceTransform =
                    glm::translate(glm::identity<float4x4>(), currGPosition) * glm::mat4_cast(currGRotation);
                auto localToGlobal = globalTransform.Value * glm::inverse(localTransform.Value);
                auto globalToLocal = glm::inverse(localToGlobal);
                auto newLocalSpaceTransform = globalToLocal * newWorldSpaceTransform;

                if (position != nullptr)
                    position->Value = newLocalSpaceTransform[3];
                if (rotation != nullptr)
                    rotation->Value = glm::quat_cast(newLocalSpaceTransform);
            }
        );

        ecs->QueryActive(
            Each<PxControllerData, PositionData>(), None<WithParentData>(),
            [](
                auto entity, PxControllerData& pxController, auto& position
            ) {
                auto currPxPosition = pxController.Controller->getFootPosition();
                position.Value = float3(currPxPosition.x, currPxPosition.y, currPxPosition.z);
            }
        );
    }

    void PhysXSystem::OnImGui(float deltaTime) {
        ImGui::Begin("Physics Debugger");

        if (ImGui::CollapsingHeader("Global Physics Stats")) {
            auto physics = GetPhysXPhysics();
            ImGui::Text("Scenes: %i", physics->getNbScenes());
            ImGui::Text("Materials: %i", physics->getNbMaterials());
            ImGui::Text("Shapes: %i", physics->getNbShapes());
        }

        if (ImGui::CollapsingHeader("Scene Physics Stats")) {
            auto nbActors = _scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC);
            ImGui::Text("Dynamic RigidBodies: %i", nbActors);
            nbActors = _scene->getNbActors(PxActorTypeFlag::eRIGID_STATIC);
            ImGui::Text("Static RigidBodies: %i", nbActors);
        }

        if (ImGui::CollapsingHeader("Character Controller Stats")) {
            auto nbControllers = _controllerManager->getNbControllers();
            ImGui::Text("Character Controllers: %i", nbControllers);
        }

        ImGui::End();
    }
}
