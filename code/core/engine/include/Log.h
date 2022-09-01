/**
 * \brief
 * \author Daniel Götz
 */


#pragma once
#include <memory>
#include "spdlog/logger.h"

using Logger = spdlog::logger;

namespace modulith {
    class ModuleResources;

    /**
     * This class contains the singleton instance of the spdlog loggers
     */
    class ENGINE_API Log {
        friend class ModuleResources;
    public:
        /**
         * Needs to be called on each process, e.g. once for each module.
         * It initializes spdlog
         */
        static void Init();

        /**
         * Called only on the main process of the executable.
         * Creates the default loggers
         */
        static void CreateDefaultLoggers();

        /**
         * @return Returns the current core / engine logger
         */
        static std::shared_ptr<Logger>& GetCoreLogger();

        /**
         * @return Returns the current module / executable logger
         */
        static std::shared_ptr<Logger>& GetClientLogger();

    private:
        static void setLoggers(std::shared_ptr<Logger> coreLogger, std::shared_ptr<Logger> clientLogger);
        static void resetLoggers();
    };
}

