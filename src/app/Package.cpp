#include "Package.h"

#include <utility>

using namespace std;
using json = nlohmann::json;

Package::Package(string cameraIp, string licensePlateLabel,
                 cv::Mat carImage, cv::Mat plateImage,
                 string strBoundingBox, string carModel, basic_string<char> direction, std::string resultSendUrlParam,
                 std::string secondaryResultSendUrlParam,
                 bool secondaryResultSendUrlFlagParam)
        : cameraIp(std::move(cameraIp)), licensePlateLabel(std::move(licensePlateLabel)),
          carImage(std::move(carImage)), plateImage(std::move(plateImage)),
          strBoundingBox(std::move(strBoundingBox)), carModel(std::move(carModel)), direction(std::move(direction)),
          resultSendUrl{std::move(resultSendUrlParam)}, secondaryResultSendUrl{std::move(secondaryResultSendUrlParam)},
          secondaryResultSendUrlFlag{secondaryResultSendUrlFlagParam} {
    eventTime = time_t(nullptr);
};

string Package::convertBoundingBoxToStr(const shared_ptr<LicensePlate> &licensePlate) {
    auto frameSize = licensePlate->getCarImageSize();
    auto frameWidth = (float) frameSize.width;
    auto frameHeight = (float) frameSize.height;
    return Utils::pointToStr(licensePlate->getLeftTop().x / frameWidth,
                             licensePlate->getLeftTop().y / frameHeight) + ", " +
           Utils::pointToStr(licensePlate->getLeftBottom().x / frameWidth,
                             licensePlate->getLeftBottom().y / frameHeight) + ", " +
           Utils::pointToStr(licensePlate->getRightTop().x / frameWidth,
                             licensePlate->getRightTop().y / frameHeight) + ", " +
           Utils::pointToStr(licensePlate->getRightBottom().x / frameWidth,
                             licensePlate->getRightBottom().y / frameHeight);
}

string Package::getDisplayPackageJsonString() const {
    json packageJson;
    packageJson["ip_address"] = cameraIp;
    packageJson["event_time"] = Utils::dateTimeToStr(eventTime);
    packageJson["car_number"] = licensePlateLabel;
    packageJson["car_model"] = carModel;
    packageJson["lp_rect"] = strBoundingBox;
    packageJson["direction"] = "forward";
    packageJson["car_picture"] = "Car Image";
    packageJson["lp_picture"] = "License Plate Image";
    return packageJson.dump();
}


string Package::getPackageJsonString() const {
    json packageJson;
    packageJson["ip_address"] = cameraIp;
    packageJson["event_time"] = Utils::dateTimeToStr(eventTime);
    packageJson["car_number"] = licensePlateLabel;
    packageJson["car_picture"] = Utils::encodeImgToBase64(carImage);
    packageJson["lp_picture"] = Utils::encodeImgToBase64(plateImage);
    packageJson["lp_rect"] = strBoundingBox;
    packageJson["car_model"] = carModel;
    packageJson["direction"] = "forward";

    return packageJson.dump();
}

const string &Package::getPlateLabel() const {
    return licensePlateLabel;
}

const string &Package::getCameraIp() const {
    return cameraIp;
}

const std::string &Package::getCarModel() const {
    return carModel;
}

string Package::getDirection() const {
    return direction;
}

string Package::getPackageJsonStringS3(const string &objectName, const string &placement, const string &accessKey,
                                       const string &secretKey) const {
    json packageJson;
    packageJson["accessKey"] = accessKey;
    packageJson["secretKey"] = secretKey;
    packageJson["placement"] = placement;
    packageJson["objectName"] = objectName;
    packageJson["cam_name"] = cameraIp;
    packageJson["car_number"] = licensePlateLabel;
    packageJson["car_picture"] = Utils::encodeImgToBase64(carImage);
    packageJson["isSaveFS"] = true;
    return packageJson.dump();
}

const std::string &Package::getResultSendUrl() const {
    return resultSendUrl;
}

const std::string &Package::getSecondaryResultSendUrl() const {
    return secondaryResultSendUrl;
}

bool Package::doesSecondaryResultSendUrlEnabled() const {
    return secondaryResultSendUrlFlag;
}

