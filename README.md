# VBUS-decoder-library

The aim of the project is to create library which open way to decode VBUS communication on VBUS. VBUS is point-to-multipoint communication environment define by RESOL.

The primary reason why the project has been started is to decode information from Solar regulator from Viessmann - Vitosolic 200. However, the way to add any new devices is open and is pretty easy.

## Features

Current version 1.1.0+ supports decoding data from:

### Supported Data Fields
- **Temperature sensors** (up to 32 channels)
- **Pump power and states** (up to 32 channels)
- **Relay states** (up to 32 channels)
- **Error masks** - System error codes
- **Operating hours** - Up to 8 counter channels
- **Heat quantity** - Energy measurements
- **System time** - Controller time in minutes
- **System variant** - Hardware/software variant ID

### Supported Devices
- **Viessmann Vitosolic 200** (0x1060) - Full support with 12 temperature sensors, 7 relays
- **DeltaSol BX Plus** (0x7E11) - 6 temperature sensors, 2 pumps, operating hours, heat quantity
- **DeltaSol BX** (0x7E21) - 6 temperature sensors, 2 pumps, operating hours, heat quantity
- **DeltaSol MX** (0x7E31) - 4 temperature sensors, 4 pumps, operating hours, heat quantity, error mask
- **Generic RESOL devices** - Basic support for 4 temperature sensors

## Security

This library has been reviewed for security vulnerabilities and malicious code. No security issues were found.

## API

### Basic Methods
- `begin()` - Initialize the decoder
- `loop()` - Main processing loop, call frequently
- `isReady()` - Check if valid data is available
- `getVbusStat()` - Get communication status

### Data Access Methods
- `getTemp(idx)` - Get temperature sensor value
- `getPump(idx)` - Get pump power percentage
- `getRelay(idx)` - Get relay state (ON/OFF)
- `getTempNum()` - Get number of temperature sensors
- `getPumpNum()` - Get number of pumps
- `getRelayNum()` - Get number of relays

### Extended Data Methods (v1.1.0+)
- `getErrorMask()` - Get system error mask
- `getSystemTime()` - Get system time in minutes
- `getOperatingHours(idx)` - Get operating hours counter
- `getHeatQuantity()` - Get heat quantity in Wh
- `getSystemVariant()` - Get system variant ID

## Usage

See the example in `examples/vbusdecoder/vbusdecoder.ino` for complete usage demonstration.
