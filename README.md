# Viessmann Multi-Protocol Library

A comprehensive Arduino library for communicating with Viessmann heating systems using multiple protocols. This library supports modern and legacy Viessmann communication protocols, enabling integration with a wide range of heating controllers and solar regulators.

## Supported Protocols

- **VBUS (RESOL Protocol 1.0)** - Point-to-multipoint communication protocol used in Viessmann Vitosolic and RESOL DeltaSol controllers
- **KW-Bus (VS1)** - Legacy Viessmann protocol for older heating systems
- **P300 (VS2/Optolink)** - Service protocol accessed via optical interface on Viessmann boilers
- **KM-Bus** - Viessmann 2-wire communication bus for controllers and expansion modules (basic support)

## Features

Current version 2.0.0+ provides multi-protocol support with the following capabilities:

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

#### VBUS Protocol Devices
- **Viessmann Vitosolic 200** (0x1060) - Full support with 12 temperature sensors, 7 relays
- **DeltaSol BX Plus** (0x7E11) - 6 temperature sensors, 2 pumps, operating hours, heat quantity
- **DeltaSol BX** (0x7E21) - 6 temperature sensors, 2 pumps, operating hours, heat quantity
- **DeltaSol MX** (0x7E31) - 4 temperature sensors, 4 pumps, operating hours, heat quantity, error mask
- **Generic RESOL devices** - Basic support for 4 temperature sensors

#### KW-Bus (VS1) Protocol Devices
- **Viessmann Vitotronic controllers** - Legacy heating controllers using KW protocol
- Basic support for temperature and status data

#### P300 (VS2) Protocol Devices
- **Viessmann Vitodens series** - Modern condensing boilers with Optolink interface
- **Viessmann Vitocrossal** - Commercial heating systems
- Basic support for standard datapoints

#### KM-Bus Protocol Devices
- **Viessmann expansion modules** - Basic framework for future implementation

## Security

This library has been reviewed for security vulnerabilities and malicious code. No security issues were found.

## API

### Basic Methods
- `begin(protocol)` - Initialize the decoder with specified protocol (default: PROTOCOL_VBUS)
- `loop()` - Main processing loop, call frequently
- `isReady()` - Check if valid data is available
- `getVbusStat()` - Get communication status
- `getProtocol()` - Get currently active protocol

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

For detailed protocol information, see [PROTOCOLS.md](PROTOCOLS.md).

For migration from v1.x, see [MIGRATION.md](MIGRATION.md).

## Quick Start

```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial vbusSerial(8,9);  // RX, TX
VBUSDecoder vbus(&vbusSerial);

void setup() {
  vbusSerial.begin(9600);
  
  // Select protocol:
  // PROTOCOL_VBUS - for RESOL VBUS devices (default)
  // PROTOCOL_KW - for KW-Bus devices  
  // PROTOCOL_P300 - for P300/Optolink devices
  // PROTOCOL_KM - for KM-Bus devices
  vbus.begin(PROTOCOL_VBUS);
  
  Serial.begin(115200);
}

void loop() {
  vbus.loop();
  
  if (vbus.isReady()) {
    Serial.print("Temp 1: ");
    Serial.println(vbus.getTemp(0));
  }
}
```

## Protocol Selection Guide

| Your Device | Protocol | Baud Rate | Settings |
|-------------|----------|-----------|----------|
| Vitosolic 200, DeltaSol | PROTOCOL_VBUS | 9600 | 8N1 |
| Vitotronic (old) | PROTOCOL_KW | 4800 | 8E2 |
| Vitodens, Vitocrossal (new) | PROTOCOL_P300 | 4800 | 8E2 |
| Remote controls | PROTOCOL_KM | Varies | Varies |

## Contributing

Contributions are welcome! Areas for improvement:
- Additional device decoders for existing protocols
- Enhanced KM-Bus protocol implementation
- Protocol-specific optimizations
- Additional test cases and examples

## License

See LICENSE file for details.
