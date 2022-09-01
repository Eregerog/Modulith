/**
 * \brief
 * \author Daniel Götz
 */


#include "Log.h"

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace modulith{

    static std::shared_ptr<spdlog::logger> _coreLogger;
    static std::shared_ptr<spdlog::logger> _clientLogger;

    void Log::Init() {
        spdlog::set_pattern("%^[%T] %n: %v%$");
    }

    void Log::CreateDefaultLoggers() {
        _coreLogger = spdlog::stdout_color_mt("Engine");
        _coreLogger->set_level(spdlog::level::trace);

        _clientLogger = spdlog::stdout_color_mt("Runtime");
        _clientLogger->set_level(spdlog::level::trace);
    }

    std::shared_ptr<spdlog::logger>& Log::GetCoreLogger() {
        return _coreLogger;
    }

    std::shared_ptr<spdlog::logger>& Log::GetClientLogger() {
        return _clientLogger;
    }

    void Log::setLoggers(std::shared_ptr<Logger> coreLogger, std::shared_ptr<Logger> clientLogger) {
        _coreLogger = std::move(coreLogger);
        _clientLogger = std::move(clientLogger);
    }

    void Log::resetLoggers() {
        _coreLogger.reset();
        _clientLogger.reset();
    }

}
