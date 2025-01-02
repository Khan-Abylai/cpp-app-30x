//
// Created by artkbvk on 8/16/21.
//

#include "ResnetEngine.h"

using namespace std;
using namespace nvinfer1;

ResnetEngine::ResnetEngine() {
    createEngine();
}


nvinfer1::ITensor *
ResnetEngine::batchNorm(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                        vector<float> &weights, int channels) {
    for (int channel = 0; channel < channels; channel++) {

        weights[index + channel] /= sqrt(weights[index + channels * 3 + channel] + 1e-5);

        weights[index + channels + channel] -=
                weights[index + channels * 2 + channel] * weights[index + channel];

        weights[index + channels * 2 + channel] = 1.0;
    }

    auto layerScale = Weights{DataType::kFLOAT, &weights[index], channels};
    index += channels;

    auto layerBias = Weights{DataType::kFLOAT, &weights[index], channels};
    index += channels;

    auto layerPower = Weights{DataType::kFLOAT, &weights[index], channels};
    index += 2 * channels;

    auto scaleLayer = network->addScale(*inputTensor, ScaleMode::kCHANNEL,
                                        layerBias, layerScale, layerPower);
    return scaleLayer->getOutput(0);
}

nvinfer1::ITensor *
ResnetEngine::convBnRelu(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                         vector<float> &weights, int inChannels, int outChannels, int kernelSize, int stride,
                         int padding, int numGroups, bool useRelu, bool useBatchNorm, bool useBias) {
    DimsHW kernel{kernelSize, kernelSize};
    DimsHW strideSize{stride, stride};
    DimsHW paddingSize{padding, padding};

    int convWeightsCount = inChannels * outChannels * kernelSize * kernelSize / numGroups;
    auto convWeights = Weights{DataType::kFLOAT, &weights[index], convWeightsCount};

    index += convWeightsCount;

    auto convBias = Weights{DataType::kFLOAT, nullptr, 0};
    if (useBias) {
        convBias = Weights{DataType::kFLOAT, &weights[index], outChannels};
        index += outChannels;
    }
    auto convLayer = network->addConvolution(*inputTensor, outChannels, kernel, convWeights, convBias);

    convLayer->setStride(strideSize);
    convLayer->setPadding(paddingSize);
    convLayer->setNbGroups(numGroups);

    auto batchLayer = convLayer->getOutput(0);
    if (useBatchNorm) {
        batchLayer = batchNorm(batchLayer, network, index, weights, outChannels);
    }

    if (useRelu) {
        return network->addActivation(*batchLayer, ActivationType::kRELU)->getOutput(0);
    }
    return batchLayer;
}

nvinfer1::ITensor *
ResnetEngine::bottleneckX(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                          vector<float> &weights, int inplanes, int planes, int cardinality, int stride,
                          bool downsample) {
    auto residualLayer = inputTensor;
    auto out = convBnRelu(inputTensor, network, index, weights, inplanes, planes * 2, 1, 1, 0, 1, true, true, false);

    out = convBnRelu(out, network, index, weights, planes * 2, planes * 2, 3, stride, 1, cardinality, true, true,
                     false);

    out = convBnRelu(out, network, index, weights, planes * 2, planes * 4, 1, 1, 0, 1, false, true, false);


    out = selayer(out, network, index, weights, planes * 4);

    if (downsample) {
        residualLayer = convBnRelu(inputTensor, network, index, weights, inplanes, planes * EXPANSION, 1, stride, 0,
                                   1, false, true, false);
    }

    out = network->addElementWise(*out, *residualLayer, ElementWiseOperation::kSUM)->getOutput(0);

    out = network->addActivation(*out, ActivationType::kRELU)->getOutput(0);

    return out;
}

nvinfer1::ITensor *
ResnetEngine::selayer(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                      vector<float> &weights, int inplanes) {
    int width = inputTensor->getDimensions().d[1];

    auto avgPoolLayer = network->addPooling(*inputTensor, PoolingType::kAVERAGE, DimsHW{width, width});

    auto out = convBnRelu(avgPoolLayer->getOutput(0), network, index, weights, inplanes, inplanes / 16, 1, 1, 0, 1,
                          true, false, true);

    out = convBnRelu(out, network, index, weights, inplanes / 16, inplanes, 1, 1, 0, 1, false, false, true);

    out = network->addActivation(*out, ActivationType::kSIGMOID)->getOutput(0);

    auto residualProdLayer = network->addElementWise(*out, *inputTensor, ElementWiseOperation::kPROD);

    return residualProdLayer->getOutput(0);
}

nvinfer1::ITensor *
ResnetEngine::makelayer(nvinfer1::ITensor *inputTensor, nvinfer1::INetworkDefinition *network, int &index,
                        vector<float> &weights, int planes, int blocks, int stride) {
    auto out = bottleneckX(inputTensor, network, index, weights, this->inputPlanes, planes, CARDINALITY, stride, true);
    this->inputPlanes = planes * EXPANSION;
    for (int i = 1; i < blocks; i++) {
        out = bottleneckX(out, network, index, weights, this->inputPlanes, planes, CARDINALITY, 1, false);
    }

    return out;
}

void ResnetEngine::createEngine() {
    auto builder = unique_ptr<IBuilder, TensorRTDeleter>(createInferBuilder(TensorRTEngine::getLogger()),
                                                         TensorRTDeleter());
    vector<float> weights;
    ifstream weightFile(WEIGHTS_FILENAME, ios::binary);

    float parameter;

    weights.reserve(weightFile.tellg() / 4); // char to float

    while (weightFile.read(reinterpret_cast<char *>(&parameter), sizeof(float))) {
        weights.push_back(parameter);
    }


    auto network = unique_ptr<INetworkDefinition, TensorRTDeleter>(
            builder->createNetworkV2(0), TensorRTDeleter());

    ITensor *inputLayer = network->addInput(NETWORK_INPUT_NAME.c_str(), DataType::kFLOAT,
                                            Dims3{IMG_HEIGHT, IMG_WIDTH, IMG_CHANNELS});

    IShuffleLayer *shuffle = network->addShuffle(*inputLayer);
    shuffle->setFirstTranspose(Permutation{2, 1, 0});
    shuffle->setReshapeDimensions(Dims3{IMG_CHANNELS, IMG_WIDTH, IMG_HEIGHT});

    float initialScale = 1.f / 255;
    float initialBias = 0;
    float initialPower = 1;
    auto scaleLayer = network->addScale(*shuffle->getOutput(0),
                                        ScaleMode::kUNIFORM,
                                        Weights{DataType::kFLOAT, &initialBias, 1},
                                        Weights{DataType::kFLOAT, &initialScale, 1},
                                        Weights{DataType::kFLOAT, &initialPower, 1});

    int index = 0;
    vector<int> layers{3, 4, 6, 3};
    vector<int> channels{64, 128, 256, 512};
    vector<int> strides{1, 2, 2, 2};

    auto prevLayer = convBnRelu(scaleLayer->getOutput(0), network.get(), index, weights, IMG_CHANNELS,
                                channels[0], 7, 2, 3, 1, true, true, false);
    auto poolLayer = network->addPooling(*prevLayer, PoolingType::kMAX, DimsHW{3, 3});
    poolLayer->setStride(DimsHW(2, 2));
    poolLayer->setPadding(DimsHW(1, 1));
    prevLayer = poolLayer->getOutput(0);

    for (int i = 0; i < layers.size(); i++) {
        prevLayer = makelayer(prevLayer, network.get(), index, weights, channels[i], layers[i], strides[i]);
    }

    int width = prevLayer->getDimensions().d[1];
    prevLayer = network->addPooling(*prevLayer, PoolingType::kAVERAGE, DimsHW{width, width})->getOutput(0);

    int fcInChannels = prevLayer->getDimensions().d[0];
    auto fcWeights = Weights{DataType::kFLOAT, &weights[index], fcInChannels * OUTPUT_SIZE};
    index += fcInChannels * OUTPUT_SIZE;
    auto fcBiases = Weights{DataType::kFLOAT, &weights[index], OUTPUT_SIZE};
    prevLayer = network->addFullyConnected(*prevLayer, OUTPUT_SIZE, fcWeights, fcBiases)->getOutput(0);

    network->markOutput(*prevLayer);

    unique_ptr<IBuilderConfig, TensorRTDeleter> builderConfig(builder->createBuilderConfig(), TensorRTDeleter());

    builder->setMaxBatchSize(MAX_BATCH_SIZE);
    builderConfig->setMaxWorkspaceSize(1 << 30);

    engine = builder->buildEngineWithConfig(*network, *builderConfig);

}

nvinfer1::IExecutionContext *ResnetEngine::createExecutionContext() {
    return engine->createExecutionContext();
}