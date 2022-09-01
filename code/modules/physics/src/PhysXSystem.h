/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "PhysicsModule.h"
#include "PxPhysicsAPI.h"
#include "PhysicsModuleCore.h"

namespace modulith::physics{

    class PhysXFilerCallback : public physx::PxSimulationFilterCallback {
    public:
        ~PhysXFilerCallback() override = default;

        std::vector<modulith::Entity> ContactsOf(modulith::ref<modulith::EntityManager> ecs, physx::PxActor* actor) {
            auto result = std::vector<modulith::Entity>();
            auto range = _currentContacts.equal_range(actor);
            if (range.first != range.second)
                std::for_each(
                    range.first, range.second, [&result, ecs](auto& kvp) {
                        auto entity = *reinterpret_cast<modulith::Entity*>(static_cast<const physx::PxActor*>(kvp.second)->userData);
                        if(entity.IsAlive(ecs)){
                            result.emplace_back(entity);
                        } else
                        LogWarn("A collision with entity {} was encountered, but the entity no longer exists!", entity)
                    }
                );
            return result;
        }

        void ClearForNextFrame() { _currentContacts.clear(); }

        physx::PxFilterFlags pairFound(
            physx::PxU32 pairID, physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
            const physx::PxActor* a0, const physx::PxShape* s0, physx::PxFilterObjectAttributes attributes1,
            physx::PxFilterData filterData1, const physx::PxActor* a1, const physx::PxShape* s1,
            physx::PxPairFlags& pairFlags
        ) override {
            _currentContacts.emplace(a0, a1);
            _currentContacts.emplace(a1, a0);
            return physx::PxFilterFlags();
        }

        void pairLost(
            physx::PxU32 pairID, physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
            physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, bool objectRemoved
        ) override {
        }

        bool statusChange(physx::PxU32& pairID, physx::PxPairFlags& pairFlags, physx::PxFilterFlags& filterFlags) override {
            return false;
        }

    private:
        std::unordered_multimap<const physx::PxActor*, const physx::PxActor*> _currentContacts{};
    };

    class PhysXSystem : public modulith::System {
    public:
        explicit PhysXSystem() : System("PhysX System") {}

        void OnInitialize() override;

        void OnUpdate(float deltaTime) override;

        void OnImGui(float deltaTime) override;

        void OnShutdown() override;

        physx::PxScene* GetScene() { return _scene; }

    private:
        void updateComponents(const modulith::ref<modulith::EntityManager>& ecs, modulith::Profiler& profiler);

        void stepSimulation(const modulith::ref<modulith::EntityManager>& ecs, float deltaTime);

        void updateContacts(const modulith::ref<modulith::EntityManager>& ecs);

        void updateTransforms(const modulith::ref<modulith::EntityManager>& ecs);

        physx::PxScene* _scene = nullptr;
        physx::PxControllerManager* _controllerManager = nullptr;
        PhysXFilerCallback _filterCallback{};
        physx::PxMaterial* _defaultMaterial = nullptr;
        float _accumulator = 0.0f;
        float _stepSize = 1.0f / 60.0f;
    };



}
