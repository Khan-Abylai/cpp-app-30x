#include "FrameData.h"

using namespace std;

FrameData::FrameData(string ip, cv::Mat frame, double rtpTimestamp,
                     chrono::high_resolution_clock::time_point startTime)
        : ip{move(ip)}, frame{move(frame)} {

    this->rtpTimestamp = rtpTimestamp;
    if (DEBUG) setStartTime(startTime);
}

const string &FrameData::getIp() {
    return ip;
}

const cv::Mat &FrameData::getFrame() {
    return frame;
}

float FrameData::getFrameWidth() const {
    return (float) frame.cols;
}

float FrameData::getFrameHeight() const {
    return (float) frame.rows;
}

double FrameData::getRTPtimestamp() const {
    return rtpTimestamp;
}
