#pragma once

#include "BaseCarTracker.h"
#include "CalibParams.h"
#include "CalibParamsUpdater.h"
#include "../car_model/CarModelRecognizer.h"
#include "../car_model/MobilenetEngine.h"

class MaskCarTracker : public BaseCarTracker {
public:
    explicit MaskCarTracker(std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue,
                            const std::shared_ptr<CalibParams>& calibParams,
                            bool useDirection,
                            bool usePlateSizes,
                            int platesCount,
                            int timeBetweenResendingPlates,
                            bool useCarModelClassification,
                            bool useSendCarDirection);

    void track(const std::shared_ptr<LicensePlate> &licensePlate) override;

    void run() override;

    void shutdown() override;

private:
    bool USE_DIRECTION;
    bool USE_SEND_DIRECTION;
    bool USE_PLATE_SIZE;
    bool USE_CAR_MODEL_CLASSIFICATION;
    int platesCount;
    int timeBetweenResendingPlates;

    std::shared_ptr<CalibParams> calibParams;
    std::unique_ptr<CalibParamsUpdater> calibParamsUpdater;


    std::unique_ptr<MobilenetEngine> mobilenetEngine;
    std::shared_ptr<CarModelRecognizer> carModelClassifier;

    bool isPlateAlreadySent{false};
    double lastTimeLPSent = 0;

    void sendMostCommonPlate() override;

    void saveFrame(const std::shared_ptr<LicensePlate> &licensePlate);

    bool isSufficientTimePassedToSendPlate();

    void considerToResendLP(const std::shared_ptr<LicensePlate> &licensePlate);

    bool isSufficientMomentToSendLP(const std::shared_ptr<LicensePlate> &licensePlate,
                                    const std::string &typeOfSend);
};
