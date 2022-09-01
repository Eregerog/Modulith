/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"

namespace fs = std::filesystem;

namespace modulith {

    /**
     * Contains utility methods related to module file and folder paths
     */
    class ENGINE_API ModulePathUtils {
    public:
        /**
         * @return An absolute file path to the modules folder.
         * This is always the /module/ subfolder relative to the executable
         */
        static fs::path GetModulesRootFolder();

        /**
         * @param moduleName The name of a module
         * @return An absolute path to the provided module's module folder
         */
        static fs::path GetModuleFolder(const std::string& moduleName);

        /**
         * @param moduleFolder A path to a valid module's folder
         * @return The path to the module's module info file
         */
        static fs::path GetModuleConfigFile(const fs::path& moduleFolder);

        /**
         * @param moduleName The name of a valid module
         * @return The path to the module's module info file
         */
        static fs::path GetModuleConfigFile(const std::string& moduleName);

        /**
         * @param moduleFolder The path to the module's folder
         * @param moduleName The name of the module
         * @return Returns the path to the hotloadable dll for a given module
         */
        static fs::path GetHotloadableModuleDllPath(const fs::path& moduleFolder, const std::string& moduleName);
    };


}
