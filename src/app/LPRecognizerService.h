#pragma once

#include <fstream>
#include <iostream>

#include <opencv2/opencv.hpp>

#include "../IThreadLauncher.h"
#include "../ILogger.h"
#include "../SharedQueue.h"
#include "../RandomStringGenerator.h"
#include "LPRecognizer.h"
#include "CalibParams.h"
#include "TemplateMatching.h"
#include "Package.h"
#include "BaseCarTracker.h"
#include "SensorCarTracker.h"
#include "MaskCarTracker.h"
#include "LicensePlate.h"
#include "Constants.h"
#include "CameraScope.h"

class LPRecognizerService : public IThreadLauncher, public ::ILogger {
public:
    LPRecognizerService(const std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> &packageQueue,
                        const std::vector<CameraScope> &cameras, float recognizerThreshold,
                        std::pair<float, float> calibrationSizes);


    void addToQueue(std::shared_ptr<LicensePlate> licensePlate);

    std::string getNodeIp();

    void run() override;

    void shutdown() override;

    CameraScope getCameraScope(const std::string &cameraIp) const;

private:
    const std::vector<CameraScope> &cameraScopeVector;
    float RECOGNIZER_THRESHOLD;
    std::string NODE_IP;
    cv::Mat whiteImage = cv::Mat(Constants::RECT_LP_H, Constants::RECT_LP_W, CV_8UC3, cv::Scalar(255, 255, 255));

    std::unique_ptr<SharedQueue<std::shared_ptr<LicensePlate>>> lpQueue;
    std::unordered_map<std::string, std::unique_ptr<BaseCarTracker>> cameraIpToCarTrackerMap;
    std::unique_ptr<LPRecognizer> lpRecognizer;
    std::unique_ptr<TemplateMatching> templateMatching;

    std::pair<std::string, float>
    getLicensePlateLabel(const std::vector<std::pair<std::string, float>> &recognizerResult, bool isSquarePlate);


    bool isValidLicensePlate(const std::string &lpLabel, float probability);


    std::pair<cv::Mat, cv::Mat> divideImageIntoHalf(const cv::Mat &lpImage);

    void addPlateToTrack(const std::shared_ptr<LicensePlate> &licensePlate);

    static bool isChooseThisFrame();

    static void saveFrame(const std::shared_ptr<LicensePlate> &plate);

    std::vector<cv::Mat> getLicensePlateImages(const std::shared_ptr<LicensePlate> &licensePlate);

    void initializeCameraIpCarTrackerMap(const std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> &packageQueue,
                                         const std::vector<CameraScope> &cameras,
                                         std::pair<float, float> calibrationSizes);
};
