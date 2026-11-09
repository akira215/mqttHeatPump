//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "main.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_random.h" //todel




static const char *TAG = "MainApp";

Main App;


Main::Main()
{
    // Setting the log level for each module
    esp_log_level_set("Main_app", ESP_LOG_VERBOSE);  // Put verbose to check available stack
    esp_log_level_set("Wifi", ESP_LOG_VERBOSE);
    esp_log_level_set("MqttClient", ESP_LOG_VERBOSE);
}

void Main::setup(void)
{
    _wifi = new Wifi();
    if (_wifi->isConnected()) {
        _mqtt = new MqttClient();
        _mqtt->start();
    }

    _mqtt->subscribeTopic("akira/test1",0);
}



void Main::run(void)
{
    int val = esp_random()%100;

    _mqtt->sendData("akira/test1", std::to_string(val));
    vTaskDelay(pdMS_TO_TICKS(2000)); // let time to reset watchdog
}


extern "C" void app_main(void)
{
    ESP_LOGI(TAG,"---------- Starting App ----------");
    App.setup();

    

    while (true)
    {
        App.run();
    }    

    //should not reach here
}


