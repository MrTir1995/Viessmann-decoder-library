# Migration Guide: v1.x to v2.0

This guide helps you migrate your code from version 1.x (VBUS Decoder) to version 2.0 (Viessmann Multi-Protocol Library).

## What's Changed

### Library Name
- Old: "VBUS Decoder"
- New: "Viessmann Multi-Protocol Library"

### Major Changes
1. **Multi-protocol support** - Added support for KW-Bus, P300, and KM-Bus protocols
2. **Protocol selection** - `begin()` now accepts a protocol parameter
3. **New API method** - `getProtocol()` returns the active protocol

## Backward Compatibility

**Good news:** Version 2.0 is backward compatible with v1.x code!

If you don't specify a protocol, the library defaults to VBUS protocol, maintaining compatibility with existing code.

## Migration Options

### Option 1: No Changes Required (Recommended for VBUS users)

If you're using VBUS protocol devices (Vitosolic 200, DeltaSol), your existing code will continue to work:

```cpp
// v1.x code - still works in v2.0!
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial vbusSerial(8,9);
VBUSDecoder vbus(&vbusSerial);

void setup() {
  vbusSerial.begin(9600);
  vbus.begin();  // Defaults to PROTOCOL_VBUS
  Serial.begin(115200);
}

void loop() {
  vbus.loop();
  
  if (vbus.isReady()) {
    Serial.println(vbus.getTemp(0));
  }
}
```

### Option 2: Explicit Protocol Selection (Recommended for clarity)

Update your code to explicitly specify the VBUS protocol:

```cpp
// v2.0 code with explicit protocol
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial vbusSerial(8,9);
VBUSDecoder vbus(&vbusSerial);

void setup() {
  vbusSerial.begin(9600);
  vbus.begin(PROTOCOL_VBUS);  // Explicitly select VBUS
  Serial.begin(115200);
}

void loop() {
  vbus.loop();
  
  if (vbus.isReady()) {
    Serial.println(vbus.getTemp(0));
  }
}
```

### Option 3: Migrate to Other Protocols

If you're switching to a different Viessmann protocol:

#### Migrating to KW-Bus

```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial kwSerial(8,9);
VBUSDecoder decoder(&kwSerial);

void setup() {
  // KW-Bus uses 4800 baud, 8E2
  kwSerial.begin(4800, SERIAL_8E2);
  decoder.begin(PROTOCOL_KW);  // Select KW-Bus protocol
  Serial.begin(115200);
}

void loop() {
  decoder.loop();
  
  if (decoder.isReady()) {
    // Same API as before
    for (uint8_t i = 0; i < decoder.getTempNum(); i++) {
      Serial.print("Temp ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(decoder.getTemp(i));
    }
  }
}
```

#### Migrating to P300 (Optolink)

```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial optoSerial(8,9);
VBUSDecoder decoder(&optoSerial);

void setup() {
  // P300 uses 4800 baud, 8E2
  optoSerial.begin(4800, SERIAL_8E2);
  decoder.begin(PROTOCOL_P300);  // Select P300 protocol
  Serial.begin(115200);
}

void loop() {
  decoder.loop();
  
  if (decoder.isReady()) {
    // Same API as before
    Serial.println(decoder.getTemp(0));
  }
}
```

## API Changes

### Changed Methods

| v1.x | v2.0 | Notes |
|------|------|-------|
| `begin()` | `begin(protocol)` | Protocol parameter is optional, defaults to VBUS |

### New Methods

| Method | Description |
|--------|-------------|
| `getProtocol()` | Returns the currently active protocol |

### Unchanged Methods

All data access methods remain the same:
- `getTemp(idx)`
- `getPump(idx)`
- `getRelay(idx)`
- `getTempNum()`
- `getPumpNum()`
- `getRelayNum()`
- `getVbusStat()`
- `isReady()`
- `getErrorMask()`
- `getSystemTime()`
- `getOperatingHours(idx)`
- `getHeatQuantity()`
- `getSystemVariant()`

## Hardware Changes

### VBUS Protocol (No Changes)
- Baud rate: 9600
- Same wiring as v1.x

### New Protocols

If migrating to KW-Bus or P300:

| Protocol | Baud Rate | Settings | Hardware Notes |
|----------|-----------|----------|----------------|
| KW-Bus | 4800 | 8E2 | May need level converter |
| P300 | 4800 | 8E2 | Requires Optolink adapter |
| KM-Bus | Varies | Varies | May need M-Bus transceiver |

## Testing Your Migration

1. **Verify compilation:**
   ```cpp
   // Add to setup() for debugging
   Serial.print("Protocol: ");
   Serial.println(vbus.getProtocol());
   ```

2. **Check data reception:**
   ```cpp
   if (vbus.isReady()) {
     Serial.println("Data ready!");
   } else {
     Serial.println("Waiting for data...");
   }
   ```

3. **Monitor communication status:**
   ```cpp
   if (!vbus.getVbusStat()) {
     Serial.println("Communication error!");
   }
   ```

## Common Migration Issues

### Issue 1: Compilation Errors with `begin()`

**Error:** `no matching function for call to 'VBUSDecoder::begin(int)'`

**Solution:** You're passing the wrong type. Use protocol constants:
```cpp
// Wrong:
vbus.begin(0);

// Correct:
vbus.begin(PROTOCOL_VBUS);
```

### Issue 2: No Data Received After Migration

**Check:**
1. Baud rate matches protocol (9600 for VBUS, 4800 for KW/P300)
2. Parity settings (None for VBUS, Even for KW/P300)
3. Stop bits (1 for VBUS, 2 for KW/P300)

### Issue 3: Existing Variables Naming

If you have variables named `protocol`, they might conflict with internal library names. Rename them:

```cpp
// If you had:
int protocol = 1;

// Rename to:
int myProtocol = 1;
```

## Getting Help

If you encounter issues during migration:

1. **Check the examples** in `examples/vbusdecoder/`
2. **Read the protocol documentation** in `PROTOCOLS.md`
3. **Open an issue** on GitHub with:
   - Your v1.x code
   - The error message or unexpected behavior
   - Your device model and protocol

## Benefits of Upgrading

Even if you're staying with VBUS, upgrading to v2.0 provides:

1. **Better code organization** - Cleaner protocol separation
2. **Future-proofing** - Easy to add other protocols later
3. **Active development** - Bug fixes and improvements
4. **Extended device support** - More Viessmann devices supported

## Rollback Instructions

If you need to rollback to v1.x:

1. In Arduino Library Manager, select version 1.1.0
2. Or manually copy v1.x files from GitHub

Your v1.x code will work unchanged.

## Example: Complete Migration

### Before (v1.x)

```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial vbusSerial(8,9);
VBUSDecoder vbus(&vbusSerial);

void setup() {
  vbusSerial.begin(9600);
  vbus.begin();
  Serial.begin(115200);
}

void loop() {
  vbus.loop();
  
  if (vbus.isReady()) {
    Serial.print("Temp 1: ");
    Serial.println(vbus.getTemp(0));
    Serial.print("Temp 2: ");
    Serial.println(vbus.getTemp(1));
  }
}
```

### After (v2.0 - No Changes Required)

```cpp
// Exact same code works!
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial vbusSerial(8,9);
VBUSDecoder vbus(&vbusSerial);

void setup() {
  vbusSerial.begin(9600);
  vbus.begin();  // Still works, defaults to VBUS
  Serial.begin(115200);
}

void loop() {
  vbus.loop();
  
  if (vbus.isReady()) {
    Serial.print("Temp 1: ");
    Serial.println(vbus.getTemp(0));
    Serial.print("Temp 2: ");
    Serial.println(vbus.getTemp(1));
  }
}
```

### After (v2.0 - Recommended Update)

```cpp
#include <SoftwareSerial.h>
#include "vbusdecoder.h"

SoftwareSerial vbusSerial(8,9);
VBUSDecoder vbus(&vbusSerial);

void setup() {
  vbusSerial.begin(9600);
  vbus.begin(PROTOCOL_VBUS);  // Explicit protocol selection
  Serial.begin(115200);
  
  // Optional: Display active protocol
  Serial.print("Active protocol: ");
  switch(vbus.getProtocol()) {
    case PROTOCOL_VBUS:
      Serial.println("VBUS");
      break;
    case PROTOCOL_KW:
      Serial.println("KW-Bus");
      break;
    case PROTOCOL_P300:
      Serial.println("P300");
      break;
    case PROTOCOL_KM:
      Serial.println("KM-Bus");
      break;
  }
}

void loop() {
  vbus.loop();
  
  if (vbus.isReady()) {
    Serial.print("Temp 1: ");
    Serial.println(vbus.getTemp(0));
    Serial.print("Temp 2: ");
    Serial.println(vbus.getTemp(1));
  }
}
```

## Summary

- **v1.x code continues to work** in v2.0 without changes
- **Explicit protocol selection** is recommended but optional
- **New protocols** available: KW-Bus, P300, KM-Bus
- **Same API** for data access methods
- **Easy rollback** if needed

The library is designed for smooth migration while providing powerful new capabilities.
