#pragma once

#include <ctime>
#include <future>
#include <unordered_map>

#include <cpr/cpr.h>

#include "../IThreadLauncher.h"
#include "../ILogger.h"
#include "../SharedQueue.h"
#include "LicensePlate.h"
#include "Package.h"
#include "Utils.h"
#include "S3Scope.h"
#include "CameraScope.h"

class PackageSender : public IThreadLauncher, public ::ILogger {
public:
    PackageSender(std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue,
                  const std::vector<CameraScope> &cameraScope,
                  const std::shared_ptr<S3Scope> &s3Scope);

    void run() override;

    void shutdown() override;

private:

    const int SEND_REQUEST_TIMEOUT = 10000;
    const int MAX_FUTURE_RESPONSES = 30;

    double avgDetectionTime = 0;
    double avgRecognizerTime = 0;
    double avgOverallTime = 0;

    double maxDetectionTime = -1.0;
    double maxRecognizerTime = -1.0;
    double maxOverallTime = -1.0;

    int iteration = 0;

    bool useS3 = false;
    std::shared_ptr<S3Scope> s3scope;

    std::unordered_map<std::string, std::time_t> lastSendTimes;
    std::string s3URL;
    std::shared_ptr<SharedQueue<std::shared_ptr<Package>>> packageQueue;

    std::future<cpr::Response> sendRequestAsync(const std::string &jsonString);
    std::future<cpr::AsyncResponse > sendRequestAsyncNew(const std::string &jsonString);
    cpr::AsyncResponse sendRequests(const std::string &jsonString, const std::string &serverUrl);
    double getAverageTime(double averageTime, double currentTime) const;
};