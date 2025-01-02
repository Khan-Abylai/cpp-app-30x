#pragma once

#include <string>

#include <opencv2/opencv.hpp>

namespace Constants {

    const std::string UNIDENTIFIED_COUNTRY;
    const std::string JPG_EXTENSION = ".jpg";
    const std::string IMAGE_DIRECTORY = "./images/";

    const std::string modelWeightsFolder = "./models/";

    const int DETECTION_IMG_W = 512;
    const int DETECTION_IMG_H = 512;
    const int IMG_CHANNELS = 3;
    const int PLATE_COORDINATE_SIZE = 13;
    const int DETECTION_BATCH_SIZE = 1;

    const int RECT_LP_H = 32;
    const int RECT_LP_W = 128;

    const int SQUARE_LP_H = 64;
    const int SQUARE_LP_W = 64;

    const int RECOGNIZER_MAX_BATCH_SIZE = 4;

    constexpr float PIXEL_MAX_VALUE = 255;

    const std::vector<cv::Point2f> RECT_LP_COORS{
            cv::Point2f(0, 0),
            cv::Point2f(0, 31),
            cv::Point2f(127, 0),
            cv::Point2f(127, 31),
    };

    const std::vector<cv::Point2f> SQUARE_LP_COORS{
            cv::Point2f(0, 0),
            cv::Point2f(0, 63),
            cv::Point2f(63, 0),
            cv::Point2f(63, 63),
    };

    const int CAR_MODEL_IMG_WIDTH = 320;
    const int CAR_MODEL_IMG_HEIGHT = 320;
    const int CAR_MODEL_BATCH_SIZE = 1;
    const int CAR_MODEL_OUTPUT_SIZE = 417;
    const int CAR_MODEL_OUTPUT_2_SIZE = 4;
    const int CAR_COLOR_OUTPUT_SIZE = 14;
    const int CAR_CLASSIFIER_OUTPUT_SIZE = CAR_MODEL_OUTPUT_SIZE + CAR_COLOR_OUTPUT_SIZE;
}

