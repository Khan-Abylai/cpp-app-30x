#include "LPRecognizerService.h"

using namespace std;


LPRecognizerService::LPRecognizerService(const shared_ptr<SharedQueue<std::shared_ptr<Package>>> &packageQueue,
                                         const vector<CameraScope> &cameras,
                                         float recognizerThreshold,
                                         pair<float, float> calibrationSizes)
        : ILogger(
        "LP Recognizer Service "),
          RECOGNIZER_THRESHOLD{
                  recognizerThreshold}, cameraScopeVector{cameras} {
    LOG_INFO(
            "RecognizerThreshold:%f,Calibration Width:%f, Calibration Height:%f", recognizerThreshold,
            calibrationSizes.first, calibrationSizes.second);
    initializeCameraIpCarTrackerMap(packageQueue, cameras, calibrationSizes);

    lpQueue = make_unique<SharedQueue<shared_ptr<LicensePlate>>>();
    lpRecognizer = make_unique<LPRecognizer>();
    templateMatching = make_unique<TemplateMatching>();
}

void LPRecognizerService::initializeCameraIpCarTrackerMap(
        const shared_ptr<SharedQueue<std::shared_ptr<Package>>> &packageQueue, const vector<CameraScope> &cameras,
        pair<float, float> calibrationSizes) {

    for (const auto &camera: cameras) {
        unique_ptr<BaseCarTracker> carTracker;


        if (camera.isSensorCamera()) {
            carTracker = make_unique<SensorCarTracker>(packageQueue, camera.getCameraIp());
        } else {
            auto calibParams = make_shared<CalibParams>(camera.getNodeIP(), camera.getCameraIp(),
                                                        camera.useProjection(),
                                                        calibrationSizes);
            carTracker = make_unique<MaskCarTracker>(packageQueue, std::move(calibParams), camera.useDirection(),
                                                     camera.usePlateSizes(), camera.platesCount(),
                                                     camera.timeBetweenSentPlates(),
                                                     camera.useCarModelRecognition(), camera.useSentDirection());
        }
        carTracker->run();
        cameraIpToCarTrackerMap.insert({camera.getCameraIp(), std::move(carTracker)});
    }

}

void LPRecognizerService::addToQueue(shared_ptr<LicensePlate> licensePlate) {
    lpQueue->push(std::move(licensePlate));
}

bool LPRecognizerService::isValidLicensePlate(const string &lpLabel, float probability) {
    auto plateCountry = templateMatching->getCountryCode(lpLabel);
    auto isTemplateMatched = plateCountry != Constants::UNIDENTIFIED_COUNTRY;

    return probability > RECOGNIZER_THRESHOLD && isTemplateMatched;
}

pair<string, float>
LPRecognizerService::getLicensePlateLabel(const vector<pair<string, float>> &recognizerResult, bool isSquarePlate) {
    float probability;
    string licensePlateLabel;

    if (isSquarePlate) {
        licensePlateLabel = templateMatching->processSquareLicensePlate(recognizerResult.front().first,
                                                                        recognizerResult.back().first);
        probability = recognizerResult.front().second * recognizerResult.back().second;
    } else {
        licensePlateLabel = recognizerResult.front().first;
        probability = recognizerResult.front().second;
    }
    return make_pair(licensePlateLabel, probability);
}


void LPRecognizerService::addPlateToTrack(const shared_ptr<LicensePlate> &licensePlate) {
    string cameraIp = licensePlate->getCameraIp();
    auto event = cameraIpToCarTrackerMap.find(cameraIp);
    if (event != cameraIpToCarTrackerMap.end())
        event->second->track(licensePlate);
}


pair<cv::Mat, cv::Mat> LPRecognizerService::divideImageIntoHalf(const cv::Mat &lpImage) {
    lpImage(cv::Rect(0, 0, Constants::SQUARE_LP_W, Constants::SQUARE_LP_H / 2)).copyTo(
            whiteImage.colRange(0, Constants::SQUARE_LP_W).rowRange(0, Constants::RECT_LP_H));
    auto firstHalf = whiteImage.clone();

    lpImage(cv::Rect(0, Constants::SQUARE_LP_H / 2, Constants::SQUARE_LP_W, Constants::SQUARE_LP_H / 2)).copyTo(
            whiteImage.colRange(0, Constants::SQUARE_LP_W).rowRange(0, Constants::RECT_LP_H));
    auto secondHalf = whiteImage.clone();

    return make_pair(firstHalf, secondHalf);
}

vector<cv::Mat> LPRecognizerService::getLicensePlateImages(const shared_ptr<LicensePlate> &licensePlate) {
    vector<cv::Mat> lpImages;
    if (licensePlate->isSquare()) {
        auto [firstHalf, secondHalf] = divideImageIntoHalf(licensePlate->getPlateImage());
        lpImages.push_back(std::move(firstHalf));
        lpImages.push_back(std::move(secondHalf));
    } else {
        lpImages.push_back(licensePlate->getPlateImage());
    }
    return lpImages;
}

void LPRecognizerService::run() {
    while (!shutdownFlag) {
        auto licensePlate = lpQueue->wait_and_pop();
        if (licensePlate == nullptr) continue;

        vector<cv::Mat> lpImages = getLicensePlateImages(licensePlate);

        auto startTime = chrono::high_resolution_clock::now();
        auto recognizerResult = lpRecognizer->predict(lpImages);
        auto endTime = chrono::high_resolution_clock::now();

        if (DEBUG)
            licensePlate->setRecognizerTime(chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count());

        auto [licensePlateLabel, probability] = getLicensePlateLabel(recognizerResult, licensePlate->isSquare());

        bool isValid = isValidLicensePlate(licensePlateLabel, probability);
        if (!isValid) {
            continue;
        };
        if (DEBUG && IMSHOW) {
            cv::imshow(licensePlate->getCameraIp() + "_license_plate", licensePlate->getPlateImage());
            cv::waitKey(60);
        }
        licensePlate->setLicensePlateLabel(std::move(licensePlateLabel));
        addPlateToTrack(licensePlate);
    }
}

CameraScope LPRecognizerService::getCameraScope(const string &cameraIp) const {
    int index = 0;
    for (int i = 0; i < cameraScopeVector.size(); ++i) {
        auto item = cameraScopeVector[i];
        if (item.getCameraIp() == cameraIp)
            index = i;
    }
    return cameraScopeVector[index];

}

void LPRecognizerService::shutdown() {
    LOG_INFO("service shutting down");
    shutdownFlag = true;
    lpQueue->push(nullptr);
    for (auto &cameraIpToCarTracker: cameraIpToCarTrackerMap) {
        cameraIpToCarTracker.second->shutdown();
    }
}

bool LPRecognizerService::isChooseThisFrame() {
    srand(time(nullptr));
    auto randomNumber = 1 + rand() % 100; // generating number between 1 and 100
    return (randomNumber < 51);
}

void LPRecognizerService::saveFrame(const shared_ptr<LicensePlate> &plate) {
    if (!isChooseThisFrame()) return;

    string fileName = RandomStringGenerator::generate(30, Constants::IMAGE_DIRECTORY, Constants::JPG_EXTENSION);
    auto frame = plate->getCarImage();
    cv::imwrite(fileName, frame);
}

std::string LPRecognizerService::getNodeIp() {
    return NODE_IP;
}


