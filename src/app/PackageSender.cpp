#include "PackageSender.h"

#include <utility>

using namespace std;


PackageSender::PackageSender(shared_ptr<SharedQueue<shared_ptr<Package>>> packageQueue,
                             const vector<CameraScope> &cameraScope,
                             const shared_ptr<S3Scope> &s3Scope)
        : ILogger("Package Sender --------------------"),
          packageQueue{std::move(packageQueue)}, s3scope{s3Scope} {

    for (const auto &camera: cameraScope) {
        LOG_INFO("Camera ip:%s will send packages to the: %s. Does it have secondary result send url:%s,%s",
                 camera.getCameraIp().c_str(),
                 camera.getResultSendURL().c_str(), camera.useSecondaryResultSendURL() ? "True" : "False",
                 camera.getSecondaryResultSendUrl().c_str());
    }
    if (s3Scope != nullptr) {
        useS3 = true;
        this->s3scope = s3Scope;
        s3URL = this->s3scope->getS3URL();
    }

    for (const auto &camera: cameraScope)
        lastSendTimes.insert({camera.getCameraIp(), time(nullptr)});
}

void PackageSender::run() {
    time_t beginTime = time(nullptr);

    queue<future<cpr::Response>> futureResponses;
    queue<future<cpr::Response>> futureResponsesS3;
    queue<future<cpr::Response>> futureResponsesSecondary;

    queue<cpr::AsyncResponse> responses;
    queue<cpr::AsyncResponse> responses2;


    while (!shutdownFlag) {
        auto package = packageQueue->wait_and_pop();
        if (package == nullptr) continue;

        if ((string) package->getCarModel().data() == "NotDefined")
            LOG_INFO("%s %s %s", package->getCameraIp().data(), package->getPlateLabel().data(),
                     Utils::dateTimeToStr(time_t(nullptr)).c_str());
        else
            LOG_INFO("%s %s %s %s", package->getCameraIp().data(), package->getPlateLabel().data(),
                     Utils::dateTimeToStr(time_t(nullptr)).c_str(),
                     package->getCarModel().data());
        if (DEBUG)
            LOG_INFO("Data for sending: %s", package->getDisplayPackageJsonString().c_str());

        responses.push(sendRequests(package->getPackageJsonString(), package->getResultSendUrl()));

        if (package->doesSecondaryResultSendUrlEnabled()) {
            responses2.push(
                    sendRequests(package->getPackageJsonString(), package->getSecondaryResultSendUrl()));
        }


        while (responses.size() > MAX_FUTURE_RESPONSES) {
            responses.pop();
        }

        while (responses2.size() > MAX_FUTURE_RESPONSES) {
            responses2.pop();
        }

        if (DEBUG) {
            avgDetectionTime = getAverageTime(avgDetectionTime, package->getDetectionTime());
            avgRecognizerTime = getAverageTime(avgRecognizerTime, package->getRecognizerTime());
            avgOverallTime = getAverageTime(avgOverallTime, package->getOverallTime());

            maxDetectionTime = max(maxDetectionTime, package->getDetectionTime());
            maxRecognizerTime = max(maxRecognizerTime, package->getRecognizerTime());
            maxOverallTime = max(maxOverallTime, package->getOverallTime());

            iteration++;

            if (time(nullptr) - beginTime > 300) {
                beginTime = time(nullptr);
                LOG_INFO("average detection time %lf", avgDetectionTime);
                LOG_INFO("average recognizer time %lf", avgRecognizerTime);
                LOG_INFO("average overall time %lf", avgOverallTime);
                LOG_INFO("max detection time %lf", maxDetectionTime);
                LOG_INFO("max recognizer time %lf", maxRecognizerTime);
                LOG_INFO("max overall time %lf", maxOverallTime);
            }
        }
    }
}

double PackageSender::getAverageTime(double averageTime, double currentTime) const {
    return averageTime * (static_cast<double>(iteration) / (iteration + 1)) + currentTime / (iteration + 1);
}

void PackageSender::shutdown() {
    LOG_INFO("service is shutting down");
    shutdownFlag = true;
    packageQueue->push(nullptr);
}

cpr::AsyncResponse PackageSender::sendRequests(const string &jsonString, const std::string &serverUrl) {
    return cpr::PostAsync(
            cpr::Url{serverUrl},
            cpr::VerifySsl(false),
            cpr::Body{jsonString},
            cpr::Timeout{SEND_REQUEST_TIMEOUT},
            cpr::Header{{"Content-Type", "application/json"}});
}


