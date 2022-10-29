# MultiBus C Examples

## Compile

A CMake build file is provided. [libev](http://software.schmorp.de/pkg/libev.html) is detected if installed

```
$ mkdir build
$ cd build
$ cmake ..
...
example test_async
example test_async_libev (with libev)
...
$ make
...
```

## Examples

Both examples assume that a BH1750 ambient light sensor is connected via I2C to the bridge device and are tested
on macOS. They should work in the same way on any POSIX system, e.g. Linux.

### test_async

Shows how to configure and read the current ambient lux value.
Intead of a real event loop, the MultiBus Serial Transport `multibus_serial_posix.c` implementation is directly
polled from the main() function.

### test_async_libev

Same as the `test_async`. However, this example shows how the MultiBus Serial Transport can be used with 
a common event loop like libev.
