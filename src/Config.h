#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "app/CameraScope.h"
#include "app/Utils.h"

class Config {
public:

    static bool parseJson(const std::string &fileName);

    static const std::vector<CameraScope> &getCameras();

    static const std::vector<CameraScope> &getSensorCameras();

    static const std::vector<CameraScope> &getStreamCameras();

    static bool useGPUDecode();

    static const int &getTimeSentSnapshots();

    static const float &getRecognizerThreshold();

    static const float &getDetectorThreshold();

    static const std::string &getS3SecretKey();

    static const std::string &getS3AccessKey();

    static const std::string &getS3Placement();

    static const std::string &getS3ObjectName();

    static const std::string &getS3URL();

    static bool useS3();

    static std::pair<float, float> getCalibrationSizes();

};