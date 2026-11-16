#pragma once
#include "deviceDatas.h"

#include <vector>


// Driver class used for testing only
class DriverDaikin final
{
    static const uint8_t CRC_TABLE[];
    DeviceDatas* _pDatas = nullptr;



    ////////// DEFAULT VALUE TO DEL /////////////////////////////
    float _setpointTemperature = 20.5f;
    bool _mode = false;
    ////////// DEFAULT VALUE TO DEL /////////////////////////////

public:
    DriverDaikin(DeviceDatas* _pDatas);
    ~DriverDaikin();

    void onDataReceived(const std::vector<uint8_t>& buffer);

    void setSetpointTemperature(float temp);
    void setMode(const std::string& mode);
private:
    static void printPacket(const std::vector<uint8_t>& data);
    static uint8_t computeCRC(const std::vector<uint8_t>& data);

    void onOperatingPacketReceived(const std::vector<uint8_t>& buffer, bool isResponse);
    void onTemperaturePacketReceived(const std::vector<uint8_t>& buffer, bool isResponse);
    
    static void printDashboard();
};
