/*
  diverAhd
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  This class decode the SELTRON AHD20 message on ASCII modbus com

*/

#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include <vector>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define UART1_NUM           UART_NUM_1

#define UART1_TX_PIN        GPIO_NUM_18 // Unused in this unidirectional test
#define UART1_RX_PIN        GPIO_NUM_17

#define UART1_BAUD_RATE     9600
#define UART1_BUFFER_SIZE   512

#define QUEUE_SIZE          20

#define START_BYTE          0x3A
#define STOP_BYTE1          0x0A
#define STOP_BYTE2          0x0D

#define MODBUS_FUNC_READ    0x03
#define MODBUS_FUNC_WRITE   0x10

#define REG_SLAVE_POINTER   0x0001


// Driver class used for testing only
class DriverAhd final
{
    

    //AHD is kind of Modbus Ascii, frame shall be converted to ASCII first
    std::vector<char>    _data;   // The raw data as read on UART
    std::vector<uint8_t> _bytes; // Data which is interpretation of the Ascii from Raw
    size_t _rxIndex = 0;            // index for the UART read buffer

    uint8_t     _slaveId        = 0;
    uint8_t     _function       = 0;
    uint8_t     _nBytes         = 0;
    uint16_t    _startingAdd    = 0;
    uint16_t    _nRegisters     = 0;
    uint16_t    _writeAdd       = 0;
    uint16_t    _nWrite         = 0;
    TaskHandle_t  _rxTaskHandle = nullptr;
    static QueueHandle_t uart_event_queue;
    
public:
    DriverAhd();
    ~DriverAhd();

    static void uartRxTask(void *pvParameters);

    void start();
    void stop();

    void sentBytes(const std::vector<uint8_t>& bytes);

private:
    void processFrame(size_t size);
    
    void buildFrame(size_t size);
    void decodeFrame();
    
    
    // Helper function to convert a single hex char to its 4-bit numeric value
    static uint8_t hex_to_val(char ch);
    static char val_to_hex(uint8_t val);
    static uint8_t checkSum(const std::vector<uint8_t>& bytes);
    static void printVectorInline(const std::vector<uint8_t>& bytes);
    static void printRawFrame(const std::vector<char>& data, size_t size);


}; //class DriverAdh