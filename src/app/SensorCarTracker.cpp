#include "SensorCarTracker.h"

using namespace std;

SensorCarTracker::SensorCarTracker(shared_ptr<SharedQueue<shared_ptr<Package>>> packageQueue,
                                   const string &cameraIp)
        : BaseCarTracker(move(packageQueue), cameraIp) {}

void SensorCarTracker::track(const shared_ptr<LicensePlate> &licensePlate) {
    if (DEBUG) licensePlate->setOverallTime();

    if (!awakeLPSenderThread) {
        if (currentCar && currentCar->isLicensePlateBelongsToCar(licensePlate, lastFrameRTPTimestamp)) {
            if (licensePlate->getRTPtimestamp() - lastFrameRTPTimestamp >= RESEND_PLATE_AGAIN_SECONDS) { // resend again
                LOG_INFO("resending plate....");
                createAndPushPackage(licensePlate);
            }
        } else {
            currentCar = createNewCar(PLATES_COUNT);
            awakeLPSenderThread = true;
            sendPlateEvent.notify_one();

            LOG_INFO("tracking new car %s", licensePlate->getPlateLabel().data());
        }
    }
    lastFrameRTPTimestamp = licensePlate->getRTPtimestamp();
    currentCar->addTrackingPoint(licensePlate->getCenter());

    if (awakeLPSenderThread) {
        lock_guard<mutex> guard(mostCommonLPMutex);
        currentCar->addLicensePlateToCount(licensePlate);
    }

    if (IMSHOW) showResult(licensePlate);
}

void SensorCarTracker::sendMostCommonPlate() {
    unique_lock<mutex> lock(sendPlateEventMutex);
    auto timeout = chrono::minutes(WAIT_LOCK_MINUTES);
    while (!shutdownFlag) {
        if (sendPlateEvent.wait_for(lock, timeout, [this] { return awakeLPSenderThread.load(); })) {
            if (shutdownFlag) return;

            this_thread::sleep_for(chrono::seconds(SLEEP_TIME_SECONDS));

            shared_ptr<LicensePlate> licensePlate = getMostCommonLP();
            createAndPushPackage(licensePlate);
            awakeLPSenderThread = false;
        }
    }
}

shared_ptr<LicensePlate> SensorCarTracker::getMostCommonLP() {
    lock_guard<mutex> guard(mostCommonLPMutex);
    return currentCar->getMostCommonLicensePlate();
}

void SensorCarTracker::run() {
    lpSenderThread = thread(&SensorCarTracker::sendMostCommonPlate, this);
}

void SensorCarTracker::shutdown() {
    LOG_INFO("service is shutting down");
    shutdownFlag = true;
    awakeLPSenderThread = true;
    sendPlateEvent.notify_one();
    if (lpSenderThread.joinable())
        lpSenderThread.join();
}
