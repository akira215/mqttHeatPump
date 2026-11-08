//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "main.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MainApp";


Main App;


Main::Main()
{
    // Setting the log level for each module
    esp_log_level_set("Main_app", ESP_LOG_VERBOSE);  // Put verbose to check available stack
    esp_log_level_set("Wifi", ESP_LOG_VERBOSE);
    esp_log_level_set("DriverDaikin", ESP_LOG_VERBOSE);
}

void Main::setup(void)
{

    wifi = new Wifi();
}



void Main::run(void)
{
    //_hbs->run();
    vTaskDelay(pdMS_TO_TICKS(10)); // let time to reset watchdog
}


extern "C" void app_main(void)
{
    ESP_LOGI(TAG,"---------- Starting App ----------");
    App.setup();
/*
    while (true)
    {
        App.run();
    }    
*/
    //should not reach here
}


