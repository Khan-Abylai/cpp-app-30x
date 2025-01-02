#pragma once

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include "Constants.h"
#include "Utils.h"
#include "../ITimer.h"

class LicensePlate : public ITimer {
public:

    LicensePlate(int x, int y, int w, int h, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

    const cv::Point2i &getCenter() const;

    const cv::Point2f &getLeftTop() const;

    const cv::Point2f &getRightBottom() const;

    const cv::Point2f &getLeftBottom() const;

    const cv::Point2f &getRightTop() const;

    bool isSquare() const;

    float getArea() const;

    int getWidth() const;

    int getHeight() const;

    cv::Size getCarImageSize() const;

    const cv::Mat &getPlateImage() const;

    void setPlateImage(const cv::Mat &frame);

    const std::string &getPlateLabel() const;

    void setLicensePlateLabel(std::string lpLabel);

    const std::string &getCameraIp() const;

    void setCameraIp(std::string ip);

    const cv::Mat &getCarImage() const;

    void setCarImage(cv::Mat image);

    void setRTPtimestamp(double timestamp);

    double getRTPtimestamp() const;

    void setCarModel(std::string carModelParam);

    const std::string &getCarModel() const;

    void setDirection(const std::string direction);

    const std::string &getDirection() const;


    const std::string &getResultSendUrl() const;

    void setResultSendUrl(const std::string &url);

    const std::string &getSecondaryResultSendUrl() const;

    void setSecondaryResultSendUrl(const std::string &url);

    bool doesSecondaryUrlEnabled() const;

    void setSecondaryUrlEnabledFlag(bool flag);

private:
    const float SQUARE_LP_RATIO = 2.6;
    cv::Mat plateImage;
    cv::Mat carImage;
    std::string licensePlateLabel;
    std::string carModel;
    std::string cameraIp;
    std::string direction;

    std::string resultSendUrl, secondaryResultSendUrl;
    bool secondarySendUrlEnabled;

    double rtpTimestamp;

    cv::Point2i center;
    cv::Point2f leftTop;
    cv::Point2f leftBottom;
    cv::Point2f rightTop;
    cv::Point2f rightBottom;
    int width, height;
    bool square = false;
};