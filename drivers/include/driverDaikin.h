#pragma once
#include "deviceDatas.h"

#include <vector>


// Driver class used for testing only
class DriverDaikin final
{
    static const uint8_t CRC_TABLE[];
    DeviceDatas* _pDatas = nullptr;
public:
    DriverDaikin(DeviceDatas* _pDatas);
    ~DriverDaikin();

    void callback(const std::vector<uint8_t>& buffer);
private:
    static void printPacket(const std::vector<uint8_t>& data);
    static uint8_t computeCRC(const std::vector<uint8_t>& data);
    static void printDashboard();
};
