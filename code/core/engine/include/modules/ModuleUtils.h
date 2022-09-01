/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include "ModuleInfo.h"

namespace fs = std::filesystem;

namespace modulith {

    /**
     * This class contains static methods for loading, parsing and writing module info or modlists from yaml files
     */
    class ENGINE_API ModuleUtils {
    public:
        /**
         * Tries to read and parse a module info YAML file at the given path
         * @param modconfigPath The path the module info is located at
         * @return The contained ModuleInfo if the file existed and was valid, nullopt otherwise
         */
        static std::optional<ModuleInfo> LoadInfoFromFile(const fs::path& modconfigPath);

        /**
         * Writes the given module info into a YAML file at its designated path.
         * The path is derived from the info's name and the directory the engine is currently running in.
         * @param info The info to write into a file
         */
        static void WriteInfoToFile(const ModuleInfo& info);

        /**
         * Parses the modlist located at (relative to the executable) /modules/Default.modlist.
         * The modlist must be valid, which means that is is a YAML sequence of module names.
         * It can be empty
         * @return A list of all module names in that modlist.
         */
        static std::vector<std::string> ParseModlist();

        /**
         * Finds all valid modules located at (relative to the executable) /modules/.
         * A valid module is inside a folder with its name and has a valid Module.modconfig inside it.
         * @return ModuleInfo for each of the valid modules
         */
        static std::vector<ModuleInfo> GetAllAvailableModules();
    };
}

namespace YAML {
    using namespace modulith;

    /**
     * This partial template instantiation allows the {@link modulith::Version} struct to be encoded into and decoded from YAML
     */
    template<>
    struct convert<Version> {
        static Node encode(const Version& rhs) {
            Node node;
            node.push_back(rhs.ToString());
            return node;
        }

        static bool decode(const Node& node, Version& rhs) {
            return rhs.TryParse(node.as<std::string>());
        }
    };


    /**
     * This partial template instantiation allows the {@link modulith::ModuleInfo} struct to be encoded into and decoded from YAML
     */
    template<>
    struct convert<ModuleInfo> {

        static Node encode(const ModuleInfo& info) {
            Node node;

            node["name"] = info.Name;
            node["guid"] = info.Guid.str();
            node["description"] = info.Description;
            node["authors"] = info.Authors;
            node["version"] = info.Version.ToString();
            node["initializeFunctionOverride"] = info.InitializeFunctionOverride;
            node["shutdownFunctionOverride"] = info.ShutdownFunctionOverride;

            if (!info.Dependencies.empty()) {
                Node dependencies;

                for (const auto& dependency : info.Dependencies) {
                    Node dependencyNode;

                    dependencyNode["module"] = dependency.ModuleName;
                    dependencyNode["version"] = dependency.Version.ToString();

                    dependencies.push_back(dependencyNode);
                }

                node["dependencies"] = dependencies;
            }

            return node;
        }

        static bool decode(const Node& node, ModuleInfo& info) {
            info.Name = node["name"].as<std::string>();
            info.Guid = xg::Guid(node["guid"].as<std::string>());
            info.Description = node["description"].as<std::string>("");
            info.Authors = node["authors"].as<std::string>("");
            info.Version = node["version"].as<Version>(Version{0, 1, 0});
            info.InitializeFunctionOverride = node["initializeFunctionOverride"].as<std::string>("");
            info.ShutdownFunctionOverride = node["shutdownFunctionOverride"].as<std::string>("");

            auto dependencyNode = node["dependencies"];
            std::vector<ModuleDependency> moduleDependencies{};

            if (dependencyNode.IsDefined() && dependencyNode.IsSequence()) {
                for (const auto& subnode : dependencyNode) {
                    moduleDependencies.push_back(
                        ModuleDependency{subnode["module"].as<std::string>(),
                                         subnode["version"].as<Version>(Version{0, 1, 0})}
                    );
                }
            }

            info.Dependencies = moduleDependencies;
            return true;
        }
    };
}