#include "Config.h"
#include "app/CameraScope.h"

using namespace std;
using json = nlohmann::json;

std::vector<CameraScope> cameras;
std::vector<CameraScope> streamCameras;
std::vector<CameraScope> sensorCameras;
bool useDecodeGpu = false;
bool useS3Storage = false;

int timeBetweenSnapshotSend = 1;

float recognizerThreshold = 0.95;
float detectorThreshold = 0.85;
float calibrationWidth = 1920;
float calibrationHeight = 1080;

string s3AccessKey;
string s3SecretKey;
string placement;
string objectName;
string S3URL;

bool Config::parseJson(const string &fileName) {
    try {
        ifstream configFile(fileName);
        if (!configFile.is_open())
            throw runtime_error("Config file not found");

        json configs = json::parse(configFile);

        if (configs.find("cameras") == configs.end())
            throw runtime_error("Camera IP Entities not defined");
        auto cameraEntities = configs["cameras"];


        for (const auto &cameraEntity: cameraEntities) {
            bool useSubMaskFlag, isSensor, doesSecondaryNodeIpDefined, doesSecondarySnapshotIpDefined;
            std::string nodeIp, snapshotSendIp, zmqServerIp, resultSendIp, secondaryResultSendIp, secondarySnapshotIp;

            bool car_model_recognition, use_direction, use_send_direction, use_projection, use_plate_size;
            int time_between_sent_plates, plates_count;

            if (cameraEntity.find("client_type") == cameraEntity.end())
                throw runtime_error("You asshole, please put the type of the client");

            if (cameraEntity["client_type"].get<string>() == "sensor") {
                isSensor = true;
                if (cameraEntity.find("zmq_server_ip") == cameraEntity.end())
                    throw runtime_error("zmq server ip not defined");

                zmqServerIp = cameraEntity["zmq_server_ip"].get<string>();
                useSubMaskFlag = false;
                nodeIp = "";
                snapshotSendIp = "";
                secondarySnapshotIp = "";
                doesSecondarySnapshotIpDefined = false;
                car_model_recognition = false;
                use_direction = false;
                use_send_direction = false;
                time_between_sent_plates = 0;
                use_projection = false;
                plates_count = 0;
                use_plate_size = false;


            } else {
                isSensor = false;
                if (cameraEntity.find("node_ip") == cameraEntity.end())
                    throw runtime_error("node ip not defined");
                nodeIp = cameraEntity["node_ip"].get<string>();
                if (cameraEntity.find("snapshot_send_ip") == cameraEntity.end())
                    throw runtime_error("Snapshot url not defined");
                else
                    snapshotSendIp = cameraEntity["snapshot_send_ip"].get<string>();
                if (cameraEntity.find("secondary_snapshot_send_ip") == cameraEntity.end()) {
                    doesSecondarySnapshotIpDefined = false;
                } else {
                    doesSecondarySnapshotIpDefined = true;
                    secondarySnapshotIp = cameraEntity["secondary_snapshot_send_ip"].get<string>();
                }
                if (cameraEntity.find("use_mask_2") == cameraEntity.end() || cameraEntity["use_mask_2"].get<int>() == 1)
                    useSubMaskFlag = true;
                else
                    useSubMaskFlag = false;
                zmqServerIp = "";

                car_model_recognition = cameraEntity["car_model_recognition"].get<int>();
                use_direction = cameraEntity["use_direction"].get<int>();
                use_send_direction = cameraEntity["use_send_direction"].get<int>();
                time_between_sent_plates = cameraEntity["time_between_sent_plates"].get<int>();
                use_projection = cameraEntity["use_projection"].get<int>();
                plates_count = cameraEntity["plates_count"].get<int>();
                use_plate_size = cameraEntity["use_plate_size"].get<int>();

            }

            if (cameraEntity.find("result_send_endpoint") == cameraEntity.end())
                throw runtime_error("result_send_endpoint not defined");

            resultSendIp = cameraEntity["result_send_endpoint"].get<string>();
            if (cameraEntity.find("secondary_result_send_endpoint") == cameraEntity.end()) {
                doesSecondaryNodeIpDefined = false;
            } else {
                doesSecondaryNodeIpDefined = true;
                secondaryResultSendIp = cameraEntity["secondary_result_send_endpoint"].get<string>();
            }


            auto rtspUrl = cameraEntity["rtsp_url"].get<string>();
            auto cameraIp = cameraEntity["camera_ip"].get<string>();

            auto cameraScope = CameraScope(cameraIp, rtspUrl, car_model_recognition,
                                           use_direction, use_send_direction, use_projection,
                                           plates_count, time_between_sent_plates, use_plate_size, resultSendIp, nodeIp,
                                           snapshotSendIp, secondaryResultSendIp,
                                           doesSecondaryNodeIpDefined, secondarySnapshotIp,
                                           doesSecondarySnapshotIpDefined, isSensor, zmqServerIp, useSubMaskFlag);
            cameras.push_back(cameraScope);

            if (cameraScope.isSensorCamera())
                sensorCameras.push_back(cameraScope);
            else
                streamCameras.push_back(cameraScope);
        }

        if (configs.find("use_gpu_decode") == configs.end() || configs["use_gpu_decode"].get<int>() == 0)
            useDecodeGpu = false;
        else
            useDecodeGpu = true;

        if (configs.find("calibration_width") == configs.end())
            calibrationWidth = 1920.0;
        else
            calibrationWidth = configs["calibration_width"].get<float>();

        if (configs.find("calibration_height") == configs.end())
            calibrationHeight = 1080.0;
        else
            calibrationHeight = configs["calibration_height"].get<float>();

        if (configs.find("time_between_snapshot_send") == configs.end())
            timeBetweenSnapshotSend = 1;
        else
            timeBetweenSnapshotSend = configs["time_between_snapshot_send"].get<int>();

        if (configs.find("rec_threshold") == configs.end())
            recognizerThreshold = 0.95;
        else
            recognizerThreshold = configs["rec_threshold"].get<float>();

        if (configs.find("det_threshold") == configs.end())
            detectorThreshold = 0.8;
        else
            detectorThreshold = configs["det_threshold"].get<float>();


        if (configs.find("s3") != configs.end()) {
            auto finder = configs.find("s3");
            if (finder->find("accessKey") != finder->end() && finder->find("secretKey") != finder->end() &&
                finder->find("placement") != finder->end() && finder->find("objectName") != finder->end() &&
                finder->find("s3url") != finder->end()) {
                useS3Storage = true;
                s3AccessKey = configs["s3"]["accessKey"];
                s3SecretKey = configs["s3"]["secretKey"];
                placement = configs["s3"]["placement"];
                objectName = configs["s3"]["objectName"];
                S3URL = configs["s3"]["s3url"];
            }
        }
    } catch (exception &e) {
        cout << "Exception occurred during config parse: " << e.what() << endl;
        return false;
    }
    return true;
}

const std::vector<CameraScope> &Config::getCameras() {
    return cameras;
}

bool Config::useGPUDecode() {
    return useDecodeGpu;
}

const float &Config::getRecognizerThreshold() {
    return recognizerThreshold;
}

const float &Config::getDetectorThreshold() {
    return detectorThreshold;
}

const std::string &Config::getS3SecretKey() {
    return s3SecretKey;
}

const std::string &Config::getS3AccessKey() {
    return s3AccessKey;
}

const std::string &Config::getS3Placement() {
    return placement;
}

const std::string &Config::getS3ObjectName() {
    return objectName;
}

const std::string &Config::getS3URL() {
    return S3URL;
}

bool Config::useS3() {
    return useS3Storage;
}

const int &Config::getTimeSentSnapshots() {
    return timeBetweenSnapshotSend;
}

std::pair<float, float> Config::getCalibrationSizes() {
    return std::pair<float, float>{calibrationWidth, calibrationHeight};
}

const std::vector<CameraScope> &Config::getSensorCameras() {
    return sensorCameras;
}

const std::vector<CameraScope> &Config::getStreamCameras() {
    return streamCameras;
}

























