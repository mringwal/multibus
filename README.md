# The MultiBus Project

An embedded device driver, i.e. an application that communicates to sensors or other devices via I2C or SPI interfaces,
usually is developed directly on a target platform, e.g. an microcontroller using the built-in interfaces. 
However, it's often more convenient to develop and test the driver logic on a desktop system, 
which unfortunately lacks these standard peripheral interfaces.

A common approach to make the application independent of the MCU peripherals is to quickly implement a minimal bridge
that runs on a common microcontroller and provides the peripheral interface via ad-hoc protocol. 

This project aims to generalize this approach by defining a re-usable protocol, called 'MultiBus'
for the desktop to microcontroller communication and libraries to provide the peripheral interface in 
different environments (Linux, macOS, Windows, Python, Java, ...). It also collects implementation of 
the MultiBus protocol for popular dev kits like Raspberry Pi Pico, STM32 Bluepill or other Nucleo board, 
or a ESP32. We call such a microcontrolle togehter with the firmware 'MultiBus Bridge'. 
As the protocol is transport independent, it's also possible to connect to a MultiBus bridge remotely via TCP/IP. 
Finally, the abstract C APIs could allow to run the driver code directly on an embedded system using the native HALs. 

![MultiBus Architecture](multibus_architecture.svg)

## Architecture Guidelines
- the protocol is asynchronous and transport agnostic.
- transport candidates: UART, USB CDC, USB Control/Interrupt Endpoints, TCP/IP, ..
- binary message format. 5 byte header, big-endian
  - component (8-bit)
  - operation (8-bit), protocol specific
  - channel (8-bit), protocol specific
  - length (16-bit)
  - payload
- terminal functionality can be provided on host for exploration similar to:
  - [BusPirate](http://dangerousprototypes.com/docs/Bus_Pirate)
  - [ESP32 I2C Tool](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2c/i2c_tools)

## Documentation
- The protocol messages are documented in [multibus.yml](protocol/multibus.yml)

## Firmware
The firmware folder contains MultiBus Bridge implementations for different dev kits. Each implementation contains
its own README.md file that includes a getting started guide.

## Examples
The examples folder contains different examples for the support Host APIs / language binding.

## Host APIs
The message definition is provided in the abstract [multibus.yml](protocol/multibus.yml) and allows to generate custom
language bindings on the fly. New components and operations can be added easily without manual parsing of the messages.
At the moment, generators for C and Python are provided. Additional generators for other languages are welcome.

### C
The C Host API is asynchronous and aims to be highly portable. To allow integration into into any existing 
event loop/run loop architecture, the code does not assume any particular architecture and leaves the 
run loop integration to the user. For this, the existing `multibus_serial_posix.c` driver provides a getter for the 
serial filedescriptors and a process functions that need to be called when the filedescriptor becomes readable/writable.

To support other transports than the POSIX Serial, only the `multibus_driver_t` interface has to be implemented.

The protocol generator generates `multibus_protocol.h`, `multibus_protocol.c` and `multibus_transport_protocol.h`.
The functions defined in `multibus_protocol.h` provide message setup and getter functions for all messages.
The `multibus_transport_protocol.h` wrapper provides convenience functions to setup a message and send it over the provided `mb_transport_t` implementation.

An example for reading a light sensor over I2C without an actual run loop is provided, as well as an integration into the 
popular [libev](http://software.schmorp.de/pkg/libev.html) event loop.

### Python
The Python binding provides a simple API to access and test connected devices.

To suport other transports than the one provided via Python Serial module, only the abstract `multibus_connection.py` 
interface has to be implemented

The protocol generator generates `multibus_protocol.py` in `host/python/generated`.

An example for reading a light sensor over I2C is provided as `example/multibus_luxor_sync_draft.py`.

## Related projects
- [Firmata](https://github.com/firmata/protocol/). Very similar goal. Firmwata messages are based on MIDI protocol.
- [BusPirate](http://dangerousprototypes.com/docs/Bus_Pirate). Mainly for exploration
- [Telemetrix For The Raspberry Pi Pico](https://mryslab.github.io/telemetrix-rpi-pico/). 


