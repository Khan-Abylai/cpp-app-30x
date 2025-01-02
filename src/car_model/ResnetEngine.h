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


class ResnetEngine {

public:
    ResnetEngine();

    nvinfer1::IExecutionContext *createExecutionContext();

private:
    void createEngine();

    nvinfer1::ITensor *batchNorm(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                                 std::vector<float> &weights, int channels);

    nvinfer1::ITensor *convBnRelu(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                                  std::vector<float> &weights, int inChannels, int outChannels, int kernelSize,
                                  int stride, int padding, int numGroups, bool useRelu, bool useBatchNorm,
                                  bool useBias);

    nvinfer1::ITensor *bottleneckX(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                                   std::vector<float> &weights, int inplanes, int planes, int cardinality, int stride,
                                   bool downsample);

    nvinfer1::ITensor *selayer(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                               std::vector<float> &weights, int inplanes);

    nvinfer1::ITensor *makelayer(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                                 std::vector<float> &weights, int planes, int blocks, int stride);

    int inputPlanes = 64;


    nvinfer1::ICudaEngine *engine = nullptr;

    const int
            MAX_BATCH_SIZE = Constants::CAR_MODEL_BATCH_SIZE,
            IMG_WIDTH = Constants::CAR_MODEL_IMG_WIDTH,
            IMG_HEIGHT = Constants::CAR_MODEL_IMG_HEIGHT,
            IMG_CHANNELS = Constants::IMG_CHANNELS,
            EXPANSION = 4,
            CARDINALITY = 32,
            OUTPUT_SIZE = Constants::CAR_CLASSIFIER_OUTPUT_SIZE;

    const std::string NETWORK_INPUT_NAME = "INPUT",
            WEIGHTS_FILENAME = "models/se_resnext50.np";
};