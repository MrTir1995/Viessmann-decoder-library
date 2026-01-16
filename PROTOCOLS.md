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

**Description:** Legacy Viessmann protocol used in older Vitotronic heating controllers. Also known as VS1 protocol.

**Physical Layer:**
- Baud Rate: 4800
- Data Bits: 8
- Parity: Even
- Stop Bits: 2

**Frame Format:**
```
Start: 0x01
Length: <byte>
Address: <byte>
Data: <n bytes>
Checksum: XOR of all previous bytes
```

**Supported Devices:**
- Vitotronic 100, 200, 300 series controllers
- Vitodens boilers (older models)
- Vitocrossal systems (older models)

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
    for (uint8_t i = 0; i < decoder.getTempNum(); i++) {
      Serial.print("Temperature ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(decoder.getTemp(i));
    }
  }
}
```

**Notes:**
- This protocol requires reverse-engineering or protocol documentation specific to your device
- Basic temperature reading is supported
- Extended features depend on device-specific implementation

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

**Description:** Viessmann proprietary 2-wire communication bus for connecting controllers, expansion modules, and remote controls.

**Physical Layer:**
- Similar to M-Bus (Meter-Bus)
- 2-wire connection (power + data)
- Voltage: 24V DC

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
- Remote controls (e.g., Vitotrol)
- Expansion modules
- Mixer circuits

**Example:**
```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial kmSerial(8,9);  // RX, TX
VBUSDecoder decoder(&kmSerial);

void setup() {
  kmSerial.begin(9600);  // Baud rate may vary by device
  decoder.begin(PROTOCOL_KM);
  Serial.begin(115200);
}

void loop() {
  decoder.loop();
  
  if (decoder.isReady()) {
    Serial.println("KM-Bus data received");
    // Process data
  }
}
```

**Notes:**
- KM-Bus implementation is currently a placeholder
- Full implementation requires detailed protocol specification
- Physical interface may require special transceiver chip (e.g., NCN5150)
- Cable length limited (typically up to 50 meters)

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
