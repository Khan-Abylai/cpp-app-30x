#include "SensorCameraTrigger.h"

using namespace std;

void SensorCameraTrigger::changeState() {
    firstSignalTime.store(time(nullptr), memory_order_release);
}

bool SensorCameraTrigger::isValidFrame(const cv::Mat &frame) {
    auto elapsed = time(nullptr) - firstSignalTime.load(memory_order_acquire);
    return elapsed >= 0 && elapsed <= SEND_FRAME_SECONDS;
}