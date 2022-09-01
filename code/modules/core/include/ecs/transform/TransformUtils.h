/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include <ecs/ECSUtils.h>
#include <ecs/EntityManager.h>

namespace modulith{

    class CORE_API TransformUtils{
    public:
        /**
         * For the given entity, immediately update the LocalTransformData and GlobalTransformData
         * @param ecs The entity manager the entity is contained in
         * @param entity The entity whose transform data should be updated
         */
        static void UpdateTransformOf(ref<EntityManager> ecs, Entity entity);

        /**
         * For a given entity, execute the given function for it and all its children
         * (deep search, includes children of children etc)
         * Implemented as DFS
         * @param ecs The entity manager the root entity is contained in
         * @param entity The entity to start with
         * @param fn The function that is called for every entity
         */
        static void ForAllChildren(ref<EntityManager> ecs, Entity entity, const std::function<void(ref<EntityManager>, Entity)>& fn);
    };
}
