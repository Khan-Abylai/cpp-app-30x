#pragma once

#include <unordered_set>

#include "LicensePlate.h"
#include "Utils.h"
#include "Counter.h"
#include "../ILogger.h"

enum class Directions {
    initial = 0,
    forward = 1,
    reverse = -1
};

struct CounterHashFunction {
    struct Hash {
        std::size_t operator()(std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>> const &p) const noexcept {
            return std::hash<std::string>{}(p->getItem()->getPlateLabel());
        }
    };

    struct Compare {
        size_t operator()(std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>> const &a,
                          std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>> const &b) const {
            return a->getItem()->getPlateLabel() == b->getItem()->getPlateLabel();
        }
    };
};

class Car : public ILogger {
public:
    explicit Car(const int &platesCount);

    void addLicensePlateToCount(std::shared_ptr<LicensePlate> licensePlate);

    const std::shared_ptr<LicensePlate> &getMostCommonLicensePlate() const;

    bool isLicensePlateBelongsToCar(const std::string &otherPlateLabel);

    bool isLicensePlateBelongsToCar(const std::shared_ptr<LicensePlate> &curLicensePlate,
                                    double lastFrameRTPTimestamp);

    void drawTrackingPoints(cv::Mat &image);

    static void drawBoundingBoxPoints(cv::Mat &image, const std::shared_ptr<LicensePlate> &licensePlate);

    void addTrackingPoint(cv::Point2f point);

    bool isTrackPoint(const cv::Point2f &centerPoint);

    bool doesPlatesCollected();

    void addTrackingCarModel(std::string &carModel);

    std::string getCarModel();

    std::pair<std::string, int> getCarModelWithOccurrence();

    int getSizeOfCarModelVector() { return trackingCarModels.size(); }

    Directions getDirection();

private:
    Directions direction = Directions::initial;
    const int MIN_DISTANCE_BETWEEN_POINTS = 8;
    const int MAX_DIFFERENCE_BETWEEN_TRACKING_POINTS = 2;
    const float MAX_DIFFERENCE_BETWEEN_TIMESTAMPS = 1.0;
    static const int EDIT_DISTANCE_THRESHOLD = 1;
    static const int TRACK_POINTS_THRESHOLD = 5;
    int MIN_NUM_PLATES_COLLECTED;

    std::unordered_set<std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>>,
            CounterHashFunction::Hash, CounterHashFunction::Compare> licensePlates;

    std::shared_ptr<Counter<std::shared_ptr<LicensePlate>>> mostCommonPlate;
    std::vector<cv::Point2f> trackingPoints;
    std::map<std::string, int> trackingCarModels;

    static std::pair<std::string, int> findEntryWithLargestValue(std::map<std::string, int> &sampleMap) {
        std::pair<std::string, int> entryWithMaxValue = std::make_pair("", 0);

        std::map<std::string, int>::iterator currentEntry;
        for (currentEntry = sampleMap.begin();
             currentEntry != sampleMap.end();
             ++currentEntry) {
            if (currentEntry->second > entryWithMaxValue.second) {
                entryWithMaxValue = std::make_pair(
                        currentEntry->first,
                        currentEntry->second);
            }
        }
        return entryWithMaxValue;
    }

};
