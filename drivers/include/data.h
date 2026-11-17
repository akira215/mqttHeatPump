/*
  data
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  This class is the atomic object to store value. 
  The object itself as min and max reportable intervale and reportable
  change, and keeping a timer, it knows when it should be reported (isReportable())

  The Interface class is here for conveninence.
*/


#pragma once

#include <string>
#include <format>
#include <cmath>
#include "esp_timer.h"



// Helper to verify T is one of the 4 allowed types
template <typename T>
constexpr bool is_allowed_data_type = 
    std::is_same_v<T, int> || 
    std::is_same_v<T, bool> || 
    std::is_same_v<T, float> || 
    std::is_same_v<T, std::string>;

// Non-templated base interface
class IData {
public:
    virtual ~IData() = default;

    // State & formatting accessible without knowing T
    virtual void clear() = 0;
    virtual bool isReportable() = 0;
    virtual std::string getValue(const std::string& emptyFallback = "") = 0;
    virtual std::string getTopic() const = 0;
    virtual int getQos() const = 0;
    virtual int getRetain() const = 0;

    // Direct value population via base pointer (returns true if updated)
    virtual void setValue(const int& v) { }
    virtual void setValue(const bool& v) { }
    virtual void setValue(const float& v) { }
    virtual void setValue(const std::string& v) { }
    virtual void setValue(const char* v) {
        return setValue(std::string(v ? v : ""));
    }
};


// Compile-time type-locked implementation
template <typename T>
class Data : public IData {
    static_assert(is_allowed_data_type<T>, "Unsupported type");

private:
    T           _value {};
    bool        _isEmpty = true;
    std::string _mqttTopic;               // Set at compile time
    int         _qos;
    int         _retain;
    int16_t     _minReportingInterval;   // Set at compile time
    int16_t     _maxReportingInterval;   // Set at compile time
    float       _reportableChange;       // Set at compile time
    bool        _reportable = false;
    int64_t     _lastReportTime = 0;
    
public:
    using IData::setValue;

    explicit Data(const std::string& mqttTopic, int qos = 0, int retain = 0, 
                            int16_t minInterval = 2, int16_t maxInterval = 3600, float delta = 1.0f) 
        : _mqttTopic(mqttTopic), _qos(qos), _retain(retain), _minReportingInterval(minInterval), 
          _maxReportingInterval(maxInterval), _reportableChange(delta), _lastReportTime (0) { }
    //Data(const T& v) : value(v) {}

    std::string getTopic() const { return _mqttTopic; }
    int getQos() const { return _qos; }
    int getRetain() const { return _retain; }
    

    void clear() {
        _value = T{};
        _isEmpty = true;
        _reportable = false;
    }


    bool isReportable() { 
        
        if (_reportable)
            return true;

        int64_t elapsed_s = (esp_timer_get_time() - _lastReportTime) / 1000000;   

        // If max reportable interval elapsed report (heartbeat)
        if (elapsed_s >= _maxReportingInterval)
            _reportable = true;

        return _reportable; 
    }

    void setValue(const T& v) override {
        
        if (_isEmpty) {
            _reportable = true;
            _value = v;
            _isEmpty = false;
            return;
        }        
        
        int64_t elapsed_s = (esp_timer_get_time() - _lastReportTime) / 1000000;

        // Numeric types (int, float, bool): check absolute delta
        if constexpr (std::is_arithmetic_v<T>) {
            float delta = std::abs(static_cast<float>(v) - static_cast<float>(_value));
        
            if ((delta >= _reportableChange)&&(elapsed_s >= _minReportingInterval)) {
                _reportable = true;
                _value = v;
                _isEmpty = false;
            }
            return;
        } else {  // std::string: check inequality
            if ((_value != v)&&(elapsed_s >= _minReportingInterval)) {
                _reportable = true;
                _value = v;
                _isEmpty = false;
            }
            return;
        }
    }

    // Assignment operator convenience
    Data& operator=(const T& v) {
        setValue(v);
        return *this;
    }


   std::string getValue(const std::string& emptyFallback = "") {

        _lastReportTime = esp_timer_get_time();
        _reportable = false;
        
        if (_isEmpty) {
            return emptyFallback;
        }

        if constexpr (std::is_same_v<T, std::string>) {
            return _value;
        } 
        else if constexpr (std::is_same_v<T, bool>) {
            return _value ? "ON" : "OFF";
        } 
        else if constexpr (std::is_same_v<T, int>) {
            return std::to_string(_value);
        } 
        else if constexpr (std::is_same_v<T, float>) {
            //char buf[32];
            // %.2f formats to 2 decimal places (adjust precision as needed)
            //std::snprintf(buf, sizeof(buf), "%.2f", _value);
            //return std::string(buf);
            return std::format("{:.2f}", _value);
        }
    }


};










