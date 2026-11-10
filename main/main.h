#pragma once

#include "mqttClient.h"
#include "thread.h"

// Main class used for testing only
class Main final
{

public:
    Main();

    void setupThread(void);

    void run(void);
    void setup(void);
    void onMqttConnection(bool connected, int res);
    void onMqttData(int msg_id, const std::string& topic, const std::string& data);
    void threadEvent(esp_openthread_event_t event_id);

private:
    MqttClient* _mqtt = nullptr;
    Thread*     _thread = nullptr;

}; // Main Class 