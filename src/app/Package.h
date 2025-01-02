#pragma once

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include "Utils.h"
#include "LicensePlate.h"
#include "../ITimer.h"
#include "../Config.h"

class Package : public ITimer {
public:
    Package(std::string cameraIp, std::string licensePlateLabel,
            cv::Mat carImage, cv::Mat plateImage, std::string strBoundingBox, std::string carModel,
            std::basic_string<char> direction, std::string resultSendUrlParam, std::string secondaryResultSendUrlParam,
            bool secondaryResultSendUrlFlagParam);

    std::string getPackageJsonString() const;

    std::string getDisplayPackageJsonString() const;

    const std::string &getPlateLabel() const;

    const std::string &getCameraIp() const;

    static std::string convertBoundingBoxToStr(const std::shared_ptr<LicensePlate> &licensePlate);

    const std::string &getCarModel() const;

    std::string getDirection() const;


    std::string
    getPackageJsonStringS3(const std::string &objectName, const std::string &placement, const std::string &accessKey,
                           const std::string &secretKey) const;

    const std::string &getResultSendUrl() const;

    const std::string &getSecondaryResultSendUrl() const;

    bool doesSecondaryResultSendUrlEnabled() const;


private:
    std::string resultSendUrl, secondaryResultSendUrl;
    bool secondaryResultSendUrlFlag;
    std::string direction;
    std::string carModel;
    std::string cameraIp;
    time_t eventTime;
    std::string licensePlateLabel;
    cv::Mat carImage;
    cv::Mat plateImage;
    std::string strBoundingBox;
};
