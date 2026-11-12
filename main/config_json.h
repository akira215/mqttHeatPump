// json_data.h
#pragma once

#include <string_view>

#define SLASH           "/"
#define TOPIC_BASE      "HeatPump"
#define TOPIC_SETPOINT  "Setpoint"
#define TOPIC_STATE     "state"
#define TOPIC_SET       "set"

#define PORT_NUMBER 8080

// Two-step stringification macros
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

// Using R"json( ... )json" avoids conflicts if your JSON contains )"
inline constexpr std::string_view JSON_DEVICE_CONFIG = R"json(
{
  "device": {
    "identifiers": ["HeatPump_01"],
    "name": "Pompe à Chaleur",
    "manufacturer": "DIY",
    "model": "Controller PAC"
  },
  "origin": {
    "name": "PAC_MQTT_Bridge",
    "sw_version": "1.0"
  },
  "components": {
    "thermostat": {
      "platform": "climate",
      "name": "Thermostat",
      "unique_id": "pac_01_climate",
      "current_temperature_topic": ")json" TOPIC_BASE R"json(/ambience/temperature",
      "temperature_state_topic": ")json" TOPIC_BASE SLASH TOPIC_SETPOINT SLASH TOPIC_STATE R"json(",
      "temperature_command_topic": ")json" TOPIC_BASE SLASH TOPIC_SETPOINT SLASH TOPIC_SET R"json(",
      "mode_state_topic": ")json" TOPIC_BASE R"json(/mode/etat",
      "mode_command_topic": ")json" TOPIC_BASE R"json(/mode/set",
      "modes": ["off", "heat"],
      "min_temp": 15,
      "max_temp": 26,
      "temp_step": 0.5,
      "temperature_unit": "C"
    },
    "alimentation": {
      "platform": "switch",
      "name": "Marche / Arrêt",
      "unique_id": "pac_01_switch",
      "state_topic": ")json" TOPIC_BASE R"json(/power/etat",
      "command_topic": ")json" TOPIC_BASE R"json(/power/set",
      "payload_on": "ON",
      "payload_off": "OFF",
      "icon": "mdi:heat-pump"
    },
    "pression_eau": {
      "platform": "sensor",
      "name": "Pression d'eau",
      "unique_id": "pac_01_pression",
      "state_topic": ")json" TOPIC_BASE R"json(/pression/etat",
      "device_class": "pressure",
      "unit_of_measurement": "bar",
      "state_class": "measurement"
    },
    "temp_depart": {
      "platform": "sensor",
      "name": "Température départ d'eau",
      "unique_id": "pac_01_temp_depart",
      "state_topic": ")json" TOPIC_BASE R"json(/eau/depart/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "temp_exterieure": {
      "platform": "sensor",
      "name": "Température extérieure",
      "unique_id": "pac_01_temp_ext",
      "state_topic": ")json" TOPIC_BASE R"json(/exterieur/temperature",
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