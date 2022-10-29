# MultiBus Bridge for ESP32 and Variants

## Background

The MultiBus bridge has been tested with the ESP32 and ESP32C3 variants but they should work without any
changes with other esp variants.  

## esp-idf environment

The bridge currently uses the **esp-idf-v5.0-beta1** release. Older releases (<5) should work with only minimal
adaptions (C++ 20 is a requirement, some renamings in the FreeRTOS port).

Information on how to setup the esp-idf framework is available here: 
https://docs.espressif.com/projects/esp-idf/en/v5.0-beta1/esp32/get-started/index.html

## Build

```
idf.py build flash monitor
```

## Hardware Configuration

### MultiBus UART Transport

Pins: RX: 4, TX: 5
Config: 115200 / 8 / N / 1
-> Can be changed in main.cpp

### MultiBus I2C Master

Pins: SCL: 19, SDA: 18
-> Can be changed in CI2ConfigOperation.h

## Testing

### Bridge

- Go to `multibus/example/python/`

- Install dependencies as described in `multibus/example/python/README.md`

- Run bridge_info.py
	`python3 multibus_bridge_synchronous.py path-to-esp32-usb-cdc-serial-port`

### I2C Master e.g. with BH1750 ambient light sensor

- Go to `multibus/example/python/`

- Install dependencies as described in `multibus/example/python/README.md`

  - Run multibus_luxor_sync_draft.py
    `python3 multibus_lux_library_synchronous.py path-to-esp32-usb-cdc-serial-port`
