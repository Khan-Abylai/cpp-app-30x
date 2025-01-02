#pragma once

#include <chrono>
#include <thread>
#include <atomic>
#include <ctime>

#include <opencv2/opencv.hpp>

class SensorCameraTrigger {
public:
    bool isValidFrame(const cv::Mat &frame);

    void changeState();

private:
    const int SEND_FRAME_SECONDS = 3;

    std::atomic<std::time_t> firstSignalTime;
};