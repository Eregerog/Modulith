/*
 * \brief
 * \author Daniel Götz
 */

#include <ecs/transform/TransformComponents.h>
#include <ecs/ECSContext.h>
#include "ecs/transform/GlobalTransformSystem.h"
#include "Context.h"

namespace modulith{

    void GlobalTransformSystem::OnUpdate(float deltaTime) {

        auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

        // All components with a local transform or parent also have a global transform
        ecs->QueryAll(
            Each(),
            Any<LocalTransformData, WithParentData>(), None<GlobalTransformData>(),
            [ecs](auto entity, auto* localTransform, auto* parent) {
                entity.template AddDeferred<GlobalTransformData>(ecs);
            }
        );

        // Calculate the WorldTransform of all entities
        ecs->QueryAll(
            Each(), None<WithParentData>(), [ecs](auto entity) {
                updateGlobalTransformRec(ecs, entity, float4x4(1.0f), false);
            }
        );
    }

    void GlobalTransformSystem::updateGlobalTransformRec(
        ref<EntityManager> ecs,
        Entity current,
        float4x4 currentObjectToWorld,
        bool parentDisabled
    ) {
        auto localTransform = current.Get<LocalTransformData>(ecs);
        if (localTransform != nullptr)
            currentObjectToWorld = currentObjectToWorld * localTransform->Value;

        auto globalTransform = ecs->GetComponent<GlobalTransformData>(current);
        if (globalTransform != nullptr)
            globalTransform->Value = currentObjectToWorld;

        auto isDisabled = current.Has<DisabledTag>(ecs);

        auto isDisabledInHierarchy = current.Has<IndirectlyDisabledTag>(ecs);
        if((isDisabled || parentDisabled) && !isDisabledInHierarchy) {
            if(ecs->IsInsideQuery()) // this method may be called outside of a query through UpdateGlobalTransformsBelow
                current.AddDeferred<IndirectlyDisabledTag>(ecs);
            else
                current.Add<IndirectlyDisabledTag>(ecs);
        }
        else if((!isDisabled && !parentDisabled) && isDisabledInHierarchy) {
            if(ecs->IsInsideQuery()) // this method may be called outside of a query through UpdateGlobalTransformsBelow
                current.RemoveDeferred<IndirectlyDisabledTag>(ecs);
            else
                current.Remove<IndirectlyDisabledTag>(ecs);
        }

        auto children = current.Get<WithChildrenData>(ecs);
        if (children != nullptr) {
            for (auto& child : children->Values)
                updateGlobalTransformRec(ecs, child, currentObjectToWorld, (parentDisabled || isDisabled));
        }
    }

    void GlobalTransformSystem::UpdateGlobalTransformsBelow(ref<EntityManager> ecs, Entity entity) {
        auto transform = glm::identity<float4x4>();

        auto parent = ecs->GetComponent<WithParentData>(entity);

        bool parentDisabled = false;

        if (parent) {
            auto parentTransform = parent->Value.Get<GlobalTransformData>(ecs);

            if (parentTransform)
                transform = parentTransform->Value;

            parentDisabled = parent->Value.Has<IndirectlyDisabledTag>(ecs);
        }

        updateGlobalTransformRec(ecs, entity, transform, parentDisabled);
    }

}
