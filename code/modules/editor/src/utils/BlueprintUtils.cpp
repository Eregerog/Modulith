/**
 * \brief
 * \author Daniel Götz
 */

#include "BlueprintUtils.h"

namespace fs = std::filesystem;

namespace modulith::editor{

    fs::path BlueprintUtils::GetEditorBlueprintDirectory() {
        auto workingDirectory = fs::current_path();
        return workingDirectory / "modules" / "Editor" / "assets";
    }

    void BlueprintUtils::InstantiateBlueprintAt(
        const fs::path& blueprintPath, const fs::path& destinationPath,
        const std::vector<BlueprintVariable>& variables
    ) {
        InstantiateBlueprintsAt({blueprintPath}, destinationPath, variables);
    }


    void BlueprintUtils::InstantiateBlueprintsAt(
        const std::vector<fs::path>& blueprintPaths, const fs::path& destinationPath,
        const std::vector<BlueprintVariable>& variables
    ) {
        std::ofstream outputStream;
        outputStream.open(destinationPath);

        std::ifstream inputStream;

        for (auto& blueprintPath : blueprintPaths) {

            auto variableDefines = boolinq::from(variables)
                .where([](auto variable){ return std::holds_alternative<VariableDefine>(variable); })
                .select([](auto& variable){ return std::get<VariableDefine>(variable).VariableName; })
                .toStdUnorderedSet();

            std::stack<std::string> skipWhile{};
            inputStream.open(blueprintPath);

            std::string currentLine;
            while (std::getline(inputStream, currentLine)) {
                for (const BlueprintVariable & variable : variables) {
                    if(std::holds_alternative<VariableReplacement>(variable)){
                        auto replacement = std::get<VariableReplacement>(variable);

                        std::regex regex(R"(\$\$)" + replacement.VariableName + R"(\$\$)");
                        currentLine = std::regex_replace(currentLine, regex, replacement.Replacement);
                    }else if(std::holds_alternative<VariableDefine>(variable)){
                        auto define = std::get<VariableDefine>(variable);

                    }
                }

                auto skipCurrent = false;

                std::regex beginIfRegex(R"(\$\$BeginIf_([A-Za-z0-9]*)\$\$)");
                std::smatch beginIfMatches;
                if(std::regex_search(currentLine, beginIfMatches, beginIfRegex)){
                    skipCurrent = true;
                    auto requiredDefine = beginIfMatches[1].str();
                    if(variableDefines.count(requiredDefine) == 0){
                        skipWhile.push(requiredDefine);
                    }
                }else{
                    std::regex endIfRegex(R"(\$\$EndIf_([A-Za-z0-9]*)\$\$)");
                    std::smatch endIfMatches;
                    if(std::regex_search(currentLine, endIfMatches, endIfRegex)){
                        skipCurrent = true;
                        auto requiredDefine = endIfMatches[1].str();
                        if(variableDefines.count(requiredDefine) == 0){
                            Assert(!skipWhile.empty() && skipWhile.top() == requiredDefine, "Parse error of {}: Encountered EndIf_{} out of place!", blueprintPath.generic_string(), requiredDefine)
                            skipWhile.pop();
                        }
                    }
                }

                if(skipWhile.empty() && !skipCurrent)
                    outputStream << currentLine << "\n";
            }

            inputStream.close();
        }
        outputStream.flush();
        outputStream.close();
    }

}
