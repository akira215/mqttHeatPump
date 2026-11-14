#pragma once


#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "data.h"
#include <array>


// Define your keys as sequential integers starting at 0
enum class DataKey : size_t {
    roomTemperature = 0,
    backupHeater,
    heatPumpMode,
    status,
    Count // Automatically tracks total number of keys (4)
};

class DeviceDatas {

    // Single FreeRTOS mutex protecting all DeviceData members (~80 bytes RAM)
    mutable SemaphoreHandle_t mutex = nullptr;

public:
    // RAII Scoped Lock helper for safe multi-step operations
    class ScopedLock {
            SemaphoreHandle_t _handle;
        public:
            explicit ScopedLock(SemaphoreHandle_t m) : _handle(m) {
                if (_handle) xSemaphoreTake(_handle, portMAX_DELAY);
            }
            ~ScopedLock() {
                if (_handle) xSemaphoreGive(_handle);
            }
            ScopedLock(const ScopedLock&) = delete;
            ScopedLock& operator=(const ScopedLock&) = delete;
    };  

    DeviceDatas() {
        mutex = xSemaphoreCreateMutex();
    }

    ~DeviceDatas() {
        if (mutex) {
            vSemaphoreDelete(mutex);
        }
    }

    // Prevent accidental copying of mutex handle
    DeviceDatas(const DeviceDatas&) = delete;
    DeviceDatas& operator=(const DeviceDatas&) = delete;



    //  Concrete compile-time locked members (no heap allocation)
    Data<float>         roomTemperature{"room/temperature"};        // Empty, minDelta = 0.5f
    Data<int>           backupHeater{"BackupHeater/level"};         // Empty, minDelta = 2.0f
    Data<bool>          heatPumpMode{"mode/set"};                   // Empty, minDelta = 0.0f
    Data<std::string>   status{"status/set"};  // Initialized to "Booting"
    
    // O(1) Direct-indexed array (order must match DataKey enum)
    static constexpr size_t kCount = static_cast<size_t>(DataKey::Count);
    const std::array<IData*, kCount> registry{{
        &roomTemperature,
        &backupHeater,
        &heatPumpMode,
        &status
    }};

private:
    // Instant O(1) lookup by enum key
    IData* get(DataKey key) const {
        const auto idx = static_cast<size_t>(key);
        return (idx < kCount) ? registry[idx] : nullptr;
    }

public:
    // Direct helper to update a value by enum key
    template <typename T>
    void setByKey(DataKey key, const T& val) {
        if (IData* item = get(key)) {
        
            ScopedLock lock(mutex);
            item->setValue(val);
            return;
        }
    }

    std::string getByKey(DataKey key) {
        if (IData* item = get(key)) {

            ScopedLock lock(mutex);
            return item->getValue();
        }
    }

    // ========================================================================
    // Atomic Batch Access (For reading/writing multiple fields together)
    // ========================================================================
    template <typename Func>
    void withLock(Func&& callback) {
        ScopedLock lock(mutex);
        callback(*this);
    }


    // Thread-safe iteration over all registered entries
    template <typename Func>
    void forEach(Func&& callback) const {
        ScopedLock lock(mutex);
        for (size_t i = 0; i < kCount; ++i) {
            callback(static_cast<DataKey>(i), *registry[i]);
        }
    }
};

/*
_deviceDatas.withLock([](DeviceDatas& data) {
            if (!data.temp().empty() && !data.hum().empty()) {
                float t = data.temp().get();
                int h   = data.hum().get();
                ESP_LOGI("NET", "Snapshot -> T: %.2f, H: %d%%, Status: %s",
                         t, h, data.deviceStatus().get().c_str());
            }
        });



        // Loop through every entry safely under one mutex lock
_deviceDatas.forEach([](DataKey key, const IDeviceData& item) {
    if (item.empty()) {
        ESP_LOGW("TELEMETRY", "Key %zu is empty", static_cast<size_t>(key));
    } else {
        ESP_LOGI("TELEMETRY", "Key %zu = %s",
                 static_cast<size_t>(key),
                 item.toString().c_str());
    }
});

*/