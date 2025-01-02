#pragma once

#include <string>
#include <thread>

#include "CameraClientLauncher.h"
#include "Sensor.h"
#include "../app/CameraScope.h"

class SensorClientLauncher : public CameraClientLauncher {
public:
    SensorClientLauncher(const std::vector<CameraScope> &cameras,
                         const std::vector<std::shared_ptr<SharedQueue<std::unique_ptr<FrameData>>>> &frameQueues,
                         bool useGPUDecode);

    void run();

    void shutdown();

private:

    std::vector<std::shared_ptr<Sensor>> sensors;
};


