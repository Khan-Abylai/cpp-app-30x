//
// Created by artyk on 7/20/2022.
//

#include "S3Scope.h"

#include <utility>

S3Scope::S3Scope(std::string accessKey, std::string secretKey, std::string placement, std::string objectName,
                 std::string url) {
    this->objectName = std::move(objectName);
    this->placement = std::move(placement);
    this->accessKey = std::move(accessKey);
    this->secretKey = std::move(secretKey);
    this->s3url = std::move(url);
}

const std::string &S3Scope::getAccessKey() {
    return accessKey;
}

const std::string &S3Scope::getSecretKey() {
    return secretKey;
}

const std::string &S3Scope::getPlacement() {
    return placement;
}

const std::string &S3Scope::getObjectName() {
    return objectName;
}

const std::string &S3Scope::getS3URL() {
    return s3url;
}
