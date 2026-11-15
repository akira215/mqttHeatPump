#include "main.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config_json.h"

#include <esp_log.h>

static const char *TAG = "MainApp";

Main App;

DeviceDatas Main::_deviceDatas{};

Main::Main()
{
    // Setting the log level for each module
    esp_log_level_set("MainApp", ESP_LOG_VERBOSE);  // Put verbose to check available stack
    esp_log_level_set("MqttClient", ESP_LOG_DEBUG);
    esp_log_level_set("DriverHomebus", ESP_LOG_INFO);
    esp_log_level_set("DriverDaikin", ESP_LOG_VERBOSE);
}

void Main::setup(void)
{
    ESP_LOGI(TAG,"---------- Starting App V0.3----------");
    // Thread Network (mqtt iniitialized after thread connection is ok)
    _thread = new Thread();
    _thread->registerHandler(&Main::threadEvent, this);
    _thread->init();

    // HBS communication
    _hbs = new DriverHomebus();
    _daikin = new DriverDaikin(&_deviceDatas);

    _hbs->registerCallback(&DriverDaikin::onDataReceived, _daikin);

    _hbs->start();

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
        _mqtt->subscribeTopic(CONFIG_TOPIC_BASE "setpoint/set" , 2);
        _mqtt->subscribeTopic(CONFIG_TOPIC_BASE "mode/set" , 2);
        
        // send config message
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
    /******************************************************************** */
    /******************************************************************** */
    /******************************************************************** */
    /******************************************************************** */
    
    if (topic == (CONFIG_TOPIC_BASE "setpoint/set") ) {
        _daikin->setSetpointTemperature(std::stof(data)); 
    } else if (topic == (CONFIG_TOPIC_BASE "mode/set") ) {
        _daikin->setMode(data);
    } else {
        ESP_LOGW(TAG, "MQTT Data received not catched. Message ID: %d, Topic: %s, Data: %s", msg_id, topic.c_str(), data.c_str());
    }
        
}

void Main::run(void)
{
    if (!_mqtt || !_mqtt->isConnected()) {
        // to avoid triggering the timer before mqtt connect
        vTaskDelay(pdMS_TO_TICKS(2000));
        return;
    }

    size_t count = 0;
    // get all the data to be reported, locking the mutex
    _deviceDatas.forEach([&](DataKey key, IData& item) {
        if (item.isReportable()) {
            //ESP_LOGW("datas", "Key %zu is reportable", static_cast<size_t>(key));
            _messages[count++] = {item.getTopic(), item.getValue()};
        } 
    });

    // Send all data to mqtt
    for (size_t i = 0; i < count; ++i) {
        _mqtt->sendData(std::string(CONFIG_TOPIC_BASE) + _messages[i].topic, _messages[i].payload);
    }

    vTaskDelay(pdMS_TO_TICKS(2000)); // let time to reset watchdog
}




extern "C" void app_main(void)
{
    App.setup();

    while (true)
    {
        App.run();
    }    

    //should not reach here
}


