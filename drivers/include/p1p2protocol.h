#pragma once

#include <stdint.h>

// =============================================================================
// P1P2 Protocol Constants
// =============================================================================
// Header is Direction | Address | Packet Type
namespace P1P2
{
  constexpr size_t HEADER_SIZE = 3;
}
// Packet direction (first byte)
namespace Direction
{
  constexpr uint8_t REQUEST = 0x00;
  constexpr uint8_t RESPONSE = 0x40;
  constexpr uint8_t INIT = 0x80;
}

// Device addresses (second byte)
namespace Address
{
  constexpr uint8_t MAIN_CONTROLLER = 0x00;
  constexpr uint8_t AUX_CONTROLLER = 0xF0;
}

// Packet types (third byte)
namespace PacketType
{
  constexpr uint8_t OPERATING_STATUS = 0x10;
  constexpr uint8_t TEMPERATURES = 0x11;
  constexpr uint8_t POLL_AUX = 0x30;
  constexpr uint8_t UNKNOWN_32 = 0x32;
  constexpr uint8_t OUTDOOR_UNIT_NAME = 0x35;
  constexpr uint8_t INDOOR_UNIT_NAME = 0x36;
  constexpr uint8_t OPERATION_CONTROL = 0x38;
  constexpr uint8_t COUNTER_ALARM = 0x39;
  constexpr uint8_t UNKNOWN_3A = 0x3A;
  constexpr uint8_t UNKNOWN_31 = 0x31;
}

// Operating modes
namespace OperatingMode
{
  constexpr uint8_t FAN_ONLY = 0x60;
  constexpr uint8_t HEAT = 0x61;
  constexpr uint8_t COOL = 0x62;
  constexpr uint8_t AUTO = 0x63;
  constexpr uint8_t DRY = 0x67;
}

// Current action/compressor state
namespace ActionState
{
  constexpr uint8_t IDLE = 0x00;      // Fan running, compressor off
  constexpr uint8_t HEATING = 0x01;
  constexpr uint8_t COOLING = 0x02;
}

// Fan speed values (bits 4-6, masked with 0x70)
namespace FanSpeed
{
  constexpr uint8_t LOW = 0x10;
  constexpr uint8_t MEDIUM = 0x30;
  constexpr uint8_t HIGH = 0x50;
  constexpr uint8_t MASK = 0x70;
  constexpr uint8_t CHANGED_FLAG = 0x80;
}

// Power state
namespace PowerState
{
  constexpr uint8_t OFF = 0x00;
  constexpr uint8_t ON = 0x01;
}


// =============================================================================
// Packet Structures
// =============================================================================

// Generic P1P2 packet header
struct PacketHeader
{
  uint8_t direction;
  uint8_t address;
  uint8_t type;
};


struct ControlMode {
    uint8_t heatingMode   : 1; // Bit 0 (LSB)
    uint8_t coolingmode   : 1; // Bit 1
    uint8_t unknown       : 6; // Bits 2-7 (MSB)
};
static_assert(sizeof(ControlMode) == 1);

struct DhwMode {
    uint8_t dhwMode       : 1; // Bit 0 (on/off) (LSB)
    uint8_t unkmown       : 7; // Bits 2-7 (MSB)
};
static_assert(sizeof(DhwMode) == 1);

struct ZonesMode {
    uint8_t heating   : 1; // Bit 0 (on/off) (LSB)
    uint8_t cooling   : 1; // Bit 1 (on/off)
    uint8_t unknown   : 3; // Bit 2-4 
    uint8_t main      : 1; // Bit 5 (on/off)
    uint8_t additional: 1; // Bit 6 (on/off)
    uint8_t dhwTank   : 1; // Bits 7 (on/off) (MSB)
};


// Packet Type 0x10 - Operation Control (Request payload offsets)
namespace OperatingPacket
{
  constexpr size_t CONTROL_MODE     = 0 + P1P2::HEADER_SIZE;
  constexpr size_t DHW_POWER_STATUS = 1 + P1P2::HEADER_SIZE;
  constexpr size_t ZONES_STATUS     = 2 + P1P2::HEADER_SIZE;
  constexpr size_t DHW_SETPOINT_TEMPERATURE = 4 + P1P2::HEADER_SIZE;
  constexpr size_t SETPOINT_TEMPERATURE     = 8 + P1P2::HEADER_SIZE;
  constexpr size_t DISINFECTION_MODE        = 11 + P1P2::HEADER_SIZE;
  constexpr size_t ERROR_CODE               = 12 + P1P2::HEADER_SIZE; // 3 bytes
  constexpr size_t DEFROST_OP         = 10 + P1P2::HEADER_SIZE; 
  constexpr size_t CIRCULATION_PUMP   = 18 + P1P2::HEADER_SIZE; 
  constexpr size_t DHW_MODE           = 19 + P1P2::HEADER_SIZE;
}

// Packet Type 0x11 - Temperature readings
// first data bytes is 3 as header is | direction | address | packet type |
// Response (direction=0x40): Actual room temperature is 15-16
// Request (direction=0x00): Actual room temperature is 3-4
namespace TemperaturePacket
{
  constexpr size_t LWT_TEMP_OFFSET      = 0 + P1P2::HEADER_SIZE;
  constexpr size_t DWT_TEMP_OFFSET      = 2 + P1P2::HEADER_SIZE;
  constexpr size_t OUTDOOR_TEMP_OFFSET  = 4 + P1P2::HEADER_SIZE;
  constexpr size_t RWT_TEMP_OFFSET      = 6 + P1P2::HEADER_SIZE;
  constexpr size_t ROOM_TEMP_OFFSET     = 12 + P1P2::HEADER_SIZE;
  constexpr size_t REQ_ROOM_TEMP_OFFSET     = 0 + P1P2::HEADER_SIZE;
}



// Packet Type 0x38 - Operation Control (Response structure)
struct ControlResponse
{
  uint8_t header[3];           // 0-2: direction, address, type
  uint8_t power_status;        // 3: 0=off, 1=on
  uint8_t operating_mode;      // 4: see OperatingMode
  uint8_t cooling_setpoint;    // 5: target temp for cooling
  uint8_t reserved_6;          // 6: usually 0x00
  uint8_t cooling_fan_speed;   // 7: fan speed + flags
  uint8_t reserved_8;          // 8: usually 0x00
  uint8_t heating_setpoint;    // 9: target temp for heating
  uint8_t reserved_10;         // 10: usually 0x00
  uint8_t heating_fan_speed;   // 11: fan speed + flags
  uint8_t unknown_12;          // 12: copied from request
  uint8_t reserved_13;         // 13: usually 0x00
  uint8_t reserved_14;         // 14: usually 0x00
  uint8_t reserved_15;         // 15: usually 0x00
  uint8_t status_flags;        // 16: 0x20 when power on
  uint8_t reserved_17;         // 17: usually 0x00
  uint8_t reserved_18;         // 18: usually 0x00
  uint8_t reserved_19;         // 19: usually 0x00
  uint8_t crc;                 // 20: CRC byte
};

namespace StatusFlags
{
  constexpr uint8_t POWER_ON = 0x20;
}

// =============================================================================
// Helper Functions
// =============================================================================

// Convert fixed-point temperature (integer + fraction/256) to float
inline float fixed_point_to_float(uint8_t integer, uint8_t fraction)
{
  return static_cast<float>(integer) + (static_cast<float>(fraction) / 256.0f);
}

inline float fixedPointToFloat(const std::vector<uint8_t>& buffer, size_t offset) {
  return static_cast<float>(buffer[offset]) + (static_cast<float>(buffer[offset + 1]) / 256.0f);
}