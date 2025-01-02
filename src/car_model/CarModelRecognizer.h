#pragma once

#include <cuda.h>
#include <cuda_runtime.h>
#include "opencv2/opencv.hpp"
#include "NvInfer.h"
#include "../app/Constants.h"
#include "Labels.h"
#include "../ILogger.h"
#include "../app/TrtLogger.h"
#include "../Profile.h"
#include "../ILogger.h"
#include "../app/LicensePlate.h"

class CarModelRecognizer : public ::ILogger {
public:
    explicit CarModelRecognizer(nvinfer1::IExecutionContext *executionContext);

    ~CarModelRecognizer();

    std::string classify(const std::shared_ptr<LicensePlate> &licensePlate);

    std::string classify2(const std::shared_ptr<LicensePlate> &licensePlate);

private:
    const int
            MAX_BATCH_SIZE = Constants::CAR_MODEL_BATCH_SIZE,
            IMG_WIDTH = Constants::CAR_MODEL_IMG_WIDTH,
            IMG_HEIGHT = Constants::CAR_MODEL_IMG_HEIGHT,
            IMG_CHANNELS = Constants::IMG_CHANNELS,
            INPUT_SIZE = IMG_CHANNELS * IMG_HEIGHT * IMG_WIDTH,
            OUTPUT_SIZE = Constants::CAR_CLASSIFIER_OUTPUT_SIZE,
            OUTPUT_SIZE_2 = Constants::CAR_MODEL_OUTPUT_2_SIZE,
            CAR_MODEL_OUTPUT_SIZE = Constants::CAR_MODEL_OUTPUT_SIZE;
    void *cudaBuffer[2];
    nvinfer1::IExecutionContext *executionContext;
    cudaStream_t stream;
    float CAR_MODEL_RECOGNITION_THRESHOLD = 0.6;

    std::vector<float> softmax(std::vector<float> &score_vec);
};