#include "DetectionService.h"

using namespace std;

DetectionService::DetectionService(shared_ptr<SharedQueue<unique_ptr<FrameData>>> frameQueue,
                                   const std::shared_ptr<SharedQueue<std::shared_ptr<Snapshot>>> &snapshotQueue,
                                   shared_ptr<LPRecognizerService> lpRecognizerService,
                                   shared_ptr<Detection> detection,
                                   const string &cameraIp,
                                   const string &nodeIp,
                                   bool useSubMask,
                                   bool useProjection,
                                   int timeBetweenSendingSnapshots, std::pair<float, float> calibrationSizes) : ILogger(
        "DetectionService"), frameQueue{std::move(frameQueue)}, snapshotQueue{std::move(snapshotQueue)}, detection{
        std::move(detection)}, USE_MASK_2{useSubMask}, lpRecognizerService{std::move(lpRecognizerService)},
                                                                                                                timeBetweenSendingSnapshots{
                                                                                                                        timeBetweenSendingSnapshots} {

    LOG_INFO("Camera ip:%s, useSubMask:%s, snapshotSendingInterval:%d", cameraIp.c_str(), useSubMask ? "True" : "False",
             timeBetweenSendingSnapshots);

    if (USE_MASK_2) {
        calibParams2 = make_shared<CalibParams>(nodeIp, cameraIp, useProjection, calibrationSizes);
        calibParamsUpdater2 = make_unique<CalibParamsUpdater>(calibParams2);
        calibParamsUpdater2->run();
        calibParams2->getMask();
    }

    lpRecognizerService = std::move(lpRecognizerService);

};

shared_ptr<LicensePlate> DetectionService::getMaxAreaPlate(vector<shared_ptr<LicensePlate>> &licensePlates) {
    float maxArea = -1.0;
    shared_ptr<LicensePlate> licensePlate;
    for (auto &curPlate: licensePlates) {
        float area = curPlate->getArea();
        if (area > maxArea) {
            maxArea = area;
            licensePlate = std::move(curPlate);
        }
    }
    return licensePlate;
}

shared_ptr<LicensePlate> DetectionService::chooseOneLicensePlate(vector<shared_ptr<LicensePlate>> &licensePlates) {

    shared_ptr<LicensePlate> licensePlate;

    if (licensePlates.size() > 1)  // if more than one license plate
        licensePlate = getMaxAreaPlate(licensePlates); // move licensePlate
    else
        licensePlate = std::move(licensePlates.front());

    return licensePlate;
};

void DetectionService::run() {
    while (!shutdownFlag) {
        auto frameData = frameQueue->wait_and_pop();
        if (frameData == nullptr) continue;
        auto frame = frameData->getFrame();
        auto cameraScope = lpRecognizerService->getCameraScope(frameData->getIp());

        lastFrameRTPTimestamp = time(nullptr);
        if (lastFrameRTPTimestamp - lastTimeSnapshotSent >= timeBetweenSendingSnapshots &&
            !cameraScope.getSnapshotSendUrl().empty()) {
            auto snapshot = make_shared<Snapshot>(frameData->getIp(), frame, cameraScope.getSnapshotSendUrl(), cameraScope.useSecondarySnapshotIp(), cameraScope.getSecondarySnapshotIp());
            snapshotQueue->push(std::move(snapshot));
            lastTimeSnapshotSent = time(nullptr);
        }


        auto startTime = chrono::high_resolution_clock::now();
        auto detectionResult = detection->detect(frame);

        auto endTime = chrono::high_resolution_clock::now();

        if (detectionResult.empty()) continue;

        auto licensePlate = chooseOneLicensePlate(detectionResult);

        licensePlate->setPlateImage(frame); // perspective transform and set to plateImage
        licensePlate->setCameraIp(frameData->getIp());
        licensePlate->setCarImage(std::move(frame));
        licensePlate->setRTPtimestamp(frameData->getRTPtimestamp());

        if (USE_MASK_2) if (!calibParams2->isLicensePlateInSelectedArea(licensePlate, "main")) continue;

        if (DEBUG) {
            licensePlate->setDetectionTime(chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count());
            licensePlate->setStartTime(frameData->getStartTime());
        }


        licensePlate->setResultSendUrl(cameraScope.getResultSendURL());
        if (cameraScope.useSecondaryResultSendURL()) {
            licensePlate->setSecondaryUrlEnabledFlag(true);
            licensePlate->setSecondaryResultSendUrl(cameraScope.getSecondaryResultSendUrl());
        } else {
            licensePlate->setSecondaryUrlEnabledFlag(false);
        }
        lpRecognizerService->addToQueue(std::move(licensePlate));
    }
}


void DetectionService::shutdown() {
    LOG_INFO("service is shutting down");
    shutdownFlag = true;
    frameQueue->push(nullptr);
}

bool DetectionService::isChooseThisFrame() {
    srand(time(nullptr));
    auto randomNumber = 1 + rand() % 100; // generating number between 1 and 100
    return (randomNumber < 20);
}


void DetectionService::saveFrame(const shared_ptr<LicensePlate> &plate) {
    if (!isChooseThisFrame()) return;
    string fileName = RandomStringGenerator::generate(30, Constants::IMAGE_DIRECTORY, Constants::JPG_EXTENSION);
    auto frame = plate->getCarImage();
    cv::imwrite(fileName, frame);
}

