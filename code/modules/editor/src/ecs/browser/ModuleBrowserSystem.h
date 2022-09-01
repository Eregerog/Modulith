/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "EditorModule.h"
#include "utils/DependencyGraph.h"
#include "BrowserSystem.h"

namespace modulith::editor{

    class ModuleBrowserSystem  : public modulith::System {
    public:
        ModuleBrowserSystem() : modulith::System("Module Debug System") {}

        void OnInitialize() override;

        void OnShutdown() override;

    private:
        static void drawModuleProperties(std::vector<modulith::Module> modules, modulith::Context& ctx);
    };


    class ModuleBrowserTab : public BrowserTab {
    public:
        std::string DisplayName() override;

        void Draw() override;

    private:
        bool _autoReload = false;
    };

}
