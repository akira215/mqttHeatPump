#pragma once

#include "wifi.h"
#include "mqttClient.h"

// Main class used for testing only
class Main final
{

public:
    Main();
    void run(void);
    void setup(void);
    void onMqttConnection(bool connected, int res);
    void onMqttData(int msg_id, const std::string& topic, const std::string& data);

private:
    Wifi*       _wifi = nullptr;
    MqttClient* _mqtt = nullptr;

}; // Main Class 