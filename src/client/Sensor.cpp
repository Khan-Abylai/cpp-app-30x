#include "Sensor.h"

using namespace std;
using json = nlohmann::json;

Sensor::Sensor(string serverUrl) : ILogger("Sensor " + serverUrl) {
    zmqServerUrl = move(serverUrl);
}

bool Sensor::isConnected() {
    return socket.handle() != nullptr;
}

void Sensor::connectSocket() {
    if (!isConnected()) {
        socket = zmq::socket_t{context, zmq::socket_type::sub};
        socket.connect("tcp://" + zmqServerUrl + ":" + PORT);
    }
    socket.set(zmq::sockopt::subscribe, "queue_message");
    LOG_INFO("Socket connected to " + zmqServerUrl + " zmq server");
}

void Sensor::addCamera(const string &cameraIp, shared_ptr<SensorCameraTrigger> cameraClient) {
    cameraToSensorTriggers.insert({cameraIp, move(cameraClient)});
}

void Sensor::triggerCamera(const string &cameraIp) {
    if (auto cameraClient = cameraToSensorTriggers.find(cameraIp); cameraClient != cameraToSensorTriggers.end()) {
        LOG_INFO(cameraIp + " trigerred");
        cameraClient->second->changeState();
    }
}

void Sensor::run() {
    connectSocket();

    while (!shutdownFlag) {
        try {
            vector<zmq::message_t> receiveMessages;
            zmq::recv_result_t result = zmq::recv_multipart(socket, back_inserter(receiveMessages));
            if (!result || shutdownFlag) continue;

            auto jsonResult = json::parse(receiveMessages[1].to_string());
            if (jsonResult.contains("message") && jsonResult["message"].contains("cmd")) {
                auto command = jsonResult["message"]["cmd"].get<string>();
                if (command != "start") continue;

                auto cameraIp = jsonResult["camera_ip"].get<string>();
                triggerCamera(cameraIp);
            }
        } catch (const zmq::error_t &ex) {
            if (ex.num() != ETERM)
                throw;
        } catch (exception &e) {
            LOG_ERROR(e.what());
        }
    }
}

void Sensor::shutdown() {
    LOG_INFO(zmqServerUrl + ": service is shutting down");
    shutdownFlag = true;
    socket.close();
    context.close();
}

