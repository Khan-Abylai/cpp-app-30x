#pragma once

#include <chrono>
#include <thread>
#include <unordered_map>
#include <iostream>

#include <nlohmann/json.hpp>
#include "../zeromq/zmq.hpp"
#include "../zeromq/zmq_addon.hpp"

#include "../IThreadLauncher.h"
#include "../ILogger.h"
#include "SensorCameraTrigger.h"

class Sensor : public IThreadLauncher, public ::ILogger {
public:
    Sensor(std::string serverUrl);

    void connectSocket();

    bool isConnected();

    void triggerCamera(const std::string &cameraIp);

    void run() override;

    void shutdown() override;

    void addCamera(const std::string &cameraIp, std::shared_ptr<SensorCameraTrigger> cameraClient);

private:
    const std::string PORT = "3002";

    zmq::socket_t socket;
    zmq::context_t context{1};
    std::string zmqServerUrl;
    std::unordered_map<std::string, std::shared_ptr<SensorCameraTrigger>> cameraToSensorTriggers;

};