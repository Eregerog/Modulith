/*
 * \brief
 * \author Daniel Götz
 */

#include "resources/Resource.h"

namespace modulith{

    void Resource::InvokeLoadCallbacksOn(std::vector<shared<Resource>>& resources, const std::string& description) {
        std::sort(
            resources.begin(), resources.end(), [](const auto& lhs, const auto& rhs) {
                return lhs->GetPriority() > rhs->GetPriority();
            }
        );

        for (const auto& resource : resources) {
            resource->OnLoad(description);
        }

        for (const auto& resource : resources) {
            resource->OnInitializeAfterLoad(description);
        }
    }

    void Resource::InvokeUnloadCallbacksOn(std::vector<shared<Resource>>& resources, const std::string& description) {
        std::sort(
            resources.begin(), resources.end(), [](const auto& lhs, const auto& rhs) {
                return lhs->GetPriority() < rhs->GetPriority();
            }
        );

        for (const auto& resource : resources) {
            resource->OnShutdownBeforeUnload(description);
        }

        for (const auto& resource : resources) {
            resource->OnUnload(description);
        }
    }
}