#pragma once

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

class S3Scope {
public:
    S3Scope(std::string accessKey, std::string secretKey, std::string placement, std::string objectName,
            std::string url);

    const std::string &getAccessKey();

    const std::string &getSecretKey();

    const std::string &getPlacement();

    const std::string &getObjectName();

    const std::string &getS3URL();

private:
    std::string accessKey, secretKey, placement, objectName, s3url;
};
