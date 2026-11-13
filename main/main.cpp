#include "main.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config_json.h" //todel

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
    _thread = new Thread();
    _thread->registerHandler(&Main::threadEvent, this);
    _thread->init();

}

void  Main::threadEvent(esp_openthread_event_t event_id)
{
   if (event_id == OPENTHREAD_EVENT_GOT_IP6) {
        bool first = true;
        if(_mqtt)
            first = false;

        if(first)
            _mqtt = new MqttClient();
        
        
        _mqtt->stop();    
        _mqtt->start();

        if(first){
            _mqtt->registerConnectionHandler(&Main::onMqttConnection, this);
            _mqtt->registerDataHandler(&Main::onMqttData, this);
        }

    }
   
}

void Main::onMqttConnection(bool connected, int res)
{
    if (connected) {
        ESP_LOGI(TAG, "MQTT Connected. Session present: %d", res);
        _mqtt->subscribeTopic(TOPIC_BASE SLASH TOPIC_SETPOINT SLASH TOPIC_STATE ,0);
        
        // Construct std::string from the string_view
        std::string jsonData(JSON_DEVICE_CONFIG);

        ESP_LOGI(TAG, "Sending %zu bytes of JSON", jsonData.size());
        _mqtt->sendData("homeassistant/device/HeatPump/config",jsonData,1,1); // retain

    } else {
        ESP_LOGW(TAG, "MQTT Disconnected. Error type: %d", res);
    }
}

void Main::onMqttData(int msg_id, const std::string& topic, const std::string& data)
{
    ESP_LOGI(TAG, "MQTT Data received. Message ID: %d, Topic: %s, Data: %s", msg_id, topic.c_str(), data.c_str());
}

void Main::run(void)
{
    int val = esp_random()%100;

    _mqtt->sendData(TOPIC_BASE SLASH "ambience/temperature", std::to_string(val));
    vTaskDelay(pdMS_TO_TICKS(2000)); // let time to reset watchdog
}


extern "C" void app_main(void)
{
    ESP_LOGI(TAG,"---------- Starting App V0.3----------");
    App.setup();

    while (true)
    {
        App.run();
    }    

    //should not reach here
}


