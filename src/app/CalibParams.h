#pragma once

#include <opencv2/opencv.hpp>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "limits.h"
#include "../IThreadLauncher.h"
#include "../ILogger.h"
#include "LicensePlate.h"

class CalibParams : public ::ILogger {
public:
    CalibParams(const std::string &serverIp, const std::string &cameraIp,
                bool useProjection, std::pair<float, float> calibrationSizes);

    bool isLicensePlateInSelectedArea(const std::shared_ptr<LicensePlate> &licensePlate, const std::string &maskType);

    bool isLicensePlateSuitableInSize(const std::shared_ptr<LicensePlate> &licensePlate);

    void getMask();

    bool getUseProjection() const;

    const std::string &getCameraIp() const;

private:
    float FRAME_WIDTH_HD, FRAME_HEIGHT_HD;
    const float RECT_LP_H_CM = 0.12;
    const float SQUARE_LP_H_CM = 0.2;
    const float AVERAGE_LP_H_FROM_GROUND_CM = 0.25;
    const int WHITE_COLOR = 255;

    bool USE_PROJECTION;

    std::string cameraIp;
    cv::Mat mask;
    cv::Mat mask2;
    std::vector<cv::Point2i> maskPoints;
    std::vector<cv::Point2i> subMaskPoints;
    std::string calibParamsUrl;
    std::mutex maskAccessChangeMutex;

    int maxWidth, minWidth, maxHeight, minHeight;

    int getDimension(const std::string &dimension, const std::string &key, const std::string &parsedStr);

    bool isPointInTheMask(const cv::Point2i &point);

    bool isPointInTheSubMask(const cv::Point2i &point);

    std::string sendRequestForMaskPoints();

    float projectYPointToGround(const std::shared_ptr<LicensePlate> &licensePlate);

    cv::Point2i getRelatedPoint(const cv::Point2f &point, const cv::Size &imageSize) const;

    static void
    showCenterPointInGround(const std::shared_ptr<LicensePlate> &licensePlate, const cv::Point2f &centerPointInGround);

    std::vector<cv::Point2i> getPolygonPoints(const std::string &polygonPointsStr, const std::string &maskType) const;


};
