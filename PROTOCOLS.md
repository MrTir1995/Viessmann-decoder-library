# Viessmann Multi-Protocol Support

This library supports multiple Viessmann communication protocols, enabling integration with both modern and legacy heating systems.

## Supported Protocols

### 1. VBUS (RESOL Protocol)

**Protocol Type:** `PROTOCOL_VBUS`

**Description:** Point-to-multipoint communication protocol developed by RESOL and used in Viessmann Vitosolic solar controllers and DeltaSol devices.

**Physical Layer:**
- Baud Rate: 9600
- Data Bits: 8
- Parity: None
- Stop Bits: 1

**Supported Devices:**
- Viessmann Vitosolic 200 (0x1060)
- DeltaSol BX Plus (0x7E11)
- DeltaSol BX (0x7E21)
- DeltaSol MX (0x7E31)
- Generic RESOL devices

**Example:**
```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial vbusSerial(8,9);  // RX, TX
VBUSDecoder vbus(&vbusSerial);

void setup() {
  vbusSerial.begin(9600);
  vbus.begin(PROTOCOL_VBUS);
  Serial.begin(115200);
}

void loop() {
  vbus.loop();
  
  if (vbus.isReady()) {
    Serial.print("Temperature 1: ");
    Serial.println(vbus.getTemp(0));
  }
}
```

---

### 2. KW-Bus (VS1 Protocol)

**Protocol Type:** `PROTOCOL_KW`

**Description:** Legacy Viessmann protocol used in older Vitotronic heating controllers. Also known as VS1 protocol. The KW-Bus is a serial communication bus used primarily for boiler control and monitoring.

**Physical Layer:**
- Baud Rate: 4800
- Data Bits: 8
- Parity: Even
- Stop Bits: 2
- Interface: Direct serial UART (RS-232 or TTL levels)

**Frame Format:**
```
Start: 0x01
Length: <byte>
Address: <byte>
Data: <n bytes>
Checksum: XOR of all previous bytes
```

**Supported Devices:**
- Vitotronic 100 series controllers
- Vitotronic 200 series controllers (including Type KW1)
- Vitotronic 300 series controllers
- Vitodens boilers (older models)
- Vitocrossal systems (older models)

**Vitotronic 200 KW1 Specifics:**
- Part Numbers: Best.-Nr. 7450 740 to 7450 743
- Weather-compensated digital boiler control
- Supports multiple temperature sensors
- Configurable via boiler coding plug
- Service manual: See doc/VITOTRONIC_200_KW1.md

**Example:**
```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial kwSerial(8,9);  // RX, TX
VBUSDecoder decoder(&kwSerial);

void setup() {
  kwSerial.begin(4800, SERIAL_8E2);  // 4800 baud, 8 data bits, even parity, 2 stop bits
  decoder.begin(PROTOCOL_KW);
  Serial.begin(115200);
}

void loop() {
  decoder.loop();
  
  if (decoder.isReady()) {
    Serial.println("KW-Bus Data Received:");
    
    // Read temperature sensors
    for (uint8_t i = 0; i < decoder.getTempNum(); i++) {
      Serial.print("Temperature Sensor ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(decoder.getTemp(i));
      Serial.println(" °C");
    }
    
    // Check communication status
    if (!decoder.getVbusStat()) {
      Serial.println("Warning: Communication error detected");
    }
  }
}
```

**Hardware Connection Notes:**
- For Vitotronic 200 KW1: Connect to the controller's serial interface
- Maximum cable length: 35m with 1.5mm² wire for sensors
- Level conversion may be required depending on your microcontroller (RS-232 to TTL)
- Ensure proper ground connection between controller and Arduino

**Sensor Installation (Vitotronic 200 KW1):**
- **Outside Temperature Sensor**: Install on north/northwest wall, 2-2.5m above ground
- **Boiler Temperature Sensor**: Pre-installed on boiler
- **Storage Temperature Sensor**: Optional, for hot water tanks

**Notes:**
- This protocol is device-specific; data structure varies by controller model
- Basic temperature reading is supported in the library
- Extended features depend on device-specific implementation
- For Vitotronic 200 KW1, see detailed documentation in doc/VITOTRONIC_200_KW1.md
- Boiler coding plug configuration required for proper operation

---

### 3. P300 Protocol (VS2/Optolink)

**Protocol Type:** `PROTOCOL_P300`

**Description:** Service protocol used in modern Viessmann boilers, accessed via the Optolink optical interface.

**Physical Layer:**
- Baud Rate: 4800
- Data Bits: 8
- Parity: Even
- Stop Bits: 2
- Interface: Optical (IR LED + Phototransistor)

**Frame Format:**
```
Start: 0x05 (response) or 0x01 (request)
Length: <byte>
Type: <byte>
Address: <2 bytes, big-endian>
Data: <n bytes>
Checksum: Sum of all previous bytes
```

**Supported Devices:**
- Viessmann Vitodens series (200, 300, 333)
- Viessmann Vitocrossal series
- Viessmann Vitovalor
- Most modern Viessmann heating systems with Optolink port

**Hardware Setup:**
You need an Optolink adapter circuit:
- IR LED (e.g., SFH487-2) connected to TX
- IR Phototransistor (e.g., SFH309FA) connected to RX
- Interface matches UART TTL levels

**Example:**
```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial optoSerial(8,9);  // RX, TX
VBUSDecoder decoder(&optoSerial);

void setup() {
  optoSerial.begin(4800, SERIAL_8E2);  // 4800 baud, 8 data bits, even parity, 2 stop bits
  decoder.begin(PROTOCOL_P300);
  Serial.begin(115200);
}

void loop() {
  decoder.loop();
  
  if (decoder.isReady()) {
    // Display available data
    Serial.println("P300 Data received:");
    for (uint8_t i = 0; i < decoder.getTempNum(); i++) {
      Serial.print("Temp ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(decoder.getTemp(i));
    }
  }
}
```

**Resources:**
- [VitoWiFi Project](https://github.com/bertmelis/VitoWiFi)
- [OpenV vcontrold](https://github.com/openv/vcontrold)

---

### 4. KM-Bus Protocol

**Protocol Type:** `PROTOCOL_KM`

**Description:** Viessmann proprietary 2-wire communication bus for connecting controllers, expansion modules, and remote controls. KM-Bus is used primarily for user interface devices like the Vitotrol remote controls and expansion modules.

**Physical Layer:**
- Similar to M-Bus (Meter-Bus) standard
- 2-wire connection (power + data combined)
- Voltage: 24V DC
- Topology: Bus architecture with multiple participants
- Transceiver: May require M-Bus transceiver chip (e.g., NCN5150)

**Frame Format (M-Bus Long Frame):**
```
Start: 0x68
Length: <byte>
Length: <byte> (repeated)
Start: 0x68 (repeated)
Control: <byte>
Address: <byte>
Data: <n bytes>
Checksum: Sum of control + address + data
Stop: 0x16
```

**Supported Devices:**
- Vitotronic controllers with KM-Bus terminals
- Remote controls (e.g., Vitotrol 200, Vitotrol 300)
- Vitocom 100 communication interface
- Expansion modules (Schaltmodul-V)
- Mixer circuits
- Additional control panels

**Connection Requirements:**
- For multiple KM-Bus participants: KM-Bus distributor required
- Maximum cable length: typically up to 50 meters
- 2-wire cable (polarity usually not critical but check device specs)
- Power supply considerations: 24V DC provided by main controller

**Example:**
```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial kmSerial(8,9);  // RX, TX
VBUSDecoder decoder(&kmSerial);

void setup() {
  kmSerial.begin(9600);  // Baud rate may vary by device (commonly 2400-9600)
  decoder.begin(PROTOCOL_KM);
  Serial.begin(115200);
}

void loop() {
  decoder.loop();
  
  if (decoder.isReady()) {
    Serial.println("KM-Bus data received");
    // Process data according to device protocol
    
    // Check for available data
    if (decoder.getTempNum() > 0) {
      Serial.print("Temperature from remote: ");
      Serial.println(decoder.getTemp(0));
    }
  }
}
```

**Hardware Connection:**
- **Vitotronic 200 KW1**: Connect to KM-Bus terminals (aVG connector)
- **Multiple Devices**: Use KM-Bus distributor (Best.-Nr. varies by model)
- **Level Conversion**: May require M-Bus transceiver IC (NCN5150 or similar)
- **Cable**: Standard 2-wire bus cable, observe maximum length restrictions

**KM-Bus Distributor:**
When connecting multiple KM-Bus devices (e.g., multiple remote controls or a remote plus expansion module), a KM-Bus distributor is required. The distributor:
- Provides proper bus termination
- Distributes power to all participants
- Ensures signal integrity
- Typical model for Vitotronic 200: Part of accessory catalog

**Typical KM-Bus Participants:**
1. **Vitotrol 200**: Basic remote control with time programs
2. **Vitotrol 300**: Advanced remote control with extended programming
3. **Vitocom 100**: Internet gateway for remote monitoring
4. **Schaltmodul-V**: Switching module for additional outputs
5. **AM1 Expansion**: Additional mixer circuit control

**Notes:**
- KM-Bus implementation in this library is currently a basic framework
- Full implementation requires detailed protocol specification from Viessmann
- Physical interface may require special transceiver chip for proper voltage levels
- Cable length is limited (typically up to 50 meters)
- The protocol is primarily used for configuration and user interface, not real-time data
- For Vitotronic 200 KW1 integration details, see doc/VITOTRONIC_200_KW1.md

**Bus Addressing:**
- Each KM-Bus device must have a unique address on the bus
- Addresses are typically configured via DIP switches or coding plugs
- Main controller (Vitotronic) acts as master
- Remote controls and modules act as slaves

**Important Considerations:**
- KM-Bus carries both data and power on the same two wires
- Incorrect wiring can damage devices - always consult device manuals
- When in doubt, contact Viessmann technical support or qualified installer
- This bus is not suitable for high-speed data transfer
- Primarily designed for configuration and monitoring applications

---

## Protocol Selection

Choose the appropriate protocol based on your Viessmann device:

| Device Type | Protocol | Baud Rate | Parity |
|-------------|----------|-----------|--------|
| Vitosolic 200, DeltaSol | VBUS | 9600 | None |
| Vitotronic (old) | KW-Bus | 4800 | Even, 2 stop bits |
| Vitodens, Vitocrossal (new) | P300 | 4800 | Even, 2 stop bits |
| Remote controls, expansions | KM-Bus | Varies | Varies |

## Hardware Considerations

### VBUS
- Direct serial connection (TTL or RS-485 transceiver)
- Simple 3-wire connection (GND, +, -)

### KW-Bus / P300
- Requires level converter for RS-232/TTL
- P300 requires optical interface (Optolink adapter)

### KM-Bus
- May require M-Bus transceiver chip
- 2-wire bus with power supply considerations

## Common Issues

1. **No data received:**
   - Check baud rate and parity settings
   - Verify physical connections
   - Ensure correct protocol selection

2. **Checksum errors:**
   - Check for electrical noise
   - Verify cable shielding
   - Reduce cable length if possible

3. **Intermittent data:**
   - Power supply issues
   - Loose connections
   - Electromagnetic interference

## Further Development

The library provides a framework for adding more device-specific decoders. To add support for a new device:

1. Identify the device address/ID
2. Decode the data frame structure
3. Add a device-specific decoder function
4. Register the decoder in the appropriate protocol handler

Contributions are welcome! See the examples in the source code for implementation patterns.

## References

- [RESOL VBUS Specification](http://danielwippermann.github.io/resol-vbus/vbus-specification.html)
- [VitoWiFi Project](https://github.com/bertmelis/VitoWiFi)
- [OpenV Wiki](https://github.com/openv/openv/wiki)
- [M-Bus Specification](https://m-bus.com/)
