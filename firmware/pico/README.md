# MultiBus for Raspberry Pi Pico[W]

## Compile and install
- clone PICO SDK from [GitHub](https://github.com/raspberrypi/pico-sdk)
	`git clone https://github.com/raspberrypi/pico-sdk.git`
- set PICO_SDK_PATH:
	`export PICO_SDK_PATH=/path/to/pico-sdk`
- clone MultiBus from [GitHub](https://github.com/mringwal/multibus)
	`git clone `https://github.com/mringwal/multibus.git`
- go to pico firmware folder
	`cd multibus/firmware/pico`	
- create build directory
	`mkdir build && cd build`
- configute build
	`cmake ..`
- build
	`make`
- Enter Pico Bootloader mode:
	- Unplug Pico
	- Hold BOOTSEL button while plugging USB cable in
	- Pico should show up as Mass Storage Device in your OS
- flash firmware
	`cp multibus-pico.uf2 /path/to/pico/MassStorageDevice`
- verify that a new virtual serial port has become available

## Test
- Go to `multibus/example/python/`
- Run bridge_info.py
	`python3 bridge_info.py path-to-pico-usb-cdc-serial-port`





