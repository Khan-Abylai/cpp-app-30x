//
// Created by kartykbayev on 8/22/22.
//

#include "SnapshotSender.h"

#include <utility>
#include <glib.h>

using namespace std;

SnapshotSender::SnapshotSender(
        std::shared_ptr<SharedQueue<std::shared_ptr<Snapshot>>> snapshotQueue,
        const std::vector<CameraScope> &cameraScope) : ILogger("Snapshot Sender -------------"),
                                                       snapshotQueue{std::move(snapshotQueue)} {
    for (const auto &cameraIp: cameraScope) {
        lastSendTimes.insert({cameraIp.getCameraIp(), time(nullptr)});
        LOG_INFO("Camera ip:%s will send snapshot to:%s", cameraIp.getCameraIp().c_str(),
                 cameraIp.getSnapshotSendUrl().c_str());
        if (cameraIp.useSecondarySnapshotIp())
            LOG_INFO("Camera ip:%s will send also to another snapshot url:%s", cameraIp.getCameraIp().c_str(),
                     cameraIp.getSecondarySnapshotIp().c_str());
        else
            LOG_INFO("Camera ip:%s not defines secondary snapshot url", cameraIp.getCameraIp().c_str());
    }
}


void SnapshotSender::run() {
    time_t beginTime = time(nullptr);

    queue<future<cpr::Response>> futureResponses;
    queue<future<cpr::Response>> futureResponsesSecondary;

    queue<cpr::AsyncResponse> responses;
    queue<cpr::AsyncResponse> responses2;

    while (!shutdownFlag) {
        auto package = snapshotQueue->wait_and_pop();
        if (package == nullptr)
            continue;
        lastFrameRTPTimestamp = time(nullptr);
        if (lastFrameRTPTimestamp - lastTimeSnapshotSent >= 100) {
            if (DEBUG)
                LOG_INFO("snapshots sending from ip:%s", package->getCameraIp().c_str());
            lastTimeSnapshotSent = time(nullptr);
        }

        responses.push(sendRequests(package->getPackageJsonString(), package->getSnapshotUrl()));
        if (package->useSecondaryUrl())
            responses2.push(
                    sendRequests(package->getPackageJsonString(), package->getSecondarySnapshotUrl()));
        while (responses.size() > MAX_FUTURE_RESPONSES) {
            std::queue<cpr::AsyncResponse> empty;
            std::swap(responses, empty);
            if (DEBUG)
                LOG_INFO("swapped snapshot queue with empty queue");
        }

        while (responses2.size() > MAX_FUTURE_RESPONSES) {
            std::queue<cpr::AsyncResponse> empty;
            std::swap(responses2, empty);
            if (DEBUG)
                LOG_INFO("swapped snapshot queue with empty queue");
        }


    }
}

void SnapshotSender::shutdown() {
    LOG_INFO("service is shutting down");
    shutdownFlag = true;
    snapshotQueue->push(nullptr);
}

cpr::AsyncResponse SnapshotSender::sendRequests(const string &jsonString, const string &serverUrl) {
    return cpr::PostAsync(cpr::Url{serverUrl}, cpr::VerifySsl(false), cpr::Body{jsonString},
                          cpr::Timeout{SEND_REQUEST_TIMEOUT}, cpr::Header{{"Content-Type", "application/json"}});
}
