//
// Created by kartykbayev on 4/5/22.
//

#include "CarModelRecognizer.h"

using namespace std;
using namespace nvinfer1;


CarModelRecognizer::CarModelRecognizer(nvinfer1::IExecutionContext *executionContext) : ILogger(
        "Car Model Classifier: ") {
    cudaMalloc(&cudaBuffer[0], INPUT_SIZE * sizeof(float));
    cudaMalloc(&cudaBuffer[1], OUTPUT_SIZE * sizeof(float));

    cudaStreamCreate(&stream);

    this->executionContext = executionContext;
}

CarModelRecognizer::~CarModelRecognizer() {
    cudaFree(cudaBuffer[0]);
    cudaFree(cudaBuffer[1]);

    cudaStreamDestroy(stream);

}

std::string CarModelRecognizer::classify(const std::shared_ptr<LicensePlate> &licensePlate) {
    cv::Mat frame = licensePlate->getCarImage();
    cv::Mat resizedFrame;

    auto centerPoint = licensePlate->getCenter();
    int y_0 = centerPoint.y - 700 < 0 ? 0 : centerPoint.y - 700;
    int x_0 = centerPoint.x - 300 < 0 ? 0 : centerPoint.x - 300;
    int y_1 = centerPoint.y + 200 > frame.rows ? frame.rows - 1 : centerPoint.y + 200;
    int x_1 = centerPoint.x + 600 > frame.cols ? frame.cols - 1 : centerPoint.x + 600;

    cv::Mat cropped = frame(cv::Range(y_0, y_1),
                            cv::Range(x_0, x_1));


    cv::Mat resized_frame;
    cv::resize(cropped, resized_frame, cv::Size(IMG_WIDTH, IMG_HEIGHT));
    if (DEBUG && IMSHOW) {
        cv::imshow(licensePlate->getCameraIp() + "_car_model", cropped);
        cv::waitKey(60);
    }


    resized_frame.convertTo(resized_frame, CV_32F);

    vector<float> out;
    out.resize(OUTPUT_SIZE, 0.0);
    {
        cudaMemcpyAsync(cudaBuffer[0], resized_frame.data, MAX_BATCH_SIZE * INPUT_SIZE * sizeof(float),
                        cudaMemcpyHostToDevice, stream);

        executionContext->enqueue(MAX_BATCH_SIZE, cudaBuffer, stream, nullptr);


        cudaMemcpyAsync(out.data(), cudaBuffer[1], MAX_BATCH_SIZE * OUTPUT_SIZE * sizeof(float),
                        cudaMemcpyDeviceToHost, stream);

        cudaStreamSynchronize(stream);
    }


    vector<float> carModelOut;
    carModelOut.resize(CAR_MODEL_OUTPUT_SIZE, 0.0);

    for (int i = 0; i < CAR_MODEL_OUTPUT_SIZE; ++i) {
        carModelOut[i] = out[i];
    }

    vector<float> resultSoftmax = softmax(carModelOut);

    int carModelInd = 0;
    for (int i = 1; i < resultSoftmax.size(); i++) {
        if (resultSoftmax[i] > resultSoftmax[carModelInd]) carModelInd = i;
    }
//    int  carColorInd = CAR_MODEL_OUTPUT_SIZE;
//
//    for (int i = CAR_MODEL_OUTPUT_SIZE; i < OUTPUT_SIZE - 1; i++) {
//        if (out[i] > out[carColorInd]) carColorInd = i;
//    }
//    carColorInd -= CAR_MODEL_OUTPUT_SIZE;

//    licensePlate->setSmallCarImage(cropped);`
//    licensePlate->setCarModel(Labels::CAR_MODEL_LABELS[carModelInd]);
    string car_type = Labels::CAR_MODEL_LABELS[carModelInd];
    float prob = resultSoftmax[carModelInd];

    if(DEBUG)
        LOG_INFO("prob: %f car_type: %s", prob, car_type.c_str());

    if (prob >= CAR_MODEL_RECOGNITION_THRESHOLD)
        return car_type;
    else
        return "NotDefined";
}


std::vector<float> CarModelRecognizer::softmax(vector<float> &score_vec) {
    vector<float> softmax_vec(score_vec.size());
    double score_max = *(max_element(score_vec.begin(), score_vec.end()));
    double e_sum = 0;
    for (int j = 0; j < score_vec.size(); j++) {
        softmax_vec[j] = exp((double) score_vec[j] - score_max);
        e_sum += softmax_vec[j];
    }
    for (int k = 0; k < score_vec.size(); k++)
        softmax_vec[k] /= e_sum;
    return softmax_vec;
}
