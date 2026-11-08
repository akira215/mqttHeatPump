# Test Wifi


## Build Instructions

The embedded software required `esp-ash-components`:

```
git submodule add https://github.com/akira215/esp-ash-components.git components
```

Create a partition.csv for zigbee and configure menuconfig with custom partition table (shall be done by default)

## Configuration

Provide a `sdkconfig.secret` files for your Wifi credentials:

```
CONFIG_WIFI_SSID="MyWifi"
CONFIG_WIFI_PASSWORD="MyWifiPassword"
```

Configuration is available in `menuconfig` in the `Project setting` section:
