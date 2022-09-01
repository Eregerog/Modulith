/*
 * \brief
 * \author Daniel Götz
 */

#include <ecs/transform/TransformComponents.h>
#include "ecs/transform/ParentSystem.h"
#include "Context.h"
#include "ecs/ECSContext.h"

namespace modulith{

    void ParentSystem::OnUpdate(float deltaTime) {
        auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

        std::unordered_multimap<Entity, Entity, EntityHasher> childrenOf;

        // Collect all the Parent's Children
        ecs->QueryAll(
            Each<WithParentData>(), [&childrenOf](auto entity, auto& parent) {
                childrenOf.emplace(parent.Value, entity);
            }
        );

        // All entities with children have a child component
        for (auto& withChildren : childrenOf) {
            ecs->AddComponent<WithChildrenData>(withChildren.first);
        }

        // Apply the children
        ecs->QueryAll(
            Each<WithChildrenData>(), [&childrenOf, ecs](auto entity, auto& children) {
                children.Values.clear();
                auto range = childrenOf.equal_range(entity);
                auto hasChildren = range.first != range.second;
                if (hasChildren)
                    for_each(
                        range.first, range.second, [&children](auto& child) { children.Values.push_back(child.second); }
                    );
                else
                    entity.template RemoveDeferred<WithChildrenData>(ecs);
            }
        );
    }

}
