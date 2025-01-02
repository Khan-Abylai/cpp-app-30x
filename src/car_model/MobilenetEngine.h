#pragma once

#include <memory>
#include <vector>
#include <cmath>
#include <fstream>

#include "NvInfer.h"
#include "../app/TensorRTDeleter.h"
#include "../app/TrtLogger.h"
#include "../app/TensorRTEngine.h"
#include "../app/Constants.h"


class MobilenetEngine {

public:
    MobilenetEngine();

    nvinfer1::IExecutionContext *createExecutionContext();

private:
    void createEngine();

    nvinfer1::ITensor *batchNorm(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network,
                                 int &index,
                                 std::vector<float> &weights,
                                 int channels);

    nvinfer1::ITensor *convBnRelu(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network,
                                  int &index, std::vector<float> &weights, int inChannels, int outChannels,
                                  int kernelSize, int strideStep, int numGroups);

    nvinfer1::ITensor *invertedResidual(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network,
                                        int &index, std::vector<float> &weights,
                                        int inChannels, int outChannels, int expandRatio, int strideStep);


    nvinfer1::ICudaEngine *engine = nullptr;

    const int
            MAX_BATCH_SIZE = Constants::CAR_MODEL_BATCH_SIZE,
            IMG_WIDTH = Constants::CAR_MODEL_IMG_WIDTH,
            IMG_HEIGHT = Constants::CAR_MODEL_IMG_HEIGHT,
            IMG_CHANNELS = Constants::IMG_CHANNELS,
            OUTPUT_SIZE = Constants::CAR_CLASSIFIER_OUTPUT_SIZE;

    const std::string NETWORK_INPUT_NAME = "INPUT",
            ENGINE_NAME = "mobilenet.engine",
            WEIGHTS_FILENAME = "car_classificator_weights.np";
};
