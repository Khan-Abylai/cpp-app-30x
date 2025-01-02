cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_COMPILER /usr/bin/gcc-11)
set(CMAKE_CXX_COMPILER /usr/bin/g++-11)
project(parking)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(OpenCV REQUIRED)
find_package(CUDA REQUIRED)
find_package(PkgConfig)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

pkg_check_modules(ZMQ REQUIRED libzmq)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST_RTP REQUIRED gstreamer-rtp-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)

include_directories(${ZMQ_INCLUDE_DIRS})
include_directories("${CUDA_INCLUDE_DIRS}")
include_directories(${CURL_INCLUDE_DIR})
include_directories(${GST_INCLUDE_DIRS} ${GST_APP_INCLUDE_DIRS} ${GST_RTP_INCLUDE_DIRS})

option(DEBUG_LOG "Debug Mode" OFF)
option(IMSHOW "Show Images" OFF)
option(IMSAVE "Save Images" OFF)
option(MORE_LOGS "Log tracking points" OFF)

if (DEBUG_LOG)
    add_definitions(-DDEBUG=1)
else ()
    add_definitions(-DDEBUG=0)
endif ()

if (IMSHOW)
    add_definitions(-DIMSHOW=1)
else ()
    add_definitions(-DIMSHOW=0)
endif ()

if (MORE_LOGS)
    add_definitions(-DMORE_LOGS=1)
else ()
    add_definitions(-DMORE_LOGS=0)
endif ()

if (IMSAVE)
    add_definitions(-DIMSAVE=1)
else ()
    add_definitions(-DIMSAVE=0)
endif ()

add_executable(parking
        src/Main.cpp
        src/zeromq/zmq.hpp
        src/zeromq/zmq_addon.hpp
        src/Config.cpp
        src/Config.h
        src/app/Utils.cpp
        src/app/Utils.h
        src/SharedQueue.h
        src/app/Constants.h
        src/ILogger.cpp
        src/ILogger.h
        src/IThreadLauncher.h
        src/ITimer.h
        src/Profile.h
        src/RandomStringGenerator.cpp
        src/RandomStringGenerator.h
        src/app/LicensePlate.h
        src/app/LicensePlate.cpp
        src/app/Package.cpp
        src/app/Package.h
        src/client/FrameData.cpp
        src/client/FrameData.h
        src/app/DetectionEngine.cpp
        src/app/DetectionEngine.h
        src/app/TensorRTDeleter.h
        src/app/TrtLogger.h
        src/app/LPRecognizerService.cpp
        src/app/LPRecognizerService.h
        src/app/LPRecognizer.cpp
        src/app/LPRecognizer.h
        src/app/CalibParams.cpp
        src/app/CalibParams.h
        src/app/CalibParamsUpdater.cpp
        src/app/CalibParamsUpdater.h
        src/app/BaseCarTracker.cpp
        src/app/BaseCarTracker.h
        src/app/TemplateMatching.cpp
        src/app/TemplateMatching.h
        src/app/SensorCarTracker.cpp
        src/app/SensorCarTracker.h
        src/app/MaskCarTracker.cpp
        src/app/MaskCarTracker.h
        src/app/Car.cpp
        src/app/Car.h
        src/app/Counter.h
        src/app/DetectionService.cpp
        src/app/DetectionService.h
        src/app/Detection.cpp
        src/app/Detection.h
        src/app/TensorRTEngine.cpp
        src/app/TensorRTEngine.h
        src/client/CameraClientLauncher.cpp
        src/client/CameraClientLauncher.h
        src/client/DahuaSettingsUpdater.cpp
        src/client/DahuaSettingsUpdater.h
        src/client/GstreamerReader.cpp
        src/client/GstreamerReader.h
        src/client/Sensor.cpp
        src/client/Sensor.h
        src/client/SensorCameraTrigger.cpp
        src/client/SensorCameraTrigger.h
        src/client/SensorClientLauncher.cpp
        src/client/SensorClientLauncher.h
        src/app/PackageSender.cpp
        src/app/PackageSender.h
        src/car_model/CarModelRecognizer.cpp
        src/car_model/CarModelRecognizer.h
        src/car_model/Labels.h
        src/car_model/MobilenetEngine.h
        src/car_model/MobilenetEngine.cpp
        src/app/CameraScope.cpp
        src/app/CameraScope.h
        src/app/S3Scope.cpp
        src/app/S3Scope.h
        src/app/Snapshot.cpp
        src/app/Snapshot.h
        src/app/SnapshotSender.cpp
        src/app/SnapshotSender.h
        )

target_link_libraries(parking nvinfer ${OpenCV_LIBS} ${ZMQ_LIBRARIES} ${GST_LIBRARIES} ${GST_APP_LIBRARIES} ${GST_RTP_LIBRARIES}
        nlohmann_json::nlohmann_json /usr/local/cuda/lib64/libcudart.so
        ${CURL_LIBRARIES} /usr/local/lib/libcpr.so)