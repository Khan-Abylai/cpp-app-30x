//
// Created by kartykbayev on 8/22/22.
//
#pragma once

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

#include "../Config.h"
#include "../ITimer.h"
#include "LicensePlate.h"
#include "Utils.h"

class Snapshot : public ITimer {
public:
    Snapshot(std::string cameraIp, cv::Mat carImage, std::string snapshotUrl, bool useSecondary, std::string secondaryUrl);

    std::string getPackageJsonString() const;

    const std::string &getCameraIp() const;

    std::string getSnapshotUrl() const;

    bool useSecondaryUrl();

    std::string getSecondarySnapshotUrl() const;

private:
    time_t eventTime;
    std::string cameraIp, snapshotUrl, secondarySnapshotUrl;
    cv::Mat carImage;
    bool useSecondary;
};
