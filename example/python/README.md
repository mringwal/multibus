# MultiBus python examples

## Dependencies

Please install the following dependencies:

```
pip install pyserial
pip install python_abc
```

## Examples

### multibus_bridge_synchronous.py

Shows how to access the bridge by using the generated python bindings. The bindings
are generated if not yet available.
Access to multibus is synchronous.

### multibus_i2c_master_synchronous.py

Shows how to access the i2c master component by using the generated python bindings. 
The bindings are generated if not yet available.
The example shows hot to configure and read a BH1750 ambient light sensor.
Access to multibus is synchronous.

### multibus_lux_library_synchronous.py

Shows how a simple library component can be constructed to access the ambient light sensor. The library simply
wraps calls to the i2c master component.
Access to multibus is synchronous.

### multibus_playground_raw_bridge.py

Simple playground script which shows how mulitbub messages can be constructed manually without using the 
python bindings. Shows how to access bridge functions.

### multibus_playground_raw_i2c.py

Simple playground script which shows how mulitbub messages can be constructed manually without using the 
python bindings. Shows how to access i2c master functions.
