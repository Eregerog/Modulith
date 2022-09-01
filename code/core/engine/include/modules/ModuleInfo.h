/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include <sstream>
#include <regex>
#include <crossguid/guid.hpp>
#include "modules/Module.h"

namespace modulith {

    /**
     * This struct represents a version, made for semantic versioning
     */
    struct Version {
        /**
         * A major version change denotes iincompatibility with the previous major versions
         */
        int Major = 0;
        /**
         * A minor version change occurs when features are added, but backwards-compatibility is ensured
         */
        int Minor = 1;
        /**
         * Non-feature changes, such as hotfixes are denoted with an increment of the fix counter.
         */
        int Fix = 0;

        /**
         * @return A formatted string of the version, in the format Major.Minor.Fix
         */
        [[nodiscard]] std::string ToString() const {
            std::ostringstream ss{};
            ss << Major << "." << Minor << "." << Fix;
            return ss.str();
        }

        /**
         * This to parse the given string and set this version's values to it.
         * The given string has to be of the format Major.Minor.Fix, where each of the three has to be a number
         * @return Returns whether the string was successfully parsed
         */
        bool TryParse(const std::string& string) {
            auto versionStrings = split(string, '.');
            switch (versionStrings.size()) {
                case 3:
                    Fix = std::stoi(versionStrings[2]);
                case 2:
                    Minor = std::stoi(versionStrings[1]);
                case 1:
                    Major = std::stoi(versionStrings[0]);
                    return true;
            }
            return false;
        }

    private:
        static std::vector<std::string> split(const std::string& s, char delimiter) {
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream tokenStream(s);
            while (std::getline(tokenStream, token, delimiter)) {
                tokens.push_back(token);
            }
            return tokens;
        }

    };

    /**
     * This struct descibes a dependency to a given module of a certain version
     */
    struct ModuleDependency {
        std::string ModuleName;
        Version Version;
    };

    /**
     * This struct contains metadata for a given module
     */
    struct ModuleInfo {

        ModuleInfo() = default;

        ModuleInfo(
            const xg::Guid& guid,
            std::string name,
            std::string description,
            std::string authors,
            const Version& version,
            std::vector<ModuleDependency> dependencies
        ) : Guid(guid),
            Name(std::move(name)),
            Description(std::move(description)),
            Authors(std::move(authors)),
            Dependencies(std::move(dependencies)),
            Version(version) {}

        /**
         * A unique identifier for a module.
         * This must not change once the module has been created.
         */
        xg::Guid Guid;

        /**
         * The name of the module.
         * This can change after a module's creation, because it is identified by its guid.
         */
        std::string Name;

        /**
         * An extended description of a module, that can be defined by the author.
         */
        std::string Description;

        /**
         * This string contains the author(s) of the module
         */
        std::string Authors;

        /**
         * A list of module's this module depends on
         */
        std::vector<ModuleDependency> Dependencies;

        /**
         * The current version of the module
         */
        Version Version;

        /**
         * If this string is non-empty it is the dll function name of this module's Initialize function.
         * Should generally not be changed, unless the module is compiled differently that the others
         * and therefore this function has a different name in the dll.
         */
        std::string InitializeFunctionOverride = "";

        /**
         * If this string is non-empty it is the dll function name of this module's Shutdown function.
         * Should generally not be changed, unless the module is compiled differently that the others
         * and therefore this function has a different name in the dll.
         */
        std::string ShutdownFunctionOverride = "";

        /**
         * @return A Module struct, which is the handle by which this info's module is identified
         */
        [[nodiscard]] Module AsModule() const { return Module(Name, Guid); }
    };
}

