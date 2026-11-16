// json_data.h
#pragma once

#include <string_view>

// Two-step stringification macros
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

// Using R"json( ... )json" avoids conflicts if your JSON contains )"

// To be aligned with definition of deviceDatas.h
// Domestic hot water DHW
inline constexpr std::string_view JSON_DEVICE_CONFIG = R"json(
{
  "device": {
    "identifiers": ["HeatPump_01"],
    "name": "ESP Heat Manager",
    "manufacturer": "Akira Corp.",
    "model": "mqttHeatPump"
  },
  "origin": {
    "name": "PAC_MQTT_Bridge",
    "sw_version": "1.0"
  },
  "components": {
    "heating": {
      "platform": "climate",
      "name": "Heat Pump Thermostat",
      "unique_id": "HeatPump_01_climate",
      "precision": 0.1,
      "temp_step": 0.5,
      "min_temp": 5,
      "max_temp": 35,
      "temperature_unit": "C",
      "current_temperature_topic": ")json" CONFIG_TOPIC_BASE R"json(room/temperature",
      "temperature_state_topic": ")json" CONFIG_TOPIC_BASE R"json(setpoint/state",
      "temperature_command_topic": ")json" CONFIG_TOPIC_BASE R"json(setpoint/set",
      "mode_state_topic": ")json" CONFIG_TOPIC_BASE R"json(mode/state",
      "mode_command_topic": ")json" CONFIG_TOPIC_BASE R"json(mode/set",
      "modes": ["off", "heat"]
    },
    "water_pressure": {
      "platform": "sensor",
      "name": "Water Pressure",
      "unique_id": "HeatPump_01_pressure",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(pressure/state",
      "device_class": "pressure",
      "unit_of_measurement": "bar",
      "state_class": "measurement"
    },
    "leaving_water_temperature": {
      "platform": "sensor",
      "name": "Leaving Water Temperature",
      "unique_id": "HeatPump_01_leaving_temp",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(leavingwater/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "outdoor_temperature": {
      "platform": "sensor",
      "name": "Outdoor Temperature",
      "unique_id": "HeatPump_01_outdoor_temperature",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(outdoor/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "return_water_temperature": {
      "platform": "sensor",
      "name": "Return Water Temperature",
      "unique_id": "HeatPump_01_return_temp",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(returnwater/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "flow_rate": {
      "platform": "sensor",
      "name": "Flow rate",
      "unique_id": "HeatPump_01_flow_rate",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(flowrate/flow",
      "device_class": "volume_flow_rate",
      "unit_of_measurement": "L/min",
      "state_class": "measurement"
    },
    "backup_heater": {
      "platform": "sensor",
      "name": "Backup Heater Level",
      "unique_id": "HeatPump_01_backup_heater",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(backup_heater/level",
      "icon": "mdi:heating-coil",
      "device_class": "enum",
      "options": [
        "none",
        "backup_1",
        "backup_2",
        "backup_1_2"
      ]
    },
    "operation_mode": {
      "platform": "sensor",
      "name": "Operation mode",
      "unique_id": "HeatPump_01_operation_mode",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(operation/mode",
      "icon": "mdi:cog-sync",
      "device_class": "enum",
      "options": [
        "normal",
        "defrost",
        "oil_return"
      ]
    },
    "dhw": {
      "platform": "climate",
      "name": "Domestic Hot Water",
      "unique_id": "HeatPump_01_dhw",
      "precision": 0.1,
      "temp_step": 0.5,
      "min_temp": 5,
      "max_temp": 70,
      "temperature_unit": "C",
      "current_temperature_topic": ")json" CONFIG_TOPIC_BASE R"json(dhw_current/temperature",
      "temperature_state_topic": ")json" CONFIG_TOPIC_BASE R"json(dhw_setpoint/state",
      "temperature_command_topic": ")json" CONFIG_TOPIC_BASE R"json(dhw_setpoint/set",
      "mode_state_topic": ")json" CONFIG_TOPIC_BASE R"json(dhw_mode/state",
      "mode_command_topic": ")json" CONFIG_TOPIC_BASE R"json(dhw_mode/set",
      "modes": ["off", "heat"]
    },
    "upstair": {
      "platform": "climate",
      "name": "Underfloor heating upstair",
      "unique_id": "HeatPump_01_upstair",
      "precision": 0.1,
      "temp_step": 0.5,
      "min_temp": 5,
      "max_temp": 35,
      "temperature_unit": "C",
      "current_temperature_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/room/temperature",
      "temperature_state_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/setpoint/state",
      "temperature_command_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/setpoint/set",
      "mode_state_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/mode/state",
      "mode_command_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/mode/set",
      "modes": ["off", "heat"]
    },
    "Upstair_leaving_temperature": {
      "platform": "sensor",
      "name": "Upstair Leaving Water Temperature",
      "unique_id": "HeatPump_01_upstair_leaving_temp",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/leavingwater/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "Upstair_return_temperature": {
      "platform": "sensor",
      "name": "Upstair Retunr Water Temperature",
      "unique_id": "HeatPump_01_upstair_return_temp",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/returnwater/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "Upstair_outdoor_temperature": {
      "platform": "sensor",
      "name": "Upstair Outdoor Temperature",
      "unique_id": "HeatPump_01_upstair_outdoor_temp",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/outdoor/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "Upstair_source_temperature": {
      "platform": "sensor",
      "name": "Upstair Source Temperature",
      "unique_id": "HeatPump_01_upstair_source_temperature",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/source/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "upstair_flow_rate": {
      "platform": "sensor",
      "name": "Upstair Flow rate",
      "unique_id": "HeatPump_01_upstair_flow_rate",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(upstair/flowrate/flow",
      "device_class": "volume_flow_rate",
      "unit_of_measurement": "L/min",
      "state_class": "measurement"
    }
  }
}
)json";

// Clean up helper macros
#undef STRINGIFY
#undef STRINGIFY_IMPL