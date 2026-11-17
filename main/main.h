#pragma once

#include "mqttClient.h"
#include "thread.h"
#include "deviceDatas.h"
#include <string>

#include "driverHomebus.h"
#include "driverDaikin.h"

#include "driverAhd.h"


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
    MqttClient*     _mqtt = nullptr;
    Thread*         _thread = nullptr;
    DriverHomebus*  _hbs    = nullptr;
    DriverDaikin*   _daikin = nullptr;

    DriverAhd*      _ahd = nullptr;

    //uint8_t buffer[UART0_BUFFER_SIZE];


    static DeviceDatas _deviceDatas;  // data structure for all the device datas

    struct MqttMessage {
        std::string topic;
        std::string payload;
        int qos = 0;
        int retain = 0;
    }; // to send message to mqtt broker
    
    std::array<MqttMessage, DeviceDatas::kCount> _messages {};


}; // Main Class 