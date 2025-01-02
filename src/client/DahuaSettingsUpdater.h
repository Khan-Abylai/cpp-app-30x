#pragma once

#include <cpr/cpr.h>
#include <string>
#include <regex>
#include <strings.h>
#include <iostream>
#include "../ILogger.h"
#include "../app/CameraScope.h"

class DahuaSettingsUpdater : public ::ILogger {
public:
    explicit DahuaSettingsUpdater(const CameraScope &cameraScope);

    bool setDefaultSettings();

    bool checkCameraAvailability();

private:
    enum class AuthType {
        Basic,
        Digest,
        None
    };
    static void tokenize(std::string const &str, const char delim,
                  std::vector<std::string> &out)
    {
        // construct a stream from the string
        std::stringstream ss(str);

        std::string s;
        while (std::getline(ss, s, delim)) {
            out.push_back(s);
        }
    }
    const std::string FPS = "10";
    const std::string FRAME_WIDTH = "704";
    const std::string FRAME_HEIGHT = "576";
    const std::string BIT_RATE = "1280";
    const int REQUEST_TIMEOUT = 5000;
    std::string username;
    std::string passwd;
    std::string cameraRTSPurl;

    int countToCheckResponseTime = 0;
    AuthType cameraAuthType = AuthType::Basic;

    std::string defaultSettings;
    std::string currentTimeUrl;

    bool wasRequestSuccessful(const cpr::Response &response);

    cpr::Response sendGetRequest(const std::string &url);
};
