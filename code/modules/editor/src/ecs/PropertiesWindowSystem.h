/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "EditorModule.h"

namespace modulith::editor{

    class PropertiesWindowSystem : public modulith::System {

    public:
        PropertiesWindowSystem() : modulith::System("Property Window") {}

        template<class T>
        void RegisterPropertyDrawer(std::function<void(std::vector<T>)> drawer) {
            _drawers.emplace(
                typeid(T).hash_code(),
                [drawer](std::vector<std::any> selection) {
                    drawer(boolinq::from(selection)
                        .select([](const std::any& item) { return std::any_cast<T>(item); })
                        .toStdVector());
                }
            );
        }

        template<class T>
        void DeregisterPropertyDrawer(){
            _drawers.erase(typeid(T).hash_code());
        }

        void OnImGui(float deltaTime) override;

        void OnShutdown() override;

    private:

        modulith::PersistentTypeMap<std::function<void(std::vector<std::any>)>> _drawers;
    };
}
