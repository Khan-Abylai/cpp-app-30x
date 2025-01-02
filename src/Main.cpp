#include <thread>
#include <chrono>
#include <csignal>
#include <condition_variable>
#include <mutex>

#include "client/SensorClientLauncher.h"
#include "client/CameraClientLauncher.h"
#include "Config.h"
#include "SharedQueue.h"
#include "app/LPRecognizerService.h"
#include "app/DetectionService.h"
#include "app/PackageSender.h"
#include "app/Package.h"
#include "app/S3Scope.h"
#include "app/Snapshot.h"
#include "app/SnapshotSender.h"

using namespace std;

//TODO: remove all vehicle detection
atomic<bool> shutdownFlag = false;
condition_variable shutdownEvent;
mutex shutdownMutex;

void signalHandler(int signum) {
    cout << "signal is to shutdown" << endl;
    shutdownFlag = true;
    shutdownEvent.notify_all();
}

int main(int argc, char *argv[]) {

    char env[] = "CUDA_MODULE_LOADING=LAZY";
    putenv(env);

    if (IMSHOW) {
        char env[] = "DISPLAY=:0";
        putenv(env);
    }

    string configFileName;

    if (argc <= 1)
        configFileName = "config.json";
    else
        configFileName = argv[1];

    if (!Config::parseJson(configFileName))
        return -1;

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGABRT, signalHandler);

    auto packageQueue = make_shared<SharedQueue<shared_ptr<Package>>>();
    auto snapshotQueue = make_shared<SharedQueue<shared_ptr<Snapshot>>>();
    vector<shared_ptr<IThreadLauncher>> services;
    auto cameras = Config::getCameras();
    vector<shared_ptr<SharedQueue<unique_ptr<FrameData>>>> frameQueues;

    auto detectionEngine = make_shared<DetectionEngine>();
    auto lpRecognizerService = make_shared<LPRecognizerService>(packageQueue, cameras,
                                                                Config::getRecognizerThreshold(),
                                                                Config::getCalibrationSizes());
    services.emplace_back(lpRecognizerService);
    for (const auto &camera: cameras) {
        auto frameQueue = make_shared<SharedQueue<unique_ptr<FrameData>>>();
        auto detectionService = make_shared<DetectionService>(
                frameQueue, snapshotQueue, lpRecognizerService,
                make_shared<Detection>(detectionEngine->createExecutionContext(), Config::getDetectorThreshold()),
                camera.getCameraIp(), camera.getNodeIP(), camera.useSubMask(), camera.useProjection(),
                Config::getTimeSentSnapshots(),
                Config::getCalibrationSizes()
        );
        services.emplace_back(detectionService);
        frameQueues.push_back(std::move(frameQueue));
    }


    //TODO: check the snapshot sender
    if (!Config::getSensorCameras().empty()) {
        shared_ptr<IThreadLauncher> clientStarter;
        clientStarter = make_shared<SensorClientLauncher>(Config::getSensorCameras(), frameQueues,
                                                          Config::useGPUDecode());
        services.emplace_back(clientStarter);
    }

    if (!Config::getStreamCameras().empty()) {
        shared_ptr<IThreadLauncher> clientStarter;
        clientStarter = make_shared<CameraClientLauncher>(Config::getStreamCameras(), frameQueues,
                                                          Config::useGPUDecode());
        services.emplace_back(clientStarter);
    }

    bool useS3 = Config::useS3();
    shared_ptr<S3Scope> s3Scope;

    if (useS3) {
        string accesskey = Config::getS3AccessKey();
        string secretKey = Config::getS3SecretKey();
        string placement = Config::getS3Placement();
        string objectName = Config::getS3ObjectName();
        string s3url = Config::getS3URL();
        s3Scope = make_shared<S3Scope>(accesskey, secretKey,
                                       placement,
                                       objectName, s3url);
    } else {
        s3Scope = nullptr;
    }


    auto packageSender = make_shared<PackageSender>(packageQueue, Config::getCameras(),
                                                    s3Scope);
    services.emplace_back(packageSender);


    auto snapshotSender = make_shared<SnapshotSender>(snapshotQueue, Config::getCameras());

    services.emplace_back(snapshotSender);


    vector<thread> threads;
    for (const auto &service: services) {
        threads.emplace_back(thread(&IThreadLauncher::run, service));
    }

    unique_lock<mutex> shutdownLock(shutdownMutex);
    while (!shutdownFlag) {
        auto timeout = chrono::hours(24);
        if (shutdownEvent.wait_for(shutdownLock, timeout, [] { return shutdownFlag.load(); })) {
            cout << "shutting all services" << endl;
        }
    }

    for (int i = 0; i < services.size(); i++) {
        services[i]->shutdown();
        if (threads[i].joinable())
            threads[i].join();
    }
}
