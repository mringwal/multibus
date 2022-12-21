# MultiBus for Raspberry Pi Pico[W]

## Compile and install
- Clone PICO SDK from [GitHub](https://github.com/raspberrypi/pico-sdk)
    `git clone https://github.com/raspberrypi/pico-sdk.git`
- Set PICO_SDK_PATH:
    `export PICO_SDK_PATH=/path/to/pico-sdk`
- Clone MultiBus from [GitHub](https://github.com/mringwal/multibus)
    `git clone `https://github.com/mringwal/multibus.git`
- Go to pico firmware folder
    `cd multibus/firmware/pico`	
- Create build directory
    `mkdir build && cd build`
- Configure build
    `cmake ..`
- Build
    `make`
- Enter Pico Bootloader mode:
    - Unplug Pico
    - Hold BOOTSEL button while plugging USB cable in
    - Pico should show up as Mass Storage Device in your OS
- Flash firmware
    `cp multibus-pico.uf2 /path/to/pico/MassStorageDevice`
- Berify that a new virtual serial port has become available

## Test
- Go to `multibus/example/python/`
- Run bridge_info.py
	`python3 bridge_info.py path-to-pico-usb-cdc-serial-port`

## Hardware Info

The RP2040 has 2 I2C and 2 SPI Instances. For both, instance 0 is used in master mode.

| GPIO       | Function  |
|------------|-----------|
| 4          | I2C0-SDA  |
| 5          | I2C0-SCL  |
| 18         | SPI0-SCK  |
| 19         | SPI0-MOSI |
| 16         | SPI0-MISO |
| 17         | SPI0-CSN  |
