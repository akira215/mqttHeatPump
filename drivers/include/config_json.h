// json_data.h
#pragma once

#include <string_view>

// Two-step stringification macros
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

// Using R"json( ... )json" avoids conflicts if your JSON contains )"


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
    "thermostat": {
      "platform": "climate",
      "name": "Thermostat Heat Pump",
      "unique_id": "HeatPump_01_climate",
      "precision": 0.1,
      "temp_step": 0.5,
      "min_temp": 15,
      "max_temp": 26,
      "temperature_unit": "C",
      "current_temperature_topic": ")json" CONFIG_TOPIC_BASE R"json(room/temperature",
      "temperature_state_topic": ")json" CONFIG_TOPIC_BASE R"json(setpoint/state",
      "temperature_command_topic": ")json" CONFIG_TOPIC_BASE R"json(setpoint/set",
      "mode_state_topic": ")json" CONFIG_TOPIC_BASE R"json(mode/state",
      "mode_command_topic": ")json" CONFIG_TOPIC_BASE R"json(mode/set",
      "modes": ["off", "heat"]
    },
    "alimentation": {
      "platform": "switch",
      "name": "Marche / Arrêt",
      "unique_id": "pac_01_switch",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(power/state",
      "command_topic": ")json" CONFIG_TOPIC_BASE R"json(power/set",
      "payload_on": "ON",
      "payload_off": "OFF",
      "icon": "mdi:heat-pump"
    },
    "pression_eau": {
      "platform": "sensor",
      "name": "Pression d'eau",
      "unique_id": "pac_01_pression",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(pression/etat",
      "device_class": "pressure",
      "unit_of_measurement": "bar",
      "state_class": "measurement"
    },
    "temp_depart": {
      "platform": "sensor",
      "name": "Température départ d'eau",
      "unique_id": "pac_01_temp_depart",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(eau/depart/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "temp_exterieure": {
      "platform": "sensor",
      "name": "Température extérieure",
      "unique_id": "pac_01_temp_ext",
      "state_topic": ")json" CONFIG_TOPIC_BASE R"json(exterieur/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    }
  }
}
)json";

// Clean up helper macros
#undef STRINGIFY
#undef STRINGIFY_IMPL