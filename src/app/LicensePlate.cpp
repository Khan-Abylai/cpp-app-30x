#include "LicensePlate.h"

using namespace std;

LicensePlate::LicensePlate(int x, int y, int w, int h, int x1, int y1,
                           int x2, int y2, int x3, int y3, int x4, int y4) {

    width = (int) w;
    height = (int) h;
    center = cv::Point2i(x, y);
    leftTop = cv::Point2f(x1, y1);
    leftBottom = cv::Point2f(x2, y2);
    rightTop = cv::Point2f(x3, y3);
    rightBottom = cv::Point2f(x4, y4);

    if ((rightTop.x - leftTop.x) / (leftBottom.y - leftTop.y) < SQUARE_LP_RATIO) {
        square = true;
    }
}

float LicensePlate::getArea() const {
    return width * height;
}

void LicensePlate::setPlateImage(const cv::Mat &frame) {

    cv::Mat transformationMatrix;
    cv::Size lpSize;

    if (square) {
        transformationMatrix = cv::getPerspectiveTransform(vector<cv::Point2f>{
                leftTop, leftBottom, rightTop, rightBottom
        }, Constants::SQUARE_LP_COORS);

        lpSize = cv::Size(Constants::SQUARE_LP_W, Constants::SQUARE_LP_H);

    } else {
        transformationMatrix = cv::getPerspectiveTransform(vector<cv::Point2f>{
                leftTop, leftBottom, rightTop, rightBottom
        }, Constants::RECT_LP_COORS);

        lpSize = cv::Size(Constants::RECT_LP_W, Constants::RECT_LP_H);
    }

    cv::warpPerspective(frame, plateImage, transformationMatrix, lpSize);
}

const cv::Mat &LicensePlate::getPlateImage() const {
    return plateImage;
}

const cv::Point2i &LicensePlate::getCenter() const {
    return center;
}

const cv::Point2f &LicensePlate::getLeftTop() const {
    return leftTop;
}

const cv::Point2f &LicensePlate::getRightBottom() const {
    return rightBottom;
}

int LicensePlate::getWidth() const {
    return width;
}

int LicensePlate::getHeight() const {
    return height;
}

const cv::Point2f &LicensePlate::getLeftBottom() const {
    return leftBottom;
}

const cv::Point2f &LicensePlate::getRightTop() const {
    return rightTop;
}

bool LicensePlate::isSquare() const {
    return square;
}

const string &LicensePlate::getPlateLabel() const {
    return licensePlateLabel;
}

void LicensePlate::setLicensePlateLabel(string lpLabel) {
    licensePlateLabel = move(lpLabel);
}

void LicensePlate::setCameraIp(string ip) {
    cameraIp = move(ip);
}

const string &LicensePlate::getCameraIp() const {
    return cameraIp;
}

void LicensePlate::setCarImage(cv::Mat image) {
    carImage = move(image);
}

const cv::Mat &LicensePlate::getCarImage() const {
    return carImage;
}

void LicensePlate::setRTPtimestamp(double timestamp) {
    rtpTimestamp = timestamp;
}

double LicensePlate::getRTPtimestamp() const {
    return rtpTimestamp;
}

cv::Size LicensePlate::getCarImageSize() const {
    return carImage.size();
}

void LicensePlate::setCarModel(std::string carModelParam) {
    carModel = std::move(carModelParam);
}

const std::string &LicensePlate::getCarModel() const {
    return carModel;
}

const string &LicensePlate::getDirection() const {
    return direction;
}

void LicensePlate::setDirection(std::string directions) {
    direction = std::move(directions);
}

const std::string &LicensePlate::getResultSendUrl() const {
    return resultSendUrl;
}

void LicensePlate::setResultSendUrl(const std::string &url) {
    resultSendUrl = url;
}

const std::string &LicensePlate::getSecondaryResultSendUrl() const {
    return secondaryResultSendUrl;
}

void LicensePlate::setSecondaryResultSendUrl(const std::string &url) {
    secondaryResultSendUrl = url;
}

void LicensePlate::setSecondaryUrlEnabledFlag(bool flag) {
    secondarySendUrlEnabled = flag;
}

bool LicensePlate::doesSecondaryUrlEnabled() const {
    return secondarySendUrlEnabled;
}







