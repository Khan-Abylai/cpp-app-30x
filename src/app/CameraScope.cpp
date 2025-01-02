//
// Created by Камалхан Артыкбаев on 14.10.2022.
//

#include "CameraScope.h"

CameraScope::CameraScope(std::string &cameraIp, std::string &rtspUrl, bool isCarModelRecognize, bool isUseDirection,
                         bool isUseSentDirection,
                         bool isUseProjection, int platesCollectionCount,
                         int resendingInterval, bool usePlateSize,
                         std::string &resultSendURL, std::string &nodeIp,
                         std::string &snapshotUrl, std::string &secondaryResultSendURL,
                         bool useSecondaryURL, std::string &secondarySnapshotIp, bool useSecondarySnapshotIp,
                         bool isSensor, std::string &zmqServerIp, bool useSubMask) {
    this->CAMERA_IP = std::move(cameraIp);
    this->RTSP_URL = std::move(rtspUrl);
    this->CAR_MODEL_RECOGNITION_ENABLED = isCarModelRecognize;
    this->TIME_BETWEEN_SENT_PLATES = resendingInterval;
    this->PLATES_COUNT = platesCollectionCount;
    this->USE_PROJECTION = isUseProjection;
    this->USE_SEND_DIRECTION = isUseSentDirection;
    this->USE_DIRECTION = isUseDirection;
    this->USE_PLATE_SIZES = usePlateSize;
    this->RESULT_SEND_URL = std::move(resultSendURL);
    this->NODE_IP = std::move(nodeIp);
    this->SNAPSHOT_SEND_IP = std::move(snapshotUrl);
    this->SECONDARY_RESULT_SEND_URL = std::move(secondaryResultSendURL);
    this->USE_SECONDARY_RESULT_SEND_URL = useSecondaryURL;
    this->IS_SENSOR = isSensor;
    this->ZMQ_SERVER_IP = std::move(zmqServerIp);
    this->USE_SUB_MASK_2 = useSubMask;
    this->USE_SECONDARY_SNAPSHOT_IP = useSecondarySnapshotIp;
    this->SECONDARY_SNAPSHOT_SEND_IP = secondarySnapshotIp;
}

const std::string &CameraScope::getCameraIp() const {
    return CAMERA_IP;
}

const std::string &CameraScope::getCameraRTSPUrl() const {
    return RTSP_URL;
}

bool CameraScope::useCarModelRecognition() const {
    return CAR_MODEL_RECOGNITION_ENABLED;
}

bool CameraScope::useSentDirection() const {
    return USE_SEND_DIRECTION;
}

bool CameraScope::useDirection() const {
    return USE_DIRECTION;
}

bool CameraScope::useProjection() const {
    return USE_PROJECTION;
}

int CameraScope::platesCount() const {
    return PLATES_COUNT;
}

int CameraScope::timeBetweenSentPlates() const {
    return TIME_BETWEEN_SENT_PLATES;
}

bool CameraScope::usePlateSizes() const {
    return USE_PLATE_SIZES;
}

const std::string &CameraScope::getResultSendURL() const {
    return RESULT_SEND_URL;
}

const std::string &CameraScope::getNodeIP() const {
    return NODE_IP;
}

const std::string &CameraScope::getSnapshotSendUrl() const {
    return SNAPSHOT_SEND_IP;
}

const std::string &CameraScope::getSecondaryResultSendUrl() const {
    return SECONDARY_RESULT_SEND_URL;
}

bool CameraScope::useSecondaryResultSendURL() const {
    return USE_SECONDARY_RESULT_SEND_URL;
}

const std::string &CameraScope::getZmqServerIp() const {
    return ZMQ_SERVER_IP;
}

bool CameraScope::isSensorCamera() const {
    return IS_SENSOR;
}

bool CameraScope::useSubMask() const {
    return USE_SUB_MASK_2;
}

const std::string &CameraScope::getSecondarySnapshotIp() const {
    return SECONDARY_SNAPSHOT_SEND_IP;
}

bool CameraScope::useSecondarySnapshotIp() const {
    return USE_SECONDARY_SNAPSHOT_IP;
}
