/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <EditorModule.h>
#include "ecs/SelectionSystem.h"

namespace modulith::editor {

    class EntityDebugContext : public modulith::Subcontext {

    public:
        EntityDebugContext() : modulith::Subcontext("Entity Debug Context") {}

        void OnInitialize() override;

        void OnImGui(float deltaTime, bool renderingToImguiWindow) override;

        void OnBeforeUnloadModules(const std::vector<Module>& modules) override;

        void OnAfterLoadModules(const std::vector<Module>& modules) override;

        void OnAfterUnloadModules(const std::vector<Module>& modules) override;

    private:

        void recalculateSerializableComponentCache();

        void entityPropertyDrawer(Context& ctx, const std::vector<Entity>& toDraw);

        InstancedTypeMap<shared<DynamicSerializable>> _serializableComponents;

        modulith::shared<SelectionSystem> _selectionSystem;
    };

}