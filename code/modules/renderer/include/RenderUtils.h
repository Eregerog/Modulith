/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include <renderer/ModelLoaderUtils.h>
#include "RendererModule.h"

namespace modulith::renderer {

    /**
     * Contains utility methods for the rendering module components
     */
    class RENDERER_API RenderUtils {

    public:
        /**
         * Instantiates the given models by creating child entities under the given root
         * with appropriate RenderMeshData.
         * These child entities are immediately registered to a WithChildrenData component in the root.
         * @param ecs The entity manager the entities should be contained in
         * @param root The entity the models should be attached to
         * @param models The models to instantiate
         */
        static void InstantiateModelsUnder(
            modulith::ref<modulith::EntityManager>& ecs, modulith::Entity root, std::vector<modulith::Model> models
        );

    };
}
