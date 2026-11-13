# mqttHeatPump

This device allow to interface Daikin Heat pump + ASCII modbus hydraulic module for second heat area to mqtt, using OpenThread network

## Build Instructions

The embedded software required `esp-ash-components`:

```
git submodule add https://github.com/akira215/esp-ash-components.git components
```

Create a partition.csv for zigbee and configure menuconfig with custom partition table (shall be done by default)

## Configuration

Provide a `sdkconfig.secret` files for your Mqtt credentials and thread active tvl dataset:

```
CONFIG_MQTT_USERNAME="user-mqtt"
CONFIG_MQTT_PASSWORD="mqttpassword"

# OpenThread
CONFIG_TARGET_THREAD_TLV="0e080000......."
```

Configuration is available in `menuconfig` in the `Project setting` section:

## MQTT message
```json
{
  "device": {
    "identifiers": ["pac_principale_01"],
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
      "current_temperature_topic": "pac/ambiance/temperature",
      "temperature_state_topic": "pac/consigne/etat",
      "temperature_command_topic": "pac/consigne/set",
      "mode_state_topic": "pac/mode/etat",
      "mode_command_topic": "pac/mode/set",
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
      "state_topic": "pac/power/etat",
      "command_topic": "pac/power/set",
      "payload_on": "ON",
      "payload_off": "OFF",
      "icon": "mdi:heat-pump"
    },
    "pression_eau": {
      "platform": "sensor",
      "name": "Pression d'eau",
      "unique_id": "pac_01_pression",
      "state_topic": "pac/pression/etat",
      "device_class": "pressure",
      "unit_of_measurement": "bar",
      "state_class": "measurement"
    },
    "temp_depart": {
      "platform": "sensor",
      "name": "Température départ d'eau",
      "unique_id": "pac_01_temp_depart",
      "state_topic": "pac/eau/depart/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    },
    "temp_exterieure": {
      "platform": "sensor",
      "name": "Température extérieure",
      "unique_id": "pac_01_temp_ext",
      "state_topic": "pac/exterieur/temperature",
      "device_class": "temperature",
      "unit_of_measurement": "°C",
      "state_class": "measurement"
    }
  }
}
```


| Entité | Rôle | Topic MQTT | Valeurs attendues |
| :--- | :--- | :--- | :--- |
| **Thermostat** | Envoi temp. ambiante | `pac/ambiance/temperature` | Ex. `20.5` |
| **Thermostat** | Envoi consigne actuelle | `pac/consigne/etat` | Ex. `21.0` |
| **Thermostat** | Réception nouvelle consigne | `pac/consigne/set` | Ex. `21.5` *(envoyé par HA)* |
| **Thermostat** | Envoi du mode | `pac/mode/etat` | `off` ou `heat` |
| **Thermostat** | Réception du mode | `pac/mode/set` | `off` ou `heat` *(envoyé par HA)* |
| **Switch** | Envoi état On/Off | `pac/power/etat` | `ON` ou `OFF` |
| **Switch** | Réception commande On/Off | `pac/power/set` | `ON` ou `OFF` *(envoyé par HA)* |
| **Capteur** | Envoi pression d'eau | `pac/pression/etat` | Ex. `1.5` |
| **Capteur** | Envoi temp. départ d'eau | `pac/eau/depart/temperature` | Ex. `38.2` |
| **Capteur** | Envoi temp. extérieure | `pac/exterieur/temperature` | Ex. `4.5` |



## Credits

 * [Daikin ESP32 WiFi controller by Greg Davill](https://gregdavill.com/posts/daikin-esp-001/)
 * [Control your heat pump from Home Assistant with P1P2MQTT](https://github.com/Arnold-n/P1P2MQTT)