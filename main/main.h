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

private:
    Wifi*       _wifi = nullptr;
    MqttClient* _mqtt = nullptr;
    //void wifi_init_sta(void);
    //DriverHomebus*  _hbs    = nullptr;


}; // Main Class