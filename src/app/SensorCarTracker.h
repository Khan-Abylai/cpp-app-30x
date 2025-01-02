#pragma once

#include "BaseCarTracker.h"

class SensorCarTracker : public BaseCarTracker {
public:
    explicit SensorCarTracker(std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue,
                              const std::string &cameraIp);

    void track(const std::shared_ptr<LicensePlate> &licensePlate) override;

    void run() override;

    void shutdown() override;

private:
    const int SLEEP_TIME_SECONDS = 3;
    const int WAIT_LOCK_MINUTES = 5;
    const int RESEND_PLATE_AGAIN_SECONDS = 3;
    const int PLATES_COUNT = 5;
    std::thread lpSenderThread;

    std::mutex mostCommonLPMutex;
    std::condition_variable sendPlateEvent;
    std::mutex sendPlateEventMutex;
    std::atomic<bool> awakeLPSenderThread{false};

    void sendMostCommonPlate() override;

    std::shared_ptr<LicensePlate> getMostCommonLP();
};
