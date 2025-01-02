#pragma once

#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>

#include "Package.h"
#include "Car.h"
#include "../IThreadLauncher.h"
#include "../ILogger.h"
#include "../SharedQueue.h"

class BaseCarTracker : public IThreadLauncher, public ::ILogger {
public:
    explicit BaseCarTracker(std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue,
                            const std::string &cameraIp);

    virtual void track(const std::shared_ptr<LicensePlate> &licensePlate) = 0;

protected:
    std::shared_ptr<Car> currentCar;
    double lastFrameRTPTimestamp = 0;

    static std::shared_ptr<Car> createNewCar(const int &platesCount);

    void showResult(const std::shared_ptr<LicensePlate> &licensePlate);

    static std::shared_ptr<Package> createPackage(const std::shared_ptr<LicensePlate> &licensePlate);

    virtual void sendMostCommonPlate() = 0;

    void createAndPushPackage(const std::shared_ptr<LicensePlate> &licensePlate);

private:
    std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue;
};