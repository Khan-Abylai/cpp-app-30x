#include "MaskCarTracker.h"

using namespace std;

MaskCarTracker::MaskCarTracker(shared_ptr<SharedQueue<shared_ptr<Package>>> packageQueue,
                               const shared_ptr<CalibParams> &calibParams, bool useDirection, bool usePlateSizes,
                               int platesCount, int timeBetweenResendingPlates, bool useCarModelClassification,
                               bool useSendCarDirection)
        : BaseCarTracker(std::move(packageQueue), calibParams->getCameraIp()), calibParams{(calibParams)},
          platesCount{platesCount}, timeBetweenResendingPlates{timeBetweenResendingPlates},
          USE_CAR_MODEL_CLASSIFICATION{useCarModelClassification}, USE_DIRECTION{useDirection},
          USE_PLATE_SIZE{usePlateSizes}, USE_SEND_DIRECTION{useSendCarDirection} {
    mobilenetEngine = make_unique<MobilenetEngine>();
    carModelClassifier = make_shared<CarModelRecognizer>(mobilenetEngine->createExecutionContext());
    LOG_INFO(
            "Направление движения:%s, Фильтр по размеру ГРНЗ:%s, Интервал между переотправкой событии: %d, Использование MMC:%s, Отправка с направлением движения:%s, Использование проекции:%s, Количество ГРНЗ:%d",
            useDirection ? "True" : "False", usePlateSizes ? "True" : "False", timeBetweenResendingPlates,
            useCarModelClassification ? "True" : "False", useSendCarDirection ? "True" : "False",
            calibParams->getUseProjection() ? "True" : "False", platesCount);
}

void MaskCarTracker::track(const shared_ptr<LicensePlate> &licensePlate) {
    if (DEBUG) licensePlate->setOverallTime();

    if (!calibParams->isLicensePlateSuitableInSize(licensePlate) && USE_PLATE_SIZE) return;

    if (!currentCar || !currentCar->isLicensePlateBelongsToCar(licensePlate, lastFrameRTPTimestamp)) {

        if (currentCar && !isPlateAlreadySent) {
            LOG_INFO("plate %s wasn't sent...", currentCar->getMostCommonLicensePlate()->getPlateLabel().data());
            if (IMSAVE)
                saveFrame(licensePlate);
        }

        currentCar = createNewCar(platesCount);
        isPlateAlreadySent = false;

        LOG_INFO("tracking new car %s", licensePlate->getPlateLabel().data());
    }
    lastFrameRTPTimestamp = licensePlate->getRTPtimestamp();
    currentCar->addTrackingPoint(licensePlate->getCenter());

    if (!USE_SEND_DIRECTION && currentCar->getDirection() == Directions::reverse) return;

    if (IMSHOW) showResult(licensePlate);

    currentCar->addLicensePlateToCount(licensePlate);

    if (isSufficientMomentToSendLP(licensePlate, "main") && !isPlateAlreadySent) {
        if ((currentCar->getDirection() == Directions::forward && currentCar->doesPlatesCollected()) ||
            (currentCar->getDirection() == Directions::reverse && currentCar->doesPlatesCollected()) ||
            (!USE_DIRECTION && currentCar->doesPlatesCollected())) {
            sendMostCommonPlate();
        }
    }
    considerToResendLP(licensePlate);
}

void MaskCarTracker::considerToResendLP(const shared_ptr<LicensePlate> &licensePlate) {
    if ((isPlateAlreadySent && isSufficientTimePassedToSendPlate() &&
         isSufficientMomentToSendLP(licensePlate, "resend") && currentCar->getDirection() == Directions::forward) ||
        (isPlateAlreadySent && isSufficientTimePassedToSendPlate() && !USE_DIRECTION)) {
        LOG_INFO("resending plate....");
        licensePlate->setCarModel(currentCar->getCarModelWithOccurrence().first);
        licensePlate->setDirection("forward");
        createAndPushPackage(licensePlate);
        lastTimeLPSent = lastFrameRTPTimestamp;
    }
}

bool MaskCarTracker::isSufficientTimePassedToSendPlate() {
    return lastFrameRTPTimestamp - lastTimeLPSent >= timeBetweenResendingPlates;
}

void MaskCarTracker::sendMostCommonPlate() {
    shared_ptr<LicensePlate> mostCommonLicensePlate = currentCar->getMostCommonLicensePlate();
    mostCommonLicensePlate->setCarModel(currentCar->getCarModelWithOccurrence().first);
    if (currentCar->getDirection() == Directions::forward) {
        mostCommonLicensePlate->setDirection(("forward"));
    } else if (currentCar->getDirection() == Directions::reverse) {
        mostCommonLicensePlate->setDirection("reverse");
    }
    createAndPushPackage(mostCommonLicensePlate);
    isPlateAlreadySent = true;
    lastTimeLPSent = lastFrameRTPTimestamp;
}

void MaskCarTracker::run() {
    calibParamsUpdater = make_unique<CalibParamsUpdater>(calibParams);
    calibParamsUpdater->run();
}

void MaskCarTracker::shutdown() {
    LOG_INFO("service is shutting down");
    calibParamsUpdater->shutdown();
}

void MaskCarTracker::saveFrame(const shared_ptr<LicensePlate> &licensePlate) {
    auto curPlate = currentCar->getMostCommonLicensePlate();
    cv::imwrite(Constants::IMAGE_DIRECTORY + licensePlate->getPlateLabel() + Constants::JPG_EXTENSION,
                licensePlate->getCarImage());
}

bool
MaskCarTracker::isSufficientMomentToSendLP(const shared_ptr<LicensePlate> &licensePlate, const string &typeOfSend) {
    if (typeOfSend == "main" && USE_CAR_MODEL_CLASSIFICATION) {
        if (calibParams->isLicensePlateInSelectedArea(licensePlate, "main") &&
            !calibParams->isLicensePlateInSelectedArea(licensePlate, "sub")) {
            string carModel = carModelClassifier->classify(licensePlate);
            if (carModel != "NotDefined")
                currentCar->addTrackingCarModel(carModel);
            return false;
        } else if (calibParams->isLicensePlateInSelectedArea(licensePlate, "main") &&
                   calibParams->isLicensePlateInSelectedArea(licensePlate, "sub")) {

            if (currentCar->getCarModelWithOccurrence().second >= platesCount || currentCar->doesPlatesCollected()) {
                string carModel = carModelClassifier->classify(licensePlate);
                if (carModel == "NotDefined")
                    carModel = (string) currentCar->getCarModelWithOccurrence().first;
                currentCar->addTrackingCarModel(carModel);
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }

    } else if (typeOfSend == "main" && !USE_CAR_MODEL_CLASSIFICATION) {
        if (calibParams->isLicensePlateInSelectedArea(licensePlate, "sub") && currentCar->doesPlatesCollected()) {
            string model = "NotDefined";
            currentCar->addTrackingCarModel(model);
            return true;
        } else {
            return false;
        }
    } else {
        if (isPlateAlreadySent && isSufficientTimePassedToSendPlate()) {
            return true;
        } else {
            return false;
        }
    }
}
