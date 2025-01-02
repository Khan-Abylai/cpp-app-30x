#include "DahuaSettingsUpdater.h"

using namespace std;

DahuaSettingsUpdater::DahuaSettingsUpdater(const CameraScope &cameraScope)
        : ILogger("Dahua Camera Updater " + cameraScope.getCameraIp()), cameraRTSPurl{cameraScope.getCameraRTSPUrl()} {


    std::string credentials = cameraRTSPurl.substr(0, cameraRTSPurl.find('@')).substr(
            7,
            (unsigned long) "rtsp://");
    std::vector<std::string> out;
    tokenize(credentials, ':', out);

    username = out[0];
    passwd = out[1];


    currentTimeUrl = "http://" + cameraScope.getCameraIp() + "/cgi-bin/global.cgi?action=getCurrentTime";
}

cpr::Response DahuaSettingsUpdater::sendGetRequest(const string &url) {
    cpr::Session session;
    session.SetUrl(url);
    session.SetVerifySsl(cpr::VerifySsl(false));
    session.SetTimeout(cpr::Timeout(REQUEST_TIMEOUT));
    if (cameraAuthType == AuthType::Basic) {
        session.SetAuth(cpr::Authentication(username, passwd, cpr::AuthMode::BASIC));
    } else {
        session.SetAuth(cpr::Authentication(username, passwd, cpr::AuthMode::DIGEST));
    }
    return std::move(session.Get());
}

bool DahuaSettingsUpdater::setDefaultSettings() {
    return wasRequestSuccessful(sendGetRequest(defaultSettings));
}

bool DahuaSettingsUpdater::wasRequestSuccessful(const cpr::Response &response) {
    countToCheckResponseTime++;
    if (countToCheckResponseTime == 50) {
        LOG_INFO("Response elapsed time: %d ms", response.elapsed);
        countToCheckResponseTime = 0;
    }
    return !(response.elapsed > REQUEST_TIMEOUT / 1000.0 || response.status_code < 200 || response.status_code > 299);

}

bool DahuaSettingsUpdater::checkCameraAvailability() {

    LOG_INFO("Checking camera availability");

    auto response = sendGetRequest(currentTimeUrl);

    if (response.elapsed >= REQUEST_TIMEOUT / 1000.0 || response.status_code == 0) {
        LOG_ERROR("Camera is not available");
        return false;
    }

    if (response.status_code == 401) {
        cameraAuthType = (cameraAuthType == AuthType::Basic) ? AuthType::Digest : AuthType::Basic;
    }

    return true;
}

