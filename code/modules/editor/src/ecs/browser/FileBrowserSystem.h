/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "EditorModule.h"
#include "BrowserSystem.h"
#include "ecs/SelectionSystem.h"
#include "ecs/modules/EditedModuleSystem.h"

namespace modulith::editor{

    class FileBrowserSystem : public modulith::System {
    public:
        FileBrowserSystem() : modulith::System("FileBrowserSystem") {}

        void OnInitialize() override;

        void OnShutdown() override;
    };

    class FileBrowserTab : public BrowserTab {

    public:
        FileBrowserTab(modulith::shared<EditedModuleSystem>  editingSystem, modulith::shared<SelectionSystem>  selectionSystem);

        std::string DisplayName() override;
        void Draw() override;

    private:
        modulith::shared<EditedModuleSystem> _editingSystem;
        modulith::shared<SelectionSystem> _selectionSystem;
    };

}
