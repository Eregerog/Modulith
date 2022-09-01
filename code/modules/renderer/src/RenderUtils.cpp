/*
 * \brief
 * \author Daniel Götz
 */

#include "RenderUtils.h"
#include "RenderComponents.h"

namespace modulith::renderer {

    void RenderUtils::InstantiateModelsUnder(ref<EntityManager>& ecs, Entity root, std::vector<Model> models) {
        auto* withChildren = root.template Get<WithChildrenData>(ecs);
        if(!withChildren){
            withChildren = root.Add(ecs, WithChildrenData{ std::vector<Entity>() });
        }

        for(auto& model : models){
            auto child = ecs->CreateEntityWith(PositionData(), RenderMeshData(model.Mesh, model.Material), WithParentData(root));
            withChildren->Values.push_back(child);
        }

    }
}
