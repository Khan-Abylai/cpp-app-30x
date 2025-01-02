#pragma once

#include <chrono>

#include "../IThreadLauncher.h"
#include "../ILogger.h"
#include "../SharedQueue.h"
#include "../client/FrameData.h"

#include "Detection.h"
#include "DetectionEngine.h"
#include "LPRecognizerService.h"
#include "../RandomStringGenerator.h"
#include "Snapshot.h"

class DetectionService : public IThreadLauncher, public ::ILogger {
public:
    DetectionService(std::shared_ptr<SharedQueue<std::unique_ptr<FrameData>>> frameQueue,
                     const std::shared_ptr<SharedQueue<std::shared_ptr<Snapshot>>> &snapshotQueue,
                     std::shared_ptr<LPRecognizerService> lpRecognizerService,
                     std::shared_ptr<Detection> detection,
                     const std::string &cameraIp,
                     const std::string &nodeIp,
                     bool useSubMask, bool useProjection, int timeBetweenSendingSnapshots,
                     std::pair<float, float> calibrationSizes);

    void run() override;

    void shutdown() override;

private:
    static bool isChooseThisFrame();

    bool USE_MASK_2;
    time_t lastFrameRTPTimestamp = time(nullptr);
    time_t lastTimeSnapshotSent = time(nullptr);
    int timeBetweenSendingSnapshots = 1;


    std::shared_ptr<Detection> detection;
    std::shared_ptr<LPRecognizerService> lpRecognizerService;
    std::shared_ptr<SharedQueue<std::unique_ptr<FrameData>>> frameQueue;
    std::shared_ptr<SharedQueue<std::shared_ptr<Snapshot>>> snapshotQueue;

    static void saveFrame(const std::shared_ptr<LicensePlate> &plate);

    static std::shared_ptr<LicensePlate> getMaxAreaPlate(std::vector<std::shared_ptr<LicensePlate>> &licensePlates);


    std::shared_ptr<CalibParams> calibParams2;
    std::unique_ptr<CalibParamsUpdater> calibParamsUpdater2;

    static std::shared_ptr<LicensePlate>
    chooseOneLicensePlate(std::vector<std::shared_ptr<LicensePlate>> &licensePlates);

};


