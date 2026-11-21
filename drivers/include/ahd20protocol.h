/*
  ahd20protovol
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  This file gather the definition of the AHD20 protocol, a weather compensated controller for temperature
  regulation

*/


#pragma once

#include <vector>
#include <stdint.h>

// =============================================================================
// AHD20 Protocol Constants
// =============================================================================
// Header is Direction | Address | Packet Type
namespace ModbusASCII
{
  constexpr uint8_t START_BYTE = 0x3A;
  constexpr uint8_t STOP_BYTE1 = 0x0A;
  constexpr uint8_t STOP_BYTE2 = 0x0D;
}
// Packet direction (first byte)
namespace ModbusFunction
{
  constexpr uint8_t READ  = 0x03;
  constexpr uint8_t WRITE = 0x10;
}

// Device addresses (second byte)
namespace ModbusAddress
{
  constexpr uint8_t SLAVE = 0x03;

}



// Operating modes


// =============================================================================
// Packet Structures
// =============================================================================

// Generic AHD20 packet header

// =============================================================================
// Helper Functions
// =============================================================================

inline uint16_t uint16from2Bytes(const std::vector<uint8_t>& data, size_t start)
{
  return (static_cast<uint16_t>(data[start]) << 8) | data[start+1];
}