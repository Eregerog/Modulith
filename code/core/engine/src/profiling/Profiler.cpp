#include "profiling/Profiler.h"

namespace modulith{

    void Profiler::BeginMeasurement(const std::string& name) {
        const auto measurement = new ActiveMeasurement{name, std::chrono::high_resolution_clock::now()};
        _activeMeasurements.push(measurement);
    }

    void Profiler::EndMeasurement() {
        auto endTime = std::chrono::high_resolution_clock::now();

        auto measurement = _activeMeasurements.top();
        _activeMeasurements.pop();

        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(measurement->StartTime).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

        auto duration = (end - start) * 0.001;
        const auto completedMeasurement = new CompleteMeasurement{measurement->Name, duration};
        delete measurement;

        _completedMeasurements.push_back(completedMeasurement);
    }

    void Profiler::ClearMeasurements() {
        for (auto measurement : _completedMeasurements) {
            delete measurement;
        }
        _completedMeasurements.clear();
    }
}