#pragma once
#include "deviceDatas.h"
#include "driverHomebus.h"

#include <vector>


// Driver class used for testing only
class DriverDaikin final
{
    static const uint8_t CRC_TABLE[];
    DeviceDatas*    _pDatas = nullptr;
    DriverHomebus*  _hbs    = nullptr;


    ////////// DEFAULT VALUE TO DEL /////////////////////////////
    float _setpointTemperature = 20.5f;
    bool _mode = false;
    ////////// DEFAULT VALUE TO DEL /////////////////////////////

public:
    DriverDaikin(DeviceDatas* datas, DriverHomebus* hbs);
    ~DriverDaikin();

    void onDataReceived(const std::vector<uint8_t>& buffer);

    // Setters
    void setSetpointTemperature(float temp);
    void setMode(const std::string& mode);
    void setDhwSetpointTemperature(float temp);
    void setDhwMode(const std::string& mode);

private:
    static uint8_t computeCRC(const std::vector<uint8_t>& data);
    static void printPacket(const std::vector<uint8_t>& data);
    static std::string bytesToHex(const std::vector<uint8_t>& data);
    static std::string byteToBinary(const std::vector<uint8_t>& data, size_t index);

    void onOperatingPacketReceived(const std::vector<uint8_t>& buffer, bool isResponse);
    void onTemperaturePacketReceived(const std::vector<uint8_t>& buffer, bool isResponse);
    void onDedicatedPacketReceived(const std::vector<uint8_t>& buffer);
    
    void sendAcknowledge(uint8_t packetType);
    void sendResponse(uint8_t packetType, const std::vector<uint8_t>& datas);
};
