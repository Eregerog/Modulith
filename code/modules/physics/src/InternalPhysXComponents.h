/*
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "PhysicsModule.h"
#include "PxPhysicsAPI.h"
#include "PxScene.h"
#include "../../../extern/PhysX/physx/include/PxActor.h"

using namespace physx;

namespace modulith::physics{

    template<class TActor>
    struct PxActorWrapper {

        using Self = PxActorWrapper<TActor>;

        PxActorWrapper() = default;

        PxActorWrapper(PxScene** containedIn, TActor* actor, modulith::Entity attachedTo) : _containedIn(containedIn), _actor(actor) {
            _actor->userData = new Entity(attachedTo);
            (*_containedIn)->addActor(*actor);
        }

        PxActorWrapper(const Self&) = delete;

        PxActorWrapper(Self&& orig) noexcept : _containedIn(std::exchange(orig._containedIn, nullptr)), _actor(std::exchange(orig._actor, nullptr))  {}

        ~PxActorWrapper() {
            if(_containedIn != nullptr && *_containedIn != nullptr && _actor != nullptr){
                delete reinterpret_cast<modulith::Entity*>(_actor->userData);
                (*_containedIn)->removeActor(*static_cast<PxActor*>(_actor));
                _actor->release();
            }
        }

        Self& operator=(const Self& rhs) = delete;

        PxActorWrapper<TActor>& operator=(Self&& rhs) noexcept {
            _containedIn = std::exchange(rhs._containedIn, nullptr);
            _actor = std::exchange(rhs._actor, nullptr);
            return *this;
        }

        [[nodiscard]] TActor* GetActor() const { return _actor;}

    private:
        PxScene** _containedIn = nullptr;
        TActor* _actor = nullptr;
    };

    struct PxRigidDynamicData {

        PxRigidDynamicData() = default;

        explicit PxRigidDynamicData(PxActorWrapper<PxRigidDynamic>&& wrapper) : ActorWrapper(std::move(wrapper)) {}

        [[nodiscard]] PxRigidDynamic* GetRigidBody() const {return ActorWrapper.GetActor();}

        PxActorWrapper<PxRigidDynamic> ActorWrapper{};

    };

    struct PxRigidStaticData {

        PxRigidStaticData() = default;

        explicit PxRigidStaticData(PxActorWrapper<PxRigidStatic>&& wrapper) : ActorWrapper(std::move(wrapper)) {}

        [[nodiscard]] PxRigidStatic* GetRigidBody() const {return ActorWrapper.GetActor();}

        PxActorWrapper<PxRigidStatic> ActorWrapper{};

    };

    struct PxControllerData{

        PxControllerData() = default;

        PxControllerData(physx::PxControllerManager** containedIn, physx::PxController* controller, modulith::Entity attachedTo) : ContainedIn(containedIn), Controller(controller) {
            controller->getActor()->userData = new modulith::Entity(attachedTo);
        }

        PxControllerData(const PxControllerData&) = delete;

        PxControllerData(PxControllerData&& orig) noexcept : ContainedIn(std::exchange(orig.ContainedIn, nullptr)), Controller(std::exchange(orig.Controller, nullptr)) {}

        ~PxControllerData(){
            // We safe a reference to the controller manager the controller belongs to
            // to check if that instance still exists.
            // If it doesn't, then we don't need to release the controller, because that has already been done for us
            if(ContainedIn != nullptr && (*ContainedIn) != nullptr && Controller != nullptr){
                delete reinterpret_cast<modulith::Entity*>(Controller->getActor()->userData);
                Controller->release();
            }
        }

        PxControllerData& operator=(PxControllerData&& rhs) noexcept {
            Controller = std::exchange(rhs.Controller, nullptr);
            return *this;
        }

        physx::PxControllerManager** ContainedIn = nullptr;
        physx::PxController* Controller = nullptr;
    };

    struct PxShapeData {
        PxShapeData() = default;

        explicit PxShapeData(physx::PxShape* collisionShape) : CollisionShape(collisionShape) {}

        PxShapeData(PxShapeData&) = delete;

        PxShapeData(PxShapeData&& orig) noexcept : CollisionShape(std::exchange(orig.CollisionShape, nullptr)) {}

        ~PxShapeData(){
            if(CollisionShape != nullptr)
                CollisionShape->release();
        }

        PxShapeData& operator=(PxShapeData&& rhs) noexcept {
            CollisionShape = std::exchange(rhs.CollisionShape, nullptr);
            return *this;
        }

        physx::PxShape* CollisionShape = nullptr;
    };
}
