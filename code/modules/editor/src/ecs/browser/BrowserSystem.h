/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "EditorModule.h"

namespace modulith::editor{

    class BrowserTab {
    public:
        virtual std::string DisplayName() = 0;
        virtual void Draw() = 0;
    };

    class BrowserSystem : public modulith::System{
    public:
        BrowserSystem() : modulith::System("Browser System") {}

        template<class TTab, class... TArgs>
        void RegisterBrowserTab(TArgs... args){
            _browsers.emplace(typeid(TTab).hash_code(), std::make_shared<TTab>(args...));
        }

        template<class TTab>
        void DeregisterBrowserTab(){
            _browsers.erase(typeid(TTab).hash_code());
        }


    public:
        void OnImGui(float deltaTime) override;

        void OnShutdown() override;

    private:

        modulith::PersistentTypeMap<modulith::shared<BrowserTab>> _browsers;
    };

}
