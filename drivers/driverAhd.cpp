/*
  diverAhd
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  This class decode the SELTRON AHD20 message on ASCII modbus com

*/

#include <esp_log.h>
#include <numeric> // Required for std::accumulate
#include <cmath>
#include <cstring>
#include "driverAhd.h"

static const char *TAG = "DriverAhd";
QueueHandle_t DriverAhd::uart_event_queue = nullptr;

DriverAhd::DriverAhd() : _data(UART1_BUFFER_SIZE)
{
    _bytes.reserve(UART1_BUFFER_SIZE/2);

    uart_config_t uart_config = {};
    uart_config.baud_rate = UART1_BAUD_RATE;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 122;
    uart_config.source_clk = UART_SCLK_DEFAULT;  // Use APB clock for ESP32-C6

    ESP_ERROR_CHECK(uart_driver_install(UART1_NUM, UART1_BUFFER_SIZE, 0, 
                QUEUE_SIZE ,&DriverAhd::uart_event_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(UART1_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART1_NUM, UART1_TX_PIN, UART1_RX_PIN, 
                        UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    
    // Optional: Tune idle timeout (in symbol lengths) to detect end-of-packet faster
    uart_set_rx_timeout(UART1_NUM, 3);

    ESP_LOGI(TAG, "UART1 initialized at %d baud", UART1_BAUD_RATE);

    //Reserve to avoid heap allocation
    //_registerMap.reserve(REGISTER_MAP_SIZE);

}

DriverAhd::~DriverAhd()
{

}

void DriverAhd::start()
{
    xTaskCreate(DriverAhd::uartRxTask, "DriverAhd_Task", 3072, this, 5, &_rxTaskHandle);
   
    printf("\e[2J\e[H");
}

void DriverAhd::stop()
{
    // Delete the task using its handle
    if (_rxTaskHandle != nullptr) {
        vTaskDelete(_rxTaskHandle);
        _rxTaskHandle = nullptr; // Prevent using an invalid handle
    }

    ESP_LOGI(TAG, "Rx stopped");
}

void DriverAhd::sentBytes(const std::vector<uint8_t>& bytes) 
{
    uint8_t chk = checkSum(bytes);
    std::vector<char> toSend;
    toSend.reserve(1 + (bytes.size() * 2) + 4); // 1 start byte 2 checksum 2 stops
    
    toSend.push_back(START_BYTE);

    for (uint8_t byte : bytes) {

        char high_char = val_to_hex(byte >> 4);   // Upper 4 bits
        char low_char  = val_to_hex(byte & 0x0F); // Lower 4 bits

        toSend.push_back(high_char);
        toSend.push_back(low_char);
    }

    char high_chk = val_to_hex(chk >> 4);   // Upper 4 bits
    char low_chk  = val_to_hex(chk & 0x0F); // Lower 4 bits

    toSend.push_back(high_chk);
    toSend.push_back(low_chk);

    toSend.push_back(STOP_BYTE2);
    toSend.push_back(STOP_BYTE1);

    //printRawFrame(toSend, toSend.size());


    int bytesSent = uart_write_bytes(UART1_NUM, toSend.data(), toSend.size());
    
    if (bytesSent < toSend.size()) {
        ESP_LOGE(TAG, "UART TX error");
    } else {
        ESP_LOGV(TAG, "Sent line packet: %d raw bytes", bytesSent);
    }
    
}


//Static
void DriverAhd::uartRxTask(void *pvParameters)
{
    DriverAhd* instance = static_cast<DriverAhd*>(pvParameters);

    ESP_LOGI(TAG, "UART1 RX Task started. Listening on GPIO %d...", UART1_RX_PIN);

    uart_event_t event;

    while (1) {
        
        // Sleep completely (0% CPU) until the UART hardware sends an event
        if (xQueueReceive(uart_event_queue, (void *)&event,  pdMS_TO_TICKS(50))) {
            
            switch (event.type) {
                case UART_DATA:
                {
                    size_t remaining = event.size;  // Hardware RX FIFO buffer is 128 bytes (triggering at 120 bytes)
                                                    // Our frame are greater, so we need to accumulate

                    while (remaining > 0) {
                        // 1. Check how much room is left in _data
                        size_t availableSpace = instance->_data.size() - instance->_rxIndex;
                        if (availableSpace == 0) {
                            ESP_LOGW(TAG, "Frame overflow (%zu bytes). Resetting.", instance->_rxIndex);
                            instance->_rxIndex = 0;
                            availableSpace = instance->_data.size();
                        }

                        // 2. Read directly into _data at the current offset
                        size_t toRead = std::min(remaining, availableSpace);
                        int len = uart_read_bytes(UART1_NUM, instance->_data.data() + instance->_rxIndex, toRead, portMAX_DELAY);

                        if (len <= 0) {
                            break;
                        }

                        remaining -= len;
                        size_t startScan = instance->_rxIndex;
                        instance->_rxIndex += len;

                        // 3. Scan the newly added bytes for START_BYTE and STOP_BYTEs
                        size_t i = startScan;
                        while (i < instance->_rxIndex) {
                            // Case A: We haven't found START_BYTE yet
                            if (instance->_data[0] != START_BYTE) {
                                if (instance->_data[i] == START_BYTE) {
                                    // Shift everything from START_BYTE onward to index 0
                                    size_t validLen = instance->_rxIndex - i;
                                    std::memmove(instance->_data.data(), instance->_data.data() + i, validLen);
                                    instance->_rxIndex = validLen;
                                    i = 1; // Continue scanning right after START_BYTE
                                } else {
                                    i++;
                                }
                                continue;
                            }

                            // Case B: We have START_BYTE at index 0; check if 'i' completes the 2 stop bytes
                            if (i >= 2 &&
                                instance->_data[i - 1] == STOP_BYTE2 &&
                                instance->_data[i] == STOP_BYTE1) {
                                
                                size_t frameLen = i + 1;
                                ESP_LOGV(TAG, "Complete frame received (%zu bytes)", frameLen);
                                instance->processFrame(frameLen);

                                // If extra bytes from the NEXT frame arrived in the same read, shift them to index 0
                                size_t leftover = instance->_rxIndex - frameLen;
                                if (leftover > 0) {
                                    std::memmove(instance->_data.data(), instance->_data.data() + frameLen, leftover);
                                }
                                instance->_rxIndex = leftover;
                                i = 0; // Restart scan on the shifted leftover bytes
                            } else {
                                i++;
                            }
                        }

                        // If the entire buffer had no START_BYTE at all, discard it so _rxIndex stays 0
                        if (instance->_rxIndex > 0 && instance->_data[0] != START_BYTE) {
                            instance->_rxIndex = 0;
                        }
                    }
                    break;
                }
                case UART_FIFO_OVF:
                case UART_BUFFER_FULL:
                    // Hardware or ring buffer overflowed: flush to recover cleanly
                    ESP_LOGW(TAG, "UART Overflow! Flushing input.");
                    uart_flush_input(UART1_NUM);
                    xQueueReset(uart_event_queue);
                    break;

                case UART_BREAK:
                case UART_DATA_BREAK:
                case UART_FRAME_ERR:
                case UART_PARITY_ERR:
                    ESP_LOGW(TAG, "UART Frame/Parity Error");
                    break;

                case UART_PATTERN_DET:
                case UART_WAKEUP:
                case UART_EVENT_MAX:
                default:
                    break;
            }
        } else { // 50ms of silence on the UART line: discard any incomplete frame
            if (instance->_rxIndex > 0) {
                ESP_LOGW(TAG, "UART RX timeout: discarding incomplete frame (%zu bytes)", instance->_rxIndex);
                instance->_rxIndex = 0;
            }
        }
    } // While(1)

    // Should never reach here
    vTaskDelete(NULL);
}

void DriverAhd::processFrame(size_t size)
{
    //printRawFrame(_data, size);
    
    buildFrame(size);
    uint8_t lrc = _bytes.back();
    _bytes.pop_back(); // remove checksum

    if(checkSum(_bytes) != lrc){
        ESP_LOGW(TAG,"Checksum error in a frame");
        return;
    }

    #ifdef CONFIG_PRINT_AHD20_PACKETS
        printVectorInline(_bytes);
    #endif
    //sentBytes(_bytes); // to debug sendBytes
    //decodeFrame();

}

void DriverAhd::buildFrame(size_t size)
{
    // start and stops are not excluded and each pair of ascii
    // is interpreted as a byte

    _bytes.clear(); 
    // Loop through the vector in pairs, ignoring start and 2 stops
    for (size_t i = 1; i < size - 3; i += 2) {

        uint8_t high_nibble = hex_to_val(_data[i]);
        uint8_t low_nibble = hex_to_val(_data[i + 1]);
            
        // Combine the two 4-bit values into a single 8-bit byte
        uint8_t byte_val = (high_nibble << 4) | low_nibble;
        _bytes.push_back(byte_val);
    }

}

void DriverAhd::decodeFrame()
{
    _slaveId    = _bytes[0];  //First Byte is slave address
    _function   = _bytes[1];  //Modbus function

    if (_function == MODBUS_FUNC_READ) {// function read register
        if (_bytes.size() == 7) { // Master query
            _startingAdd = (static_cast<uint16_t>(_bytes[2]) << 8) | _bytes[3];
            _nRegisters = (static_cast<uint16_t>(_bytes[4]) << 8) | _bytes[5];
        } else { // Slave Response
            if (_startingAdd == REG_SLAVE_POINTER) { // Master query slave what to do next
                // Here slave answer 3 words : function - start - num of reg
                if (_bytes[4] == MODBUS_FUNC_WRITE) {  // Capture when slave ask a write
                    _writeAdd = (static_cast<uint16_t>(_bytes[5]) << 8) | _bytes[6];
                    _nWrite = (static_cast<uint16_t>(_bytes[7]) << 8) | _bytes[8];
                }
            } else { // Slave response data
                _nBytes = _bytes[2];
                if (_nBytes != (_bytes.size() - 4))
                    ESP_LOGW(TAG, "Slave Answer read - size not equal to nb Bytes");
                else {
                    for (uint8_t i = 0; i < _nBytes / 2; ++i) {
                        uint16_t value = (static_cast<uint16_t>(_bytes[3+i*2]) << 8) | _bytes[4+i*2];

                        //_registerMap.lock();
                        //RegisterMap::register_t* reg = _registerMap.getItem(_startingAdd + i);
                        //if (reg == nullptr) {
                        //    _registerMap[_startingAdd + i].value = value;
                        //    _registerMap[_startingAdd + i].updated = true;

                        //} else {
                        //    if (reg->value == value)
                        //        reg->updated = false;
                        //    else {
                        //        reg->updated = true;
                        //        reg->value = value;
                        //    }
                        //}
                        //_registerMap.unlock();

                    }
                }
            }
            _nRegisters = 0;
            _startingAdd = 0;
        }
    } else if (_function ==  MODBUS_FUNC_WRITE) { // function write register
        if (_bytes.size() == 7) { // Slave Response
        
        } else { // Master query
            uint16_t start = (static_cast<uint16_t>(_bytes[2]) << 8) | _bytes[3];
            //uint16_t n = (static_cast<uint16_t>(_bytes[4]) << 8) | _bytes[5];
            _nBytes = _bytes[6];

            if (_nBytes != (_bytes.size() - 8))
                ESP_LOGW(TAG,"Master Write - size not equal to nb Bytes      ");
            else {
                 for (uint8_t i = 0; i < _nBytes / 2; ++i) {
                    uint16_t value = (static_cast<uint16_t>(_bytes[7+i*2]) << 8) | _bytes[8+i*2];
                    
                    //_registerMap.lock();
                    //RegisterMap::register_t* reg = _registerMap.getItem(start + i);
                    //if (reg == nullptr) {
                    //   _registerMap[start + i].value = value;
                    //    _registerMap[start + i].updated = true;

                    //} else {
                    //    if (reg->value == value)
                    //        reg->updated = false;
                    //    else {
                    //        reg->updated = true;
                    //        reg->value = value;
                    //    }
                    //}
                    //_registerMap.unlock();
                }
            }

        }
    }

}

// Static
uint8_t DriverAhd::hex_to_val(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return 0;
}

//Static
char DriverAhd::val_to_hex(uint8_t val) {
    // Mask to 4 bits (0-15) for safety
    val &= 0x0F; 
    if (val < 10) return '0' + val;
    return 'A' + (val - 10); // Change 'A' to 'a' if you need lowercase hex
}


// Static
uint8_t DriverAhd::checkSum(const std::vector<uint8_t>& bytes)
{
    uint32_t sum = 0;
    // 1. Guard against empty or 1-byte vectors
    if (bytes.size() > 1) {
        // 2. Accumulate from the start up to end
        sum = std::accumulate(bytes.begin(), bytes.end(), 0);
    }

    return (uint8_t)((-sum) & 0xFF); 

}

// Static
void DriverAhd::printVectorInline(const std::vector<uint8_t>& bytes) {
    for (uint8_t byte : bytes) {
        // %02X prints 0-15 as '00'-'0F' in uppercase hex
        printf("%02X ", byte); 
    }
    printf("\n"); // Newline at the end
}

// Static
void DriverAhd::printRawFrame(const std::vector<char>& data, size_t size)
{
    //AHD is kind of Modbus Ascii, frame shall be converted to ASCII first
    ESP_LOGI(TAG, "Frame Converted to ASCII (%d bytes):", size);
    ESP_LOG_BUFFER_HEX(TAG, data.data(), size );
    //ESP_LOG_BUFFER_CHAR(TAG, data.data(), size );
}