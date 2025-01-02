//
// Created by kartykbayev on 8/22/22.
//

#include "Snapshot.h"

#include <utility>

using namespace std;
using json = nlohmann::json;

Snapshot::Snapshot(std::string cameraIp, cv::Mat carImage, std::string snapshotUrl, bool useSecondary,
                   std::string secondaryUrl)
        : cameraIp(std::move(cameraIp)), carImage(std::move(carImage)), snapshotUrl{std::move(snapshotUrl)},
          useSecondary{useSecondary}, secondarySnapshotUrl{secondaryUrl} {
    eventTime = time_t(nullptr);
}

std::string Snapshot::getPackageJsonString() const {
    json packageJson;
    packageJson["ip_address"] = cameraIp;
    packageJson["event_time"] = Utils::dateTimeToStr(eventTime);
    packageJson["car_picture"] = Utils::encodeImgToBase64(carImage);

    return packageJson.dump();
}

const std::string &Snapshot::getCameraIp() const { return cameraIp; }

std::string Snapshot::getSnapshotUrl() const {
    return snapshotUrl;
}

bool Snapshot::useSecondaryUrl() {
    return useSecondary;
}

std::string Snapshot::getSecondarySnapshotUrl() const {
    return secondarySnapshotUrl;
}
