#include "SensorClientLauncher.h"

using namespace std;

SensorClientLauncher::SensorClientLauncher(const vector<CameraScope> &cameras,
                                           const vector<shared_ptr<SharedQueue<unique_ptr<FrameData>>>> &frameQueues,
                                           bool useGPUDecode)
        : CameraClientLauncher(cameras, frameQueues, useGPUDecode) {


    for (auto &cameraReader: cameraReaders) {
        shared_ptr<SensorCameraTrigger> sensorCameraTrigger = make_shared<SensorCameraTrigger>();
        std::shared_ptr<Sensor> sensor = make_shared<Sensor>(cameraReader->getZmqServer());
        sensor->addCamera(cameraReader->getCameraIp(), sensorCameraTrigger);
        cameraReader->setSensorCameraTrigger(std::move(sensorCameraTrigger));
        sensors.push_back(sensor);
    }
}

void SensorClientLauncher::run() {
    CameraClientLauncher::run();
    for (auto &sensor: sensors)
        threads.emplace_back(&Sensor::run, sensor); // start Sensor zmq thread
}

void SensorClientLauncher::shutdown() {
    CameraClientLauncher::shutdown();
    for (auto &sensor: sensors)
        sensor->shutdown();
    if (threads.back().joinable())
        threads.back().join();
}