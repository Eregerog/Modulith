#pragma once

#include "Core.h"

namespace modulith {

    /**
     * This class can be used as a stopwatch to record how long executing code takes
     */
    class ENGINE_API Profiler {
        struct ActiveMeasurement;
        struct CompleteMeasurement;
    public:
        /**
         * Starts a measurement with the given name and pushes it onto the active measurement stack
         * @param name
         */
        void BeginMeasurement(const std::string& name);

        /**
         * Pops the most recent active measurement from the stack and stops it
         */
        void EndMeasurement();

        /**
         * Iterates over all completed measurements and executes a function for them
         * @tparam Fn The type of the callable object with a signature of void(CompleteMeasurement)
         * @param fn The callable object
         */
        template<class Fn>
        void ForEachMeasurement(const Fn& fn);

        /**
         * Clears all the completed measurements
         */
        void ClearMeasurements();

    private:

        struct ActiveMeasurement {
            std::string Name;
            std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
        };

        struct CompleteMeasurement {
            std::string Name;
            double Duration;
        };

        std::stack<ActiveMeasurement*> _activeMeasurements;
        std::vector<CompleteMeasurement*> _completedMeasurements;
    };

    template<class Fn>
    void Profiler::ForEachMeasurement(const Fn& fn) {
        for (auto measurement : _completedMeasurements) {
            fn(*measurement);
        }
    }
}
