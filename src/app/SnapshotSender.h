//
// Created by kartykbayev on 8/22/22.
//
#pragma once

#include <ctime>
#include <future>
#include <unordered_map>

#include <cpr/cpr.h>

#include "../ILogger.h"
#include "../IThreadLauncher.h"
#include "../SharedQueue.h"
#include "LicensePlate.h"
#include "Package.h"
#include "Snapshot.h"
#include "Utils.h"
#include "CameraScope.h"

class SnapshotSender : public IThreadLauncher, public ILogger {
public:
    SnapshotSender(
            std::shared_ptr<SharedQueue<std::shared_ptr<Snapshot>>> snapshotQueue,
            const std::vector<CameraScope> &cameras);

    void run() override;

    void shutdown() override;

private:
    const int SEND_REQUEST_TIMEOUT = 1000;
    const int MAX_FUTURE_RESPONSES = 30;
    time_t lastFrameRTPTimestamp = time(nullptr);
    time_t lastTimeSnapshotSent = time(nullptr);
    int iteration = 0;

    std::unordered_map<std::string, std::time_t> lastSendTimes;

    std::shared_ptr<SharedQueue<std::shared_ptr<Snapshot>>> snapshotQueue;
    cpr::AsyncResponse sendRequests(const std::string &jsonString, const std::string &serverUrl);
    std::future<cpr::Response> sendRequestAsync(const std::string &jsonString, const std::string &url);
};
