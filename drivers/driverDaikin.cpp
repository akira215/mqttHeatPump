/*
  diverDaikin
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  This class decode the data coming from homebus and populate the application data structure (DeviceData)

  The constructor shall be called with a pointer to the application data structure (DeviceData)
  and a pointer to the homebus object to sendbyte
*/


#include <cstdio>
#include <bit>
#include <bitset>

#include <format>

#include "driverDaikin.h"
#include "p1p2protocol.h"

#include "esp_random.h" //todel

#include <esp_log.h>

static const char *TAG = "DriverDaikin";


//TODEL
// Returns a random float in the range [0.0f, 1.0f)
float random_float(void) {
    // Shift right by 8 bits to fit the 24-bit mantissa of a 32-bit float cleanly
    return (esp_random() >> 8) * (1.0f / 16777216.0f);
}

// Returns a random float in a custom range [min, max)
float random_float_range(float min, float max) {
    return min + random_float() * (max - min);
}

static int random_int_range(int min, int max)
{
    if (min > max) {
        std::swap(min, max);
    }

    constexpr uint64_t randomDomain = uint64_t{1} << 32;
    const uint64_t range =
        static_cast<uint64_t>(static_cast<int64_t>(max) - min) + 1;
    const uint64_t limit = randomDomain - randomDomain % range;

    uint32_t value;
    do {
        value = esp_random();
    } while (static_cast<uint64_t>(value) >= limit);

    return static_cast<int>(
        static_cast<int64_t>(min) + static_cast<uint64_t>(value) % range
    );
}



// Static
// ==========================================================================
// CRC Calculation
// ==========================================================================
const uint8_t DriverDaikin::CRC_TABLE[256] = {
    0x00, 0xd0, 0x13, 0xc3, 0x26, 0xf6, 0x35, 0xe5, 0x4c, 0x9c, 0x5f, 0x8f,
    0x6a, 0xba, 0x79, 0xa9, 0x98, 0x48, 0x8b, 0x5b, 0xbe, 0x6e, 0xad, 0x7d,
    0xd4, 0x04, 0xc7, 0x17, 0xf2, 0x22, 0xe1, 0x31, 0x83, 0x53, 0x90, 0x40,
    0xa5, 0x75, 0xb6, 0x66, 0xcf, 0x1f, 0xdc, 0x0c, 0xe9, 0x39, 0xfa, 0x2a,
    0x1b, 0xcb, 0x08, 0xd8, 0x3d, 0xed, 0x2e, 0xfe, 0x57, 0x87, 0x44, 0x94,
    0x71, 0xa1, 0x62, 0xb2, 0xb5, 0x65, 0xa6, 0x76, 0x93, 0x43, 0x80, 0x50,
    0xf9, 0x29, 0xea, 0x3a, 0xdf, 0x0f, 0xcc, 0x1c, 0x2d, 0xfd, 0x3e, 0xee,
    0x0b, 0xdb, 0x18, 0xc8, 0x61, 0xb1, 0x72, 0xa2, 0x47, 0x97, 0x54, 0x84,
    0x36, 0xe6, 0x25, 0xf5, 0x10, 0xc0, 0x03, 0xd3, 0x7a, 0xaa, 0x69, 0xb9,
    0x5c, 0x8c, 0x4f, 0x9f, 0xae, 0x7e, 0xbd, 0x6d, 0x88, 0x58, 0x9b, 0x4b,
    0xe2, 0x32, 0xf1, 0x21, 0xc4, 0x14, 0xd7, 0x07, 0xd9, 0x09, 0xca, 0x1a,
    0xff, 0x2f, 0xec, 0x3c, 0x95, 0x45, 0x86, 0x56, 0xb3, 0x63, 0xa0, 0x70,
    0x41, 0x91, 0x52, 0x82, 0x67, 0xb7, 0x74, 0xa4, 0x0d, 0xdd, 0x1e, 0xce,
    0x2b, 0xfb, 0x38, 0xe8, 0x5a, 0x8a, 0x49, 0x99, 0x7c, 0xac, 0x6f, 0xbf,
    0x16, 0xc6, 0x05, 0xd5, 0x30, 0xe0, 0x23, 0xf3, 0xc2, 0x12, 0xd1, 0x01,
    0xe4, 0x34, 0xf7, 0x27, 0x8e, 0x5e, 0x9d, 0x4d, 0xa8, 0x78, 0xbb, 0x6b,
    0x6c, 0xbc, 0x7f, 0xaf, 0x4a, 0x9a, 0x59, 0x89, 0x20, 0xf0, 0x33, 0xe3,
    0x06, 0xd6, 0x15, 0xc5, 0xf4, 0x24, 0xe7, 0x37, 0xd2, 0x02, 0xc1, 0x11,
    0xb8, 0x68, 0xab, 0x7b, 0x9e, 0x4e, 0x8d, 0x5d, 0xef, 0x3f, 0xfc, 0x2c,
    0xc9, 0x19, 0xda, 0x0a, 0xa3, 0x73, 0xb0, 0x60, 0x85, 0x55, 0x96, 0x46,
    0x77, 0xa7, 0x64, 0xb4, 0x51, 0x81, 0x42, 0x92, 0x3b, 0xeb, 0x28, 0xf8,
    0x1d, 0xcd, 0x0e, 0xde};

DriverDaikin::DriverDaikin(DeviceDatas* datas, DriverHomebus* hbs) : _pDatas(datas), _hbs(hbs)
{
    ESP_LOGI(TAG, "DriverDaikin::DriverDaikin() starting");
}

DriverDaikin::~DriverDaikin()
{

}

//static 
uint8_t DriverDaikin::computeCRC(const std::vector<uint8_t>& data)
{
    uint8_t crc = 0;
    for (size_t i = 0; i < data.size() - 1; i++) {
        crc = CRC_TABLE[(crc ^ data[i]) & 0xFF];
    }
    return crc;
}

//static 
void DriverDaikin::printPacket(const std::vector<uint8_t>& data)
{
    // Each byte needs 3 chars ("xx "), plus prefix (~15) and suffix
    static constexpr size_t MAX_PRINT_BYTES = 48;
    char str[16 + MAX_PRINT_BYTES * 3 + 2];
    char *p = str;

    size_t print_len = (data.size() > MAX_PRINT_BYTES) ? MAX_PRINT_BYTES : data.size();
    p += sprintf(str, "len=%02d pkt=[", data.size());
    for (size_t i = 0; i < print_len; i++) {
        p += sprintf(p, "%02x ", data[i]);
    }
    if (print_len > 0) {
        *(p - 1) = ']';
    } else {
        *p++ = ']';
    }
    *p = 0;
    ESP_LOGV(TAG, "%s", str);
}

// static, for debug
std::string DriverDaikin::bytesToHex(const std::vector<uint8_t>& data)
{
    if (data.empty()) {
        return "";
    }

    static constexpr char hex_chars[] = "0123456789abcdef";
    
    // Each byte takes 2 hex chars + 1 space, minus 1 trailing space
    std::string result(data.size() * 3 - 1, ' ');

    for (size_t i = 0; i < data.size(); ++i) {
        result[i * 3]     = hex_chars[(data[i] >> 4) & 0x0F];
        result[i * 3 + 1] = hex_chars[data[i] & 0x0F];
    }

    return result;
}

std::string DriverDaikin::byteToBinary(const std::vector<uint8_t>& data, size_t index)
{
    index += 3;
    if (index >= data.size()) {
        return ""; // Return empty string if index is out of bounds
    }

    return std::format("Byte {} : ", index - 3) + std::bitset<8>(data[index]).to_string();
}


// this method only route the data to the correct handler
void DriverDaikin::onDataReceived(const std::vector<uint8_t>& buffer)
{
    #ifdef CONFIG_PRINT_DAIKIN_PACKETS
        printPacket(buffer);
    #endif

    // Validate CRC
    uint8_t received_crc = buffer.back();
    uint8_t calculated_crc = computeCRC( buffer );

    if (received_crc != calculated_crc) {
        ESP_LOGE(TAG, "CRC error: received 0x%02X, calculated 0x%02X (len=%lu)",
                received_crc, calculated_crc, buffer.size());
        ESP_LOG_BUFFER_HEXDUMP(TAG, buffer.data(), buffer.size(), ESP_LOG_INFO);
        return;
    }

    // The 3 firts bytes are the header 
    uint8_t direction = buffer[0];
    uint8_t address = buffer[1];
    uint8_t packetType = buffer[2];

    
    if (address == Address::MAIN_CONTROLLER)
    {
        bool isResponse = (direction == Direction::RESPONSE);

        // Handle temperature packets (from main controller, address 0x00)
        if(packetType == PacketType::TEMPERATURES)
        {
            onTemperaturePacketReceived(buffer, isResponse);
            return;
        }

        // Handle control packets (from main controller, address 0x00)
        if (packetType == PacketType::OPERATING_STATUS)
        {
            onOperatingPacketReceived(buffer, isResponse);
            return;
        }

    } else if (address == Address::ESP32_CONTROLLER) {
        onDedicatedPacketReceived(buffer);
        return;
    }// (address == Address::MAIN_CONTROLLER)






    // Logic start Here
    //float fVal = random_float_range(-2.0f, 40.0f);
    //_pDatas->setByKey(DataKey::roomTemperature, fVal);

    _pDatas->setByKey(DataKey::setpointTemperature, _setpointTemperature);

    _pDatas->setByKey(DataKey::heatPumpMode, _mode ? "heat" : "off");

    int ibackupHeater = random_int_range(0,3);
    switch (ibackupHeater){
    case 0:
        _pDatas->setByKey(DataKey::backupHeater, "none");
        break;
    case 1:
        _pDatas->setByKey(DataKey::backupHeater, "backup_1");
        break;
    case 2:
        _pDatas->setByKey(DataKey::backupHeater, "backup_2");
        break;
    case 3:
        _pDatas->setByKey(DataKey::backupHeater, "backup_1_2");
        break;
    }
    
}

// Packet type 10
void DriverDaikin::onOperatingPacketReceived(const std::vector<uint8_t>& buffer, bool isResponse)
{
    
    if (isResponse) // Response packet 40 00 10
    {
         if (buffer.size() < (OperatingPacket::DHW_MODE  + 1)){
            ESP_LOGW(TAG, "Request Packet type 10 length is %d < required %d. All packet is ignored", 
                        buffer.size(), (OperatingPacket::DHW_MODE  + 1));
            return;
        }

        _pDatas->setByKey(DataKey::debug, bytesToHex(buffer));

       /*
        _pDatas->setByKey(DataKey::leavingTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::LWT_TEMP_OFFSET));
        
        _pDatas->setByKey(DataKey::dhwCurrentTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::DWT_TEMP_OFFSET));
        
        _pDatas->setByKey(DataKey::outdoorTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::OUTDOOR_TEMP_OFFSET));
        
        _pDatas->setByKey(DataKey::returnTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::RWT_TEMP_OFFSET));
        
        _pDatas->setByKey(DataKey::roomTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::ROOM_TEMP_OFFSET));
*/

    } else { // Request packet 00 00 10
        if (buffer.size() < (TemperaturePacket::REQ_ROOM_TEMP_OFFSET + 2)){
            ESP_LOGW(TAG, "Request Packet type 11 length is %d < required %d. All packet is ignored", 
                        buffer.size(), (TemperaturePacket::REQ_ROOM_TEMP_OFFSET + 2));
            return;
        }

        _pDatas->setByKey(DataKey::debug, byteToBinary(buffer, 1));
        //_pDatas->setByKey(DataKey::roomTemperature, 
        //                fixedPointToFloat(buffer, TemperaturePacket::REQ_ROOM_TEMP_OFFSET));
    }
}

// Packet type 11
void DriverDaikin::onTemperaturePacketReceived(const std::vector<uint8_t>& buffer, bool isResponse)
{
    
    if (isResponse) // Response packet 40 00 11
    {
         if (buffer.size() < (TemperaturePacket::ROOM_TEMP_OFFSET  + 2)){
            ESP_LOGW(TAG, "Request Packet type 11 length is %d < required %d. All packet is ignored", 
                        buffer.size(), (TemperaturePacket::REQ_ROOM_TEMP_OFFSET + 2));
            return;
        }
       
        _pDatas->setByKey(DataKey::leavingTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::LWT_TEMP_OFFSET));
        
        _pDatas->setByKey(DataKey::dhwCurrentTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::DWT_TEMP_OFFSET));
        
        _pDatas->setByKey(DataKey::outdoorTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::OUTDOOR_TEMP_OFFSET));
        
        _pDatas->setByKey(DataKey::returnTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::RWT_TEMP_OFFSET));
        
        _pDatas->setByKey(DataKey::roomTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::ROOM_TEMP_OFFSET));


    } else { // Request packet 00 00 11
        if (buffer.size() < (TemperaturePacket::REQ_ROOM_TEMP_OFFSET + 2)){
            ESP_LOGW(TAG, "Request Packet type 11 length is %d < required %d. All packet is ignored", 
                        buffer.size(), (TemperaturePacket::REQ_ROOM_TEMP_OFFSET + 2));
            return;
        }
        _pDatas->setByKey(DataKey::roomTemperature, 
                        fixedPointToFloat(buffer, TemperaturePacket::REQ_ROOM_TEMP_OFFSET));
    }
}

// Packet dedicated to our device
void DriverDaikin::onDedicatedPacketReceived(const std::vector<uint8_t>& buffer)
{
    uint8_t packetType = buffer[2];
    switch (packetType) {
        case PacketType::POLL_AUX:
            ESP_LOGV(TAG, "Poll auxiliary controller");
            //sendAcknowledge(PacketType::POLL_AUX);
            break;

        case PacketType::OPERATION_CONTROL:
            ESP_LOGV(TAG, "OPERATION_CONTROL");
            //self->handle_control_packet(payload);
            break;

        default:
            ESP_LOGW(TAG, "Message sent to our device type %d - not registered", packetType);
    }
        

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                 Communication to Daikin                                             //
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void DriverDaikin::sendAcknowledge(uint8_t packetType)
{
    std::vector<uint8_t> res = {Direction::RESPONSE, Address::ESP32_CONTROLLER, packetType, 0xFF};
    uint8_t crc = computeCRC( res );
    res.back() = crc;
    _hbs->sendBytes( res );
}

void DriverDaikin::sendResponse(uint8_t packetType, const std::vector<uint8_t>& datas)
{
    std::vector<uint8_t> res;
    res.reserve(3 + datas.size() +1 );
    std::vector<uint8_t> header =  { Direction::RESPONSE, Address::ESP32_CONTROLLER, packetType };
    res.insert(res.end(), header.begin(), header.end());
    res.insert(res.end(), datas.begin(), datas.end());
    res.push_back(0xFF);

    uint8_t crc = computeCRC( res );
    res.back() = crc;
    _hbs->sendBytes( res );

}


///////////////////// Setters  /////////////////////
// TODEL the implementation should send to the device only 
// feedback on variables is done by the device itself, not by members 

void DriverDaikin::setSetpointTemperature(float temp)
{
    ESP_LOGD(TAG, "Setpoint temperature: %f", temp);
    _setpointTemperature = temp ;
}

void DriverDaikin::setMode(const std::string& mode)
{
    ESP_LOGD(TAG, "Turning Heat Pump Mode to %s", mode.c_str());
    if (mode == "heat"){
        _mode = true;
    } else if (mode == "off") {
        _mode = false;
    }   
}

void DriverDaikin::setDhwSetpointTemperature(float temp)
{
    ESP_LOGD(TAG, "DHW Setpoint temperature: %f", temp);
    _setpointTemperature = temp ;
}

void DriverDaikin::setDhwMode(const std::string& mode)
{
    ESP_LOGD(TAG, "Turning DHW Mode to %s", mode.c_str());
    if (mode == "heat"){
        _mode = true;
    } else if (mode == "off") {
        _mode = false;
    }
    
}