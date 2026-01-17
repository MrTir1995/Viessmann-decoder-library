# Viessmann Multi-Protocol Library

A comprehensive Arduino library for communicating with Viessmann heating systems using multiple protocols. This library supports modern and legacy Viessmann communication protocols, enabling integration with a wide range of heating controllers and solar regulators.

**⚡ NEW:** 
- **Home Assistant Addon** now available! See [Home Assistant Addon Guide](homeassistant-addon/README.md) for Docker-based deployment.
- **Linux support** now available! See [Linux Installation Guide](linux/README_LINUX.md) for Debian-based systems.
- **Windows support** now available! See [Windows Installation Guide](windows/README_WINDOWS.md) for Windows 10/11.

## Supported Protocols

- **VBUS (RESOL Protocol 1.0)** - Point-to-multipoint communication protocol used in Viessmann Vitosolic and RESOL DeltaSol controllers
- **KW-Bus (VS1)** - Legacy Viessmann protocol for older heating systems
- **P300 (VS2/Optolink)** - Service protocol accessed via optical interface on Viessmann boilers
- **KM-Bus** - Viessmann 2-wire communication bus for controllers and expansion modules (**fully implemented** with status decoding, temperature extraction, and control capabilities)

## Features

Current version 2.1.0+ provides multi-protocol support with the following capabilities:

### Bus Participant Discovery (NEW in v2.1)
- **Automatic discovery** - Automatically detects all devices on the bus
- **Manual configuration** - Add and configure devices manually
- **Device identification** - Recognizes known device types (Vitosolic 200, DeltaSol BX/MX, etc.)
- **Real-time monitoring** - Track up to 16 bus participants simultaneously
- **Status tracking** - Monitor last seen time and device status

See [Bus Participant Discovery Guide](doc/BUS_PARTICIPANT_DISCOVERY.md) for details.

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
- **Viessmann Vitotronic 100 series** - Legacy heating controllers
- **Viessmann Vitotronic 200 series** - Including Type KW1 (weather-compensated boiler control)
  - Part Numbers: Best.-Nr. 7450 740 to 7450 743
  - Detailed documentation: See doc/VITOTRONIC_200_KW1.md
- **Viessmann Vitotronic 300 series** - Advanced heating system controllers
- **Viessmann Vitodens** (older models) - Condensing boilers
- **Viessmann Vitocrossal** (older models) - Commercial heating systems
- Basic support for temperature and status data

#### P300 (VS2) Protocol Devices
- **Viessmann Vitodens series** - Modern condensing boilers with Optolink interface
- **Viessmann Vitocrossal** - Commercial heating systems
- Basic support for standard datapoints

#### KM-Bus Protocol Devices
- **Viessmann Vitotronic controllers** - Controllers with KM-Bus terminals
- **Vitotrol 200/300** - Remote control units
- **Vitocom 100** - Internet gateway for remote monitoring and control
- **Schaltmodul-V** - Switching/expansion modules
- **Expansion modules** - Mixer circuits, additional zone controls
- **Fully implemented** with status record decoding:
  - 5 temperature sensors (boiler, hot water, outdoor, setpoint, departure)
  - Burner status monitoring
  - Main circulation pump status
  - Hot water loop pump status
  - Operating mode detection (off, night, day, eco, party)
- Based on boblegal31/Heater-remote implementation
- See doc/VITOTRONIC_200_KW1.md for Vitotronic 200 KW1 specifics

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

For web-based configuration and monitoring (ESP32/ESP8266), see `examples/webserver_config/` and [WEBSERVER_SETUP.md](doc/WEBSERVER_SETUP.md).

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
| Vitotronic 100/200/300 (old) | PROTOCOL_KW | 4800 | 8E2 |
| Vitodens, Vitocrossal (new) | PROTOCOL_P300 | 4800 | 8E2 |
| Remote controls (Vitotrol), expansion modules | PROTOCOL_KM | Varies | Varies |

## Optional Web Server (ESP32/ESP8266)

For ESP32 and ESP8266 platforms, an optional web-based configuration interface is available. This allows you to:

- **Configure protocol settings** through a web browser
- **Monitor real-time data** from your heating system
- **Save configuration** to flash memory
- **Remote access** from any device on your network

### Quick Start

1. Open `examples/webserver_config/webserver_config.ino`
2. Update WiFi credentials
3. Upload to ESP32/ESP8266
4. Access web interface at device IP address

For complete documentation, see [WEBSERVER_SETUP.md](doc/WEBSERVER_SETUP.md).

## Linux Support

This library can now be used on Linux systems (Debian, Ubuntu, Raspbian, etc.) without Arduino hardware!

The Linux port provides:
- **Native Linux library** - No Arduino IDE required
- **Command-line tools** - Ready-to-use example applications
- **Easy installation** - Automated install script for Debian-based systems
- **Multiple build systems** - Support for both Make and CMake
- **Full protocol support** - All protocols (VBUS, KW-Bus, P300, KM-Bus) work on Linux

### Quick Start (Linux)

```bash
cd linux
sudo ./install.sh
```

Then run:
```bash
vbusdecoder_linux -p /dev/ttyUSB0 -b 9600 -t vbus
```

For complete Linux installation and usage instructions, see [Linux Installation Guide](linux/README_LINUX.md).

## Windows Support

This library can now be used on Windows systems (Windows 10/11) without Arduino hardware!

The Windows port provides:
- **Native Windows executable** - No Arduino IDE required
- **Command-line tools** - Ready-to-use example applications
- **Multiple build options** - Support for MinGW, Visual Studio, and CMake
- **Easy building** - Automated build scripts for quick setup
- **Full protocol support** - All protocols (VBUS, KW-Bus, P300, KM-Bus) work on Windows

### Quick Start (Windows)

Using MinGW:
```batch
cd windows
build_mingw.bat
```

Then run:
```batch
build\bin\vbusdecoder_windows.exe -p COM1 -b 9600 -t vbus
```

For complete Windows installation and usage instructions, see [Windows Installation Guide](windows/README_WINDOWS.md).

## Home Assistant Addon

This library is now available as a Home Assistant addon! Run the complete webserver in a Docker container with easy configuration through the Home Assistant UI.

The addon provides:
- **Native Home Assistant integration** - Install directly from addon store
- **Web interface** - Monitor your heating system from any browser
- **RESTful API** - Easy integration with Home Assistant sensors
- **Docker-based** - Isolated, secure, and easy to deploy
- **Multi-architecture** - Works on x86, ARM, and ARM64 systems
- **Full protocol support** - All protocols (VBUS, KW-Bus, P300, KM-Bus) available

### Quick Start (Home Assistant)

1. Add the repository to your Home Assistant addon store
2. Install "Viessmann Decoder" addon
3. Configure your serial port and protocol
4. Start the addon
5. Access the web interface through Home Assistant

For complete installation and integration instructions, see [Home Assistant Addon Guide](homeassistant-addon/README.md).

## Contributing

Contributions are welcome! Areas for improvement:
- Additional device decoders for existing protocols
- KM-Bus write/control commands (mode switching, setpoint control)
- Protocol-specific optimizations
- Additional test cases and examples

## Acknowledgments

KM-Bus protocol implementation based on the excellent work from:
- boblegal31/Heater-remote (https://github.com/boblegal31/Heater-remote)
- OpenV project (https://github.com/openv/openv/wiki/KM-Bus)

## License

See LICENSE file for details.
