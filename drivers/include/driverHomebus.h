/*
  diverHomebus
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
  
  This class decode Homebus data using RMT. Needs to configure 3 pins:
   - CONFIG_HBS_TX
   - CONFIG_HBS_RX
   - CONFIG_HBS_RST
  Application need to register a callback that using registerCallback(&Main::callback, this)
  The callback signature should be void (const std::vector<uint8_t>&)
  Bytes are send using sendBytes(const std::vector<uint8_t>&)
*/

#pragma once

#include "freertos/FreeRTOS.h"

#include <functional>
#include <vector>
#include <string>

#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"

#define HBS_BIT_US          104   // Total bit time at ~9600 bps
#define HBS_PULSE_US         52   // Active half-bit pulse time

// RMT resolution for homebus, in Hz
static constexpr uint32_t HOMEBUS_RMT_RESOLUTION_HZ = 4000000UL;

// Homebus timing parameters (in RMT ticks at HOMEBUS_RMT_RESOLUTION_HZ)
static constexpr uint32_t HOMEBUS_BIT_RATE = 9600;
static constexpr uint32_t HOMEBUS_BIT_DURATION = HOMEBUS_RMT_RESOLUTION_HZ / HOMEBUS_BIT_RATE;
static constexpr uint32_t HOMEBUS_HALF_BIT_DURATION = HOMEBUS_BIT_DURATION / 2;

// RX buffer size in symbols (enough for 32 bytes * 11 symbols/byte)
static constexpr size_t RX_BUFFER_SYMBOLS = 32 * 11;

static constexpr size_t DECODE_BUFFER_SIZE = 32;

#define DISPLAY_N_PER_LINE  4 // number of register to be displayed on one line

#define REGISTER_MAP_SIZE 100 // 100 uint16 register address could be stored to increase in case of more


/*

Read/Write 1 bit:

            |                BIT_DURATION              | NEXT
            |  HALF_BIT_DURATION  | HALF_BIT_DURATION  | BIT

------────────────────────────────────────────────────------




Read/Write 0 bit:

            |                BIT_DURATION              | NEXT
            |  HALF_BIT_DURATION  | HALF_BIT_DURATION  | BIT

------────┐                     ┌─────────────────────------
            │                     │
            │                     │
            │                     │
            └─────────────────────┘

*/


// Driver class used for testing only
class DriverHomebus final
{
    static constexpr rmt_symbol_word_t hbs_bit0 = {
       .duration0   = HOMEBUS_HALF_BIT_DURATION,
       .level0      = 0,
       .duration1   = HOMEBUS_HALF_BIT_DURATION,
       .level1      = 1
    }; // Impulsion puis repos

    static constexpr rmt_symbol_word_t hbs_bit1 = {
       .duration0   = HOMEBUS_HALF_BIT_DURATION,
       .level0      = 1,
       .duration1   = HOMEBUS_HALF_BIT_DURATION,
       .level1      = 1
    }; // repos continu

    rmt_channel_handle_t    _tx {nullptr};
    rmt_channel_handle_t    _rx {nullptr};
    rmt_encoder_handle_t    _txEncoder {nullptr};
    gpio_num_t              _enable {static_cast<gpio_num_t>(CONFIG_HBS_RST)};

    rmt_symbol_word_t*      _rxBuffers[2];   // Double buffer for pingpong operation
    uint32_t                _rxBufferSize;

    volatile uint8_t        _writeIdx;      // Buffer currently receiving data
    volatile uint8_t        _readIdx;       // Buffer ready for processing (0xFF if none)
    volatile uint32_t       _nSymbols;
    volatile bool           _overflow;

    rmt_receive_config_t    _rxCfg {};
    
    TaskHandle_t            _rxTaskHandle = nullptr;
    static std::function<void(const std::vector<uint8_t>&)> _callback; // Callback function for received data

public:
    DriverHomebus();
    ~DriverHomebus();

    void start();
    void stop();
    void sendBytes(const std::vector<uint8_t>& datas);

    /// @brief register attribute update handler for this attribute.
    /// @param func pointer to the method ex: &Main::clusterHandler
    /// @param instance instance of the object for this handler (ex: this)
    template<typename C>
    void registerCallback(void (C::* func)(const std::vector<uint8_t>&), C* instance) {
        _callback = [func, instance](const std::vector<uint8_t>& data) {
            (instance->*func)(data);
        };
    }

private:
    //
    static bool IRAM_ATTR rxRmtIsr(rmt_channel_handle_t channel,
                                const rmt_rx_done_event_data_t *edata,
                                void *user_data);

    static std::vector<uint8_t> decodeRmtData(rmt_symbol_word_t *symbols, size_t num_symbols);
    static void printPacket(const std::vector<uint8_t>& data); // For debugging

    static void rxTask(void *pvParameters);

}; //class DriverAdh
