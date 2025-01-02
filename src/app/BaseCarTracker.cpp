#include "BaseCarTracker.h"

using namespace std;

BaseCarTracker::BaseCarTracker(shared_ptr<SharedQueue<shared_ptr<Package>>> packageQueue,
                               const string &cameraIp)
        : ILogger("Car Tracker " + cameraIp),
          packageQueue{move(packageQueue)} {}

void BaseCarTracker::showResult(const shared_ptr<LicensePlate> &licensePlate) {
    if (!currentCar) return;

    cv::Mat copyImage;
    licensePlate->getCarImage().copyTo(copyImage);
    currentCar->drawBoundingBoxPoints(copyImage, licensePlate);
    currentCar->drawTrackingPoints(copyImage);
    cv::imshow(licensePlate->getCameraIp(), copyImage);
    cv::waitKey(60);
}

shared_ptr<Car> BaseCarTracker::createNewCar(const int &platesCount) {
    return make_shared<Car>(platesCount);
}

shared_ptr<Package> BaseCarTracker::createPackage(const shared_ptr<LicensePlate> &licensePlate) {
    string strBoundingBox = Package::convertBoundingBoxToStr(licensePlate);

    auto package = make_shared<Package>(licensePlate->getCameraIp(),
                                        licensePlate->getPlateLabel(),
                                        licensePlate->getCarImage(),
                                        licensePlate->getPlateImage(),
                                        strBoundingBox,
                                        licensePlate->getCarModel(),
                                        licensePlate->getDirection(), licensePlate->getResultSendUrl(),
                                        licensePlate->getSecondaryResultSendUrl(),
                                        licensePlate->doesSecondaryUrlEnabled());

    if (DEBUG) {
        package->setStartTime(licensePlate->getStartTime());
        package->setOverallTime(licensePlate->getOverallTime());
        package->setDetectionTime(licensePlate->getDetectionTime());
        package->setRecognizerTime(licensePlate->getRecognizerTime());
    }
    return package;
}

void BaseCarTracker::createAndPushPackage(const shared_ptr<LicensePlate> &licensePlate) {
    auto package = createPackage(licensePlate);
    packageQueue->push(move(package));
}