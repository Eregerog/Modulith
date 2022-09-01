/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "EditorModule.h"

namespace modulith::editor{

    struct VariableReplacement {
        std::string VariableName;
        std::string Replacement;
    };

    struct VariableDefine {
        std::string VariableName;
    };

    using BlueprintVariable = std::variant<VariableReplacement, VariableDefine>;

    class BlueprintUtils{
    public:
        static std::filesystem::path GetEditorBlueprintDirectory();

        static void InstantiateBlueprintsAt(
            const std::vector<std::filesystem::path>& blueprintPaths, const std::filesystem::path& destinationPath, const std::vector<BlueprintVariable>& variables
        );

        static void InstantiateBlueprintAt(
            const std::filesystem::path& blueprintPath, const std::filesystem::path& destinationPath, const std::vector<BlueprintVariable>& variables
        );

    };
}
