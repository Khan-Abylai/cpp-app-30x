#include "CalibParams.h"

using namespace std;
using json = nlohmann::json;

CalibParams::CalibParams(const string &serverIp, const string &cameraIp, bool useProjection,
                         pair<float, float> calibrationSizes)
        : ILogger(
        "Calib Params " + cameraIp), cameraIp(cameraIp), USE_PROJECTION{useProjection} {

    FRAME_WIDTH_HD = calibrationSizes.first;
    FRAME_HEIGHT_HD = calibrationSizes.second;
    calibParamsUrl = serverIp + cameraIp;
    LOG_INFO("Calibration Parameter Url : %s", calibParamsUrl.c_str());
    getMask();
}

void CalibParams::getMask() {
    auto responseText = sendRequestForMaskPoints();
    auto polygonPoints = getPolygonPoints(responseText, "mask2");
    auto subPolygonPoints = getPolygonPoints(responseText, "mask");

    if (responseText.empty() || responseText.length() <= 2) {
        minWidth = INT_MIN;
        minHeight = INT_MIN;
        maxHeight = INT_MAX;
        maxWidth = INT_MAX;
    } else {
        minHeight = getDimension("height", "min", responseText);
        maxHeight = getDimension("height", "max", responseText);
        minWidth = getDimension("width", "min", responseText);
        maxWidth = getDimension("width", "max", responseText);
    }
    lock_guard<mutex> guard(maskAccessChangeMutex);
    mask = cv::Mat::zeros((int) FRAME_HEIGHT_HD, (int) FRAME_WIDTH_HD, CV_8UC1);
    mask2 = cv::Mat::zeros((int) FRAME_HEIGHT_HD, (int) FRAME_WIDTH_HD, CV_8UC1);

    cv::fillConvexPoly(mask, polygonPoints, WHITE_COLOR);
    cv::fillConvexPoly(mask2, subPolygonPoints, WHITE_COLOR);
}

vector<cv::Point2i> CalibParams::getPolygonPoints(const string &polygonPointsStr, const string &maskType) const {
    vector<cv::Point2i> polygonPoints;

    if (polygonPointsStr.empty() || polygonPointsStr.length() <= 2) {
        polygonPoints.emplace_back(cv::Point2i{0, 0});
        polygonPoints.emplace_back(cv::Point2i{static_cast<int>(FRAME_WIDTH_HD), 0});
        polygonPoints.emplace_back(cv::Point2i{static_cast<int>(FRAME_WIDTH_HD), static_cast<int>(FRAME_HEIGHT_HD)});
        polygonPoints.emplace_back(cv::Point2i{0, static_cast<int>(FRAME_HEIGHT_HD)});
    } else {
        auto polygonPointsJson = json::parse(polygonPointsStr);
        for (auto &point: polygonPointsJson[maskType])
            polygonPoints.emplace_back(cv::Point2i{point["x"].get<int>(), point["y"].get<int>()});
    }
    return polygonPoints;
}

string CalibParams::sendRequestForMaskPoints() {
    cpr::Response response = cpr::Get(cpr::Url{calibParamsUrl}, cpr::VerifySsl(0));
    if (response.status_code >= 400 || response.status_code == 0) {
        LOG_ERROR("%s Error [%d] making request for mask", cameraIp.data(), response.status_code);
        return "";
    }
    return response.text;
}

cv::Point2i CalibParams::getRelatedPoint(const cv::Point2f &point, const cv::Size &imageSize) const {
    auto xPoint = (point.x * FRAME_WIDTH_HD) / imageSize.width;
    auto yPoint = (point.y * FRAME_HEIGHT_HD) / imageSize.height;
    return cv::Point2i{(int) xPoint, (int) yPoint};
}

bool CalibParams::isPointInTheMask(const cv::Point2i &point) {
    lock_guard<mutex> guard(maskAccessChangeMutex);
    return mask.at<uchar>(point.y, point.x) == WHITE_COLOR;
}

int CalibParams::getDimension(const std::string &dimension, const std::string &key, const std::string &parsedStr) {
    auto jsonVal = json::parse(parsedStr);
    return jsonVal[key][dimension].get<int>();
}


bool CalibParams::isLicensePlateSuitableInSize(const shared_ptr<LicensePlate> &licensePlate) {
    auto plateWidthSize = licensePlate->getWidth();
    auto plateHeightSize = licensePlate->getHeight();

    auto perspectiveMinWidth = (int) ((minWidth * licensePlate->getCarImageSize().width) / FRAME_WIDTH_HD);
    auto perspectiveMaxWidth = (int) ((maxWidth * licensePlate->getCarImageSize().width) / FRAME_WIDTH_HD);

    auto perspectiveMinHeight = (int) ((minHeight * licensePlate->getCarImageSize().height) / FRAME_HEIGHT_HD);
    auto perspectiveMaxHeight = (int) ((maxHeight * licensePlate->getCarImageSize().height) / FRAME_HEIGHT_HD);

    if (plateWidthSize <= perspectiveMaxWidth && plateWidthSize >= perspectiveMinWidth) return true;
    if (plateWidthSize <= perspectiveMinWidth && plateHeightSize >= perspectiveMinHeight && licensePlate->isSquare() &&
        plateHeightSize <= perspectiveMaxHeight)
        return true;
    return false;
}

bool
CalibParams::isLicensePlateInSelectedArea(const shared_ptr<LicensePlate> &licensePlate, const std::string &maskType) {
    auto yPointInGround = projectYPointToGround(licensePlate);

    cv::Point2f centerPointInGround{static_cast<float>(licensePlate->getCenter().x), yPointInGround};

    if (IMSHOW && MORE_LOGS) showCenterPointInGround(licensePlate, centerPointInGround);

    auto centerPointHd = getRelatedPoint(centerPointInGround, licensePlate->getCarImageSize());
    if (maskType == "main")
        return isPointInTheMask(centerPointHd);
    else
        return isPointInTheSubMask(centerPointHd);
}

float CalibParams::projectYPointToGround(const shared_ptr<LicensePlate> &licensePlate) {
    float lpHeightCm = (licensePlate->isSquare()) ? SQUARE_LP_H_CM : RECT_LP_H_CM;
    float lpHeightPx = ((licensePlate->getLeftBottom().y - licensePlate->getLeftTop().y) +
                        (licensePlate->getRightBottom().y - licensePlate->getRightTop().y)) / 2;

    if (USE_PROJECTION) {
        return (float) (licensePlate->getCenter().y);
    } else {
        return (float) (licensePlate->getCenter().y) + lpHeightPx * AVERAGE_LP_H_FROM_GROUND_CM / lpHeightCm;
    }
}

void CalibParams::showCenterPointInGround(const shared_ptr<LicensePlate> &licensePlate,
                                          const cv::Point2f &centerPointInGround) {
    cv::Mat copyImage;
    licensePlate->getCarImage().copyTo(copyImage);
    cv::circle(copyImage, centerPointInGround, 3, cv::Scalar(255, 0, 0), cv::FILLED);
    cv::imshow(licensePlate->getCameraIp(), copyImage);
    cv::waitKey(60);
}

const string &CalibParams::getCameraIp() const {
    return cameraIp;
}

bool CalibParams::isPointInTheSubMask(const cv::Point2i &point) {
    lock_guard<mutex> guard(maskAccessChangeMutex);
    return mask2.at<uchar>(point.y, point.x) == WHITE_COLOR;
}

bool CalibParams::getUseProjection() const {
    return USE_PROJECTION;
}


