//
// Created by Камалхан Артыкбаев on 14.10.2022.
//
#pragma once

#include <regex>
#include <string>

class CameraScope {
public:
    CameraScope(std::string &cameraIp, std::string &rtspUrl, bool isCarModelRecognize,
                bool isUseDirection, bool isUseSentDirection,
                bool isUseProjection, int platesCollectionCount,
                int resendingInterval, bool usePlateSize,
                std::string &resultSendURL, std::string &nodeIp, std::string &snapshotUrl,
                std::string &secondaryResultSendURL, bool useSecondaryURL,std::string &secondarySnapshotIp, bool useSecondarySnapshotIp, bool isSensor, std::string &zmqServerIp,
                bool useSubMask);

    const std::string &getCameraIp() const;

    const std::string &getCameraRTSPUrl() const;

    const std::string &getResultSendURL() const;

    const std::string &getNodeIP() const;

    const std::string &getSnapshotSendUrl() const;

    const std::string &getSecondaryResultSendUrl() const;

    const std::string &getSecondarySnapshotIp() const;

    const std::string &getZmqServerIp() const;

    bool useSecondaryResultSendURL() const;

    bool useSecondarySnapshotIp() const;

    bool useCarModelRecognition() const;

    bool useDirection() const;

    bool useSentDirection() const;

    bool usePlateSizes() const;

    bool useProjection() const;

    bool isSensorCamera() const;

    bool useSubMask() const;

    int platesCount() const;

    int timeBetweenSentPlates() const;


private:
    std::string CAMERA_IP, RTSP_URL, RESULT_SEND_URL, NODE_IP, SNAPSHOT_SEND_IP, SECONDARY_RESULT_SEND_URL, ZMQ_SERVER_IP, SECONDARY_SNAPSHOT_SEND_IP;
    bool CAR_MODEL_RECOGNITION_ENABLED, USE_DIRECTION, USE_PROJECTION, USE_SEND_DIRECTION, USE_PLATE_SIZES, USE_SECONDARY_RESULT_SEND_URL, USE_SECONDARY_SNAPSHOT_IP, IS_SENSOR, USE_SUB_MASK_2;
    int PLATES_COUNT, TIME_BETWEEN_SENT_PLATES;
};